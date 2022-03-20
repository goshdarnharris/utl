// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <stddef.h>
#include <utility>
#include <utl/register/register.hh>
#include <utl/register/field.hh>
#include <utl/hof.hh>







template <any_field T>
struct bit_manipulate {
    utl::register::value_t<T> set;    
    utl::register::value_t<T> clr;

    consteval auto operator()(any_register auto&& r, utl::register::value_t<T> v)
    {
        static_assert(utl::registers::contains_field<decltype(r),T>());
        return utl::tuple{r, (v | field::apply_offset());
    }
};









namespace utl::registers::ops {

BFG_TAG_INVOKE_DEF(merge);
BFG_TAG_INVOKE_DEF(evaluate);

// constexpr auto tag_invoke(merge_t, any_field_op auto...)
// {
    
// }


template <typename T>
using target_register_t = typename std::decay_t<T>::target_register_t;

template <typename T>
using target_value_t = typename target_register_t<T>::value_t;

template <typename T>
concept has_target_register = requires {
    typename T::target_register_t;
} and any_register<typename T::target_register_t>;


template <typename T>
concept any_op = has_target_register<T> and
    requires(T o, target_register_t<T> r, target_value_t<T> v) {
        { evaluate(r,v,o) } -> std::same_as<decltype(v)>;
        //ops are comparable
    };

namespace composed {

    // template <typename T>
    // consteval bool detect_composed_ops() { return false; }

    // template <any_tuple T, size_t... Is>
    // consteval bool detect_composed_ops_fold(std::index_sequence<Is...>)
    // {
    //     return (detect_composed_ops<utl::tuple_element_t<Is,T>>() and ...);
    // }

    // template <any_tuple T>
    // consteval bool detect_composed_ops()
    // {
    //     return detect_composed_ops_fold<T>(std::make_index_sequence<utl::tuple_size_v<T>>);
    // }

    // template <any_op T>
    // consteval bool detect_composed_ops()
    // {
    //     return true;
    // }

    // template <size_t I, size_t... Is>
    // constexpr auto eval(const any_register auto r, const auto v, any_composed_ops auto o)
    // {
    //     return utl::get<I>(r)
    // }




    BFG_TAG_INVOKE_DEF(flatten_operator);

    constexpr auto tag_invoke(flatten_operator_t, auto&& v, any_tuple auto&& accumulated)
    {
        using value_t = decltype(v);
        using accum_t = decltype(accumulated);
        return utl::tuple_cat(tuple{std::forward<value_t>(v)}, std::forward<accum_t>(accumulated));
    }

    constexpr auto tag_invoke(flatten_operator_t, any_tuple auto&& v, any_tuple auto&& accumulated)
    {
        using value_t = decltype(v);
        using accum_t = decltype(accumulated);
        return hof::fold(std::forward<value_t>(v), flatten_operator, 
            std::forward<accum_t>(accumulated)
        );
    }

    constexpr auto flatten(any_tuple auto&& composition)
    {
        using comp_t = decltype(composition);
        return hof::fold(std::forward<comp_t>(composition), flatten_operator, utl::tuple{});
    }




    BFG_TAG_INVOKE_DEF(merge_sort_operator);

    template <typename T>
    struct merge_state {

    };

    template <typename... Ts>
    merge_state(Ts&&...) -> merge_state<Ts&&...>;

    // template<class T, T... Ints>
    // constexpr T get(std::integer_sequence<T, Ints...>, std::size_t i) {
    //     constexpr T arr[] = {Ints...};
    //     return arr[i];
    // }


    // consteval auto merge(any_tuple auto&& accum, any_tuple auto&& a, any_tuple auto&& b)
    // {
    //     using a_head_t = std::decay_t<decltype(get<0>(a))>;
    //     using b_head_t = std::decay_t<decltype(get<0>(b))>;

    //     constexpr auto compare = [](auto&& state, auto&& a, auto&& b)
    //     {
    //         return state.get(a) <=> state.get(b);
    //     };


    //     if constexpr(compare(state,get<0>(a), get<0>(b)) == std::partial_ordering::less) {
    //         return tuple_cat(tuple{get<0>(a)}, accum, merge());
    //     } else {

    //     }

