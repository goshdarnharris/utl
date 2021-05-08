#pragma once

#include <utility>
#include <bits/refwrap.h>

namespace utl::irq {

template <typename T>
struct _bind_unwrap { using type = std::unwrap_ref_decay_t<T>; };

template <typename T>
struct _bind_unwrap<T&&> { using type = T&&; };

template <typename T>
using _bind_unwrap_t = typename _bind_unwrap<T>::type;

} //namespace utl::irq
