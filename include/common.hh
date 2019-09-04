/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#ifndef COMMON_LUMINAIRE_COMMON_HH_
#define COMMON_LUMINAIRE_COMMON_HH_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define LC_UNUSED __attribute__((unused))

#ifndef LC_BUILD_NOCXX
#define LC_BUILD_NOCXX 0
#endif

#if LC_BUILD_NOCXX == 1

namespace std {

[[noreturn]]
inline void terminate(void)
{
    while (1);
}

} // namespace std

extern void* operator new(size_t size, void* where);

#endif // LC_BUILD_NOCXX

namespace luminaire {

using uintb1_t = uint8_t;
using uintb2_t = uint16_t;
using uintb4_t = uint32_t;
using uintb8_t = uint64_t;
using sintb1_t = int8_t;
using sintb2_t = int16_t;
using sintb4_t = int32_t;
using sintb8_t = int64_t;

template <typename... Ts> void maybe_unused(Ts&&...) {}

} // namespace luminaire

namespace lmn = luminaire;

#endif // COMMON_LUMINAIRE_COMMON_HH_
