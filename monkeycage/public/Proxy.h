/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Proxy_h
#define mc_Proxy_h

#ifdef JS_SANDBOX

namespace mc {
extern const JSClassOps* ProxyClassOps();
extern const js::ClassExtension* ProxyClassExtension();
extern const js::ObjectOps* ProxyObjectOps();
}

#define MONKEYCAGE_PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, classSpec) \
  {                                                                        \
    name,                                                                  \
        JSClass::NON_NATIVE | JSCLASS_IS_PROXY |                           \
            JSCLASS_DELAY_METADATA_BUILDER | js::CheckProxyFlags<flags>(), \
        mc::ProxyClassOps(), classSpec, mc::ProxyClassExtension(),         \
        mc::ProxyObjectOps()                                               \
  }

#define MONKEYCAGE_PROXY_CLASS_DEF(name, flags) \
  MONKEYCAGE_PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, JS_NULL_CLASS_SPEC)

#else

#define MONKEYCAGE_PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, classSpec) \
  PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, classSpec)

#define MONKEYCAGE_PROXY_CLASS_DEF(name, flags) \
  PROXY_CLASS_DEF_WITH_CLASS_SPEC(name, flags, JS_NULL_CLASS_SPEC)

#endif

#endif
