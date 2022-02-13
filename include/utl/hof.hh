// Copyright 2021 George Harris
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
    
template <typename T>
concept any_tuple = true;



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

namespace detail {
    struct fold_operator_tag_t {};

    template <typename T, typename O>
    using add_fold_operator_t = tuple<fold_operator_tag_t&&, T, O>;    

    constexpr decltype(auto) add_fold_operator(auto&& item, auto&& op)
    {
        using item_t = decltype(item);
        using operator_t = decltype(op);
        return forward_as_tuple(
            fold_operator_tag_t{},
            std::forward<item_t>(item),
            std::forward<operator_t>(op)
        );
    }

    template <typename A, typename T, typename O>
        requires std::invocable<O,A&&,T>
    constexpr auto operator+(A&& accum, add_fold_operator_t<T,O>&& item)
    {
        constexpr decltype(auto) get_user_operator = [](auto&& item_) 
        { 
            return get<2>(item_); 
        };

        constexpr decltype(auto) get_input_item = [](auto&& item_) 
        { 
            return get<1>(item_); 
        };

        return get_user_operator(item) (
            std::forward<decltype(accum)>(accum),
            get_input_item(item)
        );
    }

} //namespace detail

constexpr auto fold(auto&& r, auto&& binop, auto&& id)
{
    using range_t = decltype(r);
    using binop_t = decltype(binop);
    using identity_t = decltype(id);

    constexpr size_t size = std::tuple_size_v<std::decay_t<range_t>>;
    if constexpr(size == 0) {
        return id;
    } else {
        constexpr auto impl = []<size_t... Is>(auto&& r_, auto&& op_, auto&& id_, std::index_sequence<Is...>) {
            return (
                id_ + ... + detail::add_fold_operator(
                    get<Is>(std::forward<range_t>(r_)), 
                    std::forward<binop_t>(op_)
                )
            );
        };

        return impl(
            std::forward<range_t>(r), 
            std::forward<binop_t>(binop), 
            std::forward<identity_t>(id),
            std::make_index_sequence<size>{}
        );
    }
}

template <typename T, typename F, typename I>
using fold_result_t = decltype(fold(std::declval<T>(), std::declval<F>(), std::declval<I>()));

constexpr auto identity = [](auto&& v) { return v; };
constexpr auto binary_and = [](bool a, bool b) { return a and b; };



namespace detail {
    template <any_tuple T, template <typename,typename> typename K, typename Accum, typename U>
    struct fold_types;

    template <any_tuple T, template <typename,typename> typename K, typename Accum, size_t I, size_t... Is>
    struct fold_types<T,K,Accum,std::index_sequence<I,Is...>> {
        using this_element_t = std::tuple_element_t<I,T>;
        using this_fold_t = typename K<Accum, this_element_t>::type;
        using type = typename fold_types<T, K, this_fold_t, std::index_sequence<Is...>>::type;
    };

    template <any_tuple T, template <typename,typename> typename K, typename Accum, size_t I>
    struct fold_types<T,K,Accum,std::index_sequence<I>> {
        using this_element_t = std::tuple_element_t<I,T>;
        using this_fold_t = typename K<Accum, this_element_t>::type;
        using type = this_fold_t;
    };
} //namespace detail

template <any_tuple T, template <typename,typename> typename K, typename Init>
struct fold_types : detail::fold_types<T,K,Init,std::make_index_sequence<std::tuple_size_v<T>>> {};

template <any_tuple T, template <typename,typename> typename K, typename Init>
using fold_types_t = typename fold_types<T,K,Init>::type;

namespace fold_test {
    
    template <any_tuple Accum, typename Item>
    struct null {
        //FIXME: how to get the container's type?
        using type = tuple<>;
    };

    template <any_tuple Accum, typename Item>
    struct identity {
        using type = decltype(tuple_cat(std::declval<Accum>(),std::declval<tuple<Item>>()));
    };

    template <any_tuple Accum, typename Item>
    struct reverse { 
        using type = decltype(tuple_cat(std::declval<tuple<Item>>(),std::declval<Accum>())); 
    };

    namespace tuples {
        using input_t = tuple<int,float,bool,char>;
        using initial_t = tuple<>;

        using reverse_folded_t = fold_types_t<input_t,reverse,initial_t>;
        static_assert(std::same_as<tuple<char,bool,float,int>, reverse_folded_t>);

        using identity_folded_t = fold_types_t<input_t,identity,initial_t>;
        static_assert(std::same_as<input_t, identity_folded_t>);

        using null_folded_t = fold_types_t<input_t,null,initial_t>;
        static_assert(std::same_as<tuple<>, null_folded_t>);
    } //namespace tuples
};

} //namespace hof

template <bool V>
struct foo {
    static constexpr auto v = V;
};

constexpr auto my_tuple_cat(auto&&... args)
{
    constexpr auto cat_op_impl = []<size_t... Is, size_t... Js>(auto&& accum, auto&& item, 
        std::index_sequence<Is...>, std::index_sequence<Js...>)
    {
        return utl::tuple{
            get<Is>(std::forward<decltype(accum)>(accum))..., 
            get<Js>(std::forward<decltype(item)>(item))...
        }; 
    };

    constexpr auto cat_op = [cat_op_impl](auto&& accum, auto&& item)
    {
        constexpr size_t accum_size = std::tuple_size_v<std::decay_t<decltype(accum)>>;
        constexpr size_t item_size = std::tuple_size_v<std::decay_t<decltype(item)>>;
        return cat_op_impl(
            std::forward<decltype(accum)>(accum),
            std::forward<decltype(item)>(item),
            std::make_index_sequence<accum_size>{},
            std::make_index_sequence<item_size>{});
    };

    return utl::hof::fold(
        utl::tuple{std::forward<decltype(args)>(args)...},
        cat_op,
        utl::tuple{}
    );
}
