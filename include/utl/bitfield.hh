// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/utl.hh>
#include <utl/bitset.hh>
#include <utl/hof/fold.hh>
#include <concepts>
#include <utl/utility.hh>

namespace utl {

template <typename T>
concept any_bitfield = any_bitset<T> and requires {
    typename T::tag_t;
    { T::tag } -> std::same_as<const typename T::tag_t&>;
};

template <auto N, utl::integer::convertible_to_unsigned T>
struct bitfield : public bitset<T> {
    using tag_t = std::decay_t<decltype(N)>;
    static constexpr auto tag = N;

    template <utl::any_enum auto V>
    static constexpr bool matches_tag()
    {
        using value_t = std::decay_t<decltype(V)>;
        if constexpr(std::same_as<tag_t,value_t>) {
            return V == tag;
        }
        return false;
    }
};

template <auto N, utl::integer::convertible_to_unsigned W, utl::integer::convertible_to_unsigned T, size_t O>
struct bitfield_view : public bitspan<W,T,O> {
    using bitspan<W,T,O>::operator=;
    using tag_t = std::decay_t<decltype(N)>;
    static constexpr auto tag = N;

    template <utl::any_enum auto V>
    static constexpr bool matches_tag()
    {
        using value_t = std::decay_t<decltype(V)>;
        if constexpr(std::same_as<tag_t,value_t>) {
            return V == tag;
        }
        return false;
    }
};

template <typename T, auto V>
concept has_field = utl::any_enum<decltype(V)> and requires() {
    std::decay_t<T>::template find_field_by_enum_tag<V>();
} and (std::decay_t<T>::template find_field_by_enum_tag<V>().found());

template <typename T>
concept any_bitstruct = requires(T v) {
    { v.width() } -> std::same_as<size_t>;
    //test if width is constexpr
    typename std::decay_t<T>::value_t;
    typename std::decay_t<T>::tag_t;
    { v.value } -> utl::integer::convertible_to_unsigned;
    // { v.value } -> std::common_with<typename T::value_t>;
} and (utl::integer::width<typename std::decay_t<T>::value_t>() == std::decay_t<T>::width());

template <auto O, typename T>
struct accumulator {
    static constexpr size_t offset = O;
    using found_t = T;
    static constexpr bool found() { return not std::same_as<void,found_t>; }
};

template <any_bitfield T, any_bitfield... Ts>
    requires (std::common_with<typename T::tag_t, typename Ts::tag_t> and ...)
struct bitstruct {
    static constexpr size_t width() { return T::width() + (Ts::width() + ... + 0); }
    using tag_t = std::common_type_t<typename T::tag_t, typename Ts::tag_t...>;
    
    uintn_t<width()> value;
    using value_t = std::remove_reference_t<decltype((value))>;

    template <utl::any_enum auto V>
    static constexpr auto find_field_by_enum_tag()
    {
        //FIXME: this should be generalized; it's handy   
        constexpr auto find_accumulate = [](auto&& accum, auto&& item) {
            using item_t = std::decay_t<decltype(item)>;
            using accum_t = std::decay_t<decltype(accum)>;

            if constexpr(item_t::template matches_tag<V>()) {
                return accumulator<accum_t::offset,item_t>{};
            } else if constexpr(not accum_t::found()) {
                return accumulator<accum_t::offset + item_t::width(), typename accum_t::found_t>{};
            } else {
                return accum;
            }
        };

        using accum_t = hof::foldl_result_t<tuple<T,Ts...>&, accumulator<0,void>, decltype(find_accumulate)>;
        // static_assert(accum_t::found(), "bitstruct does not contain a field matching provided tag");
        
        return accum_t{};
    }

    constexpr bitstruct() = default;
    constexpr bitstruct(bitstruct const&) = default;
    constexpr bitstruct(bitstruct const volatile& other) : value{other.value} {}
    constexpr bitstruct(bitstruct&& other) = default;

    constexpr auto& operator=(bitstruct const& other)
    {
        value = other.value;
        return *this;
    }

    constexpr auto& operator=(bitstruct&& other)
    {
        value = other.value;
        return *this;
    }

    constexpr auto& operator=(bitstruct const& other) volatile
    {
        value = other.value;
        return *this;
    }

    constexpr auto& operator=(bitstruct&& other) volatile
    {
        value = other.value;
        return *this;
    }
};

template <utl::any_enum auto V>
constexpr auto get_field(any_bitstruct auto&& b)
{
    using accum_t = std::decay_t<decltype(b.template find_field_by_enum_tag<V>())>;
    using field_value_t = typename accum_t::found_t::value_t;
    using bitstruct_value_t = std::remove_reference_t<decltype((b.value))>;
    constexpr auto field_offset = accum_t::offset;

    return bitfield_view<V,bitstruct_value_t,field_value_t,field_offset>{b.value};
}

template <utl::any_enum auto V, any_bitstruct T>
using field_value_t = typename std::remove_reference_t<decltype(std::remove_reference_t<T>::template find_field_by_enum_tag<V>())>::found_t::value_t;

namespace test {
    enum class fields_a: uint8_t {
        FIELD_A,
        FIELD_B,
        FIELD_C,
        FIELD_D
    };

    using test_struct_t = bitstruct<
        bitfield<fields_a::FIELD_A, uintn_t<3>>,
        bitfield<fields_a::FIELD_B, uintn_t<2>>,
        bitfield<fields_a::FIELD_C, uintn_t<5>>,
        bitfield<fields_a::FIELD_D, uintn_t<13>>
    >;

    constexpr auto test_struct = test_struct_t{};

    static_assert(get_field<fields_a::FIELD_A>(test_struct).offset() == 0);
    static_assert(get_field<fields_a::FIELD_A>(test_struct).width() == 3);
    static_assert(get_field<fields_a::FIELD_B>(test_struct).offset() == 3);
    static_assert(get_field<fields_a::FIELD_B>(test_struct).width() == 2);
    static_assert(get_field<fields_a::FIELD_C>(test_struct).offset() == 5);
    static_assert(get_field<fields_a::FIELD_C>(test_struct).width() == 5);
    static_assert(get_field<fields_a::FIELD_D>(test_struct).offset() == 10);
    static_assert(get_field<fields_a::FIELD_D>(test_struct).width() == 13);

    static_assert(std::same_as< 
        bitfield_view<fields_a::FIELD_C, const uintn_t<23>, uintn_t<5>, 5>, 
        std::decay_t<decltype(get_field<fields_a::FIELD_C>(test_struct))>
    >);
} //namespace test

// template <any_bitstruct T>
// constexpr size_t width()
// {
//     return T::width();
// }




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
// constexpr auto apply_offset(bitset<T> bf, size_t offset)
// {
//     return utl::integer::unsigned_cast(bf) << offset;
// }

// template <size_t N, typename T>
// constexpr auto apply_offset(bitset<T> bf)
// {
//     return utl::integer::unsigned_cast(bf) << N;
// }

// template <any_enumerated_bitset T>
// using options_t = bitset_value_t<T>;


// template <size_t W>
// struct bitset : public bitset<uintn_t<W>> {};

} //namespace utl
