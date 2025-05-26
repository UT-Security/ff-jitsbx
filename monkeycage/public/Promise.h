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

class JobQueue {
 public:
  JS::sandbox::JobQueue* inner_;
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
    return std::move(jobQueue->saveJobQueue(mcx)->inner_);
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
  virtual ~JobQueue() = default;
  virtual JSObject* getIncumbentGlobal(MCContext* cx) = 0;
  virtual bool enqueuePromiseJob(MCContext* cx, JS::HandleObject promise,
                                 JS::HandleObject job,
                                 JS::HandleObject allocationSite,
                                 JS::HandleObject incumbentGlobal) = 0;
  virtual void runJobs(MCContext* cx) = 0;
  virtual bool empty() const = 0;

  JobQueue() {
    inner_ = js_new<JS::sandbox::JobQueue>(ops(), this);
  }

 protected:
  class SavedJobQueue {
    static void destructorCb(void* p) {
      auto savedJobQueue = static_cast<SavedJobQueue*>(p);
      delete savedJobQueue;
    }

   public:
    js::UniquePtr<JS::sandbox::JobQueue::SavedJobQueue> inner_;

    SavedJobQueue()
        : inner_(js::MakeUnique<JS::sandbox::JobQueue::SavedJobQueue>(
              Sandbox::RegisterCallback(destructorCb).UNSAFE_get(), this)) {}

    virtual ~SavedJobQueue() = default;
  };
  virtual js::UniquePtr<SavedJobQueue> saveJobQueue(MCContext* cx) = 0;
};
}

namespace JS {
  
inline void SetJobQueue(MCContext* cx, MC::JobQueue* queue) {
  SetJobQueue(cx->cx_, queue->inner_);
}

inline void JobQueueIsEmpty(MCContext* cx) {
  return JobQueueIsEmpty(cx->cx_);
}

inline void JobQueueMayNotBeEmpty(MCContext* cx) {
  return JobQueueMayNotBeEmpty(cx->cx_);
}

inline void SetPromiseRejectionTrackerCallback(
    MCContext* cx, MC::SandboxCallback<PromiseRejectionTrackerCallback> callback,
    void* data = nullptr) {
  return SetPromiseRejectionTrackerCallback(cx->cx_, callback.UNSAFE_get(), data);
}

inline void InitDispatchToEventLoop(
    MCContext* cx, MC::Sandbox::Callback<DispatchToEventLoopCallback> callback,
    void* closure) {
  return InitDispatchToEventLoop(cx->cx_, callback.UNSAFE_get(), closure);
}

inline void ShutdownAsyncTasks(MCContext* cx) {
  return ShutdownAsyncTasks(cx->cx_);
}
}

#else

namespace MC {

using JobQueue = JS::JobQueue;

}

#endif

#endif
