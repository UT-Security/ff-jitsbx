/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_MemoryMetrics_h
#define mc_MemoryMetrics_h

#include "js/MemoryMetrics.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"

namespace JS {

inline bool CollectRuntimeStats(MCContext* cx, RuntimeStats* rtStats,
                                ObjectPrivateVisitor* opv, bool anonymize) {
  return CollectRuntimeStats(cx->cx_, rtStats, opv, anonymize);
}

inline size_t SystemCompartmentCount(MCContext* cx) {
  return SystemCompartmentCount(cx->cx_);
}
inline size_t UserCompartmentCount(MCContext* cx) {
  return UserCompartmentCount(cx->cx_);
}

inline size_t SystemRealmCount(MCContext* cx) {
  return SystemRealmCount(cx->cx_);
}
inline size_t UserRealmCount(MCContext* cx) {
  return UserRealmCount(cx->cx_);
}

inline size_t PeakSizeOfTemporary(const MCContext* cx) {
  return PeakSizeOfTemporary(cx->cx_);
}

inline bool AddSizeOfTab(MCContext* cx, JS::HandleObject obj,
                         mozilla::MallocSizeOf mallocSizeOf,
                         ObjectPrivateVisitor* opv, TabSizes* sizes) {
  return AddSizeOfTab(cx->cx_, obj, mallocSizeOf, opv, sizes);  
}

}  // namespace JS
#endif

#endif
