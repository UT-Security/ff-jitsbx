/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __CrossOriginObjectWrapper_h__
#define __CrossOriginObjectWrapper_h__

#include "monkeycage/Wrapper.h"

namespace xpc {

/**
 * A wrapper that's only used for cross-origin objects. This should be
 * just like a CrossCompartmentWrapper but (as an implementation
 * detail) doesn't actually do any compartment-entering and (as an
 * implementation detail) delegates all the security decisions and
 * compartment-entering to the target object, which is always a
 * proxy.
 *
 * We could also inherit from CrossCompartmentWrapper but then we
 * would need to override all the proxy hooks to avoid the
 * compartment-entering bits.
 */
class CrossOriginObjectWrapper : public mc::Wrapper {
 public:
  // We want to claim to have a security policy, so code doesn't just
  // CheckedUnwrap us willy-nilly.  But we're OK with the BaseProxyHandler
  // implementation of enter(), which allows entering.  Our target is what
  // really does the security checks.
  //
  // We don't want to inherit from CrossCompartmentWrapper, because we don't
  // want the compartment-entering behavior it has.  But we do want to set the
  // CROSS_COMPARTMENT flag on js::Wrapper so that we test true for
  // is<js::CrossCompartmentWrapperObject> and so forth.
#ifdef JS_SANDBOX
  inline explicit CrossOriginObjectWrapper()
      : mc::Wrapper(Flags::CROSS_COMPARTMENT, /* aHasPrototype = */ false,
                    /* aHasSecurityPolicy = */ true) {}
#else
  constexpr explicit CrossOriginObjectWrapper()
      : mc::Wrapper(CROSS_COMPARTMENT, /* aHasPrototype = */ false,
                    /* aHasSecurityPolicy = */ true) {}
#endif

  bool dynamicCheckedUnwrapAllowed(JS::Handle<JSObject*> obj,
                                   MCContext* cx) const override;

  // Cross origin objects should not participate in private fields.
  virtual bool throwOnPrivateField() const override { return true; }

  static const CrossOriginObjectWrapper* getSingleton();
};
}  // namespace xpc

#endif
