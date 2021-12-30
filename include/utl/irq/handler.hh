// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/utl.hh>
#include <utl/tuple.hh>
#include <utl/irq/bits.hh>
#include <utl/irq/irq.hh>
#include <utl/concepts.hh>
#include <type_traits>

#include <utl/utility.hh>

namespace utl::irq {

template <auto* F>
using handler_t = std::remove_pointer_t<decltype(F)>;

template <auto* F>
concept any_handler = 
    std::is_function_v<handler_t<F>>;
    //and same_as<void, std::result_of<handler_t<F>>>;
    //FIXME: need to check the return type, and result_of
    //(which will deduce the argument types) was removed in
    //c++20; invoke_result doesn't deduce.

//**** Static Handlers

template <size_t N>
using static_handler_arg_t = void(irq_t<N>);

template <size_t N>
constexpr auto get_static_handler_irq(static_handler_arg_t<N>*)
{
    return irq_t<N>{};
}

template <auto* F>
using static_handler_irq_t = std::decay_t<decltype(get_static_handler_irq(F))>;


//FIXME: improve note output when this gets something that doesn't
//pass; particularly if it takes too many arguments or the like
template <auto* F>
concept any_static_handler = any_handler<F> and requires() {
    F(get_static_handler_irq(F));
};

template <auto* F>
    requires any_static_handler<F>
//FIXME: this is a wrapper to allow the vector table to deduce
// the function pointer as a template parameter. it's kind of 
// a hack, so it needs a better name and probably better error
// handling.
struct wrap_static_handler {
    static constexpr auto* handler = F;
    static constexpr auto irq = get_static_handler_irq(F);
};


//**** Bound Handlers

template <size_t N, typename... Ts>
using bound_handler_arg_t = void(*)(irq_t<N>,Ts...);

template <size_t N, typename... Ts>
constexpr auto get_bound_handler_irq(bound_handler_arg_t<N,Ts...>)
{
    return irq_t<N>{};
}

template <typename T>
struct get_first_parameter_type;

template <typename R, typename T, typename... Ts>
struct get_first_parameter_type<R(T,Ts...)>
{
    using type = T;
};

template <typename T>
using get_first_parameter_t = typename get_first_parameter_type<T>::type;

template <auto* F>
using bound_handler_irq_t = std::decay_t<decltype(get_bound_handler_irq(F))>;

//FIXME: I need a way to get the argument types...
template <auto* F, typename... Ts>
concept any_bindable_handler = (
    any_handler<F>
    and any_irq<get_first_parameter_t<handler_t<F>>>
    and requires(Ts... args) {
        { F(get_bound_handler_irq(F), std::forward<Ts>(args)...) } -> std::same_as<void>;
    });

//this and handler_t are just
//getting the function type. combine them and give it a better name.




} //namespace utl::irq

