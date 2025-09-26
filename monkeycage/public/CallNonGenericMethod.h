/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_CallNonGenericMethod_h
#define mc_CallNonGenericMethod_h

#include "js/CallNonGenericMethod.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"
#include "monkeycage/Tainted.h"

namespace JS {

template <IsAcceptableThis Test, NativeImpl Impl>
inline bool CallNonGenericMethod(MCContext* cx,
                                            const CallArgs& args) {
  HandleValue thisv = args.thisv();
  if (Test(thisv)) {
    return Impl(cx->cx_, args);
  }

  return detail::CallMethodIfWrapped(cx->cx_, Test, Impl, args);
}

}

#endif

#endif
