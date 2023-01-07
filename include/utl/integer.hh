// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <stdint.h>
#include <concepts>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbit-int-extension"

namespace utl::integer {

template <size_t W>
struct intn_impl {
    using type = _BitInt(W);
};

template <>
struct intn_impl<8> { using type = int8_t; };
template <>
struct intn_impl<16> { using type = int16_t; };
template <>
struct intn_impl<32> { using type = int32_t; };

template <size_t W>
struct uintn_impl {
    using type = unsigned _BitInt(W);
};

template <>
struct uintn_impl<8> { using type = uint8_t; };
template <>
struct uintn_impl<16> { using type = uint16_t; };
template <>
struct uintn_impl<32> { using type = uint32_t; };

template <typename T>
struct ExtInt_width_helper;
template <size_t W>
struct ExtInt_width_helper<_BitInt(W)> { static constexpr auto width = W; };
template <size_t W>
struct ExtInt_width_helper<const _BitInt(W)> { static constexpr auto width = W; };
template <size_t W>
struct ExtInt_width_helper<volatile _BitInt(W)> { static constexpr auto width = W; };
template <size_t W>
struct ExtInt_width_helper<const volatile _BitInt(W)> { static constexpr auto width = W; };
template <size_t W>
struct ExtInt_width_helper<unsigned _BitInt(W)> { static constexpr auto width = W; };
template <size_t W>
struct ExtInt_width_helper<const unsigned _BitInt(W)> { static constexpr auto width = W; };
template <size_t W>
struct ExtInt_width_helper<volatile unsigned _BitInt(W)> { static constexpr auto width = W; };
template <size_t W>
struct ExtInt_width_helper<const volatile unsigned _BitInt(W)> { static constexpr auto width = W; };

#pragma clang diagnostic pop

//TODO: ideally these would be a customization point object or
//at least a first class overload set, but
//constant expression rules get in the way of consteval tag_invoke.
//specifically, forwarding references and/or variadic templates
//make it asplode.

template <typename T>
constexpr size_t width();

template <std::integral T>
constexpr size_t width()
{
    return sizeof(T)*8;
}



template <typename T>
concept any_BitInt = requires {
    ExtInt_width_helper<T>::width;
};

template <any_BitInt T>
constexpr size_t width() { return ExtInt_width_helper<T>::width; }

} //namespace utl::integer


namespace utl {

template <size_t W>
using intn_t = typename integer::intn_impl<W>::type;

template <size_t W>
using uintn_t = typename integer::uintn_impl<W>::type;

template <size_t W>
constexpr uintn_t<W> uintn_max = (1<<W) - 1;

template <size_t W>
constexpr intn_t<W> intn_max = (1<<(W-1)) - 1;

template <size_t W>
constexpr intn_t<W> intn_min = -(1<<(W-1));

static_assert(uintn_max<16> == 65535);
static_assert(intn_min<16> == -32768);
static_assert(intn_max<16> == 32767);

template <typename T>
concept integral = std::integral<T> or 
    integer::any_BitInt<T>;

static_assert(integral<int>);
static_assert(integral<const int>);
static_assert(integral<unsigned int>);
static_assert(integral<intn_t<12>>);
static_assert(integral<const intn_t<12>>);
static_assert(integral<uintn_t<12>>);
static_assert(integer::width<intn_t<12>>() == 12);
static_assert(integer::width<const intn_t<12>>() == 12);
static_assert(integer::width<uintn_t<12>>() == 12);
static_assert(integer::width<char>() == 8);
static_assert(integer::width<short>() == 16);

template <typename T>
concept has_integer_width = requires {
    { utl::integer::width<T>() } -> std::convertible_to<size_t>;
};

} //namespace utl

namespace utl::integer {


//FIXME: these aren't quite right
template <size_t W>
constexpr auto signed_cast(has_integer_width auto v)
{
    using T = decltype(v);
    return static_cast<uintn_t<::utl::integer::width<T>()>>(v);
}

constexpr auto unsigned_cast(has_integer_width auto v)
{
    using T = decltype(v);
    return static_cast<uintn_t<::utl::integer::width<T>()>>(v);
}

template <typename T>
concept convertible_to_signed = has_integer_width<T> and requires(T v) {
    signed_cast(v);
};

template <typename T>
concept convertible_to_unsigned = has_integer_width<T> and requires(T v) {
    unsigned_cast(v);
};

template <convertible_to_signed T>
using signed_cast_t = decltype(signed_cast(declval<T>()));

template <convertible_to_unsigned T>
using unsigned_cast_t = decltype(unsigned_cast(declval<T>()));

} //namespace utl::integer


#include <utl/bits/format_atoi.hh>

namespace utl::literals {

// namespace detail {
//     template <char... Cs>
//     consteval auto chars_to_sv()
//     {
//         constexpr char str[sizeof...(Cs)] = {Cs...};
//         return utl::string_view{str,sizeof...(Cs)};
//     }

//     template <size_t W, char... Cs>
//     consteval auto make_unsigned()
//     {
//         constexpr auto input = chars_to_sv<Cs...>();
//         static_assert(not input.starts_with('-'), "negative sign in utl unsigned integer literal");
//         constexpr auto value = utl::fmt::ascii_to_ulonglong(input);
//         static_assert(value <= utl::uintn_max<W>, "overflow in utl unsigned integer literal");
//         return utl::uintn_t<W>{value};
//     }

    
//     template <size_t W, char... Cs>
//     consteval auto make_signed()
//     {
//         constexpr auto get_longlong = [](string_view v){
//             return static_cast<long long int>(utl::fmt::ascii_to_ulonglong(v));
//         };

//         constexpr auto input = chars_to_sv<Cs...>();
//         constexpr auto value = input.starts_with('-') ? -get_longlong(input.substr(1,utl::npos)) : get_longlong(input);
//         static_assert(value <= utl::intn_max<W>, "overflow in utl signed integer literal");
//         static_assert(value >= utl::intn_min<W>, "underflow in utl signed integer literal");
//         return utl::intn_t<W>{value};
//     }
// }

// template <char... Cs> 
// consteval auto operator""_u1() { return detail::make_unsigned<1,Cs...>(); }



// constexpr auto my_value = 1_u1;

} //namespace utl::literals
