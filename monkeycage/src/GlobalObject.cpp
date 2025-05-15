/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifdef JS_SANDBOX

#include "jsapi.h"
#include "js/Class.h"
#include "monkeycage/GlobalObject.h"

namespace MC {

const JSClassOps* DefaultGlobalClassOps() {
  //TODO(abhishek): use sandbox addresses of function pointers.
  static const JSClassOps inner_ = {
      nullptr,                         // addProperty
      nullptr,                         // delProperty
      nullptr,                         // enumerate
      JS_NewEnumerateStandardClasses,  // newEnumerate
      JS_ResolveStandardClass,         // resolve
      JS_MayResolveStandardClass,      // mayResolve
      nullptr,                         // finalize
      nullptr,                         // call
      nullptr,                         // construct
      JS_GlobalObjectTraceHook,        // trace
  };

  return &inner_;
}
}
#else

#include "js/Class.h"
#include "monkeycage/GlobalObject.h"

namespace MC {
const JSClassOps* DefaultGlobalClassOps() {
  return &JS::DefaultGlobalClassOps;
}
   
}
#endif
