#pragma once

#include <utl/tuple.hh>
#include <platform/vector-table.hh>
#include <concepts>

namespace utl {

using irqn_t = platform::irqn_t;




// template <auto irqn, auto F, typename... Ts>
// auto set_isr(any_isr_safe auto&... args)
// {
//     static utl::tuple<decltype(args)...> capture{args...};
//     return platform::vectors().set_vector(irqn, irq_handler<F,capture>);
// }

// template <typename T, auto F, typename... Ts>
// auto set_isr(any_isr_safe auto&... args)
// {
//     set_isr<T::irqn, F>(args...);
// }

} //namespace utl
