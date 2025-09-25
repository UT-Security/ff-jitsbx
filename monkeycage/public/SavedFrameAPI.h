/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Functions and types related to SavedFrame objects created by the Debugger
 * API.
 */

#ifndef mc_SavedFrameAPI_h
#define mc_SavedFrameAPI_h

#include "js/SavedFrameAPI.h"

#ifdef JS_SANDBOX

#include "monkeycage/Context.h"
#include "monkeycage/Tainted.h"

namespace JS {

inline SavedFrameResult GetSavedFrameSource(
    MCContext* cx, JSPrincipals* principals, Handle<JSObject*> savedFrame,
    MutableHandle<JSString*> sourcep,
    SavedFrameSelfHosted selfHosted = SavedFrameSelfHosted::Include) {
  return GetSavedFrameSource(cx->cx_, principals, savedFrame, sourcep,
                             selfHosted);
}

inline SavedFrameResult GetSavedFrameSourceId(
    MCContext* cx, JSPrincipals* principals, Handle<JSObject*> savedFrame,
    MC::Tainted<uint32_t*> sourceIdp,
    SavedFrameSelfHosted selfHosted = SavedFrameSelfHosted::Include) {
  return GetSavedFrameSourceId(cx->cx_, principals, savedFrame,
                               sourceIdp.INTERNAL_unverified_safe(),
                               selfHosted);
}

inline SavedFrameResult GetSavedFrameLine(
    MCContext* cx, JSPrincipals* principals, Handle<JSObject*> savedFrame,
    MC::Tainted<uint32_t*> linep,
    SavedFrameSelfHosted selfHosted = SavedFrameSelfHosted::Include) {
  return GetSavedFrameLine(cx->cx_, principals, savedFrame,
                           linep.INTERNAL_unverified_safe(), selfHosted);
}

inline SavedFrameResult GetSavedFrameColumn(
    MCContext* cx, JSPrincipals* principals, Handle<JSObject*> savedFrame,
    MC::Tainted<uint32_t*> columnp,
    SavedFrameSelfHosted selfHosted = SavedFrameSelfHosted::Include) {
  return GetSavedFrameColumn(cx->cx_, principals, savedFrame,
                             columnp.INTERNAL_unverified_safe(), selfHosted);
}

inline SavedFrameResult GetSavedFrameFunctionDisplayName(
    MCContext* cx, JSPrincipals* principals, Handle<JSObject*> savedFrame,
    MutableHandle<JSString*> namep,
    SavedFrameSelfHosted selfHosted = SavedFrameSelfHosted::Include) {
  return GetSavedFrameFunctionDisplayName(cx->cx_, principals, savedFrame,
                                          namep, selfHosted);
}

inline SavedFrameResult GetSavedFrameAsyncCause(
    MCContext* cx, JSPrincipals* principals, Handle<JSObject*> savedFrame,
    MutableHandle<JSString*> asyncCausep,
    SavedFrameSelfHosted selfHosted = SavedFrameSelfHosted::Include) {
  return GetSavedFrameAsyncCause(cx->cx_, principals, savedFrame, asyncCausep,
                                 selfHosted);
}

inline SavedFrameResult GetSavedFrameAsyncParent(
    MCContext* cx, JSPrincipals* principals, Handle<JSObject*> savedFrame,
    MutableHandle<JSObject*> asyncParentp,
    SavedFrameSelfHosted selfHosted = SavedFrameSelfHosted::Include) {
  return GetSavedFrameAsyncParent(cx->cx_, principals, savedFrame, asyncParentp,
                                  selfHosted);
}

inline SavedFrameResult GetSavedFrameParent(
    MCContext* cx, JSPrincipals* principals, Handle<JSObject*> savedFrame,
    MutableHandle<JSObject*> parentp,
    SavedFrameSelfHosted selfHosted = SavedFrameSelfHosted::Include) {
  return GetSavedFrameParent(cx->cx_, principals, savedFrame, parentp,
                             selfHosted);
}

inline JSObject* ConvertSavedFrameToPlainObject(
    MCContext* cx, JS::HandleObject savedFrame,
    JS::SavedFrameSelfHosted selfHosted) {
  return ConvertSavedFrameToPlainObject(cx->cx_, savedFrame, selfHosted);
}
}  // namespace JS

namespace js {

inline JSObject* GetFirstSubsumedSavedFrame(
    MCContext* cx, JSPrincipals* principals, JS::Handle<JSObject*> savedFrame,
    JS::SavedFrameSelfHosted selfHosted) {
  return GetFirstSubsumedSavedFrame(cx->cx_, principals, savedFrame,
                                    selfHosted);
}

}  // namespace js

#endif

#endif
