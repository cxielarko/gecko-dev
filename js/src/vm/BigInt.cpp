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
    if (v.isBoolean()) {
        res.setBigInt(v.isTrue() ? One(cx) : Zero(cx));
        return true;
    }
    if (v.isString()) {
        RootedString str(cx, v.toString());
        RootedBigInt bi(cx, StringToBigInt(cx, str, 0));
        if (!bi) {
            JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                      JSMSG_BIGINT_INVALID_SYNTAX);
            return false;
        }
        res.setBigInt(bi);
        return true;
    }
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_NOT_BIGINT);
    return false;
}

BigInt*
BigInt::Parse(JSContext* cx, JSAtom* atom, int radix)
{
    MOZ_ASSERT(radix >= 0);
    RootedString str(cx, atom);
    RootedBigInt res(cx, StringToBigInt(cx, str, radix));
    if (!res) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                  JSMSG_BIGINT_INVALID_SYNTAX);
        return nullptr;
    }
    return res;
}

bool
BigInt::NumberValue(JSContext* cx, HandleValue val, MutableHandleValue res)
{
    MOZ_ASSERT(val.isBigInt());
    RootedBigInt x(cx, val.toBigInt());

    signed long int exp;
    double d = mpz_get_d_2exp(&exp, x->num_);
    res.setNumber(ldexp(d, exp));
    return true;
}

bool
BigInt::CompareNumber(JSContext* cx, HandleValue lhs, HandleValue rhs, MutableHandleValue res)
{
    if (lhs.isBigInt()) {
        MOZ_ASSERT(rhs.isNumber());
        RootedBigInt lhs2(cx, lhs.toBigInt());
        return CompareNumber(cx, lhs2, rhs.toNumber(), res);
    }

    MOZ_ASSERT(lhs.isNumber() && rhs.isBigInt());
    RootedBigInt rhs2(cx, rhs.toBigInt());
    return CompareNumber(cx, lhs.toNumber(), rhs2, res);
}

bool
BigInt::CompareNumber(JSContext* cx, HandleBigInt lhs, double rhs, MutableHandleValue res)
{
    if (mozilla::IsNaN(rhs)) {
        res.setUndefined();
        return true;
    }

    res.setInt32(mpz_cmp_d(lhs->num_, rhs));
    return true;
}

bool
BigInt::CompareNumber(JSContext* cx, double lhs, HandleBigInt rhs, MutableHandleValue res)
{
    if (mozilla::IsNaN(lhs)) {
        res.setUndefined();
        return true;
    }

    res.setInt32(-mpz_cmp_d(rhs->num_, lhs));
    return true;
}

bool
BigInt::CompareString(JSContext* cx, HandleValue lhs, HandleValue rhs, MutableHandleValue res)
{
    if (lhs.isBigInt()) {
        MOZ_ASSERT(rhs.isString());
        RootedBigInt lhs2(cx, lhs.toBigInt());
        double d;
        if (!ToNumber(cx, rhs, &d))
            return false;
        return CompareNumber(cx, lhs2, d, res);
    }

    MOZ_ASSERT(lhs.isString() && rhs.isBigInt());
    RootedBigInt rhs2(cx, rhs.toBigInt());
    double d;
    if (!ToNumber(cx, lhs, &d))
        return false;
    return CompareNumber(cx, d, rhs2, res);
}

JSString*
BigInt::ToString(JSContext* cx, HandleBigInt x, int radix)
{
    char* str = mpz_get_str(NULL, radix, x->num_);
    if (!str)
        return nullptr;
    return JS_NewStringCopyZ(cx, str);
}

template <typename CharT>
static bool
ParseBigIntDigit(CharT c, unsigned* res)
{
    if ('0' <= c && c <= '9') {
        *res = c - '0';
        return true;
    } else if ('a' <= c && c <= 'z') {
        *res = c - 'a' + 10;
        return true;
    } else if ('A' <= c && c <= 'Z') {
        *res = c - 'A' + 10;
        return true;
    }
    return false;
}

template <typename CharT>
bool
BigInt::StringToBigIntImpl(const CharT* chars, size_t length, unsigned radix,
                           HandleBigInt res)
{
    const CharT* end = chars + length;
    const CharT* s = SkipSpace(chars, end);

    int sign = 0;

    if (s != end && s[0] == '+') {
        sign = 1;
        s++;
    } else if (s != end && s[0] == '-') {
        sign = -1;
        s++;
    }

    if (!radix) {
        radix = 10;
        if (end - s >= 2 && s[0] == '0') {
            if (s[1] == 'x' || s[1] == 'X') {
                radix = 16;
                s += 2;
            } else if (s[1] == 'o' || s[1] == 'O') {
                radix = 8;
                s += 2;
            } else if (s[1] == 'b' || s[1] == 'B') {
                radix = 2;
                s += 2;
            }
            if (radix != 10 && s == end) {
                return false;
            }
        }
    }

    if (sign != 0 && radix != 10)
        return false;

    mpz_set_ui(res->num_, 0);

    for (; s < end; s++) {
        unsigned digit;
        if (!ParseBigIntDigit(s[0], &digit)) {
            if (SkipSpace(s, end) == end)
                break;
            return false;
        }
        if (digit >= radix)
            return false;
        mpz_mul_ui(res->num_, res->num_, radix);
        mpz_add_ui(res->num_, res->num_, digit);
    }

    if (sign < 0)
        mpz_neg(res->num_, res->num_);

    return true;
}

BigInt*
BigInt::StringToBigInt(JSContext* cx, HandleString str, unsigned radix)
{
    RootedBigInt res(cx, New(cx));

    JSLinearString* linear = str->ensureLinear(cx);
    if (!linear)
        return nullptr;

    AutoCheckCannotGC nogc;
    size_t length = str->length();
    if (linear->hasLatin1Chars()) {
        if (!StringToBigIntImpl(linear->latin1Chars(nogc), length, radix, res)) {
            return nullptr;
        }
    } else {
        if (!StringToBigIntImpl(linear->twoByteChars(nogc), length, radix, res)) {
            return nullptr;
        }
    }
    return res;
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
