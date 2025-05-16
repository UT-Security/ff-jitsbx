/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/sandbox/Proxy.h"

js::sandbox::BaseProxyHandler::BaseProxyHandler(const ProxyHandlerOps* ops,
                                                const void* handler,
                                                const void* aFamily,
                                                bool aHasPrototype,
                                                bool aHasSecurityPolicy)
    : js::BaseProxyHandler(aFamily, aHasPrototype, aHasSecurityPolicy),
      ops_(ops),
      handler_(handler) {}

DEFINE_PROXY_HANDLER_OPS_METHODS(js::sandbox::BaseProxyHandler, ops_, handler_)
