// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <type_traits>
#include <concepts>
#include <atomic>

#include <utl/utility.hh>

namespace utl::irq {

template <typename>
constexpr auto is_isr_safe() { 
    return true;
}

//lock-free atomics are always safe.
//there's probably a better way to express this.
template <typename T>
    requires requires{
        { T::is_always_lock_free } -> std::convertible_to<bool>;
        typename T::value_type;
    }
constexpr auto is_isr_safe() { 
    return true;
}

//Fundamental types are values, and we know that copying
//them is safe (because it doesn't risk copying unsafe members)
template <typename T>
    requires std::is_fundamental_v<T>
constexpr auto is_isr_safe() { 
    return true;
}

//If the parameter is an rvalue, we will be taking ownership of it
//and it is therefore safe.
template <typename T>
    requires std::is_rvalue_reference_v<T>
constexpr auto is_isr_safe() { return true; }


//this ends up hiding useful error information.
//need to find a way to rework it.
template <typename T>
concept any_isr_safe = is_isr_safe<T>();

} //namespace utl::irq
