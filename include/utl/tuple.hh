#ifndef UTL_TUPLE_HH_
#define UTL_TUPLE_HH_

#include <utl/traits.hh>
#include <utility>

namespace utl {

template <size_t N, typename T>
struct tuple_element {
    using type = T;
    T value;
};

namespace detail {

template <size_t N, typename... Ts>
struct get_type;

template <size_t N, typename T, typename... Ts>
struct get_type<N,T,Ts...> : get_type<N-1, Ts...> {};

template<typename T, typename... Ts>
struct get_type<0, T, Ts...> {
    using type = T;
};

template <typename N, typename... Ts>
struct tuple_impl;

template <size_t... N, typename... Ts>
struct tuple_impl<std::index_sequence<N...>, Ts...> : tuple_element<N,Ts>... {
    template <size_t M>
    using get_t = typename get_type<M,Ts...>::type;

    template <typename... Us>
        requires (std::is_convertible_v<Us,Ts> and ...)
    constexpr tuple_impl(Us&&... values) 
        : tuple_element<N,Ts>{std::forward<Us>(values)}... 
    {}
};

} //namespace detail


template <typename... Ts>
struct tuple : detail::tuple_impl<std::make_index_sequence<sizeof...(Ts)>, Ts...> {
    static constexpr size_t size() { return sizeof...(Ts); }
    using detail::tuple_impl<std::make_index_sequence<sizeof...(Ts)>, Ts...>::tuple_impl;
};

template <typename... Ts>
tuple(Ts...) -> tuple<Ts...>;

template <typename... Ts>
auto make_tuple(Ts&&... args) { return tuple<Ts...>{std::forward<Ts>(args)...}; }

template <typename F, size_t... Ns, typename... Ts>
constexpr void for_each(detail::tuple_impl<std::index_sequence<Ns...>,Ts...>& tup, F&& functor) {
    (functor(tup.template get<Ns>()),...);
}

template <size_t I, typename... Ts>
constexpr auto&& get(tuple<Ts...>& t) { 
    using value_t = typename tuple<Ts...>::template get_t<I>;
    if constexpr (std::is_rvalue_reference_v<value_t>) {
        return std::move(t.template tuple_element<I,value_t>::value);
    } else {
        return t.template tuple_element<I,value_t>::value; 
    }
}

template <size_t I, typename... Ts>
constexpr auto&& get(tuple<Ts...> const& t) { 
    using value_t = typename tuple<Ts...>::template get_t<I>;
    if constexpr (std::is_rvalue_reference_v<value_t>) {
        return std::move(t.template tuple_element<I,value_t>::value);
    } else {
        return t.template tuple_element<I,value_t>::value; 
    }
}

template <size_t I, typename... Ts>
constexpr auto&& get(tuple<Ts...>&& t) { 
    using value_t = typename tuple<Ts...>::template get_t<I>;
    if constexpr (std::is_lvalue_reference_v<value_t>) {
        return t.template tuple_element<I,value_t>::value;
    } else {
        return std::move(t.template tuple_element<I,value_t>::value); 
    }
}

template <typename T>
struct tuple_size;

template <typename... Ts>
struct tuple_size<tuple<Ts...>> : utl::integral_constant<size_t,sizeof...(Ts)> {};

template <typename T>
inline constexpr size_t tuple_size_v = tuple_size<T>::value;

template <typename... Args>
constexpr auto tie(Args&... args)
{
    return tuple<Args&...>(args...);
}

template <typename F, typename... Ts>
constexpr auto apply(F&& functor, tuple<Ts...>& args)
{
    return apply(std::forward<F>(functor), args, std::make_index_sequence<sizeof...(Ts)>{});
}

template <typename F, typename... Ts, size_t... Is>
    requires requires(F&& f, tuple<Ts...>& a) {
        f(get<Is>(a)...);
    }
constexpr auto apply(F&& functor, tuple<Ts...>& args, std::index_sequence<Is...>)
{
    return functor(get<Is>(args)...);
}

} //namespace utl

namespace std {

template<typename... Ts>
struct tuple_size<utl::tuple<Ts...>> : public std::integral_constant<size_t, utl::tuple<Ts...>::size()> {};

template <size_t I, typename... Ts>
struct tuple_element<I, utl::tuple<Ts...>> : public utl::tuple_element<I, typename utl::tuple<Ts...>::template get_t<I>> {};

};

#endif //UTL_TUPLE_HH_
