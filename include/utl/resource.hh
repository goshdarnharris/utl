// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utility>

namespace utl {

template <typename T>
concept any_resource = 
    std::is_const_v<std::remove_reference_t<T>> or (
        std::movable<std::decay_t<T>>
        and not std::copyable<T>
    );

template <typename T>
concept any_owned_resource = any_resource<T>
    and not std::is_rvalue_reference_v<T&&>
    and not std::is_const_v<std::remove_reference_t<T>>;

template <typename T>
concept any_unowned_resource = any_resource<T>
    and std::is_rvalue_reference_v<T&&>
    and not std::is_const_v<std::remove_reference_t<T>>;

template <typename T>
concept take_resource = any_unowned_resource<T>;

template <typename T>
concept view_resource = any_resource<T> 
    and std::is_lvalue_reference_v<T&&> 
    and std::is_const_v<std::remove_reference_t<T>>;

template <typename T>
concept something = any_resource<T>
    and std::is_rvalue_reference_v<T&&>
    and std::is_const_v<std::remove_reference_t<T>>;

auto&& give(any_owned_resource auto&& r)
{
    // static_assert(print_type<decltype(r)>);
    return std::move(r);
}

auto&& give(any_resource auto&& r)
{
    static_assert(any_owned_resource<decltype(r)>,
        "you can't give a resource you don't own"
    );
    return std::move(r);
}

auto&& give(view_resource auto&& r)
{
    static_assert(any_owned_resource<decltype(r)>,
        "you can't give a resource view (because you don't own the resource)"
    );
    return std::move(r);
}


const auto& view(any_resource auto&& r) 
{    
    // static_assert(print_type<decltype(r)>);
    return r; 
}

struct resource {
    constexpr resource() = default;
    constexpr resource(resource const&) = delete;
    constexpr resource(resource&) = delete;
    constexpr resource(resource&&) = default;

    constexpr auto operator=(resource const&) = delete;
    constexpr resource& operator=(resource&&) = default;
    // constexpr ~resource() = delete;
    // friend constexpr auto operator <=>(resource const&, resource const&)
    // {
    //     return std::partial_ordering::unordered;
    // }
};

namespace test {
    auto invoke(take_resource auto&& r)
    {
        return r;
    }

    auto invoke(view_resource auto&& r)
    {}

    template <typename T>
    concept callable = requires(T v) {
        invoke(v);
    };
}

static_assert(!test::callable<resource>);
static_assert(!test::callable<resource&>);
static_assert(test::callable<decltype(utl::view(r))>);


};

