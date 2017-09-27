/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/TracingAPI.h"
#include "gc/Tracer.h"
#include "jsapi.h"
#include "jsobjinlines.h"
#include "vm/TaggedProto.h"
#include "vm/BigInt.h"
#include "vm/SelfHosting.h"
#include "builtin/BigIntObject.h"
#include "builtin/TypedObject.h"
#include "vm/ArrayBufferObject.h"

using namespace js;

bool
js::EqualBigInts(BigInt* t1, BigInt* t2, bool* equal)
{
    *equal = (t1->sign() == t2->sign())
        && (t1->size() == t2->size())
        && (!memcmp(t1->data(), t2->data(), t1->size()));
    return true;
}

static MOZ_ALWAYS_INLINE bool
IsBigInt(HandleValue v)
{
    return v.isBigInt() || (v.isObject() && v.toObject().is<BigIntObject>());
}

static MOZ_ALWAYS_INLINE BigInt*
ToBigInt(HandleValue value)
{
    if (value.isBigInt()) {
        return value.toBigInt();
    }
    if (value.isObject()) {
        JSObject& obj = value.toObject();
        if (obj.is<BigIntObject>()) {
            return obj.as<BigIntObject>().unbox();
        }
        return nullptr;
    }
    return nullptr;
}

static JSObject*
CreateBigIntPrototype(JSContext* cx, JSProtoKey key)
{
    return GlobalObject::createBlankPrototype(cx, cx->global(), &BigIntObject::protoClass_);
}

static bool
BigIntConstructor(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);
    if (args.isConstructing()) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_NOT_CONSTRUCTOR, "BigInt");
        return false;
    }

    RootedValue nullv(cx, NullValue());
    FixedInvokeArgs<1> args2(cx);
    args2[0].set(args[0]);
    if (!CallSelfHostedFunction(cx, "BigIntConstructor", nullv, args2, args.rval()))
        return false;
    return true;
}

JSObject*
BigIntObject::create(JSContext* cx, HandleBigInt bigInt)
{
    RootedObject obj(cx, NewBuiltinClassInstance(cx, &class_));
    if (!obj)
        return nullptr;
    BigIntObject& bn = obj->as<BigIntObject>();
    bn.setFixedSlot(PRIMITIVE_VALUE_SLOT, BigIntValue(bigInt));
    return &bn;
}

BigInt*
BigIntObject::unbox() const
{
    return getFixedSlot(PRIMITIVE_VALUE_SLOT).toBigInt();
}

bool
BigIntObject::MakeBigInt(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);

    if (args.length() != 3)
        return false;

    int32_t sign = args[0].toInt32();
    int32_t size = args[1].toInt32();
    if (size < 0)
        return false;
    uint32_t usize = static_cast<uint32_t>(size);
    uint8_t* data = nullptr;
    if (usize) {
        RootedObject ao(cx, &args[2].toObject());
        data = static_cast<uint8_t*>(JS_StealArrayBufferContents(cx, ao));
        if (!data)
            return false;
    }
    RootedBigInt bi(cx, BigInt::newBigInt(cx, sign, usize, data));
    if (!bi)
        return false;
    args.rval().setBigInt(bi);
    return true;
}

bool
js::BigIntGetDigit(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedBigInt bi(cx, ToBigInt(args[0]));
    if (!bi) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_NOT_BIGINT);
        return false;
    }
    if (!bi->size()) {
        args.rval().setInt32(0);
        return true;
    }
    uint32_t* bi_data = (uint32_t*)(bi->data());
    if (!bi_data)
        return false;
    int32_t i = args[1].toInt32();
    if (0 <= i && static_cast<uint32_t>(i) < bi->size())
        args.rval().setNumber(uint32_t(bi_data[i]));
    else
        args.rval().setInt32(0);
    return true;
}

bool
js::BigIntGetSign(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedBigInt bi(cx, ToBigInt(args[0]));
    if (!bi) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_NOT_BIGINT);
        return false;
    }
    args.rval().setInt32(static_cast<int32_t>(bi->sign()));
    return true;
}

bool
js::BigIntGetSize(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedBigInt bi(cx, ToBigInt(args[0]));
    if (!bi) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_NOT_BIGINT);
        return false;
    }
    args.rval().setInt32(bi->size());
    return true;
}

bool
BigIntObject::ValueOf(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);
    HandleValue thisv = args.thisv();
    RootedBigInt bi(cx, ToBigInt(thisv));
    if (!bi) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_NOT_BIGINT);
        return false;
    }
    args.rval().setBigInt(bi);
    return true;
}

static const ClassSpec BigIntObjectClassSpec = {
    GenericCreateConstructor<BigIntConstructor, 1, gc::AllocKind::FUNCTION>,
    CreateBigIntPrototype
};

const Class BigIntObject::class_ = {
    "BigInt",
    JSCLASS_HAS_CACHED_PROTO(JSProto_BigInt) |
    JSCLASS_HAS_RESERVED_SLOTS(RESERVED_SLOTS)
};

const Class BigIntObject::protoClass_ = {
    "BigIntProto",
    JSCLASS_HAS_CACHED_PROTO(JSProto_BigInt),
    JS_NULL_CLASS_OPS,
    &BigIntObjectClassSpec
};

const JSPropertySpec BigIntObject::properties[] = {
    JS_PS_END
};

const JSFunctionSpec BigIntObject::methods[] = {
    JS_FN("valueOf", ValueOf, 0, 0),
    JS_SELF_HOSTED_FN("toString", "BigInt_toString", 1, 0),
    JS_SELF_HOSTED_FN("toLocaleString", "BigInt_toLocaleString", 2, 0),
    JS_FS_END
};

const JSFunctionSpec BigIntObject::staticMethods[] = {
    JS_SELF_HOSTED_FN("parseInt", "BigInt_parseInt", 2, 0),
    JS_SELF_HOSTED_FN("asUintN", "BigInt_asUintN", 2, 0),
    JS_SELF_HOSTED_FN("asIntN", "BigInt_asIntN", 2, 0),
    JS_FS_END
};

JSObject*
BigIntObject::initClass(JSContext* cx, HandleObject obj)
{
    Handle<GlobalObject*> global = obj.as<GlobalObject>();
    RootedPlainObject proto(cx, NewBuiltinClassInstance<PlainObject>(cx));
    if (!proto)
        return nullptr;

    Rooted<JSFunction*> ctor(cx, GlobalObject::createConstructor(cx, BigIntConstructor,
                                                                 ClassName(JSProto_BigInt, cx), 0));
    if (!ctor ||
        !LinkConstructorAndPrototype(cx, ctor, proto) ||
        !DefinePropertiesAndFunctions(cx, proto, properties, methods) ||
        !DefineToStringTag(cx, proto, cx->names().BigInt) ||
        !DefinePropertiesAndFunctions(cx, ctor, nullptr, staticMethods) ||
        !GlobalObject::initBuiltinConstructor(cx, global, JSProto_BigInt, ctor, proto))
    {
        return nullptr;
    }
    return proto;
}

JSObject*
js::InitBigIntClass(JSContext* cx, HandleObject obj)
{
    return BigIntObject::initClass(cx, obj);
}
