#ifndef UTL_TUPLE_HH_
#define UTL_TUPLE_HH_

#include <utl/type-list.hh>

namespace utl {

namespace detail {
//TODO:
// - rvalue tuple support
// - value assignment

template <size_t N, typename T>
struct tuple_element {
    T value;
    constexpr T& get() { return value; }
    constexpr T const& get() const { return value; }
};


template <typename N, typename... Ts>
struct tuple_impl;

template <size_t... N, typename... Ts>
struct tuple_impl<index_sequence<N...>, Ts...> : detail::tuple_element<N,Ts>... {
    using types_t = utl::type_list<Ts...>;

    template <size_t M>
    using get_t = typename types_t::template get_t<M>;

    template <size_t M>
    using element_t = tuple_element<M, get_t<M>>;

    template <typename... Us, std::enable_if_t<sizeof...(Us) == sizeof...(Ts),int*> = nullptr>
    constexpr tuple_impl(Us&&... values) 
        : detail::tuple_element<N,std::remove_reference_t<Us>>{std::forward<Us>(values)}... 
    {}

    template <size_t M>
    constexpr auto& get() {
        return element_t<M>::get();
    }

    template <size_t M>
    constexpr auto const& get() const {
        return element_t<M>::get();
    }
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

} //namespace utl

#endif //UTL_TUPLE_HH_
