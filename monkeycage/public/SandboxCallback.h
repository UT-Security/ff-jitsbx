/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SandboxCallback_h
#define mc_SandboxCallback_h

namespace MC {
namespace detail {
#if defined(JS_SANDBOX_NOOP)

class SandboxNoop;

template <typename T>
class SandboxCallback {
  T fn_;

  friend class SandboxNoop;
  explicit SandboxCallback(T fn) : fn_(fn) {}

 public:
  explicit SandboxCallback(const std::nullptr_t& arg) : fn_(arg) {}

  T UNSAFE_get() const { return fn_; }
};
#elif defined(JS_SANDBOX_DYLIB)

class SandboxDylib;

template <typename T>
class SandboxCallback {
  T fn_;

  friend class SandboxDylib;
  explicit SandboxCallback(T fn) : fn_(fn) {}

 public:
  explicit SandboxCallback(const std::nullptr_t& arg) : fn_(arg) {}

  T UNSAFE_get() const { return fn_; }
};
#elif defined(JS_SANDBOX_LFI)

class SandboxLFI;

template <typename T>
class SandboxCallback {
  T fn_;

  friend class SandboxLFI;
  explicit SandboxCallback(T fn) : fn_(fn) {}

 public:
  explicit SandboxCallback(const std::nullptr_t& arg) : fn_(arg) {}

  T UNSAFE_get() const { return fn_; }
};

#else
template <typename T>
using SandboxCallback = T;
#endif
}
template <typename T>
using SandboxCallback = MC::detail::SandboxCallback<T>;
}
#endif
