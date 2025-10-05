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


#define DECLARE_TYPED_ARRAY_CREATION_API(ExternalType, NativeType, Name)     \
    inline JSObject* JS_New##Name##Array(MCContext* cx, size_t nelements) {  \
      return JS_New##Name##Array(cx->cx_, nelements);                        \
    }                                                                        \
    inline JSObject* JS_New##Name##ArrayFromArray(                           \
        MCContext* cx, JS::Handle<JSObject*> array) {                        \
      return JS_New##Name##ArrayFromArray(cx->cx_, array);                   \
    }                                                                        \
    inline JSObject* JS_New##Name##ArrayWithBuffer(                          \
        MCContext* cx, JS::Handle<JSObject*> arrayBuffer, size_t byteOffset, \
        int64_t length) {                                                    \
      return JS_New##Name##ArrayWithBuffer(cx->cx_, arrayBuffer, byteOffset, \
                                           length);                          \
    }

JS_FOR_EACH_TYPED_ARRAY(DECLARE_TYPED_ARRAY_CREATION_API)
#undef DECLARE_TYPED_ARRAY_CREATION_API

#ifdef DEBUG
inline void* JS_GetArrayBufferViewData(
    JSObject* obj, MC::Tainted<bool*> isSharedMemory,
    MC::Tainted<const JS::AutoCheckCannotGC*> nogc) {
  return JS_GetArrayBufferViewData(obj, isSharedMemory.UNSAFE_unverified(),
                                   *nogc.UNSAFE_unverified());
}
#else
inline void* JS_GetArrayBufferViewData(JSObject* obj,
                                       MC::Tainted<bool*> isSharedMemory,
                                       const JS::AutoCheckCannotGC& nogc) {
  return JS_GetArrayBufferViewData(obj, isSharedMemory.UNSAFE_unverified(),
                                   nogc);
}
#endif

inline JSObject* JS_GetArrayBufferViewBuffer(
    MCContext* cx, JS::Handle<JSObject*> obj, MC::Tainted<bool*> isSharedMemory) {
    return JS_GetArrayBufferViewBuffer(cx->cx_, obj, isSharedMemory.INTERNAL_unverified_safe());
}

inline JSObject* JS_NewDataView(MCContext* cx,
                                       JS::Handle<JSObject*> buffer,
                                       size_t byteOffset, size_t byteLength) {
    return JS_NewDataView(cx->cx_, buffer, byteOffset, byteLength);    
}




#ifdef DEBUG
#  define JS_DEFINE_DATA_ACCESSOR(ExternalType, NativeType, Name)  \
    inline ExternalType* JS_Get##Name##ArrayData(                  \
        JSObject* maybeWrapped, MC::Tainted<bool*> isSharedMemory, \
        MC::Tainted<const JS::AutoCheckCannotGC*> nogc) {          \
      return JS_Get##Name##ArrayData(maybeWrapped, isSharedMemory.UNSAFE_unverified(), \
                                     *nogc.UNSAFE_unverified());   \
    }
#else
#  define JS_DEFINE_DATA_ACCESSOR(ExternalType, NativeType, Name)  \
    inline ExternalType* JS_Get##Name##ArrayData(                  \
        JSObject* maybeWrapped, MC::Tainted<bool*> isSharedMemory, \
        const JS::AutoCheckCannotGC& nogc) {                       \
      return JS_Get##Name##ArrayData(maybeWrapped, isSharedMemory.UNSAFE_unverified(), \
                                     nogc);                        \
    }
#endif
JS_FOR_EACH_TYPED_ARRAY(JS_DEFINE_DATA_ACCESSOR)
#undef JS_DEFINE_DATA_ACCESSOR

#endif

#endif
