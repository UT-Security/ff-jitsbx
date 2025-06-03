/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_StreamConsumer_h
#define mc_StreamConsumer_h

#include "js/StreamConsumer.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"

namespace JS {

inline void InitConsumeStreamCallback(
    MCContext* cx, MC::SandboxCallback<ConsumeStreamCallback> consume,
    MC::SandboxCallback<ReportStreamErrorCallback> report) {
  return InitConsumeStreamCallback(cx->cx_, consume.UNSAFE_get(),
                                   report.UNSAFE_get());
}

}  // namespace JS

#endif

#endif
