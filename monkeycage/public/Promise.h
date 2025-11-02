/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Promise_h
#define mc_Promise_h

#include "js/Promise.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Sandbox.h"
#include "js/sandbox/Promise.h"
#include "js/Utility.h"

namespace MC {
namespace detail {

template <typename MC_Sbx>
class Tainted<JS::Dispatchable*, MC_Sbx> {
  private:
    JS::Dispatchable* data;

  inline JS::Dispatchable* get_raw_value() const noexcept {
    return data;
  }
 public:
  inline auto& UNSAFE_unverified() const { return get_raw_value(); }
  inline auto& INTERNAL_unverified_safe() const { return UNSAFE_unverified(); }

  inline auto& UNSAFE_unverified() { return get_raw_value(); }
  inline auto& INTERNAL_unverified_safe() { return UNSAFE_unverified(); }

  Tainted() = default;
  Tainted(const Tainted<JS::Dispatchable*, MC_Sbx>& p) = default;

  Tainted(const std::nullptr_t& arg) : data(arg) {
  }

  Tainted<JS::Dispatchable*, MC_Sbx>& operator=(const std::nullptr_t& arg) { data = arg; return *this; }
  
  template<typename T_Rhs>
  void assign_raw_pointer(T_Rhs val) {
    static_assert(std::is_pointer_v<T_Rhs>, "Must be a pointer");
    static_assert(std::is_assignable_v<JS::Dispatchable*&, T_Rhs>,
                  "Should assign pointers of compatible types.");
    //TODO(abhishek): check that `val` is a pointer within the sandbox.
    data = val;
  }

  operator bool() const { return get_raw_value() != nullptr; }

  void run(MCContext* cx, JS::Dispatchable::MaybeShuttingDown maybeShuttingDown) const {
    js::sandbox::Dispatchable_run(get_raw_value(), cx->cx_, maybeShuttingDown);
  }
  
};
}
}


namespace MC {

class JobQueue {
 public:
  JS::sandbox::JobQueue* inner_;

  virtual ~JobQueue() = default;
  virtual JSObject* getIncumbentGlobal(MCContext* cx) = 0;
  virtual bool enqueuePromiseJob(MCContext* cx, JS::HandleObject promise,
                                 JS::HandleObject job,
                                 JS::HandleObject allocationSite,
                                 JS::HandleObject incumbentGlobal) = 0;
  virtual void runJobs(MCContext* cx) = 0;
  virtual bool empty() const = 0;

 protected:
  class SavedJobQueue {
    static void destructorCb(void* p) {
      auto savedJobQueue = static_cast<SavedJobQueue*>(p);
      delete savedJobQueue;
    }

   public:
    js::UniquePtr<JS::sandbox::JobQueue::SavedJobQueue> inner_;

    static MC::SandboxCallback<JS::sandbox::JobQueue::SavedJobQueue::DestructorOp> destructor() {
      static auto cb = Sandbox::RegisterCallback(destructorCb);
      return cb;
    }

    SavedJobQueue()
        : inner_(js::MakeUnique<JS::sandbox::JobQueue::SavedJobQueue>(
              destructor().UNSAFE_get(), this)) {}

    virtual ~SavedJobQueue() = default;
  };

  virtual mozilla::UniquePtr<SavedJobQueue> saveJobQueue(MCContext* cx) = 0;

 private:
  static void destructorCb(void* p) {
    auto jobQueue = static_cast<JobQueue*>(p);
    delete jobQueue;
  }

  static JSObject* getIncumbentGlobalCb(void* p, JSContext* cx) {
    auto jobQueue = static_cast<JobQueue*>(p);
    MCContext* mcx = JS_SanitizeContext(cx);
    return jobQueue->getIncumbentGlobal(mcx);
  }

  static bool enqueuePromiseJobCb(void* p, JSContext* cx, JS::HandleObject promise,
                                  JS::HandleObject job,
                                  JS::HandleObject allocationSite,
                                  JS::HandleObject incumbentGlobal) {    
    auto jobQueue = static_cast<JobQueue*>(p);
    MCContext* mcx = JS_SanitizeContext(cx);
    return jobQueue->enqueuePromiseJob(mcx, promise, job, allocationSite, incumbentGlobal);
  }

  static void runJobsCb(void* p, JSContext* cx) {
    auto jobQueue = static_cast<JobQueue*>(p);
    MCContext* mcx = JS_SanitizeContext(cx);
    return jobQueue->runJobs(mcx);
  }
  
  static bool emptyCb(const void* p) {
    auto jobQueue = static_cast<const JobQueue*>(p);
    return jobQueue->empty();
  }
  
  static js::UniquePtr<JS::sandbox::JobQueue::SavedJobQueue> saveJobQueueCb(void* p, JSContext* cx) {
    auto jobQueue = static_cast<JobQueue*>(p);
    MCContext* mcx = JS_SanitizeContext(cx);
    // Ownership over the outer SavedJobQueue* was given to inner_
    // at the time of construction. We just forget it here.
    SavedJobQueue* savedJobQueue = jobQueue->saveJobQueue(mcx).release();
    return std::move(savedJobQueue->inner_);
  }

