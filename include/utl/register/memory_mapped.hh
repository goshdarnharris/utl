// Copyright 2022 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/register/register.hh>
#include <utl/integer.hh>

namespace utl::registers {

template <typename T>
concept any_memory_mapped_register = any_register<T> and
    requires(T r) {
        { r.location() } -> std::same_as<volatile value_t<T>*>;
    };



namespace memory_mapped {

    struct mm_base {};

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    struct mm_shared : mm_base {
        using value_t = std::remove_pointer_t<decltype(A)>;

        static constexpr volatile value_t* location() { return A; }
        static constexpr size_t width() { return utl::integer::width<value_t>(); }
        static constexpr value_t value() { return *location(); }
        static constexpr value_t reset_value() { return R; }
    };

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    struct read_only : public mm_shared<A,R> {};

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    struct write_only : public mm_shared<A,R> {};

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    struct read_write : public mm_shared<A,R> {};

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    struct no_access : public mm_shared<A,R> {};

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    constexpr void tag_invoke(write_t, write_only<A,R> r, value_t<decltype(r)> v)
    {
        *r.location() = v;
    }

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    constexpr void tag_invoke(write_t, read_write<A,R> r, value_t<decltype(r)> v)
    {
        *r.location() = v;
    }

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    constexpr auto tag_invoke(read_t, read_only<A,R> r)
    {
        return *r.location();
    }

    template <auto* A, std::remove_pointer_t<decltype(A)> R>
        requires utl::integer::convertible_to_unsigned<std::remove_pointer_t<decltype(A)>>
    constexpr auto tag_invoke(read_t, read_write<A,R> r)
    {
        return *r.location();
    }

} //namespace memory_mapped

} //namespace utl::registers
