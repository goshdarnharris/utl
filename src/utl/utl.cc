// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0


#include <utl/utl.hh>

#if UTL_BUILD_NOCXX == 1

extern "C" {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wreserved-identifier"
[[noreturn]]
void __cxa_pure_virtual(void) //NOLINT(clang-diagnostic-missing-prototypes, bugprone-reserved-identifier)
{
    while(true);
}

//define unwind symbol so libunwind isn't linked in
//FIXME: make configurable
void __aeabi_unwind_cpp_pr0(void) {} //NOLINT(clang-diagnostic-missing-prototypes, bugprone-reserved-identifier)

} // extern "C"
#pragma clang diagnostic pop

#ifdef UTL_ENABLE_NEW_DELETE
__attribute__((weak))
void* operator new(size_t size)
{
    //NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
    return malloc(size);
}

__attribute__((weak))
void* operator new[](size_t size)
{
    //NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
    return malloc(size);
}

__attribute__((weak))
void* operator new(size_t size, void* storage) noexcept
{
    static_cast<void>(size);
    return storage;
}

__attribute__((weak))
void operator delete(void* pointer) noexcept
{
    //NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
    free(pointer);
}

__attribute__((weak))
void operator delete[](void* pointer) noexcept
{
    //NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
    free(pointer);
}

#endif

#endif
