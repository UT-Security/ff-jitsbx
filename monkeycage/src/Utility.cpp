/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Various JS utility functions. */

#include "monkeycage/Utility.h"

#ifdef JS_SANDBOX

namespace mc {

arena_id_t GetMallocArena() {
  static auto inner_ = js::GetMallocArena();
  return inner_;
}

arena_id_t GetArrayBufferContentsArena() {
  static auto inner_ = js::GetArrayBufferContentsArena();
  return inner_;
}

arena_id_t GetStringBufferArena() {
  static auto inner_ = js::GetStringBufferArena();
  return inner_;
}

}

#else

namespace mc {
arena_id_t GetMallocArena() {
  return js::MallocArena;
}

arena_id_t GetArrayBufferContentsArena() {
  return js::ArrayBufferContentsArena;
}

arena_id_t GetStringBufferArena() {
  return js::StringBufferArena;
}

}
#endif