    //     // return hof::fold(std::forward<a_t>(a), merge_op, utl::tuple{});
    // }


    // constexpr auto tag_invoke(merge_sort_operator_t, auto&& merge_state, auto&& item)
    // {
    //     // using stack_t = std::decay_t<decltype(merge_state.stack)>;
    //     // if constexpr()

        
        
    // }

    constexpr auto sort(any_tuple auto&& composition)
    {
        using comp_t = std::decay_t<decltype(composition)>;

        constexpr auto push_stack = [](any_tuple auto&& stack, auto&& item)
        {
            using item_t = std::decay_t<decltype(item)>;
            return tuple_cat(tuple{std::forward<item_t>(item)}, stack);
        };

        constexpr auto get_stack_tail = []<size_t I, size_t... Is>(any_tuple auto&& stack)
        {
            return tuple{get<Is>(stack)...};
        };

        constexpr auto pop_stack = [](any_tuple auto&& stack)
        {
            return get<0>(stack), get_stack_tail(stack);
        };

        constexpr auto do_comparison = [](size_t, size_t)
        {
            return std::weak_ordering::equivalent;
        };

        constexpr auto do_merge = [](any_tuple auto&& stack, auto&&, auto&&)
        {
            return stack;
        };

        constexpr auto sort_operator = [](any_tuple auto&& stack, auto&& item)
        {
            using item_t = std::decay_t<decltype(item)>;

            constexpr size_t top_size = tuple_size_v<std::decay_t<decltype(get<0>(stack))>>;

            if constexpr(top_size == 1) {
                auto [top,new_stack] = pop_stack(stack);
                return do_merge(new_stack, top, std::forward<item_t>(item));
            } else {
                return push_stack(stack,std::forward<item_t>(item));
            }
        };


        
        auto sorted_idxs = hof::fold(std::forward<comp_t>(composition), sort_operator, utl::tuple{});
        static_assert(tuple_size_v<decltype(sorted_idxs)> == tuple_size_v<comp_t>);

        constexpr auto make_sort_result = []<size_t... Is>(auto&& comp, auto&& idxs, std::index_sequence<Is...>)
        {
            using result_t = tuple<tuple_element_t<get<Is>(idxs),std::decay_t<decltype(comp)>>...>;
            return result_t{get<get<Is>(idxs)>>(comp)...};
        };

        return make_sort_result(
            std::forward<comp_t>(composition), 
            sorted_idxs, 
            std::make_index_sequence<tuple_size_v<comp_t>>{}
        );
    }
} //namespace composed


template <typename T>
concept any_field_op = requires {
    typename T::target_field_t;
};



template <typename T, auto pred>
concept all_elements = requires(T&& v) {
    { 
        hof::fold(
            v,
            [](auto&& accum, auto&& item) { 
                constexpr bool accum_value = std::decay_t<decltype(accum)>::value;
                constexpr bool pred_value = std::decay_t<decltype(pred(item))>::value;
                if constexpr(accum_value and pred_value) return std::true_type{};
                else return std::false_type{};
            },
            std::true_type{}
        ) 
    } -> std::same_as<std::true_type>;
};

constexpr auto identity_test = [](auto&& item) { return std::decay_t<decltype(item)>{}; };
static_assert(all_elements<tuple<std::true_type,std::true_type,std::true_type>,identity_test>);
static_assert(not all_elements<tuple<std::true_type,std::false_type,std::true_type>,identity_test>);

struct is_op_predicate {
    constexpr std::false_type operator()(auto&&) { return {}; }
    constexpr std::true_type operator()(any_op auto&&) { return {}; }
};

template <typename T>
concept any_composed_ops = utl::any_tuple<T> and 
    all_elements<std::decay_t<decltype(composed::flatten(std::declval<T>()))>,is_op_predicate{}>;

template <typename T>
concept any_op_or_composition = any_op<T> or any_composed_ops<T>;


constexpr auto tag_invoke(merge_t, any_field_op auto&&... ops)
    requires same_target_register<decltype(ops)...>
{
    //all of the ops must have the same target register
    //then, merge the masks.
}

constexpr auto compose(any_op_or_composition auto&&... ops)
{
    return tuple{std::forward<decltype(ops)>(ops)...};
}


constexpr auto operator <=>(any_op auto const&, any_op auto const&)
{
    return std::partial_ordering::unordered;
}
constexpr auto operator ==(any_op auto const& a, any_op auto const& b)
{
    return (a <=> b) == std::partial_ordering::unordered;
}


//What is composable?
//any operation targeting a single field
//any composition of operations targeting a single field
//types must have the same target register

// template <typename... Ts>
// concept composable = field::



constexpr auto apply(auto&&... args)
    requires ((any_op<decltype(args)> or any_composed_ops<decltype(args)>) and ...)
{
    any_tuple auto flattened = composed::flatten(utl::tuple{std::forward<decltype(args)>(args)...});
    any_tuple auto sorted = composed::sort(flattened);
    //sort
    //split into contiguous chunks targeting the same register
    //merge
    //evaluate
    //return an audit
}










template <typename T>
struct bitwise_modify;

template <any_register R>
struct bitwise_modify_register {
    using target_register_t = R;
    using value_t = value_t<R>;
    R target;
    value_t set_mask;
    value_t clear_mask;

