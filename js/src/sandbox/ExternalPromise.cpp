/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/sandbox/Promise.h"

using namespace JS::sandbox;

JS_PUBLIC_API void js::sandbox::Dispatchable_run(
    JS::Dispatchable* d, JSContext* cx,
    JS::Dispatchable::MaybeShuttingDown maybeShuttingDown) {
  d->run(cx, maybeShuttingDown);
}

JobQueue::JobQueue(const JobQueueOps* ops, void* jobQueue) : ops_(ops), jobQueue_(jobQueue) {}

JobQueue::~JobQueue() {
  ops_->destructor(jobQueue_);
}

JSObject* JobQueue::getIncumbentGlobal(JSContext* cx) {
  return ops_->getIncumbentGlobal(jobQueue_, cx);
}

bool JobQueue::enqueuePromiseJob(JSContext* cx, JS::HandleObject promise,
                               JS::HandleObject job,
                               JS::HandleObject allocationSite,
                               JS::HandleObject incumbentGlobal) {
  return ops_->enqueuePromiseJob(jobQueue_, cx, promise, job, allocationSite, incumbentGlobal);
}

void JobQueue::runJobs(JSContext* cx) {
  return ops_->runJobs(jobQueue_, cx);  
}

bool JobQueue::empty() const {
  return ops_->empty(jobQueue_);
}

js::UniquePtr<JS::JobQueue::SavedJobQueue> JobQueue::saveJobQueue(JSContext* cx) {
  return ops_->saveJobQueue(jobQueue_, cx);
}

JobQueue::SavedJobQueue::SavedJobQueue(DestructorOp op, void* savedJobQueue)
    : op_(op), savedJobQueue_(savedJobQueue) {}

JobQueue::SavedJobQueue::~SavedJobQueue() {
  op_(savedJobQueue_);
}
