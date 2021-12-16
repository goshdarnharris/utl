#pragma once

#include <stdlib.h>
#include <concepts>
#include <utility>
#include <utl/utility.hh>

namespace utl {

template <typename T>
struct tuple_size;

template <typename T>
inline constexpr size_t tuple_size_v = tuple_size<std::decay_t<T>>::value;

template <size_t I, typename T>
struct tuple_element;

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


namespace detail {
    template <size_t I, typename T>
        using tuple_element_ref_t = std::add_lvalue_reference_t<std::tuple_element_t<I,T>>;

    template <typename T>
    concept has_tuple_size = requires {
        { tuple_size<T>::value } -> std::convertible_to<size_t>;
    };

    template <typename T>
    concept has_tuple_element = requires {
        //FIXME: this is incomplete; needs to check all elements
        typename tuple_element_t<size_t{},T>;
    };

    template <typename T>
    concept has_get = has_tuple_element<T> and requires(T v) {
        //FIXME: this is incomplete; needs to check all elements
        { get<size_t{}>(v) } -> std::same_as<detail::tuple_element_ref_t<size_t{},T>>;
    };
} //namespace detail

template <typename T>
concept any_tuple = detail::has_tuple_size<T>
    and (tuple_size_v<T> == 0 or detail::has_get<T>);


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
    constexpr tuple_impl(std::convertible_to<Terminal> auto&& terminal) : value{std::forward<decltype(terminal)>(terminal)} {}
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

template <typename... Ts>
struct tuple_size<tuple<Ts...>> { static constexpr size_t value = sizeof...(Ts); };

template <typename... Ts>
struct tuple_size<const tuple<Ts...>> { static constexpr size_t value = sizeof...(Ts); };


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

namespace detail {

struct ignore_t {
    //NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
    constexpr void operator=(auto&&) const {}
};

} //namespace detail

inline constexpr detail::ignore_t ignore;



// using some_t = tuple_element_t<1,tuple<int,bool,char>>;


namespace detail {

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

    
}

/// Return a reference to the ith element of a tuple.
template<std::size_t I, typename... Ts>
constexpr auto& get(tuple<Ts...>& t)
{
    static_assert(I < tuple_size_v<tuple<Ts...>>, "out of bounds");
    return detail::get_impl<I,Ts...>(static_cast<tuple_impl<Ts...>&>(t));
}

/// Return a const reference to the ith element of a const tuple.
template<std::size_t I, typename... Ts>
constexpr const auto& get(const tuple<Ts...>& t)
{
    static_assert(I < tuple_size_v<tuple<Ts...>>, "out of bounds");
    
    return detail::get_impl<I,Ts...>(static_cast<const tuple_impl<Ts...>&>(t));
}

/// Return an rvalue reference to the ith element of a tuple rvalue.
template<std::size_t I, typename... Ts>
constexpr auto&& get(tuple<Ts...>&& t)
{
    static_assert(I < tuple_size_v<tuple<Ts...>>, "out of bounds");
    using element_t = tuple_element_t<I,tuple<Ts...>>;
    return std::forward<element_t&&>(detail::get_impl<I,Ts...>(t));
}

/// Return a const rvalue reference to the ith element of a const tuple rvalue.
template<std::size_t I, typename... Ts>
constexpr const auto&& get(const tuple<Ts...>&& t)
{
    static_assert(I < tuple_size_v<const tuple<Ts...>>, "out of bounds");
    using element_t = tuple_element_t<I,tuple<Ts...>>;
    return std::forward<const element_t&&>(detail::get_impl<I,Ts...>(t));
}


template <typename F, any_tuple T>
constexpr decltype(auto) apply(F&& f, T&& t)
{
    constexpr auto impl = []<size_t... Is>(F&& f_, T&& t_, std::index_sequence<Is...>)
    {
        return f_(get<Is>(std::forward<T>(t_))...);
    };

    return impl(std::forward<F>(f), std::forward<T>(t), std::make_index_sequence<tuple_size_v<T>>{});
}

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

constexpr auto tuple_cat(any_tuple auto&& head, any_tuple auto&& item, any_tuple auto&&... tail)
{
    constexpr auto do_cat = []<size_t... Hs, size_t... Is>(auto&& head_, auto&& item_, 
        std::index_sequence<Hs...>, std::index_sequence<Is...>)
    {
        using head_t = std::decay_t<decltype(head_)>;
        using item_t = std::decay_t<decltype(item_)>;
        
        return tuple<tuple_element_t<Hs,head_t>...,tuple_element_t<Is,item_t>...>{
            get<Hs>(std::forward<head_t>(head_))..., 
            get<Is>(std::forward<item_t>(item_))...
        };
    };

    using head_t = std::decay_t<decltype(head)>;
    using item_t = std::decay_t<decltype(item)>;
    constexpr auto head_size = tuple_size_v<head_t>;
    constexpr auto item_size = tuple_size_v<item_t>;

    if constexpr(sizeof...(tail) == 0) {
        return do_cat(
            std::forward<head_t>(head),
            std::forward<item_t>(item),
            std::make_index_sequence<head_size>{},
            std::make_index_sequence<item_size>{}
        );
    } else {
        return tuple_cat(
            do_cat(
                std::forward<head_t>(head),
                std::forward<item_t>(item),
                std::make_index_sequence<head_size>{},
                std::make_index_sequence<item_size>{}
            ),
            std::forward<decltype(tail)>(tail)...
        );
    }
}


} //namespace utl

namespace std {

template <size_t I, typename T>
struct tuple_element : utl::tuple_element<I,T> {};

template <typename T>
struct tuple_size : utl::tuple_size<T> {};

template <typename T>
inline constexpr size_t tuple_size_v = utl::tuple_size_v<T>;

} //namespace std
