// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0


#pragma once

namespace utl {
template<class T>
[[deprecated]] constexpr inline bool print_type = true;

template<template <typename> typename T>
[[deprecated]] constexpr inline bool print_unary_template = true;
} //namespace utl