  static const JS::sandbox::JobQueue::JobQueueOps* ops() {
    static auto inner_ = JS::sandbox::JobQueue::JobQueueOps {
        Sandbox::RegisterCallback(destructorCb).UNSAFE_get(),
        Sandbox::RegisterCallback(getIncumbentGlobalCb).UNSAFE_get(),
        Sandbox::RegisterCallback(enqueuePromiseJobCb).UNSAFE_get(),
        Sandbox::RegisterCallback(runJobsCb).UNSAFE_get(),
        Sandbox::RegisterCallback(emptyCb).UNSAFE_get(),
        Sandbox::RegisterCallback(saveJobQueueCb).UNSAFE_get(),
    };

    return &inner_;
  }
public:
  JobQueue() {
    inner_ = js_new<JS::sandbox::JobQueue>(ops(), this);
  }
};
}

namespace JS {

inline void SetJobQueue(MCContext* cx, MC::JobQueue* queue) {
  SetJobQueue(cx->cx_, queue->inner_);
}

inline void JobQueueIsEmpty(MCContext* cx) { return JobQueueIsEmpty(cx->cx_); }

inline void JobQueueMayNotBeEmpty(MCContext* cx) {
  return JobQueueMayNotBeEmpty(cx->cx_);
}

inline void SetPromiseRejectionTrackerCallback(
    MCContext* cx,
    MC::SandboxCallback<PromiseRejectionTrackerCallback> callback,
    void* data = nullptr) {
  return SetPromiseRejectionTrackerCallback(cx->cx_, callback.UNSAFE_get(),
                                            data);
}

inline JSObject* NewPromiseObject(MCContext* cx, JS::HandleObject executor) {
  return NewPromiseObject(cx->cx_, executor);
}

inline JSObject* GetPromiseConstructor(MCContext* cx) {
  return GetPromiseConstructor(cx->cx_);
}

inline JSObject* GetPromisePrototype(MCContext* cx) {
  return GetPromisePrototype(cx->cx_);
}

inline bool SetSettledPromiseIsHandled(MCContext* cx,
                                       JS::HandleObject promise) {
  return SetSettledPromiseIsHandled(cx->cx_, promise);
}

inline bool SetAnyPromiseIsHandled(MCContext* cx, JS::HandleObject promise) {
  return SetAnyPromiseIsHandled(cx->cx_, promise);
}

#ifdef DEBUG
inline void DumpPromiseAllocationSite(MCContext* cx, JS::HandleObject promise) {
  return DumpPromiseAllocationSite(cx->cx_, promise);
}

inline void DumpPromiseResolutionSite(MCContext* cx, JS::HandleObject promise) {
  return DumpPromiseResolutionSite(cx->cx_, promise);
}
#endif

inline JSObject* CallOriginalPromiseResolve(MCContext* cx,
                                            JS::HandleValue resolutionValue) {
  return CallOriginalPromiseResolve(cx->cx_, resolutionValue);
}

inline JSObject* CallOriginalPromiseReject(MCContext* cx,
                                           JS::HandleValue rejectionValue) {
  return CallOriginalPromiseReject(cx->cx_, rejectionValue);
}

inline bool ResolvePromise(MCContext* cx, JS::HandleObject promiseObj,
                           JS::HandleValue resolutionValue) {
  return ResolvePromise(cx->cx_, promiseObj, resolutionValue);
}

inline bool RejectPromise(MCContext* cx, JS::HandleObject promiseObj,
                          JS::HandleValue rejectionValue) {
  return RejectPromise(cx->cx_, promiseObj, rejectionValue);
}

inline JSObject* CallOriginalPromiseThen(MCContext* cx,
                                         JS::HandleObject promise,
                                         JS::HandleObject onFulfilled,
                                         JS::HandleObject onRejected) {
  return CallOriginalPromiseThen(cx->cx_, promise, onFulfilled, onRejected);
}

inline bool AddPromiseReactions(MCContext* cx, JS::HandleObject promise,
                                JS::HandleObject onFulfilled,
                                JS::HandleObject onRejected) {
  return AddPromiseReactions(cx->cx_, promise, onFulfilled, onRejected);
}

inline bool AddPromiseReactionsIgnoringUnhandledRejection(
    MCContext* cx, JS::HandleObject promise, JS::HandleObject onFulfilled,
    JS::HandleObject onRejected) {
  return AddPromiseReactionsIgnoringUnhandledRejection(cx->cx_, promise,
                                                       onFulfilled, onRejected);
}

inline JSObject* GetWaitForAllPromise(MCContext* cx,
                                      JS::HandleObjectVector promises) {
  return GetWaitForAllPromise(cx->cx_, promises);
}

inline void InitDispatchToEventLoop(
    MCContext* cx, MC::Sandbox::Callback<DispatchToEventLoopCallback> callback,
    void* closure) {
  return InitDispatchToEventLoop(cx->cx_, callback.UNSAFE_get(), closure);
}

inline void ShutdownAsyncTasks(MCContext* cx) {
  return ShutdownAsyncTasks(cx->cx_);
}
}  // namespace JS

#else

namespace MC {

using JobQueue = JS::JobQueue;

}

#endif

#endif
