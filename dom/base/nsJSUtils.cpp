/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * This is not a generated file. It contains common utility functions
 * invoked from the JavaScript code generated from IDL interfaces.
 * The goal of the utility functions is to cut down on the size of
 * the generated code itself.
 */

#include "nsJSUtils.h"

#include <utility>
#include "MainThreadUtils.h"
#include "js/ComparisonOperators.h"
#include "monkeycage/CompilationAndEvaluation.h"
#include "monkeycage/CompileOptions.h"
#include "monkeycage/Date.h"
#include "js/GCVector.h"
#include "js/HeapAPI.h"
#include "monkeycage/Modules.h"
#include "monkeycage/RootingAPI.h"
#include "monkeycage/SourceText.h"
#include "monkeycage/TypeDecls.h"
#include "mcfriendapi.h"
#include "mozilla/CycleCollectedJSContext.h"
#include "mozilla/dom/BindingUtils.h"
#include "mozilla/dom/Element.h"
#include "mozilla/dom/ScriptSettings.h"
#include "mozilla/fallible.h"
#include "mozilla/ProfilerLabels.h"
#include "nsContentUtils.h"
#include "nsDebug.h"
#include "nsGlobalWindowInner.h"
#include "nsINode.h"
#include "nsString.h"
#include "nsTPromiseFlatString.h"
#include "nscore.h"
#include "prenv.h"

#if !defined(DEBUG) && !defined(MOZ_ENABLE_JS_DUMP)
#  include "mozilla/StaticPrefs_browser.h"
#endif

using namespace mozilla;
using namespace mozilla::dom;

bool nsJSUtils::GetCallingLocation(MCContext* aContext, nsACString& aFilename,
                                   uint32_t* aLineno, uint32_t* aColumn) {
  MC::SandboxStack<JS::AutoFilename> filename;
  MC::SandboxStack<uint32_t> tLineno;
  MC::SandboxStack<uint32_t> tColumn;
  if (!JS::DescribeScriptedCaller(aContext, filename, tLineno, tColumn)) {
    return false;
  }

  if (aLineno) *aLineno = *tLineno.UNSAFE_unverified();
  if (aColumn) *aColumn = *tColumn.UNSAFE_unverified();

  return aFilename.Assign(filename->get(), fallible);
}

bool nsJSUtils::GetCallingLocation(MCContext* aContext, nsAString& aFilename,
                                   uint32_t* aLineno, uint32_t* aColumn) {
  MC::SandboxStack<JS::AutoFilename> filename;
  MC::SandboxStack<uint32_t> tLineno;
  MC::SandboxStack<uint32_t> tColumn;
  if (!JS::DescribeScriptedCaller(aContext, filename, tLineno, tColumn)) {
    return false;
  }

  if (aLineno) *aLineno = *tLineno.UNSAFE_unverified();
  if (aColumn) *aColumn = *tColumn.UNSAFE_unverified();

  return aFilename.Assign(NS_ConvertUTF8toUTF16(filename->get()), fallible);
}

uint64_t nsJSUtils::GetCurrentlyRunningCodeInnerWindowID(MCContext* aContext) {
  if (!aContext) return 0;

  nsGlobalWindowInner* win = xpc::CurrentWindowOrNull(aContext);
  return win ? win->WindowID() : 0;
}

nsresult nsJSUtils::UpdateFunctionDebugMetadata(
    AutoJSAPI& jsapi, JS::Handle<JSObject*> aFun,
    MC::Tainted<JS::CompileOptions*> aOptions,
    JS::Handle<JSString*> aElementAttributeName,
    JS::Handle<JS::Value> aPrivateValue) {
  MCContext* cx = jsapi.mcx();

  MC::Rooted<JSFunction*> fun(cx, JS_GetObjectFunction(aFun));
  if (!fun) {
    return NS_ERROR_FAILURE;
  }

  MC::Rooted<JSScript*> script(cx, JS_GetFunctionScript(cx, fun));
  if (!script) {
    return NS_OK;
  }

  MC::SandboxStack<JS::InstantiateOptions> instantiateOptions(*aOptions);
  if (!JS::UpdateDebugMetadata(cx, script, instantiateOptions, aPrivateValue,
                               aElementAttributeName, nullptr, nullptr)) {
    return NS_ERROR_FAILURE;
  }
  return NS_OK;
}

