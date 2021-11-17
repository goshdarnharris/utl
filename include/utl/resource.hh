#pragma once

#include <utility>

namespace utl {

template <typename T>
concept any_unowned_resource = std::is_rvalue_reference_v<T&&>;

template <typename T>
concept any_owned_resource = not std::is_rvalue_reference_v<T&&>;

auto&& give(any_owned_resource auto&& r)
{
    return std::move(r);
}

struct resource {
    constexpr resource() = default;
    constexpr resource(resource const&) = delete;
    constexpr resource(resource&) = delete;
    constexpr resource(resource&&) = default;
    constexpr auto operator=(resource const&) = delete;
    constexpr resource& operator=(resource&&) = default;
    // constexpr ~resource() = delete;
};

};

