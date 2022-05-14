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
#include <utl/utility.hh>

namespace utl::hof {

namespace detail {
    enum class direction {
        LEFT,
        RIGHT
    };
    template <direction D>
    struct fold_operator_tag_t {};    

    template <direction D>
    constexpr decltype(auto) add_fold_operator(auto&& item, auto&& op)
    {
        using item_t = decltype(item);
        using operator_t = decltype(op);
        return forward_as_tuple(
            fold_operator_tag_t<D>{},
            std::forward<item_t>(item),
            std::forward<operator_t>(op)
        );
    }

    template <typename T, typename O, direction D>
    using add_fold_operator_t = utl::tuple<fold_operator_tag_t<D>&&,T,O>;
    // decltype(add_fold_operator(std::declval<T>(),std::declval<O>()));

    

    template <typename A, typename T, typename O>
        // requires std::invocable<O,A&&,T> //FIXME: this being down here makes for terrible error messages
        //if you're looking here, then there's probably something wrong with the operator you're giving
        //to hof::fold
    constexpr decltype(auto) operator+(A&& accum, add_fold_operator_t<T,O,direction::LEFT> item)
    {
        constexpr auto get_user_operator = [](auto&& item_) -> decltype(auto)
        { 
            return get<2>(item_); 
        };

        constexpr auto get_input_item = [](auto&& item_) -> decltype(auto)
        { 
            return get<1>(item_); 
        };

        return get_user_operator(item) (
            std::forward<decltype(accum)>(accum),
            get_input_item(item)
        );
    }

    template <typename A, typename T, typename O>
        // requires std::invocable<O,A&&,T> //FIXME: this being down here makes for terrible error messages
        //if you're looking here, then there's probably something wrong with the operator you're giving
        //to hof::fold
    constexpr decltype(auto) operator+(add_fold_operator_t<T,O,direction::RIGHT> item, A&& accum)
    {
        constexpr auto get_user_operator = [](auto&& item_) -> decltype(auto)
        { 
            return get<2>(item_); 
        };

        constexpr auto get_input_item = [](auto&& item_) -> decltype(auto)
        { 
            return get<1>(item_); 
        };

        return get_user_operator(item) (
            get_input_item(item),
            std::forward<decltype(accum)>(accum)
        );
    }

} //namespace detail

constexpr decltype(auto) foldl(any_tuple auto&& r, auto&& id, auto&& binop)
{
    using range_t = decltype(r);
    using binop_t = decltype(binop);
    using identity_t = decltype(id);

    constexpr size_t size = std::tuple_size_v<std::decay_t<range_t>>;
    if constexpr(size == 0) {
        return id;
    } else {
        constexpr auto impl = []<size_t... Is>(auto&& r_, auto&& id_, auto&& op_, std::index_sequence<Is...>) -> decltype(auto)
        {
            return 
                (id_ + ... + 
                    detail::add_fold_operator<detail::direction::LEFT>(
                        get<Is>(std::forward<range_t>(r_)), 
                        std::forward<binop_t>(op_)
                    )
                );
            
        };

        return impl(
            std::forward<range_t>(r), 
            std::forward<identity_t>(id),
            std::forward<binop_t>(binop), 
            std::make_index_sequence<size>{}
        );
    }
}

constexpr auto foldr(any_tuple auto&& r, auto&& id, auto&& binop)
{
    using range_t = decltype(r);
    using binop_t = decltype(binop);
    using identity_t = decltype(id);

    constexpr size_t size = std::tuple_size_v<std::decay_t<range_t>>;
    if constexpr(size == 0) {
        return id;
    } else {
        constexpr auto impl = []<size_t... Is>(auto&& r_, auto&& id_, auto&& op_, std::index_sequence<Is...>) {
            return (
                detail::add_fold_operator<detail::direction::RIGHT>(
                    get<Is>(std::forward<range_t>(r_)), 
                    std::forward<binop_t>(op_)
                ) + ... + id_
            );
        };

        return impl(
            std::forward<range_t>(r), 
            std::forward<identity_t>(id),
            std::forward<binop_t>(binop), 
            std::make_index_sequence<size>{}
        );
    }
}

template <typename T, typename I, typename F>
using foldl_result_t = decltype(foldl(std::declval<T>(), std::declval<I>(), std::declval<F>()));

template <typename T, typename I, typename F>
using foldr_result_t = decltype(foldr(std::declval<T>(), std::declval<I>(), std::declval<F>()));

constexpr auto identity = [](auto&& v) { return v; };
constexpr auto binary_and = [](bool a, bool b) { return a and b; };

namespace detail {
    template <any_tuple T, typename Accum, template <typename,typename> typename B, typename U>
    struct fold_types;

    template <any_tuple T, typename Accum, template <typename,typename> typename B, size_t I, size_t... Is>
    struct fold_types<T,Accum,B,std::index_sequence<I,Is...>> {
        using this_element_t = std::tuple_element_t<I,T>;
        using this_fold_t = typename B<Accum, this_element_t>::type;
        using type = typename fold_types<T, this_fold_t, B, std::index_sequence<Is...>>::type;
    };

    template <any_tuple T, typename Accum, template <typename,typename> typename B, size_t I>
    struct fold_types<T,Accum,B,std::index_sequence<I>> {
        using this_element_t = std::tuple_element_t<I,T>;
        using this_fold_t = typename B<Accum, this_element_t>::type;
        using type = this_fold_t;
    };
} //namespace detail

template <any_tuple T, typename Init, template <typename,typename> typename B>
struct fold_types : detail::fold_types<T,Init,B,std::make_index_sequence<std::tuple_size_v<T>>> {};

template <any_tuple T, typename Init, template <typename,typename> typename B>
using fold_types_t = typename fold_types<T,Init,B>::type;

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

        using reverse_folded_t = fold_types_t<input_t,initial_t,reverse>;
        static_assert(std::same_as<tuple<char,bool,float,int>, reverse_folded_t>);

        using identity_folded_t = fold_types_t<input_t,initial_t,identity>;
        static_assert(std::same_as<input_t, identity_folded_t>);

        using null_folded_t = fold_types_t<input_t,initial_t,null>;
        static_assert(std::same_as<tuple<>, null_folded_t>);
    } //namespace tuples
};

} //namespace utl::hof
