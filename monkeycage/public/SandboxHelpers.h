/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SandboxHelpers_h
#define mc_SandboxHelpers_h

#include <array>
#include <type_traits>

namespace MC {
namespace detail {

#define mc_detail_static_fail_because(CondExpr, Message)              \
    static_assert(!(CondExpr), Message)

#define MC_UNUSED(...) (void)__VA_ARGS__

#define MC_REQUIRE_SEMI_COLON static_assert(true)
  
#define if_constexpr_named(varName, ...)                              \
  if constexpr (constexpr auto varName = __VA_ARGS__; varName)

#define MC_ENABLE_IF(...) std::enable_if_t<__VA_ARGS__>* = nullptr

template<typename T>
constexpr bool is_fundamental_or_enum_v =
  std::is_fundamental_v<T> || std::is_enum_v<T>;

template<typename T>
constexpr bool is_basic_type_v =
  std::is_fundamental_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>;

}
}  // namespace MC

#endif
