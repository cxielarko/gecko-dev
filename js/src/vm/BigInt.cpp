/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/FloatingPoint.h"
#include "mozilla/HashFunctions.h"

#include "jsapi.h"
#include "jscntxt.h"
#include "gc/Allocator.h"
#include "gc/Tracer.h"
#include "vm/SelfHosting.h"
#include "vm/BigInt.h"

using namespace js;

BigInt*
BigInt::New(JSContext* cx)
{
    BigInt* x = Allocate<BigInt, NoGC>(cx);
    if (!x) {
        ReportOutOfMemory(cx);
        return nullptr;
    }
    return x;
}

BigInt*
BigInt::Copy(JSContext* cx, HandleBigInt x)
{
    BigInt* copy = New(cx);
    if (!copy)
        return nullptr;
    return copy;
}

void
BigInt::finalize(js::FreeOp* fop)
{
    return;
}

void
BigInt::traceChildren(JSTracer* trc)
{
    return;
}

JSObject*
js::BigIntToObject(JSContext* cx, HandleBigInt x)
{
    return nullptr;
}

JSString*
BigInt::toString(JSContext* cx)
{
    return nullptr;
}

bool
BigInt::toBoolean()
{
    return false;
}

js::HashNumber
BigInt::hashValue()
{
    return 0;
}

JS::ubi::Node::Size
JS::ubi::Concrete<BigInt>::size(mozilla::MallocSizeOf mallocSizeOf) const
{
    MOZ_ASSERT(get().isTenured());
    return js::gc::Arena::thingSize(get().asTenured().getAllocKind());
}
