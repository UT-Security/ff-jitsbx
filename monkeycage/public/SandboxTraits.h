/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_SandboxTraits_h
#define mc_SandboxTraits_h

#include "monkeycage/SandboxHelpers.h"

#include <type_traits>

namespace MC {
namespace detail {

template<typename T, typename MC_Sbx>
class Tainted;

template<typename T, typename MC_Sbx>
class TaintedVolatile;

template<typename T, typename MC_Sbx>
class AppPointer;

#define mc_generate_wrapper_check(name)                                     \
  namespace detail_mc_is_##name                                             \
  {                                                                         \
    template<typename T>                                                    \
    struct unwrapper : std::false_type                                      \
    {};                                                                     \
                                                                            \
    template<typename T, typename MC_Sbx>                                   \
    struct unwrapper<name<T, MC_Sbx>> : std::true_type                      \
    {};                                                                     \
  }                                                                         \
                                                                            \
  template<typename T>                                                      \
  constexpr bool mc_is_##name##_v =                                         \
    detail_mc_is_##name::unwrapper<T>::value;                               \

mc_generate_wrapper_check(Tainted);
mc_generate_wrapper_check(TaintedVolatile);
mc_generate_wrapper_check(AppPointer);

#undef mc_generate_wrapper_check

template<typename T>
constexpr bool mc_is_tainted_or_vol_v =
  mc_is_Tainted_v<T> || mc_is_TaintedVolatile_v<T>;

template<typename T>
constexpr bool mc_is_wrapper_v =
  mc_is_Tainted_v<T> || mc_is_TaintedVolatile_v<T>;

namespace detail_mc_remove_wrapper {
  template<typename T>
  struct unwrapper
  {
    using type = T;
    using type_sbx = void;
  };

  template<typename T, typename MC_Sbx>
  struct unwrapper<Tainted<T, MC_Sbx>>
  {
    using type = T;
    using type_sbx = MC_Sbx;
  };

  template<typename T, typename MC_Sbx>
  struct unwrapper<TaintedVolatile<T, MC_Sbx>>
  {
    using type = T;
    using type_sbx = MC_Sbx;
  };
  
  template<typename T, typename MC_Sbx>
  struct unwrapper<TaintedVolatile<T, MC_Sbx>&>
  {
    using type = T&;
    using type_sbx = MC_Sbx;
  };

  template<typename T, typename MC_Sbx>
  struct unwrapper<AppPointer<T, MC_Sbx>>
  {
    using type = T;
    using type_sbx = MC_Sbx;
  };
}

template<typename T>
using mc_remove_wrapper_t =
  typename detail_mc_remove_wrapper::unwrapper<T>::type;

//TODO(abhishek): revisit this to ensure everything is
// required to be properly Tainted.
template<typename T>
constexpr bool mc_is_tainted_callback_arg_v =
     is_fundamental_or_enum_v<T>
  || std::is_class_v<T>
  || std::is_lvalue_reference_v<T>
  || mc_is_AppPointer_v<T>
  || mc_is_Tainted_v<T>;

namespace detail_mc_tainted_callback_arg {
  template<typename T, typename MC_Sbx>
  struct wrapper
  {
    using type = T;
  };

  template<typename T, typename MC_Sbx>
  struct wrapper<T*, MC_Sbx>
  {
    using type = Tainted<T*, MC_Sbx>;
  };

  template<typename MC_Sbx>
  struct wrapper<void*, MC_Sbx>
  {
    using type = AppPointer<void*, MC_Sbx>;
  };

  template<typename MC_Sbx>
  struct wrapper<const void*, MC_Sbx>
  {
    using type = AppPointer<const void*, MC_Sbx>;
  };
}


template<typename T, typename MC_Sbx>
using mc_tainted_callback_arg_t =
  typename detail_mc_tainted_callback_arg::wrapper<T, MC_Sbx>::type;


}
}  // namespace MC

#endif
