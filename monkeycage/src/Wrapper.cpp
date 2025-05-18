/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "monkeycage/Wrapper.h"

#ifdef JS_SANDBOX

namespace mc {
const char* Wrapper::getFamily() {
  static const char* inner_ = js::Wrapper::getFamily();
  return inner_;
}

const Wrapper* Wrapper::getSingleton() {
  static const Wrapper inner_(js::Wrapper::getSingleton());
  return &inner_;
}

const Wrapper* Wrapper::getSingletonWithPrototype() {
  static const Wrapper inner_(js::Wrapper::getSingletonWithPrototype());
  return &inner_;
}

const CrossCompartmentWrapper* CrossCompartmentWrapper::getSingleton() {
  static const CrossCompartmentWrapper inner_(js::CrossCompartmentWrapper::getSingleton());
  return &inner_;
}

const OpaqueCrossCompartmentWrapper* OpaqueCrossCompartmentWrapper::getSingleton() {
  static const OpaqueCrossCompartmentWrapper inner_(js::OpaqueCrossCompartmentWrapper::getSingleton());
  return &inner_;
}

template class SecurityWrapper<Wrapper>;
template class SecurityWrapper<CrossCompartmentWrapper>;

}

#endif
