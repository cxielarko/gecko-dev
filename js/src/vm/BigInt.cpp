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
BigInt::MakeBigInt32(JSContext* cx, bool sign, uint32_t digit)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_set_ui(z->num_, static_cast<unsigned long int>(digit));
    if (sign)
        mpz_neg(z->num_, z->num_);
    return z;
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
BigInt::Zero(JSContext* cx)
{
    return MakeBigInt32(cx, 0, 0);
}

BigInt*
BigInt::One(JSContext* cx)
{
    return MakeBigInt32(cx, 0, 1);
}

BigInt*
BigInt::Add(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_add(z->num_, x->num_, y->num_);
    return z;
}

BigInt*
BigInt::Sub(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_sub(z->num_, x->num_, y->num_);
    return z;
}

BigInt*
BigInt::Mul(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_mul(z->num_, x->num_, y->num_);
    return z;
}

BigInt*
BigInt::Div(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    if (!mpz_size(y->num_)) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                  JSMSG_BIGINT_DIVISION_BY_ZERO);
        return nullptr;
    }
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_tdiv_q(z->num_, x->num_, y->num_);
    return z;
}

BigInt*
BigInt::Mod(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    if (!mpz_size(y->num_)) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                  JSMSG_BIGINT_DIVISION_BY_ZERO);
        return nullptr;
    }
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_tdiv_r(z->num_, x->num_, y->num_);
    return z;
}

BigInt*
BigInt::Pow(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    if (mpz_sgn(y->num_) < 0) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                  JSMSG_BIGINT_NEGATIVE_EXPONENT);
        return nullptr;
    }
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_pow_ui(z->num_, x->num_, mpz_get_ui(y->num_));
    return z;
}

BigInt*
BigInt::Neg(JSContext* cx, HandleBigInt x)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_neg(z->num_, x->num_);
    return z;
}

BigInt*
BigInt::Lsh(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    if (mpz_sgn(y->num_) < 0)
        mpz_fdiv_q_2exp(z->num_, x->num_, mpz_get_ui(y->num_));
    else
        mpz_mul_2exp(z->num_, x->num_, mpz_get_ui(y->num_));
    return z;
}

BigInt*
BigInt::Rsh(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    if (mpz_sgn(y->num_) < 0)
        mpz_mul_2exp(z->num_, x->num_, mpz_get_ui(y->num_));
    else
        mpz_fdiv_q_2exp(z->num_, x->num_, mpz_get_ui(y->num_));
    return z;
}

BigInt*
BigInt::BitAnd(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_and(z->num_, x->num_, y->num_);
    return z;
}

BigInt*
BigInt::BitOr(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_ior(z->num_, x->num_, y->num_);
    return z;
}

BigInt*
BigInt::BitXor(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_xor(z->num_, x->num_, y->num_);
    return z;
}

BigInt*
BigInt::BitNot(JSContext* cx, HandleBigInt x)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_neg(z->num_, x->num_);
    mpz_sub_ui(z->num_, z->num_, 1);
    return z;
}

BigInt*
BigInt::AsUintN(JSContext* cx, HandleBigInt x, uint64_t n)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_fdiv_r_2exp(z->num_, x->num_, n);
    return z;
}

BigInt*
BigInt::AsIntN(JSContext* cx, HandleBigInt x, uint64_t n)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_fdiv_r_2exp(z->num_, x->num_, n);
    if (mpz_tstbit(z->num_, n-1)) {
        mpz_t mask;
        mpz_init_set_si(mask, -1);
        mpz_mul_2exp(mask, mask, n);
        mpz_ior(z->num_, z->num_, mask);
        mpz_clear(mask);
    }
    return z;
}

int32_t
BigInt::Compare(JSContext* cx, HandleBigInt x, HandleBigInt y)
{
    return BigInt::Compare(x, y);
}

int32_t
BigInt::Compare(HandleBigInt x, HandleBigInt y)
{
    return mpz_cmp(x->num_, y->num_);
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
BigInt::ToInt64(JSContext* cx, HandleBigInt x, int64_t& out)
{
    out = mpz_get_si(x->num_);
    return true;
}

bool
BigInt::ToUint64(JSContext* cx, HandleBigInt x, uint64_t& out)
{
    out = (mpz_sgn(x->num_) < 0)
        ? static_cast<uint64_t>(mpz_get_si(x->num_))
        : mpz_get_ui(x->num_);
    return true;
}

BigInt*
BigInt::FromInt64(JSContext* cx, int64_t n)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_set_si(z->num_, n);
    return z;
}

BigInt*
BigInt::FromUint64(JSContext* cx, uint64_t n)
{
    BigInt* z = New(cx);
    if (!z)
        return nullptr;
    mpz_set_ui(z->num_, n);
    return z;
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
