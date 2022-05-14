// Copyright 2022 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/tuple.hh>
#include <stddef.h>
#include <utility>
#include <bfg/tag_invoke.h>
#include <array>

namespace utl::hof {

template <size_t... Is>
constexpr auto map_impl(auto&& r, auto&& func, std::index_sequence<Is...>)
{
    //FIXME: how to return in the original container type?
    // heterogenous map means that the return type might not match!
    return tuple{func(get<Is>(r))...};
}

constexpr auto map(auto&& r, auto&& func)
{
    using func_t = decltype(func);
    using range_t = decltype(r);
    constexpr auto size = std::tuple_size_v<std::decay_t<range_t>>;
    return map_impl(
        std::forward<range_t>(r),
        std::forward<func_t>(func), 
        std::make_index_sequence<size>{}
    );
}

template <typename F, typename T>
concept any_map_over = requires(F func, T v) {
    map(func,v);
};    

template <typename T, typename F>
using map_result_t = std::invoke_result_t<decltype(map<T,F>),T,F>;

template <typename U, typename F>
using mapped_element_t = std::invoke_result_t<F,size_t,U>;


template <any_tuple T, template <typename> typename K, typename U>
struct map_types;

template <any_tuple T, template <typename> typename K, size_t... Is>
struct map_types<T,K,std::index_sequence<Is...>> {
    //FIXME: how to get the original container type?
    using type = tuple<typename K<std::tuple_element_t<Is,T>>::type...>;
};

template <any_tuple T, template <typename> typename K>
using type_map_t = typename map_types<T,K,std::make_index_sequence<std::tuple_size_v<T>>>::type;

namespace map_test {

    template <typename T>
    struct my_map { using type = bool; };
    template <>
    struct my_map<int> { using type = int*; };

    using input_t = tuple<float,int,char>;
    using mapped_t = type_map_t<input_t, my_map>;
    static_assert(std::same_as<tuple<bool,int*,bool>, mapped_t>);

} //namespace test

} //namespace utl::hof
