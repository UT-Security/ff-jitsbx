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
#include "monkeycage/SandboxCallback.h"

namespace JS {
  
inline void SetJobQueue(MCContext* cx, JobQueue* queue) {
  SetJobQueue(cx->cx_, queue);
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

#endif

#endif
