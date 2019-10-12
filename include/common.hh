/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#ifndef UTL_COMMON_HH_
#define UTL_COMMON_HH_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define UTL_UNUSED __attribute__((unused))

#ifndef UTL_BUILD_NOCXX
#define UTL_BUILD_NOCXX 0
#endif

#if UTL_BUILD_NOCXX == 1

namespace std {

[[noreturn]]
inline void terminate(void)
{
    while (1);
}

} // namespace std

extern void* operator new(size_t size, void* where);

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

template <typename... Ts> void maybe_unused(Ts&&...) {}

} // namespace utl


#endif // UTL_COMMON_HH_
