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

    RootedValue val(cx, UndefinedValue());
    if (args.length() > 0)
        val.set(args[0]);
    if (!ToPrimitive(cx, JSTYPE_NUMBER, &val))
        return false;

    if (val.isNumber()) {
        RootedBigInt bi(cx, BigInt::NumberToBigInt(cx, val.toNumber()));
        if (!bi)
            return false;
        args.rval().setBigInt(bi);
        return true;
    }

    if (!BigInt::ValueToBigInt(cx, val, args.rval()))
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
js::intrinsic_ToBigInt(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);
    MOZ_ASSERT(args.length() == 1);

    RootedValue res(cx);
    if (!BigInt::ValueToBigInt(cx, args[0], &res))
        return false;
    args.rval().set(res);
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

bool
BigIntObject::ToString(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);
    HandleValue thisv = args.thisv();
    RootedBigInt bi(cx, ToBigInt(thisv));
    if (!bi) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_NOT_BIGINT);
        return false;
    }

    int radix = 10;
    if (args.length() > 0) {
        double d;
        if (!ToInteger(cx, args[0], &d))
            return false;
        if (d < 2 || d > 36) {
            JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BAD_RADIX);
            return false;
        }
        radix = d;
    }

    RootedString str(cx, BigInt::ToString(cx, bi, radix));
    if (!str)
        return false;
    args.rval().setString(str);
    return true;
}

bool BigIntObject::AsUintN(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);

    uint64_t bits;
    if (!ToIndex(cx, args[0], &bits))
        return false;

    RootedValue val(cx, args[1]);
    if (!BigInt::ValueToBigInt(cx, val, &val))
        return false;
    RootedBigInt bi(cx, val.toBigInt());

    RootedBigInt res(cx, BigInt::AsUintN(cx, bi, bits));
    if (!res)
        return false;
    args.rval().setBigInt(res);
    return true;
}

bool BigIntObject::AsIntN(JSContext* cx, unsigned argc, Value* vp)
{
    CallArgs args = CallArgsFromVp(argc, vp);

    uint64_t bits;
    if (!ToIndex(cx, args[0], &bits))
        return false;

    RootedValue val(cx, args[1]);
    if (!BigInt::ValueToBigInt(cx, val, &val))
        return false;
    RootedBigInt bi(cx, val.toBigInt());

    RootedBigInt res(cx, BigInt::AsIntN(cx, bi, bits));
    if (!res)
        return false;
    args.rval().setBigInt(res);
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
    JS_FN("toString", ToString, 0, 0),
    JS_FS_END
};

const JSFunctionSpec BigIntObject::staticMethods[] = {
    JS_FN("asUintN", AsUintN, 2, 0),
    JS_FN("asIntN", AsIntN, 2, 0),
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
                                                                 ClassName(JSProto_BigInt, cx), 1));
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
