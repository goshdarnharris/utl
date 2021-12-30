// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utility>
#include <bits/refwrap.h>

namespace utl::irq {

template <typename T>
struct _unwrap_isr_safe { using type = std::unwrap_ref_decay_t<T>; };

template <typename T>
struct _unwrap_isr_safe<T&&> { using type = T&&; };

template <typename T>
using _unwrap_isr_safe_t = typename _unwrap_isr_safe<T>::type;

} //namespace utl::irq
