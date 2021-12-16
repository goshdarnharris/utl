#pragma once

#include <stddef.h>
#include <utility>
#include <bfg/tag_invoke.h>
#include <utl/integer.hh>
#include <utl/tuple.hh>

namespace utl::registers::reg {



template <typename T>
concept any_register = requires(T r) {
    r.value();
    utl::integer::width(r.value());
};

template <any_register T>
using value_t = decltype(T::value());

template <typename T, typename R>
concept same_register_as = std::same_as<std::decay_t<R>, std::decay_t<T>>;

template <typename... Ts>
concept same_target_register = ((any_register<Ts> and same_register_as<tuple_element_t<0,tuple<Ts...>>,Ts>) and ...);


BFG_TAG_INVOKE_DEF(write);
BFG_TAG_INVOKE_DEF(read);




template <typename T>
concept any_memory_mapped_register = any_register<T>
    and requires(T r) {
        { r.location() } -> std::same_as<volatile value_t<T>*>;
};

constexpr void tag_invoke(write_t, any_memory_mapped_register auto r, value_t<decltype(r)> v)
{
    *r.location() = v;
}

constexpr auto tag_invoke(read_t, any_memory_mapped_register auto r)
{
    return *r.location();
}

template <size_t W, integer::uintn_t<W>* A>
struct memory_mapped {
    // static constexpr size_t width = W;
    using value_t = integer::uintn_t<W>;
    // static volatile constexpr value_t* location = A;

    static constexpr volatile value_t* location() { return A; }
    static constexpr size_t width() { return W; }
    static constexpr value_t value() { return *location(); }
};


} //namespace utl::registers
