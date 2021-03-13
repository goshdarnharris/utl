
#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/names.hh>
#include <utl/system-error.hh>
#include <type_traits>
#include <atomic>

// template <typename H, size_t IRQn>
// concept has_isr = requires(const H* ptr) {
//     ptr->template isr<IRQn>();
// };

// template <size_t IRQn>
// static bool irq_used = false;

// template <size_t IRQn, has_isr<IRQn> H>
// struct irq {
//     static const volatile H* _target;
//     static void _handler()
//     {
//         if(_target) {
//             _target->template isr<IRQn>();
//         }
//     }

//     irq(irq&) = delete;
//     irq(irq&&) = default;
//     irq& operator=(irq&) = delete;
//     irq& operator=(irq&&) = default;

//     irq(H& target) { 
//         //enforce a single instance. probably have to do it at runtime.
//         //checking, then logging a severe warning if a target already exists
//         //for this IRQn (probably by looking at the vector table) seems 
//         //reasonable.
//         //or maybe try to do it through constraints on moves/copies?

//         //cannot copy
//         //can move (that's going to be interesting)
//         //can I enforce all references being const?
//         //maybe I can constrain overloads with reference qualifiers
//         //so that the only way anything can be done with a reference
//         //is if it is const
//         if(irq_used<IRQn>) {
//             //panic
//         }
//         irq_used<IRQn> = true;
//         _target = &target; 
//         //set up vector table
//     }

//     ~irq() {
//         //set vector table to default
//         _target = nullptr;
//     }
// };


// template <size_t IRQn, has_isr<IRQn> H>
// auto make_irq(H& target)
// {
//     //this should panic if it's called more than once for a given IRQn.
//     return irq<IRQn,H>{target};
// }

// template <size_t IRQn, has_isr<IRQn> H>
// volatile const H* irq<IRQn,H>::_target = nullptr;


// //Alright. I think I need to lift IRQ numbers. They have more semantics than just a number.
// // - they belong to a peripheral
// // - they have groupings:
// //   - by kind, e.g. TIMx_UPDATE (the group of all timer update IRQs)
// //   - by peripheral
// //   - probably some DMA-related groupings
// // - they have associated flags in peripheral registers
// //
// // All of this is constexpr.
// // Providing concepts will then allow isr methods to be usefully constrained.
// // I'll be able to interact directly with the IRQs in a more abstract way.
// // I and users will be able to easily handle special cases.

// // How is all of this organized? How do I spell things?
// // hal::irq
// // hal::


// struct driver {
//     mutable size_t data{};

//     //I almost need this to be templated on an "IRQ grouping" rather than the specific IRQn.
//     //That is, in a driver object I don't want to distinguish between specific IRQ numbers.
//     //Just a slightly more abstract grouping: which of the interrupts for this peripheral instance
//     //has occurred?
//     //Okay. so that's something I've lost by pinning the method name. previously that's how those
//     //semantics were expressed.
//     //I could... group IRQ numbers by type. or provide concepts to constrain them.
//     //  that could have interesting side effects, because suddenly you can have compound constraints...
//     void isr(auto&& irq) const volatile
//     {
//         //do_things_here
//         utl::maybe_unused(irq);
//         data = 5;
//         utl::log(utl::get_type_name(data));
//         utl::log(utl::type_name<decltype(data)>);

//         //Okay. this means that shared data gets marked.
//         //that's interesting.
//         //is there a way to enforce shared data being declared as volatile?
//         //I don't need them to be marked as volatile in here, but I do
//         //need them to be marked as volatile at their declaration.
//         //or, really, I need some predicate to be true on the member
//         //variables that are used in this method. eek.

//         //Maybe it's a type wrapper that handles mutability and volatility internally?

//         //How do I enforce shared<>?
//     }
// };

#include <utl/concepts.hh>

namespace utl::platform {

//FIXME: this should be a type that implicitly constrains its own value.
//this would mean that functions using it don't need to check that it is in range.
using irqn_t = size_t; 
using handler_t = void(*)(void);
inline constexpr size_t VECTOR_TABLE_SIZE = 10;

[[noreturn]] static void default_handler() { while(true) {} }

//FIXME: in combination with a richer irqn type, maybe there's
//a richer vector table type.
static constinit array<handler_t,VECTOR_TABLE_SIZE> table{{
    default_handler,
    default_handler,
    default_handler,
    default_handler,
    default_handler,
    default_handler,
    default_handler,
    default_handler,
    default_handler,
    default_handler
}};

inline result<void> set_vector(irqn_t irqn, handler_t handler)
{
    if(irqn >= table.size()) return failure<void>(make_error_code(system_error::UNKNOWN));
    table[irqn] = handler;
    return success();
}

inline result<void> trigger_irq(irqn_t irqn)
{
    if(irqn >= table.size()) return failure<void>(make_error_code(system_error::UNKNOWN));
    table[irqn]();
    return success();
}

}

namespace utl {

template <typename T>
struct isr_unsafe {
    using ref_t = T;
    T const& ref;
};

template <typename T>
struct isr_unsafe_mutable {
    using ref_t = T;
    T& ref;
};

template <typename T>
concept is_atomic = true;

template <typename T>
concept is_marked_unsafe = requires() {
        typename T::ref_t;
    } and (same_as<T,isr_unsafe<typename T::ref_t>> or
    same_as<T,isr_unsafe_mutable<typename T::ref_t>>);

template <typename T>
concept isr_safe = is_atomic<T> or is_marked_unsafe<T>;



template <auto& Handler, auto& Capture>
void isr_vector()
{
    static constexpr auto& handler = Handler;
    static constexpr auto& capture = Capture;
    utl::apply(handler,capture);
}

using void_f = void(*)(void);

template <auto& Handler, auto& Capture>
auto configure_vector(platform::irqn_t irqn)
{
    static constexpr auto& handler = Handler;
    static constexpr auto& capture = Capture;
    return platform::set_vector(irqn, isr_vector<handler,capture>);
}

template <typename F, typename... Ts>
auto set_isr(platform::irqn_t irqn, F handler, Ts&... args)
{
    static utl::tuple<Ts&...> capt{args...};
    static F hand = handler;
    return configure_vector<hand,capt>(irqn);
}

}


TEST_GROUP(IRQ) {};

static void my_handler(bool& flag, uint32_t& value)
{
    flag = true;
    value++;
}


TEST(IRQ, Invocation)
{
    bool flag = false;
    uint32_t value = 0u;
    
    auto res = utl::set_isr(0u, my_handler, flag, value);
    CHECK(static_cast<bool>(res));

    res = utl::platform::trigger_irq(0u);
    CHECK(static_cast<bool>(res));
    CHECK_EQUAL(true, flag);
    CHECK_EQUAL(1u, value);


    //this opens the way for a stronger formalization of data structures
    //that are used for communication with an interrupt context.
    //datatypes that are safe can be marked as such, and therefore satisfy
    //the associated isr_safe concept.
    //one thought: a relatively simple atomic wrapper that exposes:
    // - read
    // - write
    // - "mutate"
    // mutate would take a non-capturing lambda, and call it with the
    // object's current value. the lambda would be expected to return
    // a new value for the object. this call would be properly fenced
    // and guarded, and if the wrapped value changed during the call then 
    // the lambda would be tried again.
    // this could even allow for additional arguments to be passed through
    // to the mutator lambda. maybe capturing is okay?
    // the danger is in doing things that have side effects. so I think
    // it needs to have fully const inputs - I don't think that's possible
    // to specify in a lambda, so I could pass references through but
    // force them to be const.
}
