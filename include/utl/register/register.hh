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

namespace utl::registers {

BFG_TAG_INVOKE_DEF(reset_value);
BFG_TAG_INVOKE_DEF(width);
BFG_TAG_INVOKE_DEF(write);
BFG_TAG_INVOKE_DEF(read);

template <typename T>
concept any_register = requires(const T r) {
    { width(r) } -> utl::integer::any_unsigned_convertible;
    { reset_value(r) } -> utl::integer::any_unsigned_convertible;
};

template <typename T>
using value_t = std::decay_t<decltype(reset_value(declval<T>()))>;

template <typename T, typename R>
concept same_register_as = std::same_as<std::decay_t<R>, std::decay_t<T>>;

template <typename... Ts>
concept same_target_register = ((any_register<Ts> and same_register_as<tuple_element_t<0,tuple<Ts...>>,Ts>) and ...);

template <typename T>
concept any_readable_register = any_register<T> and
    requires(T r) {
        { read(r) } -> utl::integer::any_unsigned_convertible;
    };


template <typename T>
concept any_writable_register = any_register<T> and
    requires(T r, utl::integer::unsigned_cast_t<T> v) {
        { write(r,v) } -> std::same_as<void>;
    };
} //namespace utl::registers
