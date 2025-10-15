/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_StructuredClone_h
#define mc_StructuredClone_h

#include "js/StructuredClone.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/SandboxHeap.h"
#include "monkeycage/SandboxStack.h"
#include "monkeycage/Tainted.h"
#include "monkeycage/Value.h"

namespace MC {
namespace detail {

template <typename MC_Sbx>
class TaintedVolatile<JSAutoStructuredCloneBuffer, MC_Sbx> {
 private:
  JSAutoStructuredCloneBuffer inner_;

  inline auto& get_raw_value_ref() noexcept { return inner_; }
  inline auto& get_raw_value_ref() const noexcept { return inner_; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  MC::Tainted<JSStructuredCloneData*> data() {
    MC::Tainted<JSStructuredCloneData*> ret;
    ret.assign_raw_pointer(&inner_.data());
    return ret;
  }

  bool empty() const { return inner_.empty(); }

  void clear() { return inner_.clear(); }

  JS::StructuredCloneScope scope() const { return inner_.scope(); }

  void adopt(JSStructuredCloneData&& data,
             uint32_t version = JS_STRUCTURED_CLONE_VERSION,
             const JSStructuredCloneCallbacks* callbacks = nullptr,
             void* closure = nullptr) {
    MC::SandboxStack<JSStructuredCloneData> sbx_data(std::move(data));
    inner_.adopt(sbx_data.UNSAFE_unverified(), version, callbacks, closure);
  }

  void giveTo(MC::Tainted<JSStructuredCloneData*> data) { return inner_.giveTo(data.INTERNAL_unverified_safe()); }

  bool read(MCContext* cx, JS::MutableHandleValue vp,
            const JS::CloneDataPolicy& cloneDataPolicy = JS::CloneDataPolicy(),
            const JSStructuredCloneCallbacks* optionalCallbacks = nullptr,
            void* closure = nullptr) {
    return inner_.read(cx->cx_, vp, cloneDataPolicy, optionalCallbacks,
                       closure);
  }

  bool write(MCContext* cx, JS::HandleValue v,
             const JSStructuredCloneCallbacks* optionalCallbacks = nullptr,
             void* closure = nullptr) {
    return inner_.write(cx->cx_, v, optionalCallbacks, closure);
  }

  bool write(MCContext* cx, JS::HandleValue v, JS::HandleValue transferable,
             const JS::CloneDataPolicy& cloneDataPolicy,
             const JSStructuredCloneCallbacks* optionalCallbacks = nullptr,
             void* closure = nullptr) {
    return inner_.write(cx->cx_, v, transferable, cloneDataPolicy,
                        optionalCallbacks, closure);
  }

  size_t sizeOfExcludingThis(mozilla::MallocSizeOf mallocSizeOf) {
    return inner_.sizeOfExcludingThis(mallocSizeOf);
  }

  size_t sizeOfIncludingThis(mozilla::MallocSizeOf mallocSizeOf) {
    return inner_.sizeOfIncludingThis(mallocSizeOf);
  }
};

template <typename MC_Sbx>
class TaintedVolatile<const JSStructuredCloneData, MC_Sbx> {
 private:
  const JSStructuredCloneData inner_;

  inline auto& get_raw_value_ref() noexcept { return inner_; }
  inline auto& get_raw_value_ref() const noexcept { return inner_; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  JS::StructuredCloneScope scope() const { return inner_.scope(); }
  
  size_t Size() const { return inner_.Size(); }
  
  const JSStructuredCloneData::Iterator Start() const { return inner_.Start(); }
  
  bool Advance(JSStructuredCloneData::Iterator& iter, size_t distance) const {
    return inner_.Advance(iter, distance);
  }

  bool ReadBytes(JSStructuredCloneData::Iterator& iter, char* buffer, size_t size) const {
    return inner_.ReadBytes(iter, buffer, size);
  }

  bool UpdateBytes(JSStructuredCloneData::Iterator& iter, const char* data,
                   size_t size) const {
    return inner_.UpdateBytes(iter, data, size);
  }

  MC::SandboxHeap<JSStructuredCloneData> Borrow(
      JSStructuredCloneData::Iterator& iter, size_t size,
      MC::Tainted<bool*> success) const {
    MOZ_ASSERT(scope() == JS::StructuredCloneScope::DifferentProcess);
    MC::SandboxStack<JSStructuredCloneData::BufferList> bufList_(
        inner_.BufList()->Borrow<js::SystemAllocPolicy>(
            iter, size, success.INTERNAL_unverified_safe()));
    return MC::SandboxHeap<JSStructuredCloneData>(
        bufList_.UNSAFE_unverified(), scope(), IgnoreTransferablesIfAny);
  }

  template <typename FunctionToApply>
  bool ForEachDataChunk(FunctionToApply&& function) const {
    return inner_.ForEachDataChunk(std::forward<FunctionToApply>(function));
  }
};

template <typename MC_Sbx>
class TaintedVolatile<JSStructuredCloneData, MC_Sbx> {
 private:
  JSStructuredCloneData inner_;

