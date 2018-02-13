/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/FloatingPoint.h"
#include "mozilla/HashFunctions.h"

#include "jsapi.h"
#include "jscntxt.h"
#include "builtin/BigIntObject.h"
#include "gc/Allocator.h"
#include "gc/Tracer.h"
#include "vm/SelfHosting.h"
#include "vm/BigInt.h"

#include <gmp.h>
#include <math.h>

using namespace js;

static void*
js_mp_realloc(void* ptr, size_t old_size, size_t new_size)
{
    return js_realloc(ptr, new_size);
}

static void
js_mp_free(void* ptr, size_t size)
{
    return js_free(ptr);
}

void
BigInt::Init()
{
    mp_set_memory_functions(js_malloc,
                            js_mp_realloc,
                            js_mp_free);
}

BigInt*
BigInt::New(JSContext* cx)
{
    BigInt* x = Allocate<BigInt, NoGC>(cx);
    if (!x) {
        ReportOutOfMemory(cx);
        return nullptr;
    }
    mpz_init(x->num_);
    return x;
}

BigInt*
BigInt::NumberToBigInt(JSContext* cx, double d)
{
    RootedBigInt z(cx, New(cx));
    if (!z)
        return nullptr;
    double i = ToInteger(d);
    if (!mozilla::IsFinite(d) ||
        i != d ||
        i > 9007199254740991 ||
        i < -9007199254740991) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                  JSMSG_NUMBER_TO_BIGINT);
        return nullptr;
    }
    mpz_set_d(z->num_, i);
    return z;
}

BigInt*
BigInt::Copy(JSContext* cx, HandleBigInt x)
{
    BigInt* copy = New(cx);
    if (!copy)
        return nullptr;
    mpz_set(copy->num_, x->num_);
    return copy;
}

bool
BigInt::ValueToBigInt(JSContext* cx, HandleValue val, MutableHandleValue res)
{
    RootedValue v(cx, val);
    if (!ToPrimitive(cx, JSTYPE_NUMBER, &v))
        return false;
    if (v.isBigInt()) {
        res.set(v);
        return true;
    }
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_NOT_BIGINT);
    return false;
}

JSString*
BigInt::ToString(JSContext* cx, HandleBigInt x, int radix)
{
    char* str = mpz_get_str(NULL, radix, x->num_);
    if (!str)
        return nullptr;
    return JS_NewStringCopyZ(cx, str);
}

void
BigInt::finalize(js::FreeOp* fop)
{
    mpz_clear(num_);
}

void
BigInt::traceChildren(JSTracer* trc)
{
    return;
}

JSObject*
js::BigIntToObject(JSContext* cx, HandleBigInt x)
{
    return BigIntObject::create(cx, x);
}

JSString*
BigInt::toString(JSContext* cx)
{
    char* str = mpz_get_str(NULL, 10, num_);
    if (!str)
        return nullptr;
    return JS_NewStringCopyZ(cx, mpz_get_str(NULL, 10, num_));
}

bool
BigInt::toBoolean()
{
    return (mpz_sgn(num_) != 0);
}

js::HashNumber
BigInt::hashValue()
{
    const void* limbs = mpz_limbs_read(num_);
    uint32_t hash = mozilla::HashBytes(limbs, mpz_size(num_));
    hash = mozilla::AddToHash(hash, static_cast<uint32_t>(mpz_sgn(num_)));
    return hash;
}

JS::ubi::Node::Size
JS::ubi::Concrete<BigInt>::size(mozilla::MallocSizeOf mallocSizeOf) const
{
    MOZ_ASSERT(get().isTenured());
    return js::gc::Arena::thingSize(get().asTenured().getAllocKind());
}
