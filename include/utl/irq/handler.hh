#pragma once

#include <utl/utl.hh>
#include <utl/tuple.hh>
#include <utl/irq/safe.hh>
#include <utl/irq/bits.hh>
#include <utl/concepts.hh>
#include <type_traits>

namespace utl::irq {



template <typename T>
concept any_platform = true;

template <typename T, typename P>
concept any_irqn = any_platform<P>;

template <typename T>
void vector()
{
    isr(T{});
}

template <typename T, auto& Handler, auto& Capture>
void vector()
{
    static constexpr auto& handler = Handler;
    static constexpr auto& capture = Capture;
    utl::apply(handler,capture);
}

//either put it in the global scope or force the user to use a tag for adl
void isr(auto&&... args)
{ 
    static_assert(sizeof...(args) == 0, "You must define a free function "
    "named 'isr' that accepts an irq type (which you can specify, or constrain "
    "with an appropriate concept) and the argument types you're trying to bind."
    );
    utl::maybe_unused(args...);
}

//FIXME: what's idiomatic c++ for dealing with something like this vector
// return value that can capture local variables?
//how to communicate that this has global behaviour?
template <typename... Ts>
    requires (any_isr_safe<std::unwrap_ref_decay_t<Ts&&>> and ...)
[[nodiscard]] auto bind(auto irq, void(*isr)(decltype(irq),_bind_unwrap_t<Ts&&>...), Ts&&... args)
{
    using irq_t = decltype(irq);  
    using capture_t = utl::tuple<irq_t,_bind_unwrap_t<decltype(args)>...>;

    static auto handler = isr;
    static auto capture = capture_t{
        std::forward<irq_t>(irq), 
        std::forward<_bind_unwrap_t<decltype(args)>>(args)...
    };
    return vector<irq_t,handler,capture>;
}


} //namespace utl::irq

