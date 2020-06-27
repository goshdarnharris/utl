#ifndef UTL_ARRAY_HH_
#define UTL_ARRAY_HH_

#include "utl/utl.hh"
#include <initializer_list>

namespace utl {

template <typename T, size_t N>
struct array {
    T _storage[N];

    constexpr T& operator[](size_t idx) {
        return _storage[idx];
    }
    constexpr T const& operator[](size_t idx) const {
        return _storage[idx];
    }
    constexpr T volatile& operator[](size_t idx) volatile {
        return _storage[idx];
    }
    constexpr T const volatile& operator[](size_t idx) const volatile {
        return _storage[idx];
    }

    [[nodiscard]] constexpr size_t size() const {
        return N;
    }
    [[nodiscard]] constexpr size_t size() const volatile {
        return N;
    }

    T* begin() {
        return &_storage[0];
    }
    const T* begin() const {
        return &_storage[0];
    }
    volatile T* begin() volatile {
        return &_storage[0];
    }
    const volatile T* begin() const volatile {
        return &_storage[0];
    }

    T* end() {
        return &_storage[N];
    }    
    const T* end() const {
        return &_storage[N];
    }
    volatile T* end() volatile {
        return &_storage[N];
    }    
    const volatile T* end() const volatile {
        return &_storage[N];
    }

    constexpr T* data(void) {
        return _storage;
    }
    constexpr T const* data(void) const {
        return _storage;
    }
    constexpr volatile T* data(void) volatile {
        return _storage;
    }
    constexpr const volatile T* data(void) const volatile {
        return _storage;
    }
};

// template <typename T, typename... Args>
//     requires (is_same_v<Args,T> && ...)
// array(T first, Args... args) -> array<T,sizeof...(Args)>;

template <typename T, size_t N>
array(T (&)[N]) -> array<T,N>; //NOLINT(cppcoreguidelines-avoid-c-arrays)


} // namespace utl

#endif // UTL_ARRAY_HH_
