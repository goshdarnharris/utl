// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0


#pragma once

#include <cstdio>
#include <cstdint>
#include <concepts>
#include <utility>
#include <type_traits>
#include <utl/utility.hh>
#include <utl/tuple.hh>
#include <utl/string_view.hh>


namespace utl::mix {

template <size_t I, template <typename> typename... TTs>
struct self_type {
    constexpr self_type() = default;

    template <template <typename> typename TT>
    constexpr self_type(self_type<I,TTs...,TT>&) {}

    template <template <typename> typename TT>
    constexpr self_type(self_type<I,TTs...,TT> const&) {}

    template <template <typename> typename TT>
    constexpr self_type(self_type<I,TTs...,TT>&&) {}
};

template <size_t I, template <typename> typename... TTs>
constexpr auto& to_self(self_type<I,TTs...>& m) { return m; }

template <size_t I, template <typename> typename... TTs>
constexpr auto const& to_self(self_type<I,TTs...> const& m) { return m; }

template <typename T>
using self_t = copy_cvref_t<T, decltype(to_self(std::declval<std::remove_reference_t<T>&>()))>;


//template to start a search through the template list
template <size_t I, size_t J, template <typename> typename TT, template <typename> typename... TTs>
struct type_template_pack_element_find : type_template_pack_element_find<I,J+1,TTs...> {};

//specialization to stop type recursion and provide the appropriate template
//when the search index is equal to the provided index
template <size_t I, template <typename> typename TT, template <typename> typename... TTs>
struct type_template_pack_element_find<I,I,TT,TTs...> {
    template <typename T>
    using tmpl = TT<T>;
};

//utility template to get an element of a template template pack
template <size_t I, template <typename> typename... TTs>
struct type_template_pack_element : type_template_pack_element_find<I,0,TTs...> {};

template <size_t I, template <typename> typename TT, template <typename> typename... TTs>
struct get_mixin_type {
    using type = TT<self_type<I,TTs...>>;
};

template <size_t I, template <typename> typename... TTs>
using get_mixin_t = type_template_pack_element<I,TTs...>::template tmpl<self_type<I,TTs...>>;

//cast from a self type to the corresponding mixin type
template <size_t I, template <typename> typename... TTs>
constexpr auto& to_mixin(self_type<I,TTs...>& m)
{
    return static_cast<get_mixin_t<I,TTs...>&>(m);
}

template <size_t I, template <typename> typename... TTs>
constexpr auto const& to_mixin(self_type<I,TTs...> const& m)
{
    return static_cast<get_mixin_t<I,TTs...> const&>(m);
}

template <typename T>
using mixin_t = copy_cvref_t<T, decltype(to_mixin(std::declval<std::remove_reference_t<T>&>()))>;


template <typename T>
concept any_mixin = complete_type<std::remove_reference_t<T>> and requires(T v) {
    //a mixin can be implicitly converted to a reference to a self_type
    //but isn't itself that self_type
    []<size_t I, template <typename> typename... TTs>(self_type<I,TTs...>&)
        requires (not std::same_as<self_type<I,TTs...>, remove_cvref_t<T>>)
    {}(v);
};

template <typename T, template <typename> typename... TTs>
concept any_mixin_of = requires(T v) {
    //a mixin can be implicitly converted to a reference to a self_type
    []<size_t I>(self_type<I,TTs...>&){}(v);
};

template <typename... Ts>
struct mixed : Ts... {
    constexpr mixed(Ts&&... args) : Ts{std::forward<Ts>(args)}... {}
};

//base template
template <typename T, template <typename> typename... TTs>
struct make_mixed;

//specialization to extract indices from an index sequence,
//so we can make self_types to pass onto the provided templates
template <size_t... Is, template <typename> typename... TTs>
struct make_mixed<std::index_sequence<Is...>,TTs...> {
    using type = mixed<get_mixin_t<Is,TTs...>...>;
};

template <template <typename> typename... TTs>
using make_mixed_t = make_mixed<std::make_index_sequence<sizeof...(TTs)>,TTs...>::type;

//user interface for making a composed type
//this is a struct so that error messages refer to what the user wrote rather than
//a mess of template nonsense
template <template <typename> typename... TTs>
struct mix : public make_mixed_t<TTs...> {
    using make_mixed_t<TTs...>::make_mixed_t;
};


template <template <typename> typename... TTs>
constexpr auto& to_mix(mix<TTs...>& m)
{
    return m;
}

template <template <typename> typename... TTs>
constexpr auto const& to_mix(mix<TTs...> const& m)
{
    return m;
}

template <size_t I, template <typename> typename... TTs>
constexpr auto& to_mix(self_type<I,TTs...>& m)
{
    return static_cast<mix<TTs...>&>(m);
}

template <size_t I, template <typename> typename... TTs>
constexpr auto const& to_mix(self_type<I,TTs...> const& m)
{
    return static_cast<mix<TTs...> const&>(m);
}

template <typename T>
using mix_t = copy_cvref_t<T,decltype(to_mix(std::declval<std::remove_reference_t<T>&>()))>;

template <typename T>
concept any_mix = complete_type<remove_cvref_t<T>> and requires(remove_cvref_t<T> v) {
    //can be implicitly converted to reference to each of its component self_types
    []<template <typename> typename... TTs>(mix<TTs...>& m){}(v);
};



//a mix reference is either a reference-to-mix or reference-to-mixin-instance
//a self_type is not a mix reference; it can be converted to one but it doesn't satisfy
//this concept.
template <typename T>
concept any_mix_ref = complete_type<std::remove_reference_t<T>> and any_mix<T> or any_mixin<T>;

template <typename T>
concept is_complete = requires { sizeof(T); };



namespace detail {
    template <typename T>
    struct find_result {
        using result_t = T;
        size_t count;
        constexpr auto& cast(auto& m) 
        { 
            auto& mix = to_mix(m); //downcast to the mix type; if it is a mix type this is a no-op
            return static_cast<copy_cv_t<decltype(m),T&>>(mix); //upcast to the result
        }
    };

