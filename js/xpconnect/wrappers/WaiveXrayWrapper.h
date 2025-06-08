/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __CrossOriginWrapper_h__
#define __CrossOriginWrapper_h__

#include "mozilla/Attributes.h"
#include "mozilla/Maybe.h"

#include "monkeycage/Wrapper.h"

namespace xpc {

class WaiveXrayWrapper : public mc::CrossCompartmentWrapper {
 public:
#ifdef JS_SANDBOX
  explicit inline WaiveXrayWrapper(unsigned flags)
      : mc::CrossCompartmentWrapper(flags) {}
#else
  explicit constexpr WaiveXrayWrapper(unsigned flags)
      : mc::CrossCompartmentWrapper(flags) {}
#endif

  virtual bool getOwnPropertyDescriptor(
      MCContext* cx, JS::Handle<JSObject*> wrapper, JS::Handle<jsid> id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
      const override;
  virtual bool getPrototype(MCContext* cx, JS::Handle<JSObject*> wrapper,
                            JS::MutableHandle<JSObject*> protop) const override;
  virtual bool getPrototypeIfOrdinary(
      MCContext* cx, JS::Handle<JSObject*> wrapper, MC::Tainted<bool*> isOrdinary,
      JS::MutableHandle<JSObject*> protop) const override;
  virtual bool get(MCContext* cx, JS::Handle<JSObject*> wrapper,
                   JS::Handle<JS::Value> receiver, JS::Handle<jsid> id,
                   JS::MutableHandle<JS::Value> vp) const override;
  virtual bool call(MCContext* cx, JS::Handle<JSObject*> wrapper,
                    const JS::CallArgs& args) const override;
  virtual bool construct(MCContext* cx, JS::Handle<JSObject*> wrapper,
                         const JS::CallArgs& args) const override;

  virtual bool nativeCall(MCContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl,
                          const JS::CallArgs& args) const override;

  static const WaiveXrayWrapper* getSingleton();
};

}  // namespace xpc

#endif
