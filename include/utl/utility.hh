
#pragma once

namespace utl {
template<class T>
[[deprecated]] constexpr inline bool print_type = true;

template<template <typename> typename T>
[[deprecated]] constexpr inline bool print_unary_template = true;
} //namespace utl
