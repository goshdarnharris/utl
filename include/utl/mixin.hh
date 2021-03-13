
#pragma once

#include <utl/string.hh>
#include <utl/tuple.hh>
#include <utl/constraint.hh>
#include <utl/type-list.hh>
#include <concepts>


//To Do:
// - interface components! (done)
// - construction. probably this should provide a way to get
//   concrete component types by index, which can then be used
//   for relatively direct initialization.
// - allow a component template to appear more than once in a 
//   composition.
// - more algorithms? 
//   - a version of at with a fallback, or at_t with a fallback
// - algorithms for composing constraints
//   - should constraints be lambdas instead of structs?
//   - composition of constraints is probably simpler with structs because I can spell the type.
//   - is a template parameter right place for them? what about as an arg?
// - support for composing compositions
// - properly constrain public API (currently uses auto*, so error messages will be bad)
// - support a compound if syntax for has/as & related (if-with-assignment), if that requires
//    anything special
// - "data" mixins (no self_type base; unaware of other components but can be used by them)

namespace utl::mixin {

struct interface_tag_t{};
struct implementation_tag_t{};
struct composed_tag_t{};

template <typename T>
concept any_interface_traits = requires() {
    { typename T::tag_t{} } -> std::same_as<interface_tag_t>;
};

template <typename T>
concept any_implementation_traits = requires() {
    { typename T::tag_t{} } -> std::same_as<implementation_tag_t>;
};

template <typename T>
concept any_composed_traits = requires() {
    { typename T::tag_t{} } -> std::same_as<composed_tag_t>;
};

template <typename T>
concept any_traits = any_interface_traits<T> 
    or any_implementation_traits<T>
    or any_composed_traits<T>;

template <template <typename> typename T>
struct interface_traits {
    using tag_t = interface_tag_t;

    template <typename U>
    using mixin_t = T<U>;

    template <typename U>
    struct access_node_t : public T<U> {};
};

template <template <typename> typename T>
struct implementation_traits {
    using tag_t = implementation_tag_t;

    template <typename U>
    using mixin_t = T<U>;

    template <typename U>
    struct access_node_t : protected T<U> {};
};

template <typename T>
struct data_traits {
    using tag_t = interface_tag_t;

    template <typename U>
    using mixin_t = T;

    template <typename U>
    struct access_node_t : T,U {};
};

struct composed_traits {
    using tag_t = composed_tag_t;
};


template <any_traits T, any_traits... Ts>
struct self_type;

template <any_traits T, typename U>
using mixin_t = typename T::template mixin_t<U>;

template <any_traits T, typename U>
using access_node_t = typename T::template access_node_t<U>;




template <any_traits... Ts>
struct composed_root;

template <any_traits T, any_traits... Ts>
struct self_type {
    template <template<typename> typename Constraint>
    using search_t = typename constraint_filter_t<
        Constraint,
        mixin_t<Ts,self_type<Ts,Ts...>>&...
    >::tuple_t;

    template <template<typename> typename Constraint>
    friend constexpr auto search(self_type* c)
    {
        using result_types = constraint_filter_t<Constraint,mixin_t<Ts,self_type<Ts,Ts...>>&...>;
        return make_result_tuple(c, result_types{});
    }

protected:
    template <typename U>
    friend constexpr auto* mixin_cast(self_type* c)
    {
        auto* comp = static_cast<composed_root<Ts...>*>(c);
        return static_cast<U*>(comp);
    }    
};

// template <typename T>
// concept any_self_type

template <typename... Ts>
constexpr auto make_result_tuple(auto& component, utl::type_list<Ts&...>) -> utl::tuple<Ts&...>
{
    return {*mixin_cast<Ts>(component)...};
}

template <any_traits... Ts>
struct composed_root : self_type<composed_traits,Ts...>, access_node_t<Ts,self_type<Ts,Ts...>>... {
    using self_t = self_type<composed_traits,Ts...>;

    template <any_traits T, any_traits... Us>
    friend struct self_type;

    template <template<typename> typename Constraint>
    friend constexpr auto search(composed_root* c)
    {
        return search<Constraint>(static_cast<self_t*>(c));
    }
};

template <template <typename> typename... Ts>
struct interface {};

template <typename T, template <typename> typename... Ts>
struct composed;

template <template <typename> typename... Ts, template <typename> typename... Us>
struct composed<interface<Ts...>,Us...> : 
    composed_root<interface_traits<Ts>..., implementation_traits<Us>...> 
{};

template <typename T, template<typename> typename Constraint>
struct as_component {
    using result_t = typename T::template search_t<Constraint>;
    static constexpr size_t count = utl::tuple_size_v<result_t>;
    static_assert(count != 0, "no components conform this constraint.");
    static_assert(count <= 1, "more than one component conforms to this constraint.");
    using ref_t = typename result_t::template get_t<0>;
    using type = std::remove_reference_t<ref_t>;
};

template <typename T, template<typename> typename Constraint>
using as_t = typename as_component<T,Constraint>::type;

template <template<typename> typename Constraint>
constexpr auto& as(auto* c)
{
    auto items = search<Constraint>(c);
    constexpr size_t count = utl::tuple_size_v<decltype(items)>;
    static_assert(count != 0, "no components match this constraint.");
    static_assert(count <= 1, "more than one component matches this constraint.");
    return utl::get<0>(items);
}

template <template<typename> typename Constraint>
constexpr decltype(auto) as(auto* c, auto&& visitor)
{
    return visitor(as<Constraint>(c));
}

namespace detail {

template <typename... Ts, size_t... Is>
constexpr void for_each_impl(utl::tuple<Ts...>& items, std::index_sequence<Is...>, auto&& visitor)
{
    (visitor(utl::get<Is>(items)),...);
}

template <typename... Ts>
constexpr void for_each_impl(utl::tuple<Ts...>& items, auto&& visitor)
{
    detail::for_each_impl(items, std::make_index_sequence<sizeof...(Ts)>(), visitor);
}

} //namespace detail

template <template<typename> typename Constraint>
constexpr void for_each(auto* c, auto&& visitor)
{
    auto items = search<Constraint>(c);
    detail::for_each_impl(items, visitor);
}

template <template<typename> typename Constraint>
constexpr size_t count(auto* c)
{
    auto items = search<Constraint>(c);
    return utl::tuple_size_v<decltype(items)>;
}

template <typename T, template<typename> typename Constraint>
concept any_with = requires() { as_component<T,Constraint>{}; };

template <template<typename> typename Constraint>
constexpr bool has(auto* c)
{
    using T = std::remove_pointer_t<decltype(c)>;
    return any_with<T,Constraint>;
}

} //namespace utl
