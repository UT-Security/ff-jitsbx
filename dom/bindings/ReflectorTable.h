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
#include <stack>
#include "monkeycage/Tainted.h"

namespace MC {
namespace dom {

class ReflectorInfo {
  using TypeTag = uint32_t;
  
 public:
  explicit ReflectorInfo() : ptr_(nullptr), tag_(0) {}

  inline bool isEmpty(void) {
    return ptr_ == nullptr;
  }

  inline void setPtr(void* p) {
    ptr_ = p;
  }

  template<typename T>
  inline void setTag() {
    tag_ = mozilla::dom::TagVerify<T>::getTag();
  }

  inline void clear(void) {
    ptr_ = nullptr;
    tag_ = 0;
  }

  template <typename T>
  inline T* UNSAFE() {
    return static_cast<T*>(ptr_);
  }
  
  template <typename T>
  inline T* verify() {
    if(mozilla::dom::TagVerify<T>::verify(tag_)) {
      return static_cast<T*>(ptr_);
    } else {
      return nullptr;
    }
  }

 private:
  void* ptr_;
  TypeTag tag_;
};

using Address = uintptr_t;

class ReflectorTable {
  using ReflectorMap = ReflectorInfo*;

  static inline ReflectorMap table = new ReflectorInfo[2048]; 
  static inline std::stack<uintptr_t> free_list {};
  static inline size_t capacity = 2048;
  static inline size_t next_free = 0;
  static inline mozilla::RWLock tableLock = mozilla::RWLock("Reflector Table Lock");

  static void growTable(void) {
    auto oldCapacity = capacity;
    capacity *= 2;
    auto newTable = new ReflectorInfo[capacity];
    memcpy(newTable, table, oldCapacity * sizeof(ReflectorInfo));
    delete[] table;
    table = newTable;
    next_free = oldCapacity;
  }

  static uintptr_t getNextFree(void) {
    if(next_free != capacity && table[next_free].isEmpty()) {
      next_free++;
      return next_free - 1;
    }
    if(!free_list.empty()) {
      auto ref = free_list.top();
      free_list.pop();
      return ref;
    } else {
      growTable();
      next_free++;
      return next_free - 1;
    }
  }

  template <typename T>
  static T* verify_no_lock(Address ptr) {
#ifdef JS_SANDBOX_DOM_REFLECTORS
    if(ptr < capacity) {
      auto p = table[ptr].verify<T>();
      if(p) {
        return p;
      } else {
        MOZ_CRASH("Invalid DOM Reflector App Pointer");
      }
    } else {
      MOZ_CRASH("App Pointer Out of Bounds");
    }
#else
    MOZ_CRASH("Don't call verify");
#endif
  }

public:

  template <typename T>
  static T* get_UNSAFE_unverified(Address ptr) {
#ifdef JS_SANDBOX_DOM_REFLECTORS
    mozilla::AutoReadLock rLock(tableLock);
    if(ptr < capacity) {
      return table[ptr].UNSAFE<T>();
    } else {
      MOZ_CRASH("App Pointer Out of Bounds");
    }
#else
    MOZ_CRASH("Don't call verify");
#endif
  }


  template <typename T>
  static T* verify(Address ptr) {
#ifdef JS_SANDBOX_DOM_REFLECTORS
    mozilla::AutoReadLock rLock(tableLock);
    if(ptr < capacity) {
      auto p = table[ptr].verify<T>();
      if(p) {
        return p;
      } else {
        MOZ_CRASH("Invalid DOM Reflector App Pointer");
      }
    } else {
      MOZ_CRASH("App Pointer Out of Bounds");
    }
#else
    MOZ_CRASH("Don't call verify");
#endif
  }

  template<typename T>
  static Address initializeRef(T* native) {
#ifdef JS_SANDBOX_DOM_REFLECTORS
    mozilla::AutoWriteLock wLock(tableLock);
    auto ref = getNextFree();
    table[ref].setTag<T>();
    table[ref].setPtr(static_cast<void*>(native));
    return ref;
#else
    MOZ_CRASH("Initialize ref shouldn't be called");
#endif
  }

  template<typename T>
  static void deleteRef(uintptr_t native) {
#ifdef JS_SANDBOX_DOM_REFLECTORS
    mozilla::AutoWriteLock wLock(tableLock);
    verify_no_lock<T>(native);
    if(native < capacity) {
      table[native].clear();
      free_list.push(native);
    } else {
      MOZ_CRASH("Out of bounds ref");
    }
#endif
  }

  template<typename T>
  static void deleteRef(T* native_) {
    deleteRef<T>((uintptr_t)(native_) & 0xfffffffffff);
  }

  template<typename T>
  static void deleteRef(void* native_) {
    deleteRef<T>((uintptr_t)(native_) & 0xfffffffffff);
  }

};

}
}

#endif
