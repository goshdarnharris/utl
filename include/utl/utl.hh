// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#ifndef UTL_HH_
#define UTL_HH_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <utility>

#include <utl/type-list.hh>

// #define UTL_UNUSED __attribute__((unused))

#ifndef UTL_BUILD_NOCXX
#define UTL_BUILD_NOCXX 1 //NOLINT(cppcoreguidelines-macro-usage)
#endif

#if UTL_BUILD_NOCXX == 1

namespace std {

// [[noreturn]]
// inline void terminate(void)
// {
//     while (true);
// }

} // namespace std

extern void* operator new(size_t size, void* storage) noexcept;

#endif // UTL_BUILD_NOCXX

namespace utl {

using uintb1_t = uint8_t;
using uintb2_t = uint16_t;
using uintb4_t = uint32_t;
using uintb8_t = uint64_t;
using sintb1_t = int8_t;
using sintb2_t = int16_t;
using sintb4_t = int32_t;
using sintb8_t = int64_t;

template <typename... Ts> constexpr void maybe_unused(Ts&&...) {}


inline constexpr size_t npos = static_cast<size_t>(-1);

namespace MOVE {

template <typename T>
struct imprecise {
    T min;
    T max;

    constexpr bool contains(T value) const
    {
        return value >= min and value <= max;
    }
};

template <typename T>
imprecise(T a, T b) -> imprecise<T>;

}

using namespace MOVE;


} // namespace utl


#include "utl-platform.hh"
#include "units.hh"

namespace utl {
using namespace unit;
using namespace literals;
}

#endif // UTL_HH_
