/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_sandbox_Promise_h
#define js_sandbox_Promise_h

#include "js/Promise.h"

#ifdef JS_SANDBOX

namespace js {
namespace sandbox {

JS_PUBLIC_API void Dispatchable_run(JS::Dispatchable* d, JSContext* cx,
                                    JS::Dispatchable::MaybeShuttingDown maybeShuttingDown);

}
}  // namespace js

namespace JS {
namespace sandbox {

/*typedef void (*SavedJobQueueDestructorOp)(void* p);

struct JS_PUBLIC_API SavedJobQueueOps {
  SavedJobQueueDestructorOp destructor;
};

class JS_PUBLIC_API SavedJobQueue : private JS::JobQueue::SavedJobQueue {
  
};
*/

class JS_PUBLIC_API JobQueue : public JS::JobQueue {
 public:
  class JS_PUBLIC_API SavedJobQueue;

  using DestructorOp = void (*)(void* p);
  using GetIncumbentGlobalOp = JSObject* (*)(void* p, JSContext* cx);
  using EnqueuePromiseJobOp = bool (*)(void* p, JSContext* cx,
                                       JS::HandleObject promise,
                                       JS::HandleObject job,
                                       JS::HandleObject allocationSite,
                                       JS::HandleObject incumbentGlobal);
  using RunJobsOp = void (*)(void* p, JSContext* cx);
  using EmptyOp = bool (*)(const void* p);
  using SaveJobQueueOp =
      js::UniquePtr<SavedJobQueue> (*)(void* p, JSContext*);

  struct JS_PUBLIC_API JobQueueOps {
    DestructorOp destructor;
    GetIncumbentGlobalOp getIncumbentGlobal;
    EnqueuePromiseJobOp enqueuePromiseJob;
    RunJobsOp runJobs;
    EmptyOp empty;
    SaveJobQueueOp saveJobQueue;
  };

private:
  const JobQueueOps* ops_;
  void* jobQueue_;

public:
  JobQueue(const JobQueueOps* ops, void* jobQueue);

  virtual ~JobQueue() override;
  virtual JSObject* getIncumbentGlobal(JSContext* cx) override;
  virtual bool enqueuePromiseJob(JSContext* cx, JS::HandleObject promise,
                                 JS::HandleObject job,
                                 JS::HandleObject allocationSite,
                                 JS::HandleObject incumbentGlobal) override;
  virtual void runJobs(JSContext* cx) override;
  virtual bool empty() const override;

private:
  virtual js::UniquePtr<JS::JobQueue::SavedJobQueue> saveJobQueue(JSContext*) override;
};

class JS_PUBLIC_API JobQueue::SavedJobQueue : public JS::JobQueue::SavedJobQueue {
public:
  using DestructorOp = void (*)(void* p);

private:
  DestructorOp op_;
  void* savedJobQueue_;

public:
  SavedJobQueue(DestructorOp op, void* savedJobQueue);
  virtual ~SavedJobQueue() override;  
};

} 
}
#endif

#endif
