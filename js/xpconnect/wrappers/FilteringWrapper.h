/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __FilteringWrapper_h__
#define __FilteringWrapper_h__

#include "XrayWrapper.h"
#include "mozilla/Attributes.h"
#include "mozilla/Maybe.h"
#include "js/CallNonGenericMethod.h"
#include "monkeycage/Tainted.h"
#include "monkeycage/Wrapper.h"

namespace xpc {

template <typename Base, typename Policy>
class FilteringWrapper : public Base {
 public:
#ifdef JS_SANDBOX
  inline explicit FilteringWrapper(unsigned flags) : Base(flags) {}
#else
  constexpr explicit FilteringWrapper(unsigned flags) : Base(flags) {}
#endif

  virtual bool enter(MCContext* cx, JS::Handle<JSObject*> wrapper,
                     JS::Handle<jsid> id, js::Wrapper::Action act,
                     bool mayThrow, MC::Tainted<bool*> bp) const override;

  virtual bool getOwnPropertyDescriptor(
      MCContext* cx, JS::Handle<JSObject*> wrapper, JS::Handle<jsid> id,
      JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc)
      const override;
  virtual bool ownPropertyKeys(MCContext* cx, JS::Handle<JSObject*> wrapper,
                               JS::MutableHandleIdVector props) const override;

  virtual bool getOwnEnumerablePropertyKeys(
      MCContext* cx, JS::Handle<JSObject*> wrapper,
      JS::MutableHandleIdVector props) const override;
  virtual bool enumerate(MCContext* cx, JS::Handle<JSObject*> wrapper,
                         JS::MutableHandleIdVector props) const override;

  virtual bool call(MCContext* cx, JS::Handle<JSObject*> wrapper,
                    const JS::CallArgs& args) const override;
  virtual bool construct(MCContext* cx, JS::Handle<JSObject*> wrapper,
                         const JS::CallArgs& args) const override;

  virtual bool nativeCall(MCContext* cx, JS::IsAcceptableThis test,
                          JS::NativeImpl impl,
                          const JS::CallArgs& args) const override;

  virtual bool getPrototype(MCContext* cx, JS::HandleObject wrapper,
                            JS::MutableHandleObject protop) const override;

  static const FilteringWrapper* getSingleton();
};

}  // namespace xpc

#endif /* __FilteringWrapper_h__ */
