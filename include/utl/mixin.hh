
#pragma once

#include <utl/string.hh>
#include <utl/tuple.hh>
#include <utl/constraint.hh>
#include <utl/type-list.hh>
#include <concepts>
#include <utl/utility.hh>


//To Do:
// - interface components! (done)
// - construction. probably this should provide a way to get
//   concrete component types by index, which can then be used
//   for relatively direct initialization.
// - allow a component template to appear more than once in a 
//   composition. (done)
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
concept any_interface_trait = requires() {
    { typename T::tag_t{} } -> std::same_as<interface_tag_t>;
};

template <typename T>
concept any_implementation_trait = requires() {
    { typename T::tag_t{} } -> std::same_as<implementation_tag_t>;
};

template <typename T>
concept any_composed_traits = requires() {
    { typename T::tag_t{} } -> std::same_as<composed_tag_t>;
};

template <typename T>
concept any_mixin_trait = any_interface_trait<T> 
    or any_implementation_trait<T>
    or any_composed_traits<T>;



template <size_t I, any_mixin_trait... Ts>
struct self_type;

struct mixin_forwarding_tag_t{};
inline constexpr mixin_forwarding_tag_t mixin_forwarding_tag{};

template <size_t I, typename T>
struct mixin : T {
    constexpr mixin() = default;
    constexpr mixin(mixin&) = default;
    constexpr mixin(mixin&&) = default;
    constexpr mixin& operator=(mixin const&) = default;
    constexpr mixin& operator=(mixin&&) = default;
    constexpr ~mixin() = default;

    using concrete_mixin_t = T;

    template <typename... Args>
    constexpr mixin(mixin_forwarding_tag_t, tuple<Args...>&& forwarding_tuple) 
        : mixin{mixin_forwarding_tag,
            std::make_index_sequence<tuple_size_v<tuple<Args...>>>{}, 
            std::forward<tuple<Args...>>(forwarding_tuple)}
    {}

    constexpr mixin(mixin_forwarding_tag_t, tuple<>) 
        requires std::default_initializable<T>
        : T{}
    {}
private:
    // using T::T;

    template <size_t... Is, typename... Args>
        // requires std::constructible_from<T,Args...>
    constexpr mixin(mixin_forwarding_tag_t, std::index_sequence<Is...>, tuple<Args...>&& args)
        : T{std::forward<Args>(get<Is>(args))...}
    {}

    template <size_t... Is, typename... Args>
        requires std::is_aggregate_v<T> //and requires() { T{{},std::declval<Args>()...}; }
    constexpr mixin(mixin_forwarding_tag_t, std::index_sequence<Is...>, tuple<Args...>&& args)
        : T{{}, std::forward<Args>(get<Is>(args))...}
    {}
};







template <template <typename> typename T>
struct interface {};

template <size_t I, template <typename> typename T>
struct interface_trait {
    static constexpr size_t composed_index = I;

    using tag_t = interface_tag_t;

    template <typename U>
    using mixin_t = T<U>;

    template <any_mixin_trait...Ts>
    using self_t = self_type<I,Ts...>;

    template <any_mixin_trait... Ts>
    using concrete_mixin_t = T<self_type<I,Ts...>>;

    template <any_mixin_trait... Ts>
    struct access_node_t : public mixin<I,concrete_mixin_t<Ts...>> {
        using mixin<I,concrete_mixin_t<Ts...>>::mixin;
    };
};

template <size_t I, template <typename> typename T>
struct implementation_trait {
    static constexpr size_t composed_index = I;

    using tag_t = implementation_tag_t;

    template <any_mixin_trait...Ts>
    using self_t = self_type<I,Ts...>;

    template <any_mixin_trait... Ts>
    using concrete_mixin_t = T<self_t<Ts...>>;

    template <any_mixin_trait... Ts>
    struct access_node_t : protected mixin<I,concrete_mixin_t<Ts...>> {
        using mixin<I,concrete_mixin_t<Ts...>>::mixin;
    };
};

template <size_t I, typename T>
struct data_trait {
    static constexpr size_t composed_index = I;

    using tag_t = interface_tag_t;

    template <any_mixin_trait...Ts>
    using self_t = self_type<I,Ts...>;

    template <any_mixin_trait... Ts>
    using concrete_mixin_t = T;

    template <any_mixin_trait... Ts>
    struct access_node_t : mixin<I,concrete_mixin_t<Ts...>>,self_type<I,Ts...> {
        using mixin<I,concrete_mixin_t<Ts...>>::mixin;
    };
};

