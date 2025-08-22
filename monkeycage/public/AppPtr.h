/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_AppPtr_h
#define mc_AppPtr_h

#include <functional>
#include <mozilla/AlreadyAddRefed.h>
#include <mozilla/Atomics.h>
#include <mozilla/RWLock.h>
#include <mozilla/Likely.h>
#include <mozilla/HashTable.h>
#include <mozilla/dom/TypeTags.h>

namespace MC {

typedef uint32_t TypeTag;

namespace detail {

class AppPtrInfo {
    public:
    explicit AppPtrInfo() {
        refCount = 1;
        tag = 0;
    }

    template<typename T>
    inline void setTag() {
        tag = mozilla::dom::TagVerify<T>::getTag();
    }

    inline void incRefCnt(void) {
        refCount++;
    }

    inline uint32_t getRefCnt(void) const {
        return refCount;
    }

    //decrements ref count and returns true if it hits zero
    inline bool decRefCnt(void) {
        --refCount;
        return (refCount == 0);
    }

    template<typename T>
    inline bool verify(void) {
        return mozilla::dom::TagVerify<T>::verify(tag);
    }

    private:
    uint32_t refCount;
    TypeTag tag;
};

using TaintTable = mozilla::HashMap<void*, AppPtrInfo>;
#ifdef MC_APP_PTR
extern TaintTable allExternalPtr;
extern mozilla::RWLock externalPtrLock;
#endif

}

template<typename T>
static void incRefCnt(T* native) {
    #ifdef MC_APP_PTR
    mozilla::AutoWriteLock wLock (detail::externalPtrLock);
    detail::TaintTable::Ptr p = detail::allExternalPtr.lookup(static_cast<void*>(native));
    if(p) {
        p->value().incRefCnt();
    } else {
        detail::AppPtrInfo newInfo;
        newInfo.setTag<T>();
        if(!detail::allExternalPtr.putNew(static_cast<void*>(native), newInfo)) {
            MOZ_CRASH("Failed to insert AppPointer in NativeWrapping");
        }
    }
    #endif
}

template<typename T>
static void incRefCnt(T& native) {
    return incRefCnt<T>(&native);
}

template<typename T>
static void incRefCnt(mozilla::UniquePtr<T>& native) {
    return incRefCnt<T>(native.get());
}

template <typename T>
static void decRefCnt(T* native) {
    #ifdef MC_APP_PTR
    mozilla::AutoWriteLock wLock (detail::externalPtrLock);
    detail::TaintTable::Ptr p = detail::allExternalPtr.lookup(static_cast<void*>(native));
    if(p) {
        if(p->value().decRefCnt()) {
            detail::allExternalPtr.removeNoResize(p);
        }
    } else {
        MOZ_CRASH("Attempted to decrement refcount of bad object");
    }
    #endif
}

template <typename T>
static bool verifyPtr(void * ptr) {
    #ifdef MC_APP_PTR
    mozilla::AutoReadLock rLock (detail::externalPtrLock);
    detail::TaintTable::Ptr p = detail::allExternalPtr.readonlyThreadsafeLookup(ptr);
    return p && p->value().verify<T>();
    #else
    return true;
    #endif
}

template <typename T>
static bool verifyPtr(T* ptr) {
    return verifyPtr<T>(static_cast<void*>(ptr));
}

template<typename T>
class ExposedObj {
	public:
	ExposedObj() {
        incRefCnt<T>(reinterpret_cast<T*>(this));
	}
	~ExposedObj() {
        decRefCnt<T>(reinterpret_cast<T*>(this));
	}
};

template<typename T>
class AppPtr {
  public:
  AppPtr(void * ptr) : app_ptr(ptr) {}
  
  //TODO: replace w/ macros that allow branching
  //see Tainted types under mfbt
  T* verify(std::function<bool(T*)> f) {
    if(MOZ_LIKELY(f(static_cast<T*>(app_ptr)))) {
        return static_cast<T*>(app_ptr);
    } else {
        return nullptr;
    }
  }

  T* verify_as_type(void) {
    if(MOZ_LIKELY(verifyPtr<T>(app_ptr))) {
        return static_cast<T*>(app_ptr);
    } else {
        MOZ_CRASH("Invalid app pointer as native type");
    }
  }

  T* UNVERIFIED_ref(void) {
    return static_cast<T*>(app_ptr);
  }

  operator bool() {
    return app_ptr != nullptr;
  }

  private:
  void * app_ptr;
};

}

#endif
