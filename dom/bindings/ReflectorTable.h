/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_ReflectorTable_h
#define mozilla_dom_ReflectorTable_h

#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/Atomics.h"
#include "mozilla/RWLock.h"
#include "mozilla/Likely.h"
#include "mozilla/HashTable.h"
#include "mozilla/dom/TypeTags.h"
#include "monkeycage/Tainted.h"

namespace MC {
namespace dom {

class ReflectorInfo {
  using TypeTag = uint32_t;
  
 public:
  explicit ReflectorInfo() : refCount_(1) {}

  inline void addRef() { refCount_++; }

  inline bool decRef() {
    --refCount_;
    return refCount_ == 0;
  }

  inline uint32_t getRefCnt() const { return refCount_; }


  template<typename T>
  inline void setTag() {
    tag_ = mozilla::dom::TagVerify<T>::getTag();
  }

  template <typename T>
  inline bool verify() {
    return mozilla::dom::TagVerify<T>::verify(tag_);
  }

 private:
  uint32_t refCount_;
  TypeTag tag_;
};

class ReflectorTable {
  using ReflectorMap = mozilla::HashMap<void*, ReflectorInfo>;

  static inline ReflectorMap table = ReflectorMap(32);
  static inline mozilla::RWLock tableLock = mozilla::RWLock("Reflector Table Lock");

  template <typename T>
  static T* verify(void* ptr) {
#ifdef JS_SANDBOX_DOM_REFLECTORS
    mozilla::AutoReadLock rLock(tableLock);
    ReflectorMap::Ptr p = table.readonlyThreadsafeLookup(ptr);
    if (!p || !p->value().verify<T>()) MOZ_CRASH("Invalid DOM Reflector App Pointer");
    return static_cast<T*>(ptr);
#else
    return static_cast<T*>(ptr);
#endif
  }

public:
  template<typename T>
  static T* verify(MC::AppPointer<void*> ptr) {
    return verify<T>(ptr.UNSAFE_unverified());
  }

  template<typename T>
  static void addRef(T* native) {
#ifdef JS_SANDBOX_DOM_REFLECTORS
    mozilla::AutoWriteLock wLock(tableLock);
    ReflectorMap::AddPtr p = table.lookupForAdd(static_cast<void*>(native));
    if (p) p->value().addRef();
    else {
      ReflectorInfo newInfo;
      newInfo.setTag<T>();
      if(!table.add(p, static_cast<void*>(native), newInfo)) MOZ_CRASH("Failed to insert DOM Reflector App Pointer");
    }
#endif
  }

  template<typename T>
  static void decRef(T* native) {
#ifdef JS_SANDBOX_DOM_REFLECTORS
    mozilla::AutoWriteLock wLock(tableLock);
    ReflectorMap::Ptr p = table.lookup(static_cast<void*>(native));
    if (p && p->value().decRef()) {
      table.removeNoResize(p);
    } else {
      MOZ_CRASH("Attempted invalid refcount decrement");
    }
#endif
  }
};

}
}

#endif
