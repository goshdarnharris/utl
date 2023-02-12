// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0


#pragma once

#include <concepts>

namespace utl {
template<class T>
[[deprecated]] constexpr inline bool print_type = true;

template<template <typename> typename T>
[[deprecated]] constexpr inline bool print_unary_template = true;

template <auto V>
[[deprecated]] constexpr inline bool print_value = true;



template <typename T>
struct type_v {
    using type = T;

    //FIXME: should be conditionally explicit
    template <typename U>
        requires std::convertible_to<type,typename U::type>
    constexpr operator type_v<U>()
    {
        return type_v<U>{};
    }
};

template <typename T>
struct explicit_type_v {

};

template <template <typename...> class MFn, bool condition, typename T>
using apply_if_t = std::conditional_t<condition, MFn<T>, T>;

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename From, typename To>
using copy_cv_t =
    apply_if_t<std::add_volatile_t, std::is_volatile_v<From>,
        apply_if_t<std::add_const_t, std::is_const_v<From>,
            std::remove_cv_t<To>>>;

template <typename From, typename To>
using copy_ref_t =
    apply_if_t<std::add_rvalue_reference_t, std::is_rvalue_reference_v<From>,
        apply_if_t<std::add_lvalue_reference_t, std::is_lvalue_reference_v<From>,
            std::remove_reference_t<To>>>;

template <typename From, typename To>
using copy_cvref_t = copy_ref_t<From,
    copy_cv_t<std::remove_reference_t<From>, remove_cvref_t<To>>>;

} //namespace utl
