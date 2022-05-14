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

} //namespace utl
