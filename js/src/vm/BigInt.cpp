/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jscntxt.h"
#include "builtin/BigIntObject.h"
#include "gc/Allocator.h"
#include "gc/Tracer.h"
#include "vm/SelfHosting.h"
#include "vm/BigInt.h"

using namespace js;

BigInt*
BigInt::newBigInt(JSContext* cx, int32_t sign, uint32_t size, uint8_t* data)
{
    BigInt* bi = Allocate<BigInt, NoGC>(cx);
    if (!bi)
        return nullptr;
    bi->sign_ = sign;
    bi->size_ = size;
    bi->data_ = data;
    return bi;
}

void
BigInt::finalize(js::FreeOp* fop)
{
    fop->free_(data_);
}

void
BigInt::traceChildren(JSTracer* trc)
{
    return;
}

JSObject*
js::BigIntToObject(JSContext* cx, HandleBigInt bi)
{
    return BigIntObject::create(cx, bi);
}

JSString*
BigInt::toString(JSContext* cx)
{
    FixedInvokeArgs<2> args(cx);
    args[0].setBigInt(this);
    RootedValue nullv(cx, NullValue());
    RootedValue result(cx);
    if (!CallSelfHostedFunction(cx, "BigIntToString", nullv, args, &result))
        return nullptr;
    return result.toString();
}

bool
BigInt::toBoolean()
{
    return size_ != 0;
}

JS::ubi::Node::Size
JS::ubi::Concrete<BigInt>::size(mozilla::MallocSizeOf mallocSizeOf) const
{
    MOZ_ASSERT(get().isTenured());
    return js::gc::Arena::thingSize(get().asTenured().getAllocKind());
}
