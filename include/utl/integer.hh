// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <stdint.h>
#include <concepts>

namespace utl::integer {

template <size_t W>
struct intn_impl {
    using type = _ExtInt(W);
};

template <>
struct intn_impl<8> { using type = int8_t; };
template <>
struct intn_impl<16> { using type = int16_t; };
template <>
struct intn_impl<32> { using type = int32_t; };

template <size_t W>
struct uintn_impl {
    using type = unsigned _ExtInt(W);
};

template <>
struct uintn_impl<8> { using type = uint8_t; };
template <>
struct uintn_impl<16> { using type = uint16_t; };
template <>
struct uintn_impl<32> { using type = uint32_t; };

template <size_t W>
using intn_t = typename intn_impl<W>::type;

template <size_t W>
using uintn_t = typename uintn_impl<W>::type;


BFG_TAG_INVOKE_DEF(width);

template <size_t W>
constexpr size_t tag_invoke(width_t, const _ExtInt(W))
{
    return W;
}

template <size_t W>
constexpr size_t tag_invoke(width_t, const unsigned _ExtInt(W))
{
    return W;
}

constexpr size_t tag_invoke(width_t, const std::integral auto v)
{
    return sizeof(v)*8;
}

static_assert(std::integral<unsigned int>);
static_assert(tag_invoke(width, intn_t<12>{}));
static_assert(::utl::integer::tag_invoke<12>(width, intn_t<12>{}));
static_assert(bfg::tag_invoke(width, intn_t<12>{}));
static_assert(width(intn_t<12>{}) == 12);
static_assert(width(uintn_t<12>{}) == 12);
static_assert(width(char{}) == 8);
static_assert(width(short{}) == 16);

template <typename T>
concept has_integer_width = requires(T v) {
    { utl::integer::width(v) } -> std::convertible_to<size_t>;
};

template <size_t W>
constexpr auto signed_cast(const _ExtInt(W) v)
{
    return static_cast<intn_t<W>>(v);
}

constexpr auto signed_cast(std::integral auto v)
{
    return static_cast<std::make_signed_t<decltype(v)>>(v);
}

template <size_t W>
constexpr auto unsigned_cast(const unsigned _ExtInt(W) v)
{
    return static_cast<intn_t<W>>(v);
}

constexpr auto unsigned_cast(std::integral auto v)
{
    return static_cast<std::make_unsigned_t<decltype(v)>>(v);
}

template <typename T>
concept any_signed_convertible = has_integer_width<T> and requires(T v) {
    signed_cast(v);
};

template <typename T>
concept any_unsigned_convertible = has_integer_width<T> and requires(T v) {
    unsigned_cast(v);
};

template <any_signed_convertible T>
using signed_cast_t = decltype(signed_cast(declval<T>()));

template <any_unsigned_convertible T>
using unsigned_cast_t = decltype(unsigned_cast(declval<T>()));

} //namespace utl::integer
