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

namespace JS {
class BigInt : public js::gc::TenuredCell
{
  private:
    uint8_t* data_;
    uint32_t sign_;
    uint32_t size_;

    // The minimum allocation size is currently 16 bytes (see
    // SortedArenaList in jsgc.h).
    uint32_t unused_[sizeof(uint8_t*) < 8 ? 1 : 0];

  public:
    static const JS::TraceKind TraceKind = JS::TraceKind::BigInt;

    void traceChildren(JSTracer* trc);

    void finalize(js::FreeOp* fop);

    js::HashNumber hashValue() { return 0; }

    size_t sizeOfIncludingThis(mozilla::MallocSizeOf mallocSizeOf) const {
        return mallocSizeOf(this);
    }

    static BigInt* newBigInt(JSContext* cx, int32_t sign, uint32_t size, uint8_t* data);

    uint8_t* data() const { return data_; }
    uint32_t sign() const { return sign_; }
    uint32_t size() const { return size_; }

    JSString* toString(JSContext* cx);
    bool toBoolean();
};
}

namespace js {
JSObject* BigIntToObject(JSContext* cx, HandleBigInt bi);

bool EqualBigInts(BigInt* t1, BigInt* t2, bool* equal);
}

#endif
