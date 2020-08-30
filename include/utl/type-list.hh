
#ifndef UTL_TYPE_LIST_HH_
#define UTL_TYPE_LIST_HH_

#include <stdlib.h>
#include <utl/traits.hh>
#include <utility>

namespace utl {

// template <size_t... Ns>
// struct index_sequence {
//     using type = index_sequence<Ns...>;
// };

// namespace detail {

// template <size_t L, size_t I, size_t... Ns>
// struct index_sequence_impl : index_sequence_impl<L, I+1, Ns..., I> {};

// template <size_t L, size_t... Ns>
// struct index_sequence_impl<L, L, Ns...> : index_sequence<Ns...> {};

// } //namespace detail

// template <size_t L>
// using make_index_sequence = typename detail::index_sequence_impl<L,1,0>::type;

template <size_t... Ns>
using index_sequence = std::index_sequence<Ns...>;

template <size_t L>
using make_index_sequence = std::make_index_sequence<L>;

template <size_t N, typename... Ts>
struct get_type;

template <size_t N, typename T, typename... Ts>
struct get_type<N,T,Ts...> : get_type<N-1, Ts...> {};

template<typename T, typename... Ts>
struct get_type<0, T, Ts...> {
    using type = T;
};

template <size_t N, typename... Ts>
using get_t = typename get_type<N,Ts...>::type;

template <typename T, typename... Ts>
struct contains
{
    constexpr static bool value = (is_same<Ts,T>::value || ...);
};

template <typename T, typename... Ts>
inline constexpr bool contains_v = contains<T,Ts...>::value;

namespace detail {

template <size_t N, typename T, typename... Ts>
struct get_type_index;

template <size_t N, typename T, typename... Ts>
struct get_type_index {
    static_assert(contains_v<T,Ts...>, "type list does not contain T");

    static constexpr size_t value = is_same_v<get_t<N,Ts...>,T> ? N : get_type_index<N-1,T,Ts...>::value;
};

template <typename T, typename... Ts>
struct get_type_index<0,T,Ts...> {
    static constexpr size_t value = 0;
};

} //namespace detail

template <typename T, typename... Ts>
struct get_type_index : detail::get_type_index<sizeof...(Ts) - 1,T,Ts...> {};

template <typename T, typename... Ts>
static constexpr size_t get_type_index_v = get_type_index<T,Ts...>::value;


template <typename... Ts>
struct type_list {
    static constexpr size_t length = sizeof...(Ts);

    template <size_t N>
    using get_t = get_t<N,Ts...>;
};




} //namespace utl

#endif //UTL_TYPE_LIST_HH_
