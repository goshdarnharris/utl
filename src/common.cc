
#include "common.hh"

extern "C" {

[[noreturn]]
void __cxa_pure_virtual(void)
{
    while(1);
}

} // extern "C"

__attribute__((weak))
void* operator new(size_t size)
{
    return malloc(size);
}

__attribute__((weak))
void* operator new[](size_t size)
{
    return malloc(size);
}

__attribute__((weak))
void* operator new(size_t size, void* storage)
{
    static_cast<void>(size);
    return storage;
}

__attribute__((weak))
void operator delete(void* pointer) noexcept
{
    free(pointer);
}

__attribute__((weak))
void operator delete[](void* pointer) noexcept
{
    free(pointer);
}