    friend constexpr auto tag_invoke(evaluate_t, R r, value_t v, bitwise_modify_register a)
    {
        write(r, (v | a.set_mask) & (~a.clear_mask));
    }

    friend constexpr auto tag_invoke(merge_t, bitwise_modify_register a, bitwise_modify_register b)
    {
        return bitwise_modify_register{
            b.target,
            (a.set_mask | b.set_mask) & ~b.clear_mask,
            (a.clear_mask | b.clear_mask) & ~ b.set_mask
        };
    }
};

template <field::any_field T>
struct bitwise_modify_field {
    using target_field_t = T;
    using target_register_t = field::register_t<T>;
    using value_t = field::value_t<T>;
    T target;
    value_t set_mask;
    value_t clear_mask;

    friend constexpr auto tag_invoke(evaluate_t, target_register_t r, field::register_value_t<T> v, bitwise_modify_field a)
    {
        const auto set = field::align_to_register(a.target, a.set_mask);
        const auto clear = field::align_to_register(a.target, a.clear_mask);
        write(r, (v | set) & (~clear));
    }

    friend constexpr auto tag_invoke(merge_t, bitwise_modify_field a, bitwise_modify_register<target_register_t> b)
    {
        return bitwise_modify_register<target_register_t>{
            b.target,
            (field::align_to_register(a.target, a.set_mask) | b.set_mask) & ~b.clear_mask,
            (field::align_to_register(a.target, a.clear_mask) | b.clear_mask) & ~ b.set_mask
        };
    }

    friend constexpr auto tag_invoke(merge_t, bitwise_modify_register<target_register_t> a, bitwise_modify_field b)
    {
        const auto b_set = field::align_to_register(b.target, b.set_mask);
        const auto b_clear = field::align_to_register(b.target, b.clear_mask);
        return bitwise_modify_register{
            b.target,
            (a.set_mask | b_set) & ~b_clear,
            (a.clear_mask | b_clear) & ~ b_set
        };
    }

    // friend constexpr auto tag_invoke(merge_t, bitwise_modify<T> a, bitwise_modify<T> b)
    // {
    //     return bitwise_modify<register_t>{
    //         b.target,
    //         (field::align_to_register(a.target, a.set_mask) | b.set_mask) & ~b.clear_mask,
    //         (field::align_to_register(a.target, a.clear_mask) | b.clear_mask) & ~ b.set_mask
    //     };
    // }
};





















// okay. so I've been struggling a bit with how composable field operations should be.
// maybe the answer is that it's customizable?
// what does that mean?
// scenarios:
// - write to a register. order doesn't matter.
// - write to a register; local order matters.
//   this is to say that operations should be performed
//   strictly in their order of composition, even if extra
//   reads or writes are required to do so.
// - write to a register; nonlocal order matters.
//   what is this? some higher... order?
// well. I'm trying to define an ordering of operations.
// so... the spaceship operator?
// by default it's unordered.
// for local ordering, it's left-to-right.
// for nonlocal ordering, it's a user defined spaceship operator.
// then, a composition is ordered before it is evaluated.



// ultimately, a driver is a generated composition of register
// operations? does that make sense? 



}
