#ifndef UTL_ARRAY_HH_
#define UTL_ARRAY_HH_

#include "utl/utl.hh"
#include <initializer_list>

namespace utl {

namespace detail {
    struct zero_length_iterator {
        constexpr bool operator==(zero_length_iterator const&) { return true; }
        constexpr bool operator!=(zero_length_iterator const&) { return false; }
    };
}

template <typename T, size_t N>
struct array {
    using value_t = T;
    T _storage[N]{};

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

    constexpr T* begin() {
        return &_storage[0];
    }
    constexpr const T* begin() const {
        return &_storage[0];
    }
    constexpr volatile T* begin() volatile {
        return &_storage[0];
    }
    constexpr const volatile T* begin() const volatile {
        return &_storage[0];
    }

    constexpr T* end() {
        return &_storage[N];
    }    
    constexpr const T* end() const {
        return &_storage[N];
    }
    constexpr volatile T* end() volatile {
        return &_storage[N];
    }    
    constexpr const volatile T* end() const volatile {
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

template <typename T>
struct array<T,0> {
    using value_t = T;
    [[nodiscard]] constexpr size_t size() const {
        return 0;
    }
    [[nodiscard]] constexpr size_t size() const volatile {
        return 0;
    }
    constexpr auto begin() const { return detail::zero_length_iterator{}; }
    constexpr auto end() const { return detail::zero_length_iterator{}; }
    constexpr auto rbegin() const { return detail::zero_length_iterator{}; }
    constexpr auto rend() const { return detail::zero_length_iterator{}; }
    constexpr T* data() const { return nullptr; }
};

// template <typename T, typename... Args>
//     requires (is_same_v<Args,T> && ...)
// array(T first, Args... args) -> array<T,sizeof...(Args)>;

template <typename T, size_t N>
array(T (&)[N]) -> array<T,N>; //NOLINT(cppcoreguidelines-avoid-c-arrays)


} // namespace utl

#endif // UTL_ARRAY_HH_