  inline auto& get_raw_value_ref() noexcept { return inner_; }
  inline auto& get_raw_value_ref() const noexcept { return inner_; }

 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value_ref(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  JS::StructuredCloneScope scope() const { return inner_.scope(); }

  void initScope(JS::StructuredCloneScope newScope) {
    return inner_.initScope(newScope);
  }

  size_t Size() const { return inner_.Size(); }
  
  const JSStructuredCloneData::Iterator Start() const { return inner_.Start(); }
  
  bool Advance(JSStructuredCloneData::Iterator& iter, size_t distance) const {
    return inner_.Advance(iter, distance);
  }

  bool ReadBytes(JSStructuredCloneData::Iterator& iter, char* buffer, size_t size) const {
    return inner_.ReadBytes(iter, buffer, size);
  }

  bool UpdateBytes(JSStructuredCloneData::Iterator& iter, const char* data,
                   size_t size) const {
    return inner_.UpdateBytes(iter, data, size);
  }

  bool AppendBytes(const char* data, size_t size) {
    return inner_.AppendBytes(data, size);
  }
  
  MC::SandboxHeap<JSStructuredCloneData> Borrow(
      JSStructuredCloneData::Iterator& iter, size_t size,
      MC::Tainted<bool*> success) const {
    MOZ_ASSERT(scope() == JS::StructuredCloneScope::DifferentProcess);
    MC::SandboxStack<JSStructuredCloneData::BufferList> bufList_(
        inner_.BufList()->Borrow<js::SystemAllocPolicy>(
            iter, size, success.INTERNAL_unverified_safe()));
    return MC::SandboxHeap<JSStructuredCloneData>(
        bufList_.UNSAFE_unverified(), scope(), IgnoreTransferablesIfAny);
  }

  template <typename FunctionToApply>
  bool ForEachDataChunk(FunctionToApply&& function) const {
    return inner_.ForEachDataChunk(std::forward<FunctionToApply>(function));
  }

  bool Append(MC::Tainted<const JSStructuredCloneData*> other) {
    return inner_.Append(*other.INTERNAL_unverified_safe());
  }
};
}
}

inline bool JS_ReadStructuredClone(
    MCContext* cx, MC::Tainted<const JSStructuredCloneData*> data,
    uint32_t version, JS::StructuredCloneScope scope, JS::MutableHandleValue vp,
    const JS::CloneDataPolicy& cloneDataPolicy,
    const JSStructuredCloneCallbacks* optionalCallbacks, void* closure) {
  return JS_ReadStructuredClone(cx->cx_, *data.INTERNAL_unverified_safe(),
                                version, scope, vp, cloneDataPolicy,
                                optionalCallbacks, closure);
}

inline bool JS_ReadStructuredClone(
    MCContext* cx, const JSStructuredCloneData& data,
    uint32_t version, JS::StructuredCloneScope scope, JS::MutableHandleValue vp,
    const JS::CloneDataPolicy& cloneDataPolicy,
    const JSStructuredCloneCallbacks* optionalCallbacks, void* closure) {
  return JS_ReadStructuredClone(cx->cx_, data,
                                version, scope, vp, cloneDataPolicy,
                                optionalCallbacks, closure);
}

inline bool JS_StructuredClone(
    MCContext* cx, JS::HandleValue v, JS::MutableHandleValue vp,
    const JSStructuredCloneCallbacks* optionalCallbacks, void* closure) {
  return JS_StructuredClone(cx->cx_, v, vp, optionalCallbacks, closure);
}

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
  void* sbx_bytes = js_malloc(len);
  if (!sbx_bytes) return false;

  bool ret = JS_ReadBytes(r.INTERNAL_unverified_safe(), sbx_bytes, len);
  if (!ret) {
    js_free(sbx_bytes);
    return false;
  }

  memcpy(p, sbx_bytes, len);
  js_free(sbx_bytes);
  return true;
}

inline bool JS_ReadString(MC::Tainted<JSStructuredCloneReader*> r,
                          JS::MutableHandleString str) {
  return JS_ReadString(r.INTERNAL_unverified_safe(), str);
}

inline bool JS_ReadDouble(MC::Tainted<JSStructuredCloneReader*> r, double* v) {
  MC::SandboxStack<double> sbx_v;
  bool ret = JS_ReadDouble(r.INTERNAL_unverified_safe(),
                           sbx_v.UNSAFE_unverified());
  *v = *sbx_v.UNSAFE_unverified();
  return ret;
}

inline bool JS_ReadTypedArray(MC::Tainted<JSStructuredCloneReader*> r,
                              JS::MutableHandleValue vp) {
  return JS_ReadTypedArray(r.INTERNAL_unverified_safe(), vp);
}

inline bool JS_WriteUint32Pair(MC::Tainted<JSStructuredCloneWriter*> w,
                               uint32_t tag, uint32_t data) {
  return JS_WriteUint32Pair(w.INTERNAL_unverified_safe(), tag, data);
}

inline bool JS_WriteBytes(MC::Tainted<JSStructuredCloneWriter*> w,
                          const void* p, size_t len) {
  return JS_WriteBytes(w.INTERNAL_unverified_safe(), p, len);
}

inline bool JS_WriteString(MC::Tainted<JSStructuredCloneWriter*> w,
                           JS::HandleString str) {
  return JS_WriteString(w.INTERNAL_unverified_safe(), str);
}

inline bool JS_WriteDouble(MC::Tainted<JSStructuredCloneWriter*> w, double v) {
  return JS_WriteDouble(w.INTERNAL_unverified_safe(), v);
}

inline bool JS_WriteTypedArray(MC::Tainted<JSStructuredCloneWriter*> w,
                               JS::HandleValue v) {
  return JS_WriteTypedArray(w.INTERNAL_unverified_safe(), v);
}

inline bool JS_ObjectNotWritten(MC::Tainted<JSStructuredCloneWriter*> w,
                                JS::HandleObject obj) {
  return JS_ObjectNotWritten(w.INTERNAL_unverified_safe(), obj);
}

#endif

#endif
