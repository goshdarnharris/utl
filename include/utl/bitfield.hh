// Copyright 2021 George Harris
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
concept any_bitfield = requires(T v) {
    // utl::integer::unsigned_cast(v); //It can be cast to a uintn_t<>
    v.value(); //It has a value
    v.width(); //It has a width
    //That value can be cast to a uintn_t<>,
    utl::integer::unsigned_cast(v.value());
    //It can be explicitly cast to its value type
    // static_cast<decltype(v.value())>(v);
};

template <any_bitfield T>
using bitfield_value_t = decltype(std::declval<std::decay_t<T>>().value());

template <typename T>
concept any_enumerated_bitfield = any_bitfield<T> and std::is_enum_v<bitfield_value_t<T>>;


template <utl::integer::convertible_to_unsigned T>
struct bitfield {
    T val;

    constexpr T value()
    {
        return val;
    }

    constexpr size_t width()
    {
        return utl::integer::width<T>();
    }

    explicit constexpr operator auto() const
    {
        return utl::integer::unsigned_cast(val);
    }

    constexpr auto& operator=(T v) 
    { 
        val = v; 
        return *this;
    }
};

static_assert(any_bitfield<bitfield<uint8_t>>);

// template <typename T>
// concept any_bitspan = requires(T v) {

// };

template <utl::integer::convertible_to_unsigned T, utl::integer::convertible_to_unsigned U, size_t O>
class bitspan {
    private:
        using word_t = T;
        using underlying_t = U;
        static constexpr size_t bitfield_width = utl::integer::width<underlying_t>();
        static constexpr word_t bitfield_mask = (1 << bitfield_width) - 1;

        word_t& m_word;
    public:
        constexpr bitspan(word_t& w) : m_word{w} {}

        constexpr size_t offset()
        {
            return O;
        }

        constexpr size_t width()
        {
            return utl::integer::width<U>();
        }

        constexpr underlying_t value()
        {
            auto v = utl::integer::unsigned_cast(m_word);
            return {(v >> O) && bitfield_mask};
        }
        //explicitly convertible to the underlying type
        explicit constexpr operator underlying_t() const
        {
            return value();
        }

        //explicitly convertible to a bitfield with the same underlying type
        explicit constexpr operator bitfield<U>() const
        {
            return {static_cast<underlying_t>(*this)};
        }

        //assignable from the bitfield or underlying type
        constexpr auto& operator=(underlying_t value)
        {
            return operator=(bitfield<U>{value});
        }

        constexpr auto& operator=(bitfield<U> value)
        {
            const word_t set_mask = word_t{utl::integer::unsigned_cast(value)} << O;
            const word_t clear_mask = (~set_mask) & bitfield_mask;
            m_word = (m_word | set_mask) & ~clear_mask; 
            return *this;
        }
};

static_assert(any_bitfield<bitspan<uint32_t,uint8_t,0>>);


template <typename... Ts>
struct packed {
    static constexpr size_t width = (utl::integer::width<Ts>() + ...);
    using value_t = uintn_t<width>;
    value_t value;
};


// struct my_bitstruct : packed<uintn_t<3>,uintn_t<6>> {
//     packed<uintn_t<3>, uintn_t<6>>& value;
//     bitspan<my_bitstruct,0> CONF{*this, 1_u3};
//     bitspan<my_bitstruct,3> FLOOB{*this, 7_u6};
// };

// template <typename T, size_t O>
// constexpr auto make_bitspan(utl::integer::convertible_to_unsigned auto& v)
// {
//     return bitspan<std::decay_t<decltype(v)>,T,O>{v};
// }

// template <size_t O, typename... Ts>
// constexpr auto make_bitspan(packed<Ts..>& v)
// {
//     return bitspan<std::decay_t<decltype(v)>,T,O>{v.value};
// }

// template <typename T, size_t O>
// using underlying_type_at_offset_t = 

// //need to implement integer width for the packed type...

// template <typename T, size_t O>
// using packed_bitspan_t = bitspan<typename T::value_t, underlying_type_at_offset_t<T,O>, O>;




// template <typename T>
// constexpr auto apply_offset(bitfield<T> bf, size_t offset)
// {
//     return utl::integer::unsigned_cast(bf) << offset;
// }

// template <size_t N, typename T>
// constexpr auto apply_offset(bitfield<T> bf)
// {
//     return utl::integer::unsigned_cast(bf) << N;
// }

template <any_enumerated_bitfield T>
using options_t = bitfield_value_t<T>;


// template <size_t W>
// struct bitfield : public bitfield<uintn_t<W>> {};

} //namespace utl