    //FIXME: a search needs to not instantiate the type from which it's requested...

    template <auto Pred, size_t I, template <typename> typename... TTs>
    constexpr auto find_impl(size_t count)
    {
        using this_t = get_mixin_t<I,TTs...>;
        constexpr bool match = satisfies<this_t, Pred>;

        if constexpr(match) {
            if constexpr(I < sizeof...(TTs) - 1) {
                //save the first match we find, but keep counting
                return find_result<this_t>{find_impl<Pred,I+1,TTs...>(count).count + 1};
            } else {
                //this is the last one
                return find_result<this_t>{count + 1};
            }
        } else {
            if constexpr(I < sizeof...(TTs) - 1) {
                return find_impl<Pred,I+1,TTs...>(count);
            } else {
                return find_result<void>{count};
            }
        }
    }



    template <typename T>
    struct find_unwrap;

    template <size_t I, template <typename> typename... TTs>
    struct find_unwrap<self_type<I,TTs...>> {
        template <auto Pred>
        static constexpr auto impl() { 
            return find_impl<Pred,0,TTs...>(0);
        }

        template <template <typename> typename Mixin>
        static constexpr auto impl() 
        { 
            constexpr auto predicate = []<typename T>(Mixin<T>&){};
            return find_impl<predicate,0,TTs...>(0);
        }
    };

    template <template <typename> typename... TTs>
    struct find_unwrap<mix<TTs...>> : find_unwrap<self_type<0,TTs...>> {};

    template <any_mixin T>
    struct find_unwrap<T> : find_unwrap<self_t<T>> {};

    template <auto Pred, any_mix_ref T>
    constexpr auto find()
    {
        return find_unwrap<T>::template impl<Pred>();
    }

    template <template <typename> typename Mixin, any_mix_ref T>
    constexpr auto find()
    {
        return find_unwrap<T>::template impl<Mixin>();
    }

    template <auto Pred>
    constexpr auto find(any_mix_ref auto& m)
    {
        return find<Pred,std::remove_reference_t<decltype(m)>>();
    }

