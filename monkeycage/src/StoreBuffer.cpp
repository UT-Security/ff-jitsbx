/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "monkeycage/Context.h"
#include "monkeycage/StoreBuffer.h"
#include "monkeycage/TracingAPI.h"

namespace mc {
namespace gc {

StoreBuffer::StoreBuffer(MCRuntime* rt)
    : bufferVal(this, JS::GCReason::FULL_VALUE_BUFFER),
      bufStrCell(this, JS::GCReason::FULL_CELL_PTR_STR_BUFFER),
      bufBigIntCell(this, JS::GCReason::FULL_CELL_PTR_BIGINT_BUFFER),
      bufObjCell(this, JS::GCReason::FULL_CELL_PTR_OBJ_BUFFER),
      enabled_(true),
      aboutToOverflow_(false)
#ifdef DEBUG
      ,
      mEntered(false)
#endif
      ,rt_(rt)
      {
        MOZ_ASSERT(!StoreBuffer::sb_);
        StoreBuffer::sb_ = this;
      }

void StoreBuffer::checkEmpty() const { MOZ_ASSERT(isEmpty()); }

bool StoreBuffer::isEmpty() const {
  return bufferVal.isEmpty() && bufStrCell.isEmpty() &&
         bufBigIntCell.isEmpty() && bufObjCell.isEmpty();
}

bool StoreBuffer::enable() {
  if (enabled_) {
    return true;
  }

  checkEmpty();

  enabled_ = true;
  return true;
}

void StoreBuffer::disable() {
  checkEmpty();

  if (!enabled_) {
    return;
  }

  aboutToOverflow_ = false;

  enabled_ = false;
}

void StoreBuffer::clear() {
  if (!enabled_) {
    return;
  }

  aboutToOverflow_ = false;

  bufferVal.clear();
  bufStrCell.clear();
  bufBigIntCell.clear();
  bufObjCell.clear();
}

template <typename T>
void StoreBuffer::MonoTypeBuffer<T>::trace(MC::Tainted<JSTracer*> mover) {
  mozilla::ReentrancyGuard g(*owner_);
  MOZ_ASSERT(owner_->isEnabled());
  if (last_) {
    last_.trace(mover);
  }
  for (typename StoreSet::Range r = stores_.all(); !r.empty(); r.popFront()) {
    r.front().trace(mover);
  }
}

void StoreBuffer::ValueEdge::trace(MC::Tainted<JSTracer*> trc) const {
  if (!deref()) {
    return;
  }
  JS::Value thing = *edge;
  MC::SandboxStack<JS::Value> sbx_thingp(thing);
  JS::TraceSecureEdge(trc.INTERNAL_unverified_safe(),
                      sbx_thingp.UNSAFE_unverified(), "MC Store Buffer Value");
  if (*sbx_thingp.UNSAFE_unverified() != thing) {
    *edge = *sbx_thingp.UNSAFE_unverified();
  }
}

template <typename T>
void StoreBuffer::CellPtrEdge<T>::trace(MC::Tainted<JSTracer*> trc) const {
  T* thing = *edge;
  if (!thing) {
    return;
  }

  MOZ_ASSERT(js::gc::IsCellPointerValid(thing));

  //if (std::is_same_v<JSString, T>) {
    // Nursery string deduplication requires all tenured string -> nursery
    // string edges to be registered with the whole cell buffer in order to
    // correctly set the non-deduplicatable bit.
    //MOZ_ASSERT(!mover.runtime()->gc.isPointerWithinTenuredCell(
    //    edge, JS::TraceKind::String));
  //}

  MC::SandboxStack<T*> sbx_thingp(thing);
  JS::TraceSecureEdge(trc.INTERNAL_unverified_safe(),
                      sbx_thingp.UNSAFE_unverified(), "MC Store Buffer Cell");
  if (*sbx_thingp.UNSAFE_unverified() != thing) {
    *edge = *sbx_thingp.UNSAFE_unverified();
  }
}

}  // namespace gc
}  // namespace mc
