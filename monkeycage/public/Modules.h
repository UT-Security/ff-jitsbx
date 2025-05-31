/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* JavaScript module (as in, the syntactic construct) operations. */

#ifndef mc_Modules_h
#define mc_Modules_h

#include "js/Modules.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxCallback.h"

namespace JS {

inline void SetSupportedImportAssertions(
    MCRuntime* rt, const ImportAssertionVector& assertions) {
  return SetSupportedImportAssertions(rt->rt_, assertions);
}

inline ModuleResolveHook GetModuleResolveHook(MCRuntime* rt) { return GetModuleResolveHook(rt->rt_); }

inline void SetModuleResolveHook(MCRuntime* rt,
                                 MC::SandboxCallback<ModuleResolveHook> func) {
  return SetModuleResolveHook(rt->rt_, func.UNSAFE_get());
}

inline ModuleMetadataHook GetModuleMetadataHook(MCRuntime* rt) {
  return GetModuleMetadataHook(rt->rt_);
}

inline void SetModuleMetadataHook(MCRuntime* rt,
                                  MC::SandboxCallback<ModuleMetadataHook> func) {
  return SetModuleMetadataHook(rt->rt_, func.UNSAFE_get());
}

inline ModuleDynamicImportHook GetModuleDynamicImportHook(MCRuntime* rt) {
  return GetModuleDynamicImportHook(rt->rt_);
}

inline void SetModuleDynamicImportHook(
    MCRuntime* rt, MC::SandboxCallback<ModuleDynamicImportHook> func) {
  return SetModuleDynamicImportHook(rt->rt_, func.UNSAFE_get());
}

}  // namespace JS
#endif

#endif
