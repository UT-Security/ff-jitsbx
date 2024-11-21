/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef sandbox_Memory_h
#define sandbox_Memory_h

#include <stddef.h>
#include <stdint.h>

#include "sandbox/DoublyLinkedList.h"
#include "sandbox/RBTree.h"

#if defined(XP_WIN)
#  include <windows.h>
#elif defined(XP_DARWIN)
#  include "mozilla/Assertions.h"
#  include <os/lock.h>
#else
#  include <pthread.h>
#endif
#include "mozilla/Attributes.h"
#include "mozilla/ThreadSafety.h"

namespace js {
  
namespace sandbox {

static constexpr uint64_t MemoryMask = ((UINT64_C(1) << 40) - 1);
static constexpr uint32_t MemoryBaseShift = (64 - 40);
static constexpr uint32_t MemoryOffsetShift = 40;
static constexpr size_t MemorySize = UINT64_C(1) << 40;

struct MemoryRegion;

// Mutexes based on spinlocks.  We can't use normal pthread spinlocks in all
// places, because they require malloc()ed memory, which causes bootstrapping
// issues in some cases.  We also can't use constructors, because for statics,
// they would fire after the first use of malloc, resetting the locks.
struct MOZ_CAPABILITY("mutex") Mutex {
#if defined(XP_WIN)
  CRITICAL_SECTION mMutex;
#elif defined(XP_DARWIN)
  os_unfair_lock mMutex;
#else
  pthread_mutex_t mMutex;
#endif

  // Initializes a mutex. Returns whether initialization succeeded.
  inline bool Init() {
#if defined(XP_WIN)
    if (!InitializeCriticalSectionAndSpinCount(&mMutex, 5000)) {
      return false;
    }
#elif defined(XP_DARWIN)
    mMutex = OS_UNFAIR_LOCK_INIT;
#elif defined(XP_LINUX) && !defined(ANDROID)
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) {
      return false;
    }
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
    if (pthread_mutex_init(&mMutex, &attr) != 0) {
      pthread_mutexattr_destroy(&attr);
      return false;
    }
    pthread_mutexattr_destroy(&attr);
#else
    if (pthread_mutex_init(&mMutex, nullptr) != 0) {
      return false;
    }
#endif
    return true;
  }

  inline void Lock() MOZ_CAPABILITY_ACQUIRE() {
#if defined(XP_WIN)
    EnterCriticalSection(&mMutex);
#elif defined(XP_DARWIN)
    // We rely on a non-public function to improve performance here.
    // The OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION flag informs the kernel that
    // the calling thread is able to make progress even in absence of actions
    // from other threads and the OS_UNFAIR_LOCK_ADAPTIVE_SPIN one causes the
    // kernel to spin on a contested lock if the owning thread is running on
    // the same physical core (presumably only on x86 CPUs given that ARM
    // macs don't have cores capable of SMT). On versions of macOS older than
    // 10.15 the latter is not available and we spin in userspace instead.
    if (Mutex::gSpinInKernelSpace) {
      os_unfair_lock_lock_with_options(
          &mMutex,
          OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION | OS_UNFAIR_LOCK_ADAPTIVE_SPIN);
    } else {
#  if defined(__x86_64__)
      // On older versions of macOS (10.14 and older) the
      // `OS_UNFAIR_LOCK_ADAPTIVE_SPIN` flag is not supported by the kernel,
      // we spin in user-space instead like `OSSpinLock` does:
      // https://github.com/apple/darwin-libplatform/blob/215b09856ab5765b7462a91be7076183076600df/src/os/lock.c#L183-L198
      // Note that `OSSpinLock` uses 1000 iterations on x86-64:
      // https://github.com/apple/darwin-libplatform/blob/215b09856ab5765b7462a91be7076183076600df/src/os/lock.c#L93
      // ...but we only use 100 like it does on ARM:
      // https://github.com/apple/darwin-libplatform/blob/215b09856ab5765b7462a91be7076183076600df/src/os/lock.c#L90
      // We choose this value because it yields the same results in our
      // benchmarks but is less likely to have detrimental effects caused by
      // excessive spinning.
      uint32_t retries = 100;

      do {
        if (os_unfair_lock_trylock(&mMutex)) {
          return;
        }

        __asm__ __volatile__("pause");
      } while (retries--);

      os_unfair_lock_lock_with_options(&mMutex,
                                       OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
#  else
      MOZ_CRASH("User-space spin-locks should never be used on ARM");
#  endif  // defined(__x86_64__)
    }
#else
    pthread_mutex_lock(&mMutex);
#endif
  }

  inline void Unlock() MOZ_CAPABILITY_RELEASE() {
#if defined(XP_WIN)
    LeaveCriticalSection(&mMutex);
#elif defined(XP_DARWIN)
    os_unfair_lock_unlock(&mMutex);
#else
    pthread_mutex_unlock(&mMutex);
#endif
  }
};

// Mutex that can be used for static initialization.
// On Windows, CRITICAL_SECTION requires a function call to be initialized,
// but for the initialization lock, a static initializer calling the
// function would be called too late. We need no-function-call
// initialization, which SRWLock provides.
// Ideally, we'd use the same type of locks everywhere, but SRWLocks
// everywhere incur a performance penalty. See bug 1418389.
#if defined(XP_WIN)
struct MOZ_CAPABILITY("mutex") StaticMutex {
  SRWLOCK mMutex;

