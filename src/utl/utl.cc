
#include <utl/utl.hh>

#if UTL_BUILD_NOCXX == 1

extern "C" {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
[[noreturn]]
void __cxa_pure_virtual(void) //NOLINT(clang-diagnostic-missing-prototypes)
{
    while(true);
}

} // extern "C"
#pragma clang diagnostic pop

// __attribute__((weak))
// void* operator new(size_t size)
// {
//     //NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
//     return malloc(size);
// }

// __attribute__((weak))
// void* operator new[](size_t size)
// {
//     //NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
//     return malloc(size);
// }

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

//define unwind symbol so libunwind isn't linked in
//FIXME: make configurable
void __aeabi_unwind_cpp_pr0(void) {}

#endif
