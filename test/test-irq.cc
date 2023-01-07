// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0


#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/names.hh>
#include <utl/error.hh>
#include <utl/tuple.hh>
#include <utl/logger.hh>
#include <type_traits>
// #include <atomic>


#include <utl/utility.hh>


#include <utl/irq/unsafe.hh>
#include <utl/irq/handler.hh>

// // template <typename H, size_t IRQn>
// // concept has_isr = requires(const H* ptr) {
// //     ptr->template isr<IRQn>();
// // };

// // template <size_t IRQn>
// // static bool irq_used = false;

// // template <size_t IRQn, has_isr<IRQn> H>
// // struct irq {
// //     static const volatile H* _target;
// //     static void _handler()
// //     {
// //         if(_target) {
// //             _target->template isr<IRQn>();
// //         }
// //     }

// //     irq(irq&) = delete;
// //     irq(irq&&) = default;
// //     irq& operator=(irq&) = delete;
// //     irq& operator=(irq&&) = default;

// //     irq(H& target) { 
// //         //enforce a single instance. probably have to do it at runtime.
// //         //checking, then logging a severe warning if a target already exists
// //         //for this IRQn (probably by looking at the vector table) seems 
// //         //reasonable.
// //         //or maybe try to do it through constraints on moves/copies?

// //         //cannot copy
// //         //can move (that's going to be interesting)
// //         //can I enforce all references being const?
// //         //maybe I can constrain overloads with reference qualifiers
// //         //so that the only way anything can be done with a reference
// //         //is if it is const
// //         if(irq_used<IRQn>) {
// //             //panic
// //         }
// //         irq_used<IRQn> = true;
// //         _target = &target; 
// //         //set up vector table
// //     }

// //     ~irq() {
// //         //set vector table to default
// //         _target = nullptr;
// //     }
// // };


// // template <size_t IRQn, has_isr<IRQn> H>
// // auto make_irq(H& target)
// // {
// //     //this should panic if it's called more than once for a given IRQn.
// //     return irq<IRQn,H>{target};
// // }

// // template <size_t IRQn, has_isr<IRQn> H>
// // volatile const H* irq<IRQn,H>::_target = nullptr;


// // //Alright. I think I need to lift IRQ numbers. They have more semantics than just a number.
// // // - they belong to a peripheral
// // // - they have groupings:
// // //   - by kind, e.g. TIMx_UPDATE (the group of all timer update IRQs)
// // //   - by peripheral
// // //   - probably some DMA-related groupings
// // // - they have associated flags in peripheral registers
// // //
// // // All of this is constexpr.
// // // Providing concepts will then allow isr methods to be usefully constrained.
// // // I'll be able to interact directly with the IRQs in a more abstract way.
// // // I and users will be able to easily handle special cases.

// // // How is all of this organized? How do I spell things?
// // // hal::irq
// // // hal::


// // struct driver {
// //     mutable size_t data{};

// //     //I almost need this to be templated on an "IRQ grouping" rather than the specific IRQn.
// //     //That is, in a driver object I don't want to distinguish between specific IRQ numbers.
// //     //Just a slightly more abstract grouping: which of the interrupts for this peripheral instance
// //     //has occurred?
// //     //Okay. so that's something I've lost by pinning the method name. previously that's how those
// //     //semantics were expressed.
// //     //I could... group IRQ numbers by type. or provide concepts to constrain them.
// //     //  that could have interesting side effects, because suddenly you can have compound constraints...
// //     void isr(auto&& irq) const volatile
// //     {
// //         //do_things_here
// //         utl::maybe_unused(irq);
// //         data = 5;
// //         utl::log(utl::get_type_name(data));
// //         utl::log(utl::type_name<decltype(data)>);

// //         //Okay. this means that shared data gets marked.
// //         //that's interesting.
// //         //is there a way to enforce shared data being declared as volatile?
// //         //I don't need them to be marked as volatile in here, but I do
// //         //need them to be marked as volatile at their declaration.
// //         //or, really, I need some predicate to be true on the member
// //         //variables that are used in this method. eek.

// //         //Maybe it's a type wrapper that handles mutability and volatility internally?

// //         //How do I enforce shared<>?
// //     }
// // };

// #include <utl/concepts.hh>

// namespace utl::platform {

// //FIXME: this should be a type that implicitly constrains its own value.
// //this would mean that functions using it don't need to check that it is in range.
// using irqn_t = size_t; 
// using isr_vector_t = std::add_pointer_t<void()>;
// inline constexpr size_t VECTOR_TABLE_SIZE = 10;

// [[noreturn]] static void default_handler() { while(true) {} }

// //FIXME: in combination with a richer irqn type, maybe there's
// //a richer vector table type.
// static constinit array<isr_vector_t,VECTOR_TABLE_SIZE> s_table{{
//     default_handler,
//     default_handler,
//     default_handler,
//     default_handler,
//     default_handler,
//     default_handler,
//     default_handler,
//     default_handler,
//     default_handler,
//     default_handler
// }};

// template <any_platform P, any_irq<P> T>
// void map_isr(P&& platform, isr_vector_t v)
// {
//     auto table = vector_table(platform);
//     table[]
// }

// inline void set_vector(irqn_t irqn, isr_vector_t handler)
// {
//     if(irqn >= table.size()) return failure<void>(make_error_code(system_error::UNKNOWN));
//     table[irqn] = handler;
//     return success();
// }

