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
    // The minimum allocation size is currently 16 bytes (see
    // SortedArenaList in jsgc.h).
    uint8_t unused_[16];

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

    JSString* toString(JSContext* cx);
    bool toBoolean();

    static BigInt* Copy(JSContext* cx, HandleBigInt x);
};
}

namespace js {
JSObject* BigIntToObject(JSContext* cx, HandleBigInt x);
}

#endif
