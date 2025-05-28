/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mc_experimental_SourceHook_h
#define mc_experimental_SourceHook_h

#include "js/experimental/SourceHook.h"
#include "monkeycage/UniquePtr.h"

#ifdef JS_SANDBOX

#include "js/sandbox/SourceHook.h"

#include "monkeycage/Context.h"
#include "monkeycage/Sandbox.h"

namespace mc {

class SourceHook {
public:
  js::UniquePtr<js::SourceHook> inner_;
private:

  static void destructorCb(void* p) {
    auto sourceHook = static_cast<mc::SourceHook*>(p);
    delete sourceHook;
  }

  static bool loadCb(void* p, JSContext* cx, const char* filename,
                   char16_t** twoByteSource, char** utf8Source,
                   size_t* length) {
    auto sourceHook = static_cast<mc::SourceHook*>(p);
    MCContext* mcx = JS_SanitizeContext(cx);
    return sourceHook->load(mcx, filename, twoByteSource, utf8Source, length);
  }

  static const js::sandbox::SourceHook::Ops* ops() {
    static const auto inner_ = js::sandbox::SourceHook::Ops {
      MC::Sandbox::RegisterCallback(destructorCb).UNSAFE_get(),
      MC::Sandbox::RegisterCallback(loadCb).UNSAFE_get(),
    };

    return &inner_;
  }
 public:
  SourceHook(): inner_(js::MakeUnique<js::sandbox::SourceHook>(ops(), this)) {}

  virtual ~SourceHook() = default;
  virtual bool load(MCContext* cx, const char* filename,
                    char16_t** twoByteSource, char** utf8Source,
                    size_t* length) = 0;
};

}

namespace js {

inline void SetSourceHook(MCContext* cx, mozilla::UniquePtr<mc::SourceHook> hook) {
  auto hookPtr = hook.release();
  return SetSourceHook(cx->cx_, std::move(hookPtr->inner_));
}
}

#else

namespace mc {

using SourceHook = js::SourceHook;

}

#endif

#endif