    template <template <typename> typename Mixin>
    constexpr auto find(any_mix_ref auto& m)
    {
        return find<Mixin,std::remove_reference_t<decltype(m)>>();
    }
} //namespace detail



///////////// find overloads - returns a find result, which contains the type of the found mixin and the count of conforming mixins

//type predicate on a reference-to-composition
template <auto Pred, template <typename> typename... TTs>
constexpr auto find(any_mix_ref auto& m)
{
    return detail::find<Pred,std::remove_reference_t<decltype(m)>>();
}

//mixin template on a reference-to-composition
template <template <typename> typename TT, template <typename> typename... TTs>
constexpr auto find(any_mix_ref auto& m)
{
    return detail::find<TT>(m);
}

template <auto Pred, any_mix_ref T>
constexpr auto find()
{
    return detail::find<Pred,T>();
}


template <template <typename> typename TT, any_mix_ref T>
constexpr auto find()
{
    return detail::find<TT,T>();
}

///////////// count overloads - returns count of conforming mixins in composition

//type predicate on a reference-to-composition
template <auto Pred>
constexpr size_t count(any_mix_ref auto& m)
{
    return detail::find<Pred>(m).count;
}

//mixin template on a reference-to-composition
template <template <typename> typename TT>
constexpr size_t count(any_mix_ref auto& m)
{
    return detail::find<TT>(m).count;
}

//count of all mixins
constexpr size_t count(any_mix_ref auto& m)
{
    constexpr auto impl = []<template <typename> typename... TTs>(mix<TTs...>&) { return sizeof...(TTs); };
    return impl(to_mix(m));
}

//type predicate on a reference-to-composition, template version
template <auto Pred, any_mix_ref T>
constexpr size_t count()
{
    return detail::find<Pred,T>().count;
}

//mixin template on a reference-to-composition, template version
template <template <typename> typename TT, any_mix_ref T>
constexpr size_t count()
{
    return detail::find<TT,T>().count;
}

//count of all mixins
template <any_mix_ref T>
constexpr size_t count()
{
    constexpr auto impl = []<template <typename> typename... TTs>(mix<TTs...> const&) { return std::integral_constant<size_t,sizeof...(TTs)>{}; };
    return std::decay_t<decltype(impl(std::declval<mix_t<T>>()))>::value;
}

///////////// has overloads - tests whether composition contains a conforming mixin

//type predicate on a reference-to-composition
template <auto Pred, template <typename> typename... TTs>
constexpr bool has(any_mix_ref auto& m)
{
    return count<Pred>(m) > 0;
}

//mixin template on a reference-to-composition
template <template <typename> typename TT, template <typename> typename... TTs>
constexpr bool has(any_mix_ref auto& m)
{
    return count<TT>(m) > 0;
}

template <auto Pred, any_mix_ref T>
constexpr bool has()
{
    return count<Pred,T>() > 0;
}

template <template <typename> typename TT, any_mix_ref T>
constexpr bool has()
{
    return count<TT,T>() > 0;
}



// it is an error to query using an incomplete type
template <auto Pred, incomplete_type T>
constexpr bool has() = delete;

template <template <typename> typename TT, incomplete_type T>
constexpr bool has() = delete;

// it is an error to query using anything other than a mix_ref
template <auto Pred, typename T>
constexpr bool has() = delete;

template <template <typename> typename TT, typename T>
constexpr bool has() = delete;



// static_assert(not requires(){ has<[](auto&){},self_type<0)

///////////// as overloads - returns a reference to the first conforming mixin subobject in the composition

//type predicate on a reference-to-composition
template <auto Pred>
    requires (not std::same_as<size_t, std::decay_t<decltype(Pred)>>)
constexpr auto& as(any_mix_ref auto& m)
{
    return find<Pred>(m).cast(m);
}

template <size_t I>
constexpr auto& as(any_mix_ref auto& m)
{
    return to_mixin<I>(m);
}

//mixin template on a reference-to-composition
template <template <typename> typename TT>
constexpr auto& as(any_mix_ref auto& m)
{
    return find<TT>(m).cast(m);
}



///////////// add


namespace detail {
    template <template <typename> typename TT, template <typename> typename... TTs, size_t... Is>
    constexpr auto add_impl(std::index_sequence<Is...>, mix<TTs...>&& m, auto&&... args)
    {
        return mix<TTs...,TT>{
            static_cast<get_mixin_t<Is,TTs...>&&>(m)...,
            {std::forward<decltype(args)>(args)...}
        };
    }

