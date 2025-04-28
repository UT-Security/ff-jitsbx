/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/TextEncoder.h"
#include "mozilla/CheckedInt.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/UniquePtrExtensions.h"
#include "nsReadableUtils.h"
#include "monkeycage/Realm.h"

namespace mozilla::dom {

void TextEncoder::Encode(JSContext* aCx, JS::Handle<JSObject*> aObj,
                         const nsACString& aUtf8String,
                         JS::MutableHandle<JSObject*> aRetval,
                         OOMReporter& aRv) {
  MC::JSAutoRealm ar(aCx, aObj);
  JSObject* outView = Uint8Array::Create(aCx, aUtf8String);
  if (!outView) {
    aRv.ReportOOM();
    return;
  }

  aRetval.set(outView);
}

void TextEncoder::EncodeInto(JSContext* aCx, JS::Handle<JSString*> aSrc,
                             const Uint8Array& aDst,
                             TextEncoderEncodeIntoResult& aResult,
                             OOMReporter& aError) {
  aDst.ComputeState();
  monkeycage::AutoStackTainted<size_t> read;
  monkeycage::AutoStackTainted<size_t> written;
  
  char* bufCopy = (char*)js_malloc(aDst.Length());
  mozilla::Span<char> spanCopy{bufCopy, aDst.Length()};
  
  auto maybe = JS_EncodeStringToUTF8BufferPartial(
      aCx, aSrc, spanCopy, read.UNSAFE_unverified(), written.UNSAFE_unverified());
  if (!maybe) {
    js_free(bufCopy);
    aError.ReportOOM();
    return;
  }
  MOZ_ASSERT(*written.UNSAFE_unverified() <= aDst.Length());
  memcpy(aDst.Data(), bufCopy, aDst.Length());
  js_free(bufCopy);
  aResult.mRead.Construct() = *read.UNSAFE_unverified();
  aResult.mWritten.Construct() = *written.UNSAFE_unverified();
}

void TextEncoder::GetEncoding(nsACString& aEncoding) {
  aEncoding.AssignLiteral("utf-8");
}

}  // namespace mozilla::dom