nsresult nsJSUtils::CompileFunction(AutoJSAPI& jsapi,
                                    JS::HandleVector<JSObject*> aScopeChain,
                                    MC::Tainted<JS::CompileOptions*> aOptions,
                                    const nsACString& aName, uint32_t aArgCount,
                                    const char** aArgArray,
                                    const nsAString& aBody,
                                    JSObject** aFunctionObject) {
  MCContext* cx = jsapi.mcx();
  MOZ_ASSERT(js::GetContextRealm(cx));
  MOZ_ASSERT_IF(aScopeChain.length() != 0,
                js::IsObjectInContextCompartment(aScopeChain[0], cx));

  // Do the junk Gecko is supposed to do before calling into JSAPI.
  for (size_t i = 0; i < aScopeChain.length(); ++i) {
    JS::ExposeObjectToActiveJS(aScopeChain[i]);
  }

  // Compile.
  const nsPromiseFlatString& flatBody = PromiseFlatString(aBody);

  MC::SandboxStack<JS::SourceText<char16_t>> source;
  if (!source->init(cx, flatBody.get(), flatBody.Length(),
                   JS::SourceOwnership::Borrowed)) {
    return NS_ERROR_FAILURE;
  }

  MC::Rooted<JSFunction*> fun(
      cx, JS::CompileFunction(jsapi.mcx(), aScopeChain, aOptions,
                              PromiseFlatCString(aName).get(), aArgCount,
                              aArgArray, source));
  if (!fun) {
    return NS_ERROR_FAILURE;
  }

  *aFunctionObject = JS_GetFunctionObject(fun);
  return NS_OK;
}

/* static */
bool nsJSUtils::IsScriptable(JS::Handle<JSObject*> aEvaluationGlobal) {
  return xpc::Scriptability::AllowedIfExists(aEvaluationGlobal);
}

static bool AddScopeChainItem(MCContext* aCx, nsINode* aNode,
                              JS::MutableHandleVector<JSObject*> aScopeChain) {
  MC::Rooted<JS::Value> val(aCx);
  if (!GetOrCreateDOMReflector(aCx, aNode, &val)) {
    return false;
  }

  if (!aScopeChain.append(&val.toObject())) {
    return false;
  }

  return true;
}

/* static */
bool nsJSUtils::GetScopeChainForElement(
    MCContext* aCx, Element* aElement,
    JS::MutableHandleVector<JSObject*> aScopeChain) {
  for (nsINode* cur = aElement; cur; cur = cur->GetScopeChainParent()) {
    if (!AddScopeChainItem(aCx, cur, aScopeChain)) {
      return false;
    }
  }

  return true;
}

/* static */
void nsJSUtils::ResetTimeZone() { JS::ResetTimeZone(); }

/* static */
bool nsJSUtils::DumpEnabled() {
#ifdef FUZZING
  static bool mozFuzzDebug = !!PR_GetEnv("MOZ_FUZZ_DEBUG");
  return mozFuzzDebug;
#endif

#if defined(DEBUG) || defined(MOZ_ENABLE_JS_DUMP)
  return true;
#else
  return StaticPrefs::browser_dom_window_dump_enabled();
#endif
}

JSObject* nsJSUtils::MoveBufferAsUint8Array(MCContext* aCx, size_t aSize,
                                            UniquePtr<uint8_t>& aBuffer) {
  MC::Rooted<JSObject*> arrayBuffer(
      aCx, JS::NewArrayBufferWithContents(aCx, aSize, aBuffer.get()));
  if (!arrayBuffer) {
    return nullptr;
  }

  // Now the ArrayBuffer owns the buffer, so let's release our ownership
  (void)aBuffer.release();

  return JS_NewUint8ArrayWithBuffer(aCx, arrayBuffer, 0,
                                    static_cast<int64_t>(aSize));
}

//
// nsDOMJSUtils.h
//

template <typename T>
bool nsTAutoJSString<T>::init(const JS::Value& v) {
  // Note: it's okay to use danger::GetJSContext here instead of AutoJSAPI,
  // because the init() call below is careful not to run script (for instance,
  // it only calls JS::ToString for non-object values).
  MCContext* cx = danger::GetJSContext();
  if (!init(cx, v)) {
    JS_ClearPendingException(cx);
    return false;
  }
  return true;
}

template bool nsTAutoJSString<char16_t>::init(const JS::Value&);
template bool nsTAutoJSString<char>::init(const JS::Value&);