    template <template <typename> typename TT, template <typename> typename... TTs, size_t... Is>
    constexpr auto add_impl(std::index_sequence<Is...>, mix<TTs...>& m, auto&&... args)
    {
        return mix<TTs...,TT>{
            static_cast<get_mixin_t<Is,TTs...>&>(m)...,
            std::forward<decltype(args)>(args)...
        };
    }
};


template <template <typename> typename TT>
constexpr auto add(any_mix_ref auto&& m, auto&&... args)
{
    return detail::add_impl<TT>(
        std::make_index_sequence<count<decltype(m)>()>{}, 
        static_cast<mix_t<decltype(m)>>(m), 
        std::forward<decltype(args)>(args)...
    );
}


template <typename T, template <typename> typename... TTs>
struct filter_result {};

template <template <typename> typename TT, typename T, template <typename> typename UU, template <typename> typename... TTs>
struct remove_filter;

template <template <typename> typename TT, size_t I, size_t... Is, template <typename> typename... TTs>
struct remove_filter<TT,std::index_sequence<I,Is...>,TT,TTs...> { //this index matches the mixin template
    static_assert(sizeof...(Is) == sizeof...(TTs));

    //on a match, don't add this index to the sequence
    static constexpr auto recurse = []() {
        if constexpr(sizeof...(TTs) > 0) {
            return typename remove_filter<TT,std::index_sequence<Is...>,TTs...>::result_t{};
        } else {
            return filter_result<std::index_sequence<>>{};
        }
    };
    using result_t = std::decay_t<decltype(recurse())>;
};

template <template <typename> typename TT, size_t I, size_t... Is, template <typename> typename UU, template <typename> typename... TTs>
struct remove_filter<TT,std::index_sequence<I,Is...>,UU,TTs...> {
    static_assert(sizeof...(Is) == sizeof...(TTs));

    static constexpr auto prepend_result = []<size_t... Js, template <typename> typename... UUs>(filter_result<std::index_sequence<Js...>,UUs...>) { 
        return filter_result<std::index_sequence<I,Js...>,UU,UUs...>{}; 
    };

    //if they don't match, add this index to the front of the sequence
    static constexpr auto recurse = []() {
        if constexpr(sizeof...(TTs) > 0) {
            return prepend_result(typename remove_filter<TT,std::index_sequence<Is...>,TTs...>::result_t{});
        } else {
            return filter_result<std::index_sequence<I>,UU>{};
        }
    };
    using result_t = std::decay_t<decltype(recurse())>;
};


template <template <typename> typename TT>
constexpr auto remove(any_mix_ref auto&& m)
{
    auto impl = [&]<template <typename> typename... TTs>(mix<TTs...>&) {
        auto construct = [&]<size_t... Is, template <typename> typename... UUs>(filter_result<std::index_sequence<Is...>,UUs...>) {
            if constexpr(std::is_rvalue_reference_v<decltype(m)>) {
                return mix<UUs...>{std::move(as<Is>(m))...};
            } else {
                return mix<UUs...>{as<Is>(m)...};
            }
        };

        using filter_result_t = typename remove_filter<TT,std::make_index_sequence<sizeof...(TTs)>,TTs...>::result_t;
        return construct(filter_result_t{});
    };

    return impl(to_mix(m));
}


//template to decorate a mixin with another mixin
//FIXME: this is awkward to use
template <template <typename> typename TT, template <typename> typename UU>
struct decorate {
    template <typename T>
    using tmpl = UU<TT<T>>;
};

template <typename T>
struct decorator : T { using T::T; };

template <typename T>
constexpr auto& to_decorated(decorator<T>& d)
{
    return static_cast<T&>(d);
}

template <typename T>
constexpr auto const& to_decorated(decorator<T> const& d)
{
    return static_cast<T const&>(d);
}


template <typename T, template <typename> typename TT>
struct any_impl_type { static constexpr bool value = false; };

template <typename T, template <typename> typename TT>
struct any_impl_type<TT<T>,TT> { static constexpr bool value = true; };

template <typename T, template <typename> typename TT>
concept any = any_impl_type<std::remove_cvref_t<T>,TT>::value;

} //namespace utl::mix