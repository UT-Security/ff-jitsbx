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

namespace mozilla::dom {

void TextEncoder::Encode(MCContext* aCx, JS::Handle<JSObject*> aObj,
                         const nsACString& aUtf8String,
                         JS::MutableHandle<JSObject*> aRetval,
                         OOMReporter& aRv) {
  MC::SandboxStack<JSAutoRealm> ar(aCx, aObj);
  JSObject* outView = Uint8Array::Create(aCx, aUtf8String);
  if (!outView) {
    aRv.ReportOOM();
    return;
  }

  aRetval.set(outView);
}

void TextEncoder::EncodeInto(MCContext* aCx, JS::Handle<JSString*> aSrc,
                             const Uint8Array& aDst,
                             TextEncoderEncodeIntoResult& aResult,
                             OOMReporter& aError) {
  aDst.ComputeState();
  MC::SandboxStack<size_t> read;
  MC::SandboxStack<size_t> written;
  auto ok = JS_EncodeStringToUTF8BufferPartial(
      aCx, aSrc, AsWritableChars(Span(aDst.Data(), aDst.Length())), read, written);
  if (!ok) {
    aError.ReportOOM();
    return;
  }
  MOZ_ASSERT(*written.UNSAFE_unverified() <= aDst.Length());
  aResult.mRead.Construct() = *read.UNSAFE_unverified();
  aResult.mWritten.Construct() = *written.UNSAFE_unverified();
}

void TextEncoder::GetEncoding(nsACString& aEncoding) {
  aEncoding.AssignLiteral("utf-8");
}

}  // namespace mozilla::dom
