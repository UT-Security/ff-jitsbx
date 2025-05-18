/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/sandbox/Wrapper.h"

js::sandbox::ForwardingProxyHandler::ForwardingProxyHandler(const ProxyHandlerOps* ops,
                                                const void* handler,
                                                const void* aFamily,
                                                bool aHasPrototype,
                                                bool aHasSecurityPolicy)
    : js::ForwardingProxyHandler(aFamily, aHasPrototype, aHasSecurityPolicy),
      ops_(ops),
      handler_(handler) {}

DEFINE_PROXY_HANDLER_OPS_METHODS(js::sandbox::ForwardingProxyHandler, ops_, handler_)

js::sandbox::Wrapper::Wrapper(const WrapperOps* ops, const void* wrapper,
                              unsigned aFlags, bool aHasPrototype,
                              bool aHasSecurityPolicy)
    : js::Wrapper(aFlags, aHasPrototype, aHasSecurityPolicy),
      ops_(ops),
      wrapper_(wrapper) {}

DEFINE_WRAPPER_OPS_METHODS(js::sandbox::Wrapper, ops_, wrapper_)

js::sandbox::CrossCompartmentWrapper::CrossCompartmentWrapper(
    const WrapperOps* ops, const void* wrapper, unsigned aFlags,
    bool aHasPrototype, bool aHasSecurityPolicy)
    : js::CrossCompartmentWrapper(aFlags, aHasPrototype, aHasSecurityPolicy),
      ops_(ops),
      wrapper_(wrapper) {}

DEFINE_WRAPPER_OPS_METHODS(js::sandbox::CrossCompartmentWrapper, ops_, wrapper_)

js::sandbox::OpaqueCrossCompartmentWrapper::OpaqueCrossCompartmentWrapper(const WrapperOps* ops,
                                           const void* wrapper)
    : js::OpaqueCrossCompartmentWrapper(), ops_(ops), wrapper_(wrapper) {}

DEFINE_WRAPPER_OPS_METHODS(js::sandbox::OpaqueCrossCompartmentWrapper, ops_, wrapper_)

template <class Base>
js::sandbox::SecurityWrapper<Base>::SecurityWrapper(const WrapperOps* ops,
                                                    const void* wrapper,
                                                    unsigned flags,
                                                    bool hasPrototype)
    : js::SecurityWrapper<Base>(flags, hasPrototype),
      ops_(ops),
      wrapper_(wrapper) {}

DEFINE_SECURITY_WRAPPER_OPS_METHODS(js::sandbox::SecurityWrapper<Base>, ops_, wrapper_)

template class js::sandbox::SecurityWrapper<js::Wrapper>;
template class js::sandbox::SecurityWrapper<js::CrossCompartmentWrapper>;