struct composed_trait {
    using tag_t = composed_tag_t;
};

template <template <typename> typename T, typename... Args>
struct forward_to_template_mixin {
    struct is_forward_tag{};

    template <typename U>
    using concrete_mixin_t = T<U>;

    template <typename U>
    static constexpr bool can_construct = std::is_constructible_v<U,Args...>;
    tuple<Args&&...> args;
};

template <typename T, typename... Args>
struct forward_to_data_mixin {
    struct is_forward_tag{};

    template <typename U>
    using concrete_mixin_t = T;

    template <typename U>
    static constexpr bool can_construct = std::is_constructible_v<U,Args...>;
    tuple<Args&&...> args;
};



template <any_mixin_trait T, any_mixin_trait... Ts>
using self_t = typename T::template self_t<Ts...>;

template <any_mixin_trait T, any_mixin_trait... Ts>
using concrete_mixin_t = typename T::template concrete_mixin_t<Ts...>;

template <any_mixin_trait T, any_mixin_trait... Ts>
using access_node_t = typename T::template access_node_t<Ts...>;


template <template <typename> typename T, any_mixin_trait U, any_mixin_trait... Us>
inline constexpr bool mixin_matches_trait()
{
    using trait_concrete_mixin_t = concrete_mixin_t<U,Us...>;
    using trait_self_t = self_t<U,Us...>;    
    return std::is_same_v<trait_concrete_mixin_t, T<trait_self_t>>;
}

template <typename T, any_mixin_trait U, any_mixin_trait... Us>
inline constexpr bool mixin_matches_trait()
{
    using trait_concrete_mixin_t = concrete_mixin_t<U,Us...>;
    return std::is_same_v<trait_concrete_mixin_t, T>;
}


template <template <typename> typename T, template <typename> typename U>
struct is_same_mixin : std::false_type {};

template <template <typename> typename T>
struct is_same_mixin<T,T> : std::true_type {};

template <template <typename> typename T, template <typename> typename U>
inline constexpr bool is_same_mixin_v = is_same_mixin<T,U>::value;

template <any_mixin_trait... Ts>
struct composed_root;

template <size_t I, any_mixin_trait... Ts>
struct self_type {
    template <template<typename> typename Constraint>
    using search_t = typename constraint_filter_t<
        Constraint,
        concrete_mixin_t<Ts,Ts...>&...
    >::tuple_t;

    template <template<typename> typename Constraint>
    friend constexpr auto search(self_type* c)
    {
        using result_types = constraint_filter_t<Constraint,concrete_mixin_t<Ts,Ts...>&...>;
        return make_result_tuple(c, result_types{});
    }

    friend constexpr self_type* self_cast(self_type* c)
    {
        return c;
    }

protected:
    template <typename U>
    friend constexpr auto* mixin_cast(self_type* c)
    {
        auto* comp = static_cast<composed_root<Ts...>*>(c);
        return static_cast<U*>(comp);
    }    
};

template <typename... Ts>
constexpr auto make_result_tuple(auto& component, utl::type_list<Ts&...>) -> utl::tuple<Ts&...>
{
    return {*mixin_cast<Ts>(component)...};
}


//FIXME: this forwarding stuff is hideous.



template <typename T>
concept any_forward = requires() { typename T::is_forward_tag; };

template <any_forward T, typename U>
inline constexpr bool forward_can_construct = T::template can_construct<U>;

template <any_forward T, typename U, typename... Us>
inline constexpr bool forward_matches_trait = mixin_matches_trait<T::template concrete_mixin_t,U,Us...>();

template <typename T, typename U, typename... Us>
concept any_forward_matching = any_forward<T> and 
    any_mixin_trait<U> and (any_mixin_trait<Us> and ...) and
    forward_matches_trait<T,U,Us...>;

template <template <typename> typename T>
auto forward(auto&&... args)
{
    using forward_t = forward_to_template_mixin<T,decltype(args)...>;
    return forward_t{std::forward<decltype(args)>(args)...};
}

template <typename T>
auto forward(auto&&... args)
{
    using forward_t = forward_to_data_mixin<T,decltype(args)...>;
    return forward_t{std::forward<decltype(args)>(args)...};
}


inline constexpr size_t composition_index = static_cast<size_t>(-1);


template <typename M, typename T>
struct fwd_check_t;

