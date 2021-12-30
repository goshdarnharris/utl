// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <stdint.h>

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



template <size_t W>
consteval size_t width(intn_t<W>)
{
    return W;
}

template <size_t W>
consteval size_t width(uintn_t<W>)
{
    return W;
}

consteval size_t width(uint8_t)
{
    return 8;
}

consteval size_t width(uint16_t)
{
    return 16;
}

consteval size_t width(uint32_t)
{
    return 32;
}


}
