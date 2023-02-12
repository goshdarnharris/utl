// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/integer.hh>
#include <utl/register/register.hh>

#include <utl/bitfield.hh>


namespace utl::registers::field {

// template <utl::integer::convertible_to_unsigned T>
// struct field {
//     T value;


// };


// enum class my_options : uintn_t<1> {
//     ON = 0b0,
//     OFF = 0x1
// };

// template <any_uart T>
// struct field_value_t<T::SR_t::CTS_t> {
//     using type = my_options;
// }

// assign(uart.SR.CTS, uart_t::SR_t::CTS_t::option)
// assign(uart_t::SR::CTS, uart_t::SR::CTS::option)
// assign(uart_t::SR::CTS::option, uart_t::SR::TXE{0x50})
// assign(value_t<uart.SR.CTS>::option, value_t<uart.SR.TXE>{0x50})

template <typename T>
using register_t = typename std::decay_t<T>::register_t;

template <typename T>
using value_t = typename std::decay_t<T>::value_t;

template <typename T>
using register_value_t = value_t<register_t<T>>;

template <typename T>
concept any_field = requires(T r) {
    typename T::register_t;
    typename T::value_t;
} and any_register<typename T::register_t>;


template <typename T, typename R>
concept any_field_of = any_field<T> and same_register_as<R,register_t<T>>;

constexpr auto align_to_register(any_field auto f, value_t<decltype(f)> v) -> register_value_t<decltype(f)>
{
    //since we're using fixed width integers, masking isn't necessary. I think.
    return v << f.offset();
}

constexpr auto align_from_register(any_field auto f, register_value_t<decltype(f)> v) -> value_t<decltype(f)>
{
    //since we're using fixed width integers, masking isn't necessary. I think.
    return v >> f.offset();
}

template <any_register R, size_t Offset, size_t Width>
struct field {
    using register_t = R;
    static_assert(Width <= R{}.width(), "field must fit within its register");
    static_assert(Offset + Width <= R{}.width(), "field cannot extend past the end of its register");
    using value_t = uintn_t<Width>;
    static constexpr size_t width() { return Width; }
    static constexpr size_t offset() { return Offset; }
};

template <any_register R, size_t Offset, size_t Width>
struct read_only : field<R,Offset,Width> {

};

template <any_register R, size_t Offset, size_t Width>
struct write_only : field<R,Offset,Width> {

};

template <any_register R, size_t Offset, size_t Width>
struct read_write : field<R,Offset,Width> {

};

template <any_register R, size_t Offset, size_t Width>
struct write_once : field<R,Offset,Width> {

};

template <any_register R, size_t Offset, size_t Width>
struct read_write_once {

};

} //namespace utl::registers::field