template <typename M, typename T, typename... Args>
struct fwd_check_t<M,forward_to_data_mixin<T,Args...>&&> {
    using check_t = decltype(M{std::declval<Args>()...});
    static_assert(requires { {M{std::declval<Args>()...}}; }, "---- the error above this one means that "
        "you forwarded invalid arguments to one of your mixins. Take a look at the notes to see where in your code "
        "the invalid arguments are, and look for 'fwd_check_t<...>' in the notes to figure out which mixin. The "
        "error text provides details on why the arguments are invalid. Ignore any notes that suggest making changes to "
        "mixin.hh. Additionally, these invalid arguments will probably result in an error later in your compiler's "
        "output regarding the lack of viable constructors for the mixin type you are trying to initialize; I "
        "recommend ignoring it until you've fixed this one. ----");
};

template <typename M, template <typename> typename T, typename... Args>
struct fwd_check_t<M,forward_to_template_mixin<T,Args...>&&> {
    using check_t = decltype(M{std::declval<Args>()...});
    static_assert(requires { {M{std::declval<Args>()...}}; }, "---- the error above this one means that "
        "you forwarded invalid arguments to one of your mixins. Take a look at the notes to see where in your code "
        "the invalid arguments are, and look for 'fwd_check_t<...>' in the notes to figure out which mixin. The "
        "error text provides details on why the arguments are invalid. Ignore any notes that suggest making changes to "
        "mixin.hh. Additionally, these invalid arguments will probably result in an error later in your compiler's "
        "output regarding the lack of viable constructors for the mixin type you are trying to initialize; I "
        "recommend ignoring it until you've fixed this one. ----");

};

template <typename M, template <typename> typename T, typename... Args>
    requires std::is_aggregate_v<M>
struct fwd_check_t<M,forward_to_template_mixin<T,Args...>&&> {
    using check_t = decltype(M{{},std::declval<Args>()...});
    static_assert(requires { {M{{},std::declval<Args>()...}}; }, "---- the error above this one means that "
        "you forwarded invalid arguments to one of your mixins. Take a look at the notes to see where in your code "
        "the invalid arguments are, and look for 'fwd_check_t<...>' in the notes to figure out which mixin. The "
        "error text provides details on why the arguments are invalid. Ignore any notes that suggest making changes to "
        "mixin.hh. Additionally, these invalid arguments will probably result in an error later in your compiler's "
        "output regarding the lack of viable constructors for the mixin type you are trying to initialize; I "
        "recommend ignoring it until you've fixed this one. ----");
};

//FIXME: try to thin out the inheritance at the top here. the closer these constructors are
// to the type the user is interacting with directly, the better.
//FIXME: on the other hand, being able to spell the same type as the user is 
// potentially very useful for error output. maybe? would it make sense to pass
// the top composed type down the hierarchy?

template <any_mixin_trait... Ts>
struct composed_root : self_type<composition_index,Ts...>, access_node_t<Ts,Ts...>... {
    using self_t = self_type<composition_index,Ts...>;

    template <size_t I, any_mixin_trait... Us>
    friend struct self_type;

    composed_root() requires (std::default_initializable<concrete_mixin_t<Ts,Ts...>> and ...)
        = default;

    composed_root(composed_root&&) requires (std::move_constructible<concrete_mixin_t<Ts,Ts...>> and ...)
        = default;

    composed_root(composed_root&) requires (std::copy_constructible<concrete_mixin_t<Ts,Ts...>> and ...)
        = default;

    composed_root(any_forward_matching<Ts,Ts...> auto&&... forwards)
        requires requires { (fwd_check_t<typename access_node_t<Ts,Ts...>::concrete_mixin_t,decltype(forwards)>{},...); }
      : access_node_t<Ts,Ts...>{mixin_forwarding_tag, std::forward<decltype(forwards.args)&&>(forwards.args)}...
    {}

    template <template<typename> typename Constraint>
    friend constexpr auto search(composed_root* c)
    {
        return search<Constraint>(self_cast(c));
    }

    friend constexpr auto* self_cast(composed_root* c)
    {
        return static_cast<self_t*>(c);
    }
};


template <typename T>
using self_cast_t = std::remove_pointer_t<decltype(self_cast(std::declval<T*>()))>;

template <template <typename> typename Constraint, typename T>
using search_t = typename self_cast_t<T>::template search_t<Constraint>;

template <typename T>
concept any_composed = requires(T* t) {
    self_cast(t);
};

template <typename T, size_t N>
concept contains_at_least = (tuple_size_v<T> >= N);

template <typename T, size_t N>
concept contains_exactly = (tuple_size_v<T> == N);

template <typename T, template <typename> typename Constraint>
concept has_any_conforming = any_composed<T> and 
    contains_at_least<search_t<Constraint,T>,1>;

