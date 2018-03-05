/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef vm_BigInt_h
#define vm_BigInt_h

#include "jsalloc.h"
#include "jsapi.h"

#include "gc/Barrier.h"
#include "gc/Marking.h"

#include "js/GCHashTable.h"
#include "js/RootingAPI.h"
#include "js/TypeDecls.h"

#include "js/Utility.h"

#include "vm/String.h"

#include <gmp.h>

namespace JS {
class BigInt : public js::gc::TenuredCell
{
  private:
    mpz_t num_;

    // The minimum allocation size is currently 16 bytes (see
    // SortedArenaList in jsgc.h).
    uint8_t unused_[sizeof(mpz_t) < 16 ? 16 - sizeof(mpz_t) : 0];

    // Allocate and initialize a BigInt value
    static BigInt* New(JSContext* cx);

  public:
    static const JS::TraceKind TraceKind = JS::TraceKind::BigInt;

    void traceChildren(JSTracer* trc);

    void finalize(js::FreeOp* fop);

    js::HashNumber hashValue();

    size_t sizeOfIncludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return mallocSizeOf(this);
    }

    static BigInt* Zero(JSContext* cx);
    static BigInt* One(JSContext* cx);
    static BigInt* MakeBigInt32(JSContext* cx, bool sign, uint32_t digit);
    static BigInt* NumberToBigInt(JSContext* cx, double d);

    JSString* toString(JSContext* cx);
    bool toBoolean();

    static void Init();

    static BigInt* Add(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* Sub(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* Mul(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* Div(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* Mod(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* Pow(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* Neg(JSContext* cx, HandleBigInt x);
    static BigInt* Lsh(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* Rsh(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* BitAnd(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* BitOr(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* BitXor(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static BigInt* BitNot(JSContext* cx, HandleBigInt x);
    static BigInt* AsUintN(JSContext* cx, HandleBigInt x, uint64_t n);
    static BigInt* AsIntN(JSContext* cx, HandleBigInt x, uint64_t n);
    static int32_t Compare(JSContext* cx, HandleBigInt x, HandleBigInt y);
    static int32_t Compare(HandleBigInt x, HandleBigInt y);
    static BigInt* Copy(JSContext* cx, HandleBigInt x);

    static bool ToInt64(JSContext* cx, HandleBigInt x, int64_t& out);
    static bool ToUint64(JSContext* cx, HandleBigInt x, uint64_t& out);
    static BigInt* FromInt64(JSContext* cx, int64_t n);
    static BigInt* FromUint64(JSContext* cx, uint64_t n);

    static bool ValueToBigInt(JSContext* cx, HandleValue val, MutableHandleValue res);
    static bool NumberValue(JSContext* cx, HandleValue val, MutableHandleValue res);
    static JSString* ToString(JSContext* cx, HandleBigInt x, int radix);

    static bool CompareNumber(JSContext* cx, HandleValue lhs, HandleValue rhs, MutableHandleValue res);
    static bool CompareNumber(JSContext* cx, HandleBigInt lhs, double rhs, MutableHandleValue res);
    static bool CompareNumber(JSContext* cx, double lhs, HandleBigInt rhs, MutableHandleValue res);

    static bool CompareString(JSContext* cx, HandleValue lhs, HandleValue rhs, MutableHandleValue res);
};
}

namespace js {
JSObject* BigIntToObject(JSContext* cx, HandleBigInt x);
}

#endif
