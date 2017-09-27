/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef builtin_BigIntObject_h
#define builtin_BigIntObject_h

#include "jsapi.h"
#include "vm/NativeObject.h"
#include "vm/BigInt.h"

namespace js {

class BigIntObject : public NativeObject
{
    static const unsigned PRIMITIVE_VALUE_SLOT = 0;
    static const unsigned RESERVED_SLOTS = 1;

  public:
    static const Class class_;
    static const Class protoClass_;

    static JSObject* create(JSContext* cx, JS::HandleBigInt bi);
    static JSObject* initClass(JSContext* cx, HandleObject obj);
    static bool MakeBigInt(JSContext* cx, unsigned argc, Value* vp);
    static bool GetDigit(JSContext* cx, unsigned argc, Value* vp);
    static bool GetSign(JSContext* cx, unsigned argc, Value* vp);
    static bool GetSize(JSContext* cx, unsigned argc, Value* vp);
    static bool ValueOf(JSContext* cx, unsigned argc, Value* vp);

    BigInt* unbox() const;

  private:
    static const JSPropertySpec properties[];
    static const JSFunctionSpec methods[];
    static const JSFunctionSpec staticMethods[];
};

extern JSObject* InitBigIntClass(JSContext* cx, HandleObject obj);
extern bool BigIntGetDigit(JSContext* cx, unsigned argc, Value* vp);
extern bool BigIntGetSign(JSContext* cx, unsigned argc, Value* vp);
extern bool BigIntGetSize(JSContext* cx, unsigned argc, Value* vp);

}

#endif