template <typename T, template <typename> typename Constraint>
concept has_only_one_conforming = has_any_conforming<T,Constraint> and 
    contains_exactly<search_t<Constraint,T>,1>;



template <size_t I, template <typename> typename T, template <typename> typename... Ts>
struct get_template_list_element;

template <size_t I, template <typename> typename T, template <typename> typename... Ts>
struct get_template_list_element : get_template_list_element<I-1,Ts...> {};

template <template <typename> typename T, template <typename> typename... Ts>
struct get_template_list_element<0,T,Ts...> {
    template <typename U>
    using tmpl = T<U>;
};

template <template<typename> typename... TTs>
struct unary_template_list
{
    template <size_t I>
    using type = get_template_list_element<I,TTs...>;
};

namespace detail {

template <typename T, typename U, template <typename> typename... Us>
struct make_composed_root;

template <typename T, typename U, template <typename> typename... Us>
using make_composed_root_t = typename make_composed_root<T,U,Us...>::type;

template <size_t... Is, template <typename> typename... Ts, template <typename> typename... Us>
struct make_composed_root<std::index_sequence<Is...>, interface<Ts...>, Us...> {

    using concrete_mixin_templates = unary_template_list<Ts...,Us...>;

    template <size_t I>
    struct make_trait;

    template <size_t I>
        requires (I < sizeof...(Ts))
    struct make_trait<I> { 
        template <typename T>
        using mixin_t = typename concrete_mixin_templates::template type<I>::template tmpl<T>;
        using type = interface_trait<I,mixin_t>; 
    };

    template <size_t I>
        requires (I >= sizeof...(Ts))
    struct make_trait<I> { 
        template <typename T>
        using mixin_t = typename concrete_mixin_templates::template type<I>::template tmpl<T>;
        // using 
        using type = implementation_trait<I,mixin_t>; 
    };

    //these traits type turn an index and a list of templates into:
    // - a concrete self_type
    // - a concrete "access" type that inherits from the mixin at the specified index
    // - a concrete mixin type
    //a self_type takes an index and a list of trait types
    //it can then get the trait type at the specified index in order to ask for the concrete mixin type

    template <size_t I>
    using make_trait_t = typename make_trait<I>::type;

    using type = composed_root<make_trait_t<Is>...>;
};

}

template <typename T, template <typename> typename... Ts>
struct composed;

template <template <typename> typename... Ts, template <typename> typename... Us>
struct composed<interface<Ts...>,Us...> : 
    detail::make_composed_root_t<
        std::make_index_sequence<sizeof...(Ts) + sizeof...(Us)>,
        interface<Ts...>, 
        Us...
    > 
{
    using detail::make_composed_root_t<
        std::make_index_sequence<sizeof...(Ts) + sizeof...(Us)>,
        interface<Ts...>, 
        Us...
    > ::make_composed_root_t;
};


template <template<typename> typename Constraint, has_only_one_conforming<Constraint> T>
struct as_component {
    static_assert(has_any_conforming<T,Constraint> and has_only_one_conforming<T,Constraint>);

    using result_t = typename T::template search_t<Constraint>;
    using ref_t = typename result_t::template get_t<0>;
    using type = std::remove_reference_t<ref_t>;
};


template <template<typename> typename Constraint, typename T>
    requires has_only_one_conforming<T,Constraint>
using as_t = typename as_component<Constraint,T>::type;

template <template<typename> typename Constraint>
constexpr auto& as(has_only_one_conforming<Constraint> auto* c)
{    
    auto items = search<Constraint>(c);
    return utl::get<0>(items);
}

template <template<typename> typename Constraint>
constexpr decltype(auto) as(has_only_one_conforming<Constraint> auto* c, auto&& visitor)
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
constexpr void for_each(has_any_conforming<Constraint> auto* c, auto&& visitor)
{
    auto items = search<Constraint>(c);
    detail::for_each_impl(items, visitor);
}

template <template<typename> typename Constraint, typename T>
constexpr bool has_any()
{
    // using T = std::remove_pointer_t<decltype(c)>;
    return has_any_conforming<T,Constraint>;
}

template <template<typename> typename Constraint, typename T>
constexpr bool has_one()
{
    // using T = std::remove_pointer_t<decltype(c)>;
    return has_only_one_conforming<T,Constraint>;
}

template <template<typename> typename Constraint, typename T>
constexpr bool count()
{
    // using T = std::remove_pointer_t<decltype(c)>;
    return search_t<Constraint, T>::size();
}

} //namespace utl
