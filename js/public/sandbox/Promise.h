/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_Promise_h
#define js_sandbox_Promise_h

#include "js/Promise.h"
#ifdef JS_SANDBOX_API
#include "js/sandbox/lib.h"
#endif

namespace JS {
namespace sandbox {
#ifdef JS_SANDBOX_API
class JobQueue {
private:
  JS::JobQueueWithOps* base_;

public:
  virtual ~JobQueue() {
    js_free(base_);
  }
  
  static void destructorCb(void* p) {
    // TODO  
  }

  virtual JSObject* getIncumbentGlobal(JSContext* cx) = 0;

  static JSObject* getIncumbentGlobalCb(void* p, JSContext* cx) {
    auto* q = static_cast<JobQueue*>(p);
    return q->getIncumbentGlobal(cx);
  }

  virtual bool enqueuePromiseJob(JSContext* cx, JS::HandleObject promise,
                                 JS::HandleObject job,
                                 JS::HandleObject allocationSite,
                                 JS::HandleObject incumbentGlobal) = 0;

  static bool enqueuePromiseJobCb(void* p, JSContext* cx,
                                JS::HandleObject promise, JS::HandleObject job,
                                JS::HandleObject allocationSite,
                                JS::HandleObject incumbentGlobal) {
    auto* q = static_cast<JobQueue*>(p);
    return q->enqueuePromiseJob(cx, promise, job, allocationSite, incumbentGlobal);
  }

  virtual void runJobs(JSContext* cx) = 0;

  static void runJobsCb(void* p, JSContext* cx) {
    auto* q = static_cast<JobQueue*>(p);
    q->runJobs(cx);
  }

  virtual bool empty() const = 0;
  
  static bool emptyCb(void* p) {
    auto* q = static_cast<const JobQueue*>(p);
    return q->empty();
  }
  
protected:
  using SavedJobQueue = JS::JobQueue::SavedJobQueue;
  
  virtual js::UniquePtr<SavedJobQueue> saveJobQueue(JSContext* cx) = 0;

  static js::UniquePtr<SavedJobQueue> saveCb(void* p, JSContext* cx) {
    auto* q = static_cast<JobQueue*>(p);
    return q->saveJobQueue(cx);
  }
public:
  const JS::JobQueueWithOps::Ops* ops() {
    static const JS::JobQueueWithOps::Ops __ops = {
      (JS::JobQueueWithOps::DestructorOp)sbx_register_cb((void*)JobQueue::destructorCb, 0),
      (JS::JobQueueWithOps::GetIncumbentGlobalOp)sbx_register_cb((void*)JobQueue::getIncumbentGlobalCb, 0),
      (JS::JobQueueWithOps::EnqueuePromiseJobOp)sbx_register_cb((void*)JobQueue::enqueuePromiseJobCb, 0),
      (JS::JobQueueWithOps::RunJobsOp)sbx_register_cb((void*)JobQueue::runJobsCb, 0),
      (JS::JobQueueWithOps::EmptyOp)sbx_register_cb((void*)JobQueue::emptyCb, 0),
      (JS::JobQueueWithOps::SaveOp)sbx_register_cb((void*)JobQueue::saveCb, 0),
    };

    return &__ops;
  }

  JobQueue() {
    base_ = js_new<JS::JobQueueWithOps>(ops(), this);
  }

  inline JS::JobQueue* getBase() { return base_; }
};

inline JS::JobQueue* GetJobQueue(JobQueue* jq) {
  return jq->getBase();
}
#else
using JobQueue = JS::JobQueue; 

inline JS::JobQueue* GetJobQueue(JobQueue* jq) {
  return jq;
}
#endif
}
}


#endif
