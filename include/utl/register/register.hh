// Copyright 2022 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <stddef.h>
#include <utility>
#include <bfg/tag_invoke.h>
#include <utl/integer.hh>
#include <utl/tuple.hh>
#include <type_traits>

namespace utl::registers {

//TODO: these should be customization points, but
// right now it's very difficult to use function parameters
// in a consteval context, which is what I'd like these to be.
// BFG_TAG_INVOKE_DEF(reset_value);
// BFG_TAG_INVOKE_DEF(width); 
BFG_TAG_INVOKE_DEF(write);
BFG_TAG_INVOKE_DEF(read);

//TODO: it'd be wonderful if this could become a customization point.
template <typename T>
constexpr size_t width()
{
    return std::decay_t<T>::width();
}

template <typename T>
concept has_width = requires(T const& r) {    
    // { utl::registers::width(T{}) } -> utl::integer::convertible_to_unsigned;  
    { utl::registers::width<T>() } -> utl::integer::convertible_to_unsigned;   
    { std::bool_constant<(utl::registers::width<T>(), true)>() } -> std::same_as<std::true_type>;
};

template <typename T>
concept has_reset_value = requires(T const& r) {    
    { r.reset_value() } -> utl::integer::convertible_to_unsigned;    
    { std::bool_constant<(std::decay_t<T>{}.reset_value(), true)>() } -> std::same_as<std::true_type>;
    { std::decay_t<T>::reset_value() } -> utl::integer::convertible_to_unsigned;
};

template <typename T>
concept any_register = has_width<T> and has_reset_value<T>;

template <typename T>
using value_t = std::decay_t<decltype(reset_value(declval<T>()))>;

template <typename T, typename R>
concept same_register_as = std::same_as<std::decay_t<R>, std::decay_t<T>>;

template <typename... Ts>
concept same_target_register = ((any_register<Ts> and same_register_as<tuple_element_t<0,tuple<Ts...>>,Ts>) and ...);

template <typename T>
concept any_readable_register = any_register<T> and
    requires(T r) {
        { read(r) } -> utl::integer::convertible_to_unsigned;
    };


template <typename T>
concept any_writable_register = any_register<T> and
    requires(T r, utl::integer::unsigned_cast_t<T> v) {
        { write(r,v) } -> std::same_as<void>;
    };
} //namespace utl::registers
