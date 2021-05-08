#pragma once

#include <stdlib.h>
#include <concepts>
#include <utility>

namespace utl {

template <typename... Ts>
struct tuple_impl;

template <>
struct tuple_impl<> {};

template <typename Head, typename... Tail>
struct tuple_impl<Head,Tail...> : tuple_impl<Tail...> {
    Head value;

    //NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    constexpr tuple_impl(std::convertible_to<Head> auto&& head, std::convertible_to<Tail> auto&&... tail)
      : tuple_impl<Tail...>{std::forward<decltype(tail)>(tail)...}, 
        value{std::forward<decltype(head)>(head)}
    {}
};

template <typename Terminal>
struct tuple_impl<Terminal> {
    Terminal value;

    //NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    constexpr tuple_impl(std::convertible_to<Terminal> auto&& terminal) : value{std::forward<Terminal>(terminal)} {}
};

template <typename... Ts>
struct tuple : tuple_impl<Ts...> {
    static constexpr bool is_default_constructible = (sizeof...(Ts) == 0) or 
        (std::is_default_constructible_v<Ts> and ...);    
    static constexpr bool is_copy_constructible = (sizeof...(Ts) > 0) and 
        (std::is_copy_constructible_v<Ts> and ...);

    template <typename... Us>
    static constexpr bool is_constructible_from_args = (sizeof...(Us) == sizeof...(Ts)) and
        (std::is_constructible_v<Ts> and ...);


    constexpr tuple() requires is_default_constructible = default;

    constexpr explicit((not std::is_convertible_v<const Ts&,Ts> or ...)) tuple(const Ts&... args)
        requires ((sizeof...(Ts) > 0) and is_copy_constructible)
    : tuple_impl<Ts...>{args...} {}

    template <class... Us>
        requires (sizeof...(Us) == sizeof...(Ts)) and 
            (sizeof...(Ts) > 0) and
            (std::is_constructible_v<Ts,Us&&> and ...)
    constexpr explicit((not std::is_convertible_v<Us,Ts> or ...)) tuple(Us&&... args)
      : tuple_impl<Ts...>{std::forward<Us>(args)...} {}

    // template <class... Us>
    // constexpr tuple(const tuple<Us...>& other);

    // template <class... Us>
    // constexpr tuple(tuple<Us...>&& other);

    // template <class U1, class U2>
    // constexpr tuple(const pair<U1,U2>& p);

    // template <class U1, class U2>
    // constexpr tuple(pair<U1,U2>&& p);

    constexpr tuple(const tuple& other) = default;
    constexpr tuple(tuple&& other) = default;
    constexpr tuple& operator=(const tuple& other) = default;
    constexpr tuple& operator=(tuple&& other) = default;

