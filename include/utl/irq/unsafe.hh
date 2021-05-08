#pragma once

#include <utility>
#include <concepts>
#include <utl/irq/bits.hh>


//NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UTL_MARK_UNSAFE_DEPRECATED_MESSAGE "Using this will invoke undefined behaviour, "\
"which could take the form of data races, dangling references, or other unexpected funtimes. "\
"You should consider using a const& of appropriate lifetime, giving up ownership "\
"of the argument with std::move, or using an appropriate "\
"atomic type. That said, undefined behaviour isn't necessarily bad so if you're doing "\
"this for a good reason just make sure take a look here the next time you have a nontrivial "\
"bug. You can disable this warning by surrounding this line with the appropriate compiler "\
"pragmas... just don't forget about it. "\
"For clang, use the 'clang diagnostic [push|pop]' and 'clang diagnostic ignored' pragmas."

namespace utl::irq {

namespace detail {

template <typename T>
struct unsafe {
    using value_t = std::remove_reference_t<T>;
    value_t value;

    constexpr operator value_t() const { return value; }
};

template <typename T>
struct unsafe_ref {
    using value_t = std::remove_reference_t<T>;
    value_t& value;

    constexpr operator value_t&() { return value; }
};

} //namespace detail

template <typename T>
concept any_marked_unsafe = (std::same_as<std::remove_reference_t<T>,detail::unsafe<typename T::value_t>> or
        std::same_as<std::remove_reference_t<T>,detail::unsafe_ref<typename T::value_t>>);

template <any_marked_unsafe T>
constexpr auto is_isr_safe() { return true; }

template <typename T> [[deprecated(UTL_MARK_UNSAFE_DEPRECATED_MESSAGE)]]
constexpr auto unsafe(T& value) { return detail::unsafe<T>{value}; }

template <typename T> [[deprecated(UTL_MARK_UNSAFE_DEPRECATED_MESSAGE)]]
constexpr auto unsafe_ref(T& value) { return detail::unsafe_ref<T>{value}; }

template <typename T> [[deprecated(UTL_MARK_UNSAFE_DEPRECATED_MESSAGE)]]
constexpr auto unsafe_cref(T const& value) { return detail::unsafe_ref<const T>{value}; }

template <typename T>
struct _bind_unwrap<detail::unsafe<T>&&> { 
    using type = decltype(std::declval<detail::unsafe<T>&&>().value); 
};

template <typename T>
struct _bind_unwrap<detail::unsafe_ref<T>&&> { 
    using type = decltype(std::declval<detail::unsafe_ref<T>&&>().value); 
};

} //namespace utl::irq
