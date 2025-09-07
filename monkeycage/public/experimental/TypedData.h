/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Typed array, ArrayBuffer, and DataView creation, predicate, and accessor
 * functions.
 */

#ifndef mc_experimental_TypedData_h
#define mc_experimental_TypedData_h

#include "js/experimental/TypedData.h"

#ifdef JS_SANDBOX

#include "monkeycage/GCAPI.h"
#include "monkeycage/Tainted.h"

inline void* JS_GetArrayBufferViewData(
    JSObject* obj, MC::Tainted<bool*> isSharedMemory,
    MC::Tainted<const JS::AutoCheckCannotGC*> nogc) {
  return JS_GetArrayBufferViewData(obj, isSharedMemory.UNSAFE_unverified(),
                                   *nogc.UNSAFE_unverified());
}

#  define JS_DEFINE_DATA_ACCESSOR(ExternalType, NativeType, Name)  \
    inline ExternalType* JS_Get##Name##ArrayData(                  \
        JSObject* maybeWrapped, MC::Tainted<bool*> isSharedMemory, \
        MC::Tainted<const JS::AutoCheckCannotGC*> nogc) {          \
      return JS_Get##Name##ArrayData(maybeWrapped, isSharedMemory.UNSAFE_unverified(), \
                                     *nogc.UNSAFE_unverified());   \
    }
JS_FOR_EACH_TYPED_ARRAY(JS_DEFINE_DATA_ACCESSOR)
#undef JS_DEFINE_DATA_ACCESSOR

#endif

#endif
