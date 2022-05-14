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
#include <utl/utility.hh>
#include <type_traits>

namespace utl::registers {

namespace customize {
    BFG_TAG_INVOKE_DEF(width); 
    BFG_TAG_INVOKE_DEF(write);
    BFG_TAG_INVOKE_DEF(read);
}
BFG_TAG_INVOKE_DEF(reset_value);
BFG_TAG_INVOKE_DEF(location);

template <auto C, typename T>
concept is_specialized = requires {
    C(T{});
};

// template <typename T>
//     requires is_specialized<customize::reset_value,type_v<T>>
// constexpr auto reset_value()
// {
//     return customize::reset_value(type_v<T>{});
// }

template <typename T>
    requires is_specialized<customize::width,type_v<T>>
constexpr auto width()
{
    return customize::width(type_v<T>{});
}

namespace test {
    namespace inner {
        struct adl_t {
            friend constexpr auto tag_invoke(reset_value_t, adl_t)
            {
                return 0b1101;
            }

            friend constexpr auto tag_invoke(customize::width_t, type_v<adl_t>)
            {
                return 5;
            }
        };
    } //namespace inner

    static_assert(reset_value(inner::adl_t{}) == 0b1101);
    static_assert(width<inner::adl_t>() == 5);
} //namespace test

template <typename T>
concept has_width = requires(T const& r) { 
    { utl::registers::width<T>() } -> utl::integer::convertible_to_unsigned;   
    { std::bool_constant<(utl::registers::width<T>(), true)>() } -> std::same_as<std::true_type>;
};

template <typename T>
concept has_reset_value = requires(T const& r) {    
    { utl::registers::reset_value(r) } -> utl::integer::convertible_to_unsigned;    
    // { std::bool_constant<(utl::registers::reset_value<T>(), true)>() } -> std::same_as<std::true_type>;
    // { std::decay_t<T>::reset_value() } -> utl::integer::convertible_to_unsigned;
};

template <typename T>
concept any_register = has_width<T>;

template <typename T>
using value_t = std::decay_t<decltype(reset_value(std::declval<T>()))>;

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
