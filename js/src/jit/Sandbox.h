/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_Sandbox_h
#define jit_Sandbox_h

namespace js {
namespace jit {

#ifdef JS_SANDBOX_HEAP
const size_t SANDBOX_SIZE = 1UL * 1024 * 1024 * 1024 * 1024;
const size_t SANDBOX_MASK = SIZE - 1;
#endif

}  // namespace jit
}  // namespace js

#endif
