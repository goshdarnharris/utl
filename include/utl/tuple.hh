#ifndef UTL_TUPLE_HH_
#define UTL_TUPLE_HH_

#include <utl/type-list.hh>
#include <utl/traits.hh>

namespace utl {

template <size_t N, typename T>
struct tuple_element {
    using type = T;
    T value;
};

namespace detail {

template <typename N, typename... Ts>
struct tuple_impl;

template <size_t... N, typename... Ts>
struct tuple_impl<index_sequence<N...>, Ts...> : tuple_element<N,Ts>... {
    using types_t = utl::type_list<Ts...>;

    template <size_t M>
    using get_t = typename types_t::template get_t<M>;

    template <size_t M>
    using element_t = tuple_element<M, get_t<M>>;

    template <typename... Us, std::enable_if_t<sizeof...(Us) == sizeof...(Ts),int*> = nullptr>
    constexpr tuple_impl(Us&&... values) 
        : tuple_element<N,Ts>{std::forward<Ts>(values)}... 
    {}
};

} //namespace detail


template <typename... Ts>
struct tuple : detail::tuple_impl<make_index_sequence<sizeof...(Ts)>, Ts...> {
    using types_t = utl::type_list<Ts...>;
    static constexpr size_t size() { return types_t::length; }
    using detail::tuple_impl<make_index_sequence<sizeof...(Ts)>, Ts...>::tuple_impl;
};

template <typename... Ts>
tuple(Ts...) -> tuple<Ts...>;

template <typename... Ts>
auto make_tuple(Ts&&... args) { return tuple<Ts...>{std::forward<Ts>(args)...}; }

template <typename F, size_t... Ns, typename... Ts>
constexpr void for_each(detail::tuple_impl<index_sequence<Ns...>,Ts...>& tup, F&& functor) {
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

template <typename... Ts>
struct tuple_size;

template <typename... Ts>
struct tuple_size<tuple<Ts...>> : utl::integral_constant<size_t,tuple<Ts...>::size()> {};

template <typename... Args>
constexpr auto tie(Args&... args) {
    return tuple<Args&...>(args...);
}

} //namespace utl

namespace std {

template<typename... Ts>
struct tuple_size<utl::tuple<Ts...>> : public std::integral_constant<size_t, utl::tuple<Ts...>::size()> {};

template <size_t I, typename... Ts>
struct tuple_element<I, utl::tuple<Ts...>> : public utl::tuple_element<I, typename utl::tuple<Ts...>::template get_t<I>> {};

};

#endif //UTL_TUPLE_HH_
