/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Monkeycage API. */

#ifndef mc_StoreBuffer_h
#define mc_StoreBuffer_h

#include "mozilla/AllocPolicy.h"
#include "mozilla/HashFunctions.h"
#include "mozilla/HashTable.h"
#include "mozilla/ReentrancyGuard.h"

#include "js/GCAPI.h"
#include "js/HeapAPI.h"
#include "js/TracingAPI.h"
#include "js/TypeDecls.h"
#include "js/Value.h"

#include "monkeycage/Tainted.h"

struct MCRuntime;

namespace mc {
namespace gc {

/*
 * The StoreBuffer observes all writes that occur in the system and performs
 * efficient filtering of them to derive a remembered set for nursery GC.
 */
class StoreBuffer {
  friend class mozilla::ReentrancyGuard;

  /* The size at which other store buffers are about to overflow. */
  static const size_t BufferOverflowThresholdBytes = 128 * 1024;

  /*
   * This buffer holds only a single type of edge. Using this buffer is more
   * efficient than the generic buffer when many writes will be to the same
   * type of edge: e.g. Value or Cell*.
   */
  template <typename T>
  struct MonoTypeBuffer {
    /* The canonical set of stores. */
    typedef mozilla::HashSet<T, typename T::Hasher, mozilla::MallocAllocPolicy>
        StoreSet;
    StoreSet stores_;

    /*
     * A one element cache in front of the canonical set to speed up
     * temporary instances of HeapPtr.
     */
    T last_;

    StoreBuffer* owner_;

    JS::GCReason gcReason_;

    /* Maximum number of entries before we request a minor GC. */
    const static size_t MaxEntries = BufferOverflowThresholdBytes / sizeof(T);

    explicit MonoTypeBuffer(StoreBuffer* owner, JS::GCReason reason)
        : last_(T()), owner_(owner), gcReason_(reason) {}

    void clear() {
      last_ = T();
      stores_.clear();
    }

    /* Add one item to the buffer. */
    void put(const T& t) {
      sinkStore();
      last_ = t;
    }

    /* Remove an item from the store buffer. */
    void unput(const T& v) {
      // Fast, hashless remove of last put.
      if (last_ == v) {
        last_ = T();
        return;
      }
      stores_.remove(v);
    }

    /* Move any buffered stores to the canonical store set. */
    void sinkStore() {
      if (last_) {
        MOZ_RELEASE_ASSERT(stores_.put(last_));
      }
      last_ = T();

      if (MOZ_UNLIKELY(stores_.count() > MaxEntries)) {
        // owner_->setAboutToOverflow(gcReason_);
      }
    }

    /* Trace the source of all edges in the store buffer. */
    void trace(MC::Tainted<JSTracer*> mover);

    bool isEmpty() const { return last_ == T() && stores_.empty(); }

   private:
    MonoTypeBuffer(const MonoTypeBuffer& other) = delete;
    MonoTypeBuffer& operator=(const MonoTypeBuffer& other) = delete;
  };

  template <typename Edge>
  struct PointerEdgeHasher {
    using Lookup = Edge;
    static mozilla::HashNumber hash(const Lookup& l) {
      return mozilla::HashGeneric(l.edge);
    }
    static bool match(const Edge& k, const Lookup& l) { return k == l; }
  };

  template <typename T>
  struct CellPtrEdge {
    T** edge = nullptr;

    CellPtrEdge() = default;
    explicit CellPtrEdge(T** v) : edge(v) {}
    bool operator==(const CellPtrEdge& other) const {
      return edge == other.edge;
    }
    bool operator!=(const CellPtrEdge& other) const {
      return edge != other.edge;
    }

    bool maybeInRememberedSet() const { return js::gc::IsInsideNursery(*edge); }

    void trace(MC::Tainted<JSTracer*> mover) const;

    explicit operator bool() const { return edge != nullptr; }

    using Hasher = PointerEdgeHasher<CellPtrEdge<T>>;
  };

  using ObjectPtrEdge = CellPtrEdge<JSObject>;
  using StringPtrEdge = CellPtrEdge<JSString>;
  using BigIntPtrEdge = CellPtrEdge<JS::BigInt>;

  struct ValueEdge {
    JS::Value* edge;

    ValueEdge() : edge(nullptr) {}
    explicit ValueEdge(JS::Value* v) : edge(v) {}
    bool operator==(const ValueEdge& other) const { return edge == other.edge; }
    bool operator!=(const ValueEdge& other) const { return edge != other.edge; }

    js::gc::Cell* deref() const {
      return edge->isGCThing() ? static_cast<js::gc::Cell*>(edge->toGCThing())
                               : nullptr;
    }

    bool maybeInRememberedSet() const { return js::gc::IsInsideNursery(deref()); }

    void trace(MC::Tainted<JSTracer*> mover) const;

    explicit operator bool() const { return edge != nullptr; }

    using Hasher = PointerEdgeHasher<ValueEdge>;
  };

  void checkAccess() const {}

  template <typename Buffer, typename Edge>
  void unput(Buffer& buffer, const Edge& edge) {
    checkAccess();
    mozilla::ReentrancyGuard g(*this);
    buffer.unput(edge);
  }

  template <typename Buffer, typename Edge>
  void put(Buffer& buffer, const Edge& edge) {
    checkAccess();
    mozilla::ReentrancyGuard g(*this);
    if (edge.maybeInRememberedSet()) {
      buffer.put(edge);
    }
  }

  MonoTypeBuffer<ValueEdge> bufferVal;
  MonoTypeBuffer<StringPtrEdge> bufStrCell;
  MonoTypeBuffer<BigIntPtrEdge> bufBigIntCell;
  MonoTypeBuffer<ObjectPtrEdge> bufObjCell;

