/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_SourceHook_h
#define js_sandbox_SourceHook_h

#include "js/experimental/SourceHook.h"

namespace js {
namespace sandbox {

class JS_PUBLIC_API SourceHook : public js::SourceHook {
public:
  using DestructorOp = void (*)(void* p);
  using LoadOp = bool (*)(void* p, JSContext* cx, const char* filename,
                          char16_t** twoByteSource, char** utf8Source,
                          size_t* length);

  struct Ops {
    DestructorOp destructor;
    LoadOp load;
  };
private:
  const Ops* ops_;
  void* sourceHook_;

public:
  SourceHook(const Ops* ops, void* sourceHook);
  virtual ~SourceHook() override;
  virtual bool load(JSContext* cx, const char* filename,
                    char16_t** twoByteSource, char** utf8Source,
                    size_t* length) override;
};
}
}

#endif