    constexpr ~tuple() = default;
};


template <typename... Ts>
tuple(Ts...) -> tuple<Ts...>;


namespace detail {

struct ignore_t {
    //NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
    constexpr void operator=(auto&&) const {}
};

} //namespace detail

inline constexpr detail::ignore_t ignore;

template <typename T>
struct tuple_size;

template <typename... Ts>
struct tuple_size<tuple<Ts...>> { static constexpr size_t value = sizeof...(Ts); };

template <typename... Ts>
struct tuple_size<const tuple<Ts...>> { static constexpr size_t value = sizeof...(Ts); };

template <typename T>
inline constexpr const size_t tuple_size_v = tuple_size<std::remove_reference_t<T>>::value;

template <size_t I, typename T>
struct tuple_element;

template <size_t I, typename Head, typename... Tail>
struct tuple_element<I,tuple<Head,Tail...>> {
    using type = typename tuple_element<I-1,tuple<Tail...>>::type;
};

template <typename Head, typename... Tail>
struct tuple_element<0,tuple<Head,Tail...>> {
    using type = Head;
};

template <size_t I>
struct tuple_element<I,tuple<>> {
    static_assert(I < tuple_size_v<tuple<>>, "tuple index is out of range");
};

template <size_t I, typename T>
struct tuple_element<I,const T> {
  using type = typename std::add_const<typename tuple_element<I,T>::type>::type;
};

template <size_t I, typename T>
struct tuple_element<I,volatile T> {
  using type = typename std::add_volatile<typename tuple_element<I,T>::type>::type;
};

template <size_t I, typename T>
struct tuple_element<I,const volatile T> {
  using type = typename std::add_cv<typename tuple_element<I,T>::type>::type;
};

template <size_t I, typename T>
using tuple_element_t = typename tuple_element<I,T>::type;

using some_t = tuple_element_t<1,tuple<int,bool,char>>;


constexpr auto make_tuple(auto&&... args)
{
    return tuple{std::forward<decltype(args)>(args)...};
}

template <typename... Ts>
constexpr auto tie(Ts&... args)
{
    return tuple<Ts&...>{args...};
}

template <typename... Ts>
constexpr auto forward_as_tuple(Ts&&... args)
{    
    return tuple<Ts&&...>{std::forward<Ts>(args)...};
}

constexpr auto tuple_cat(auto&&... args)
{
    static_assert((std::same_as<decltype(args),void> and ...), "unimplemented");
}

template <typename T>
struct tuple_t;

/// Return a reference to the ith element of a tuple.
template<std::size_t I, typename T, typename... Ts>
constexpr auto& get_impl(tuple_impl<T, Ts...>& t)
{
    if constexpr(I == 0) {
        return t.value;
    } else {
        return get_impl<I-1>(static_cast<tuple_impl<Ts...>&>(t));
    }
}

/// Return a const reference to the ith element of a const tuple.
template<std::size_t I, typename T, typename... Ts>
constexpr const auto& get_impl(const tuple_impl<T, Ts...>& t)
{
    if constexpr(I == 0) {
        return t.value;
    } else {
        return get_impl<I-1>(static_cast<const tuple_impl<Ts...>&>(t));
    }
}


/// Return a reference to the ith element of a tuple.
template<std::size_t I, typename... Ts>
constexpr auto& get(tuple<Ts...>& t)
{
    static_assert(I < tuple_size_v<tuple<Ts...>>, "out of bounds");
    return get_impl<I,Ts...>(static_cast<tuple_impl<Ts...>&>(t));
}

/// Return a const reference to the ith element of a const tuple.
template<std::size_t I, typename... Ts>
constexpr const auto& get(const tuple<Ts...>& t)
{
    static_assert(I < tuple_size_v<tuple<Ts...>>, "out of bounds");
    return get_impl<I,Ts...>(static_cast<const tuple_impl<Ts...>&>(t));
}

/// Return an rvalue reference to the ith element of a tuple rvalue.
template<std::size_t I, typename... Ts>
constexpr auto&& get(tuple<Ts...>&& t)
{
    static_assert(I < tuple_size_v<tuple<Ts...>>, "out of bounds");
    using element_t = tuple_element_t<I,tuple<Ts...>>;
    return std::forward<element_t&&>(utl::get_impl<I,Ts...>(t));
}

/// Return a const rvalue reference to the ith element of a const tuple rvalue.
template<std::size_t I, typename... Ts>
constexpr const auto&& get(const tuple<Ts...>&& t)
{
    static_assert(I < tuple_size_v<const tuple<Ts...>>, "out of bounds");
    using element_t = tuple_element_t<I,tuple<Ts...>>;
    return std::forward<const element_t&&>(utl::get_impl<I,Ts...>(t));
}

template <typename F, typename T, size_t... Is>
constexpr decltype(auto) apply_impl(F&& f, T&& t, std::index_sequence<Is...>)
{
    return f(get<Is>(std::forward<T>(t))...);
}

template <typename F, typename T>
constexpr decltype(auto) apply(F&& f, T&& t)
{
    return apply_impl(std::forward<F>(f), std::forward<T>(t), std::make_index_sequence<tuple_size_v<T>>{});
}




} //namespace utl

namespace std {

template <size_t I, typename T>
struct tuple_element : utl::tuple_element<I,T> {};

template <typename T>
struct tuple_size : utl::tuple_size<T> {};

} //namespace std
