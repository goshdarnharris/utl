// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/traits.hh>
#include <type_traits>

namespace utl {

// namespace detail {
//     template< class T, class U >
//     concept SameHelper = std::is_same_v<T, U>;
// }
 
// template< class T, class U >
// concept same_as = detail::SameHelper<T, U> && detail::SameHelper<U, T>;

template <typename T, typename U>
concept same_as = is_same_v<T,U>;

template <typename T, typename To>
concept decays_to = same_as<To,std::decay_t<T>>;

template <typename From, typename To>
concept convertible_to =
    std::is_convertible_v<From, To> &&
    requires(std::add_rvalue_reference_t<From> (&f)()) {
            static_cast<To>(f());
    };

template <typename F, typename R = void, typename... Args>
concept callable = requires(F&& f, Args&&... args) {
    { f(args...) } -> same_as<R>;
};

template <typename T>
concept any_enum = std::is_enum_v<T>;

} //namespace utl
