#ifndef UTL_ARRAY_HH_
#define UTL_ARRAY_HH_

#include "utl/utl.hh"
#include <initializer_list>
#include <concepts>

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
    T _storage[N]{}; //NOLINT(cppcoreguidelines-avoid-c-arrays)

    constexpr T& operator[](size_t idx) {
        return _storage[idx]; //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
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
    [[nodiscard]] constexpr const T* begin() const {
        return &_storage[0];
    }
    constexpr volatile T* begin() volatile {
        return &_storage[0];
    }
    [[nodiscard]] constexpr const volatile T* begin() const volatile {
        return &_storage[0];
    }

    constexpr T* end() {
        return &_storage[N];
    }    
    [[nodiscard]] constexpr const T* end() const {
        return &_storage[N];
    }
    constexpr volatile T* end() volatile {
        return &_storage[N];
    }    
    [[nodiscard]] constexpr const volatile T* end() const volatile {
        return &_storage[N];
    }

    constexpr T* data(void) {
        return static_cast<T*>(_storage);
    }
    [[nodiscard]] constexpr T const* data(void) const {
        return static_cast<const T*>(_storage);
    }
    constexpr volatile T* data(void) volatile {
        return static_cast<volatile T*>(_storage);
    }
    [[nodiscard]] constexpr const volatile T* data(void) const volatile {
        return static_cast<const volatile T*>(_storage);
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
    [[nodiscard]] constexpr auto begin() const { return detail::zero_length_iterator{}; }
    [[nodiscard]] constexpr auto end() const { return detail::zero_length_iterator{}; }
    [[nodiscard]] constexpr auto rbegin() const { return detail::zero_length_iterator{}; }
    [[nodiscard]] constexpr auto rend() const { return detail::zero_length_iterator{}; }
    [[nodiscard]] constexpr T* data() const { return nullptr; }
};

// template <typename T, typename... Args>
//     requires (is_same_v<Args,T> && ...)
// array(T first, Args... args) -> array<T,sizeof...(Args)>;

template <typename T, size_t N>
array(T (&)[N]) -> array<T,N>; //NOLINT(cppcoreguidelines-avoid-c-arrays)


template <typename... Ts>
array(Ts&&...) -> array<std::common_type_t<Ts...>, sizeof...(Ts)>;

// template <typename T>
// array(std::initializer_list<T> il) -> array<T,il.size()>;


} // namespace utl

#endif // UTL_ARRAY_HH_