  inline void Lock() MOZ_CAPABILITY_ACQUIRE() {
    AcquireSRWLockExclusive(&mMutex);
  }

  inline void Unlock() MOZ_CAPABILITY_RELEASE() {
    ReleaseSRWLockExclusive(&mMutex);
  }
};

// Normally, we'd use a constexpr constructor, but MSVC likes to create
// static initializers anyways.
#  define STATIC_MUTEX_INIT SRWLOCK_INIT

#else
typedef Mutex StaticMutex;

#  if defined(XP_DARWIN)
#    define STATIC_MUTEX_INIT OS_UNFAIR_LOCK_INIT
#  elif defined(XP_LINUX) && !defined(ANDROID)
#    define STATIC_MUTEX_INIT PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
#  else
#    define STATIC_MUTEX_INIT PTHREAD_MUTEX_INITIALIZER
#  endif

#endif

template <typename T>
struct MOZ_SCOPED_CAPABILITY MOZ_RAII AutoLock {
  explicit AutoLock(T& aMutex) MOZ_CAPABILITY_ACQUIRE(aMutex) : mMutex(aMutex) {
    mMutex.Lock();
  }

  ~AutoLock() MOZ_CAPABILITY_RELEASE() { mMutex.Unlock(); }

  AutoLock(const AutoLock&) = delete;
  AutoLock(AutoLock&&) = delete;

 private:
  T& mMutex;
};

using MutexAutoLock = AutoLock<Mutex>;

class Memory {
private:
  Mutex mLock MOZ_UNANNOTATED;
 
  RBTree<MemoryRegion, MemoryRegion> region_tree;

  DoublyLinkedList<MemoryRegion> region_list;

  uintptr_t base_addr;

  size_t length;

  // Unit of Region allocation and alignment.
  size_t page_size;

  // Highest Memory Region end address.
  uintptr_t highest_region_end;


  uintptr_t findUnmappedRegion(size_t len, size_t align);
  uintptr_t findUnmappedEndRegion(size_t len);
  
  int findRegionLinks(uintptr_t start_addr, uintptr_t end_addr,
                      MemoryRegion** pprev, MemoryRegion*** link,
                      MemoryRegion** parent);

  MemoryRegion* findRegion(uintptr_t addr);
  MemoryRegion* findRegionAndPrev(uintptr_t addr, MemoryRegion** pprev);
  MemoryRegion* findIntersectingRegion(uintptr_t start_addr,
                                       uintptr_t end_addr);

  MemoryRegion* lookupRegion(uintptr_t addr);
  MemoryRegion* nextRegion(MemoryRegion* r);

  MemoryRegion* maybeMergeRegions(MemoryRegion* prev, uintptr_t start_addr,
                                 uintptr_t end_addr);
  void adjustRegion(MemoryRegion* r, uintptr_t start_addr, uintptr_t end_addr);

  void linkRegion(MemoryRegion *r, MemoryRegion *prev, MemoryRegion **link, MemoryRegion *parent);
  void linkRegionList(MemoryRegion *r, MemoryRegion *prev);
  void linkRegionTree(MemoryRegion *r, MemoryRegion **link, MemoryRegion *parent);
  void unlinkRegion(MemoryRegion *r, MemoryRegion *ignore);
  void unlinkRegionList(MemoryRegion *r);

 public:
  Memory() : base_addr((uintptr_t)nullptr), length(0), page_size(0) {}

  bool initialized() { return base_addr != (uintptr_t)nullptr; }

  bool init(void* addr, size_t length, size_t pageSize);

  uintptr_t base() { return (uintptr_t)base_addr; }
  void* allocateProtected_(size_t length, size_t alignment);
  void* allocateProtected(size_t length, size_t alignment);
  void* allocate(size_t length, size_t alignment);
  void deallocateProtected_(void* addr, size_t length);
  void deallocateProtected(void* addr, size_t length);
  void deallocate(void* addr, size_t length);
};

bool InitMemory();

uintptr_t MemoryBase();
bool IsValidAddress(size_t address);
void* AllocateProtectedMemory(size_t length, size_t alignment);
void* AllocateMemory(size_t length, size_t alignment);
void DeallocateProtectedMemory(void* addr, size_t length);
void  DeallocateMemory(void* addr, size_t length);

} /* namespace sandbox */
  
} /* namespace js */


#endif /* sandbox_MemoryRegion_h */
