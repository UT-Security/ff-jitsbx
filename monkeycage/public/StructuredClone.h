/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_StructuredClone_h
#define mc_StructuredClone_h

#include "js/StructuredClone.h"

#ifdef JS_SANDBOX

#include "monkeycage/SandboxStack.h"
#include "monkeycage/Tainted.h"

inline bool JS_ReadUint32Pair(MC::Tainted<JSStructuredCloneReader*> r,
                              uint32_t* p1, uint32_t* p2) {
  MC::SandboxStack<uint32_t> t_p1{0}, t_p2{0};
  bool ret =
      JS_ReadUint32Pair(r.INTERNAL_unverified_safe(), t_p1.UNSAFE_unverified(),
                        t_p2.UNSAFE_unverified());

  *p1 = *t_p1.UNSAFE_unverified();
  *p2 = *t_p2.UNSAFE_unverified();

  return ret;
}

inline bool JS_ReadBytes(MC::Tainted<JSStructuredCloneReader*> r, void* p,
                         size_t len) {
  MC::SandboxStackBytes bytes(len);
  if (JS_ReadBytes(r.INTERNAL_unverified_safe(), bytes.begin(), bytes.size())) {
    memcpy(p, bytes.begin(), len);
    return true;
  }

  return false;
}

#endif

#endif