  bool enabled_;
  bool aboutToOverflow_;
#ifdef DEBUG
  bool mEntered; /* For ReentrancyGuard. */
#endif

public:
  explicit StoreBuffer(MCRuntime*);

  bool enable();

  void disable();
  bool isEnabled() const { return enabled_; }
  bool isEmpty() const;
  void checkEmpty() const;
  void clear();

  /* Insert a single edge into the buffer/remembered set. */
  void putValue(JS::Value* vp) { put(bufferVal, ValueEdge(vp)); }
  void unputValue(JS::Value* vp) { unput(bufferVal, ValueEdge(vp)); }

  void putCell(JSString** strp) { put(bufStrCell, StringPtrEdge(strp)); }
  void unputCell(JSString** strp) { unput(bufStrCell, StringPtrEdge(strp)); }

  void putCell(JS::BigInt** bip) { put(bufBigIntCell, BigIntPtrEdge(bip)); }
  void unputCell(JS::BigInt** bip) { unput(bufBigIntCell, BigIntPtrEdge(bip)); }

  void putCell(JSObject** strp) { put(bufObjCell, ObjectPtrEdge(strp)); }
  void unputCell(JSObject** strp) { unput(bufObjCell, ObjectPtrEdge(strp)); }

  /* Methods to trace the source of all edges in the store buffer. */
  void traceValues(MC::Tainted<JSTracer*> mover) {
    bufferVal.trace(mover);
  }
  void traceCells(MC::Tainted<JSTracer*> mover) {
    bufStrCell.trace(mover);
    bufBigIntCell.trace(mover);
    bufObjCell.trace(mover);
  }

  MCRuntime* rt_;
  
  static inline thread_local StoreBuffer* sb_;
};

}  // namespace gc
}  // namespace mc

extern MCRuntime* JS_SanitizeRuntime(JSRuntime* rt);

namespace MC {

template <typename T>
inline void HeapPostWriteBarrier(T** thingp, T* prev, T* next) {
  MOZ_ASSERT(mc::gc::StoreBuffer::sb_);
  MOZ_ASSERT(thingp);

  if (next && js::gc::IsInsideNursery(reinterpret_cast<js::gc::Cell*>(next))) {
    // If we know that the prev has already inserted an entry, we can skip
    // doing the lookup to add the new entry. Note that we cannot safely
    // assert the presence of the entry because it may have been added
    // via a different store buffer.
    if (prev && js::gc::IsInsideNursery(reinterpret_cast<js::gc::Cell*>(prev))) {
      return;
    }

    mc::gc::StoreBuffer* sb = mc::gc::StoreBuffer::sb_;
    MOZ_ASSERT_IF(prev, sb->rt_ == JS_SanitizeRuntime(js::gc::detail::CellJSRuntime(reinterpret_cast<js::gc::Cell*>(prev))));
    MOZ_ASSERT_IF(next, sb->rt_ == JS_SanitizeRuntime(js::gc::detail::CellJSRuntime(reinterpret_cast<js::gc::Cell*>(next))));
    sb->putCell(thingp);
    return;
  }

  if (prev && js::gc::IsInsideNursery(reinterpret_cast<js::gc::Cell*>(prev))) {
    mc::gc::StoreBuffer* sb = mc::gc::StoreBuffer::sb_;
    MOZ_ASSERT_IF(prev, sb->rt_ == JS_SanitizeRuntime(js::gc::detail::CellJSRuntime(reinterpret_cast<js::gc::Cell*>(prev))));
    MOZ_ASSERT_IF(next, sb->rt_ == JS_SanitizeRuntime(js::gc::detail::CellJSRuntime(reinterpret_cast<js::gc::Cell*>(next))));
    sb->unputCell(thingp);
    return;
  }
}

inline void HeapValuePostWriteBarrier(JS::Value* vp, JS::Value prev, JS::Value next) {
  MOZ_ASSERT(mc::gc::StoreBuffer::sb_);
  MOZ_ASSERT(vp);

  if (next.isGCThing() && js::gc::IsInsideNursery(next.toGCThing())) {
    // If we know that the prev has already inserted an entry, we can
    // skip doing the lookup to add the new entry. Note that we cannot
    // safely assert the presence of the entry because it may have been
    // added via a different store buffer.
    if (prev.isGCThing() && js::gc::IsInsideNursery(prev.toGCThing())) {
      return;
    }

    mc::gc::StoreBuffer* sb = mc::gc::StoreBuffer::sb_;
    MOZ_ASSERT_IF(next.isGCThing() && next.toGCThing(),
                  sb->rt_ == JS_SanitizeRuntime(js::gc::detail::CellJSRuntime(
                                 next.toGCThing())));
    MOZ_ASSERT_IF(prev.isGCThing() && prev.toGCThing(),
                  sb->rt_ == JS_SanitizeRuntime(js::gc::detail::CellJSRuntime(
                                 prev.toGCThing())));
    sb->putValue(vp);
    return;
  }

  // Remove the prev entry if the new value does not need it.
  if (prev.isGCThing() && js::gc::IsInsideNursery(prev.toGCThing())) {
    mc::gc::StoreBuffer* sb = mc::gc::StoreBuffer::sb_;
    MOZ_ASSERT_IF(next.isGCThing() && next.toGCThing(),
                  sb->rt_ == JS_SanitizeRuntime(js::gc::detail::CellJSRuntime(
                                 next.toGCThing())));
    MOZ_ASSERT_IF(prev.isGCThing() && prev.toGCThing(),
                  sb->rt_ == JS_SanitizeRuntime(js::gc::detail::CellJSRuntime(
                                 prev.toGCThing())));
    sb->unputValue(vp);
    return;
  }
}

}

#endif
