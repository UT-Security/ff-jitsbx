/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ctypes.h"
#include "mcapi.h"
#include "js/experimental/CTypes.h"  // JS::CTypesCallbacks, JS::InitCTypesClass, JS::SetCTypesCallbacks
#include "monkeycage/MemoryFunctions.h"
#include "monkeycage/PropertyAndElement.h"  // JS_GetProperty
#include "nsString.h"
#include "nsNativeCharsetUtils.h"
#include "mozJSModuleLoader.h"
#include "xpc_make_class.h"

namespace mozilla::ctypes {

static MC::Tainted<char*> UnicodeToNative(MC::Tainted<JSContext*> t_cx, MC::Tainted<const char16_t*> source,
                             size_t slen) {
  MCContext* cx = t_cx.copy_and_verify_address(MC_VerifyContext);
  nsAutoCString native;
  nsDependentSubstring unicode(source.UNSAFE_unverified(), slen);
  nsresult rv = NS_CopyUnicodeToNative(unicode, native);
  if (NS_FAILED(rv)) {
    JS_ReportErrorASCII(cx, "could not convert string to native charset");
    return nullptr;
  }

  MC::Tainted<char*> result = MC::detail::tainted_static_cast<char*>(JS_malloc(cx, native.Length() + 1));
  if (!result) {
    return nullptr;
  }

  memcpy(result.UNSAFE_unverified(), native.get(), native.Length() + 1);
  return result;
}

static JS::CTypesCallbacks* sCallbacks() {
  static JS::CTypesCallbacks inner_ = {MC::Sandbox::RegisterTaintedCallback(UnicodeToNative).UNSAFE_get()};
  return &inner_;
}

NS_IMPL_ISUPPORTS(Module, nsIXPCScriptable)

Module::Module() = default;

Module::~Module() = default;

#define XPC_MAP_CLASSNAME Module
#define XPC_MAP_QUOTED_CLASSNAME "Module"
#define XPC_MAP_FLAGS XPC_SCRIPTABLE_WANT_CALL
#include "xpc_map_end.h"

static bool InitCTypesClassAndSetCallbacks(MCContext* cx,
                                           JS::Handle<JSObject*> global) {
  // Init the ctypes object.
  if (!JS::InitCTypesClass(MC_UNSAFE(cx), global)) {
    return false;
  }

  // Set callbacks for charset conversion and such.
  MC::Rooted<JS::Value> ctypes(cx);
  if (!JS_GetProperty(cx, global, "ctypes", &ctypes)) {
    return false;
  }

  JS::SetCTypesCallbacks(ctypes.toObjectOrNull(), sCallbacks());

  return true;
}

NS_IMETHODIMP
Module::Call(nsIXPConnectWrappedNative* wrapper, MCContext* cx, JSObject* obj,
             const JS::CallArgs& args, bool* _retval) {
  mozJSModuleLoader* loader = mozJSModuleLoader::Get();
  MC::Rooted<JSObject*> targetObj(cx);
  loader->FindTargetObject(cx, &targetObj);

  *_retval = InitCTypesClassAndSetCallbacks(cx, targetObj);
  return NS_OK;
}

}  // namespace mozilla::ctypes
