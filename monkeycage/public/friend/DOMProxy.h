/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Specify information about DOMProxy proxies in the DOM, for use by ICs.
 *
 * Embedders who don't need to define particularly high-performance proxies that
 * can have random properties added to them can ignore this header.
 */

#ifndef mc_friend_DOMProxy_h
#define mc_friend_DOMProxy_h

#include "js/friend/DOMProxy.h"

#ifdef JS_SANDBOX
#include "monkeycage/RootingAPI.h"

namespace MC {

struct ExpandoAndGeneration {
  ExpandoAndGeneration() : expando(JS::UndefinedValue()), generation(0) {}

  void OwnerUnlinked() { ++generation; }

  static constexpr size_t offsetOfExpando() {
    return offsetof(ExpandoAndGeneration, expando);
  }

  static constexpr size_t offsetOfGeneration() {
    return offsetof(ExpandoAndGeneration, generation);
  }

  MC::Heap<JS::Value> expando;
  uint64_t generation;
};

static_assert(sizeof(ExpandoAndGeneration) == sizeof(JS::ExpandoAndGeneration),
              "MC::ExpandoGeneration must have the same size as "
              "JS::ExpandoAndGeneration");
static_assert(MC::ExpandoAndGeneration::offsetOfExpando() ==
                  JS::ExpandoAndGeneration::offsetOfExpando(),
              "offsetOfExpando must match");
static_assert(MC::ExpandoAndGeneration::offsetOfGeneration() ==
                  JS::ExpandoAndGeneration::offsetOfGeneration(),
              "offsetOfExpando must match");

}  // namespace MC

#endif

#endif
