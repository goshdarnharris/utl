// Copyright 2022 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/hof/fold.hh>

namespace utl::hof {

template <typename... Fs>
struct composed {
    utl::tuple<Fs...> callables;
    
    constexpr decltype(auto) operator()(auto&& initial) const {
        return foldr(
            callables,
            std::forward<decltype(initial)>(initial),
            [](auto&& f, auto&& arg) -> decltype(auto) {
                return f(std::forward<decltype(arg)>(arg));
            }
        );
    }
};

constexpr auto compose(auto&&... callables)
{
    return composed<std::decay_t<decltype(callables)>...>{
        {std::forward<decltype(callables)>(callables)...}
    };
}

// constexpr auto my_tuple_cat(auto&&... args)
// {
//     constexpr auto cat_op_impl = []<size_t... Is, size_t... Js>(auto&& accum, auto&& item, 
//         std::index_sequence<Is...>, std::index_sequence<Js...>)
//     {
//         return utl::tuple{
//             get<Is>(std::forward<decltype(accum)>(accum))..., 
//             get<Js>(std::forward<decltype(item)>(item))...
//         }; 
//     };

//     constexpr auto cat_op = [cat_op_impl](auto&& accum, auto&& item)
//     {
//         constexpr size_t accum_size = std::tuple_size_v<std::decay_t<decltype(accum)>>;
//         constexpr size_t item_size = std::tuple_size_v<std::decay_t<decltype(item)>>;
//         return cat_op_impl(
//             std::forward<decltype(accum)>(accum),
//             std::forward<decltype(item)>(item),
//             std::make_index_sequence<accum_size>{},
//             std::make_index_sequence<item_size>{});
//     };

//     return utl::hof::fold(
//         utl::tuple{std::forward<decltype(args)>(args)...},
//         utl::tuple{},
//         cat_op
//     );
// }

} //namespace utl::hof
