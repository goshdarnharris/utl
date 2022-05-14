// Copyright 2022 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once
#include <utl/utl.hh>
#include <utl/integer.hh>
#include <bit>

namespace utl {

template <typename T>
concept any_bitset = requires(T v) {
    // utl::integer::unsigned_cast(v); //It can be cast to a uintn_t<>
    v.value(); //It has a value
    v.width(); //It has a width
    //That value can be cast to a uintn_t<>,
    utl::integer::unsigned_cast(v.value());
    //It can be explicitly cast to its value type
    // static_cast<decltype(v.value())>(v);
};

template <any_bitset T>
using bitset_value_t = decltype(std::declval<std::decay_t<T>>().value());

template <typename T>
concept any_enumerated_bitset = any_bitset<T> and std::is_enum_v<bitset_value_t<T>>;


template <utl::integer::convertible_to_unsigned T>
struct bitset {
    using value_t = T;
    T val;

    constexpr T& value()
    {
        return val;
    }

    constexpr T const& value() const
    {
        return val;
    }

    static constexpr size_t width()
    {
        return utl::integer::width<T>();
    }

    explicit constexpr operator auto() const
    {
        return utl::integer::unsigned_cast(val);
    }

    constexpr auto& operator=(T const& v) 
    { 
        val = v; 
        return *this;
    }

    constexpr auto& operator=(T&& v) 
    { 
        val = v; 
        return *this;
    }
};

static_assert(any_bitset<bitset<uint8_t>>);

// template <typename T>
// concept any_bitspan = requires(T v) {

// };

template <utl::integer::convertible_to_unsigned W, utl::integer::convertible_to_unsigned T, size_t O>
class bitspan {
    private:
        using word_t = W;
        using underlying_t = T;
        static constexpr size_t bitset_width = utl::integer::width<underlying_t>();
        static constexpr std::decay_t<word_t> bitset_mask = (1 << bitset_width) - 1;

        word_t& m_word;

        constexpr auto& set_value(auto&& value)
        {
            const word_t set_mask = word_t{value.value()} << O;
            const word_t clear_mask = (~set_mask) & (bitset_mask << O);
            m_word = (m_word | set_mask) & (~clear_mask);
            return *this;
        }
    public:
        constexpr bitspan(word_t& w) : m_word{w} {}

        static constexpr size_t offset()
        {
            return O;
        }

        static constexpr size_t width()
        {
            return utl::integer::width<underlying_t>();
        }

        constexpr underlying_t value() const
        {
            return static_cast<underlying_t>((m_word >> O) & bitset_mask);
        }

        //explicitly convertible to a bitset with the same underlying type
        explicit constexpr operator bitset<underlying_t>() const
        {
            return {value()};
        }

        //assignable from the bitset or underlying type
        constexpr auto& operator=(underlying_t const& value) { return operator=(bitset<underlying_t>{value}); }
        constexpr auto& operator=(underlying_t&& value) { return operator=(bitset<underlying_t>{value}); }
        constexpr auto& operator=(bitset<underlying_t> const& value) { return set_value(value); }
        constexpr auto& operator=(bitset<underlying_t>&& value) { return set_value(value); }
};

static_assert(any_bitset<bitspan<uint32_t,uint8_t,0>>);

} //namespace utl
