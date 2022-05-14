// Copyright 2022 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/hof/fold.hh>

namespace utl::hof {

namespace detail {
    template <typename T, typename V>
    struct find_accumulate_t {
        T accumulated;
        V* item;
        constexpr bool found() { return not (item == nullptr); }
        constexpr auto extract()
        {
            return tuple<T,V*>{accumulated,item};
        }
    };

    template <typename T, typename V>
    find_accumulate_t(T&&,V*) -> find_accumulate_t<std::remove_reference_t<T>,V>;

    template <typename T>
    find_accumulate_t(T&&,nullptr_t) -> find_accumulate_t<std::remove_reference_t<T>,void>;

    template <auto A, typename T>
    struct find_accumulate_heterogenous_t {
        static constexpr auto accumulated = A;
        using found_t = T;
        static constexpr bool found() { return not std::same_as<void,found_t>; }
    };


    template <auto&& P, auto&& A>
    constexpr auto make_find_heterogenous_binop()
    {        
        //FIXME: this doesn't work, because item can't be used in a constant
        //expression even if the value isn't actually used by accumulate/predicate.
        return [&](auto accum, auto item) {
            constexpr auto predicate = P;
            constexpr auto accumulate = A;
            using accum_t = decltype(accum);
            using item_t = decltype(item);
            // using accum_t = std::decay_t<decltype(accum)>;
            constexpr auto accumulated = accumulate(accum_t::accumulated,item);
            if constexpr(not accum.found() and predicate(accum_t::accumulated,item)) {
                return find_accumulate_heterogenous_t<accumulated,item_t>{};
            }
            return find_accumulate_heterogenous_t<accumulated,typename accum_t::found_t>{};
        };
    }

    constexpr auto make_find_homogenous_binop(auto&& predicate, auto&& accumulate)
    {
        //FIXME: this needs to end up returning a variant type that can
        //hold anything in the tuple-like.
        return [&](auto accum, auto item) {
            // using accum_t = std::decay_t<decltype(accum)>;
            auto accumulated = accumulate(accum.accumulated, item);
            if(!accum.found() and predicate(accum.accumulated,item)) {
                return find_accumulate_t{accumulated,&item};
            }
            return find_accumulate_t{accumulated,accum.item};
        };
    }
} //namespace detail


constexpr auto find(any_tuple auto&& r, auto initial, auto&& predicate, auto&& accumulate)
{
    return fold(r, detail::find_accumulate_t{std::forward<decltype(initial)>(initial), nullptr}, 
        detail::make_find_homogenous_binop(predicate,accumulate)).extract();
}

template <any_tuple T, typename Init, auto P, auto A>
using find_result_t = decltype(find(std::declval<T>(), std::declval<Init>(), P, A));

template <typename T, auto Init, auto P, auto A>
using find_type_t = hof::fold_result_t<T,
    detail::find_accumulate_heterogenous_t<Init,void>,
    decltype(detail::make_find_heterogenous_binop<P,A>())
>;

} //namespace utl::hof
