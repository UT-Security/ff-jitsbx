/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Infrastructure for sharing DOMString data with JSStrings.
 *
 * Importing an nsAString into JS:
 * If possible (GetSharedBufferHandle works) use the external string support in
 * JS to create a JSString that points to the readable's buffer.  We keep a
 * reference to the buffer handle until the JSString is finalized.
 *
 * Exporting a JSString as an nsAReadable:
 * Wrap the JSString with a root-holding XPCJSReadableStringWrapper, which roots
 * the string and exposes its buffer via the nsAString interface, as
 * well as providing refcounting support.
 */

#include "nscore.h"
#include "nsString.h"
#include "nsStringBuffer.h"
#include "monkeycage/Sandbox.h"
#include "jsapi.h"
#include "xpcpublic.h"

using namespace JS;

const JSExternalStringCallbacks* XPCStringConvert::sLiteralExternalString() {
  static const JSExternalStringCallbacks cbs = {
      .finalize = monkeycage::Sandbox::RegisterCallback(
                      XPCStringConvert::LiteralExternalStringFinalize)
                      .get(),
      .sizeOfBuffer = monkeycage::Sandbox::RegisterCallback(
                          XPCStringConvert::LiteralExternalStringSizeOfBuffer)
                          .get()};

  return &cbs;
}

const JSExternalStringCallbacks* XPCStringConvert::sDOMStringExternalString() {
  static const JSExternalStringCallbacks cbs = {
      .finalize = monkeycage::Sandbox::RegisterCallback(
                      XPCStringConvert::DOMExternalStringFinalize)
                      .get(),
      .sizeOfBuffer = monkeycage::Sandbox::RegisterCallback(
                          XPCStringConvert::DOMExternalStringSizeOfBuffer)
                          .get(),
  };

  return &cbs;
}

const JSExternalStringCallbacks*
XPCStringConvert::sDynamicAtomExternalString() {
  static const JSExternalStringCallbacks cbs = {
      .finalize = monkeycage::Sandbox::RegisterCallback(
                      XPCStringConvert::DynamicAtomExternalStringFinalize)
                      .get(),
      .sizeOfBuffer =
          monkeycage::Sandbox::RegisterCallback(
              XPCStringConvert::DynamicAtomExternalStringSizeOfBuffer)
              .get(),
  };
  return &cbs;
}

void XPCStringConvert::LiteralExternalStringFinalize(char16_t* aChars) {
  // Nothing to do.
}

size_t XPCStringConvert::LiteralExternalStringSizeOfBuffer(
    const char16_t* aChars, mozilla::MallocSizeOf aMallocSizeOf) {
  // This string's buffer is not heap-allocated, so its malloc size is 0.
  return 0;
}

void XPCStringConvert::DOMExternalStringFinalize(
    char16_t* aChars) {
  nsStringBuffer* buf = nsStringBuffer::FromData(aChars);
  buf->Release();
}

size_t XPCStringConvert::DOMExternalStringSizeOfBuffer(
    const char16_t* aChars, mozilla::MallocSizeOf aMallocSizeOf) {
  // We promised the JS engine we would not GC.  Enforce that:
  JS::AutoCheckCannotGC autoCannotGC;

  const nsStringBuffer* buf =
      nsStringBuffer::FromData(const_cast<char16_t*>(aChars));
  // We want sizeof including this, because the entire string buffer is owned by
  // the external string.  But only report here if we're unshared; if we're
  // shared then we don't know who really owns this data.
  return buf->SizeOfIncludingThisIfUnshared(aMallocSizeOf);
}

void XPCStringConvert::DynamicAtomExternalStringFinalize(
    char16_t* aChars) {
  nsDynamicAtom* atom = nsDynamicAtom::FromChars(aChars);
  // nsDynamicAtom::Release is always-inline and defined in a translation unit
  // we can't get to here.  So we need to go through nsAtom::Release to call
  // it.
  static_cast<nsAtom*>(atom)->Release();
}

size_t XPCStringConvert::DynamicAtomExternalStringSizeOfBuffer(
    const char16_t* aChars, mozilla::MallocSizeOf aMallocSizeOf) {
  // We return 0 here because NS_AddSizeOfAtoms reports all memory associated
  // with atoms in the atom table.
  return 0;
}

// convert a readable to a JSString, copying string data
// static
bool XPCStringConvert::ReadableToJSVal(JSContext* cx, const nsAString& readable,
                                       nsStringBuffer** sharedBuffer,
                                       MutableHandleValue vp) {
  *sharedBuffer = nullptr;

  uint32_t length = readable.Length();

  if (readable.IsLiteral()) {
    return StringLiteralToJSVal(cx, readable.BeginReading(), length, vp);
  }

  nsStringBuffer* buf = nsStringBuffer::FromString(readable);
  if (buf) {
    bool shared;
    if (!StringBufferToJSVal(cx, buf, length, vp, &shared)) {
      return false;
    }
    if (shared) {
      *sharedBuffer = buf;
    }
    return true;
  }

  // blech, have to copy.
  JSString* str = JS_NewUCStringCopyN(cx, readable.BeginReading(), length);
  if (!str) {
    return false;
  }
  vp.setString(str);
  return true;
}

namespace xpc {

bool NonVoidStringToJsval(JSContext* cx, nsAString& str,
                          MutableHandleValue rval) {
  nsStringBuffer* sharedBuffer;
  if (!XPCStringConvert::ReadableToJSVal(cx, str, &sharedBuffer, rval)) {
    return false;
  }

  if (sharedBuffer) {
    // The string was shared but ReadableToJSVal didn't addref it.
    // Move the ownership from str to jsstr.
    str.ForgetSharedBuffer();
  }
  return true;
}

}  // namespace xpc