// inline result<void> trigger_irq(irqn_t irqn)
// {
//     if(irqn >= table.size()) return failure<void>(make_error_code(system_error::UNKNOWN));
//     table[irqn]();
//     return success();
// }

// }

// namespace utl {

// template <typename T>
// struct isr_unsafe {
//     using ref_t = T;
//     T const& ref;
// };

// template <typename T>
// struct isr_unsafe_mutable {
//     using ref_t = T;
//     T& ref;
// };

// template <typename T>
// concept is_atomic = true;

// template <typename T>
// concept is_marked_unsafe = requires() {
//         typename T::ref_t;
//     } and (same_as<T,isr_unsafe<typename T::ref_t>> or
//     same_as<T,isr_unsafe_mutable<typename T::ref_t>>);

// template <typename T>
// concept any_isr_safe = is_atomic<T> or is_marked_unsafe<T>;



// void isr(auto&&... args)
// { 
//     static_assert(sizeof...(decltype(args)) == 0, "You must define a free function "
//     "named 'isr' that accepts an irq type (which you can specify, or constrain "
//     "with an appropriate concept) and the argument types you're trying to bind."
//     );
// }

// void isr(auto&&)
// {
//     while(true);
// }

// template <typename T>
// concept any_platform = true;

// template <typename T, typename P>
// concept any_irqn = any_platform<P>;

// template <typename T, typename... As>
// using isr_t = std::add_pointer_t<void(T,As...)>;

// template <typename T>
// void isr_vector()
// {
//     isr(T{});
// }

// template <typename T, auto& Handler, auto& Capture>
// void isr_vector()
// {
//     static constexpr auto& handler = Handler;
//     static constexpr auto& capture = Capture;
//     utl::apply(handler,capture);
// }

// auto bind(auto&& irq, auto&&... args)
// {
//     static constexpr isr_t<decltype(irq),decltype(args)...> handler = isr;
//     static auto capture = utl::forward_as_tuple(
//         std::forward<decltype(irq)>(irq), 
//         std::forward<decltype(args)>(args)...
//     );
//     return isr_vector<decltype(irq),handler,capture>;
// }

// }

TEST_GROUP(IRQ) {};

namespace {
struct some_irq{};

struct my_type {
    float value;
};
} //anonymous namespace

static void my_handler(auto, bool& flag, my_type& value)
{
    flag = true;
    value.value = 1.234f;
}

static void my_handler(auto, bool& flag, my_type const&)
{
    flag = true;
}

static void my_handler(auto, bool& flag, my_type value)
{
    flag = true;
    value.value = 1.234f;
    utl::maybe_unused(value);
}

TEST(IRQ,bind) {
    bool flag = false;
    my_type value{0.0f};

    //this call does a few things:
    // - generates a void(void) function that is suitable as a vector
    // - statically stores a tuple of the arguments in the generated function
    // - statically stores a pointer to the handler in the generated function
    //the compiler is able to see through 
    // const auto vector = utl::irq::static_bind(
    //     some_irq{},
    //     my_handler,
    //     utl::irq::unsafe_ref(flag),
    //     utl::irq::unsafe_ref(value)
    // );

    // const auto vector2 = utl::irq::static_bind(
    //     some_irq{},
    //     my_handler,
    //     utl::irq::unsafe_ref(flag),
    //     utl::irq::unsafe_cref(value)
    // );

    // const auto vector3 = utl::irq::static_bind(
    //     some_irq{},
    //     my_handler,
    //     utl::irq::unsafe_ref(flag),
    //     value
    // );
    // CHECK_EQUAL(true,false);

    // utl::maybe_unused(vector2,vector3);

    //this doesn't exist yet
    //set_vector(platform,some_irq{},vector);

    //pretend this is the interrupt happening.
    //
    // vector();

    CHECK_EQUAL(true,flag);
    CHECK_EQUAL(1.234f,value.value);
}

// TEST(IRQ, Invocation)
// {
//     bool flag = false;
//     uint32_t value = 0u;
    
//     auto res = utl::set_isr(0u, my_handler, flag, value);
//     CHECK(static_cast<bool>(res));

//     res = utl::platform::trigger_irq(0u);
//     CHECK(static_cast<bool>(res));
//     CHECK_EQUAL(true, flag);
//     CHECK_EQUAL(1u, value);


//     //this opens the way for a stronger formalization of data structures
//     //that are used for communication with an interrupt context.
//     //datatypes that are safe can be marked as such, and therefore satisfy
//     //the associated isr_safe concept.
//     //one thought: a relatively simple atomic wrapper that exposes:
//     // - read
//     // - write
//     // - "mutate"
//     // mutate would take a non-capturing lambda, and call it with the
//     // object's current value. the lambda would be expected to return
//     // a new value for the object. this call would be properly fenced
//     // and guarded, and if the wrapped value changed during the call then 
//     // the lambda would be tried again.
//     // this could even allow for additional arguments to be passed through
//     // to the mutator lambda. maybe capturing is okay?
//     // the danger is in doing things that have side effects. so I think
//     // it needs to have fully const inputs - I don't think that's possible
//     // to specify in a lambda, so I could pass references through but
//     // force them to be const.
// }
