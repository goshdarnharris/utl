
#ifndef UTL_TYPE_LIST_HH_
#define UTL_TYPE_LIST_HH_

#include <utl/traits.hh>

namespace utl {

template <typename... Ts>
struct type_list {};


template<class List, class T> struct contains_impl;

template<template<class...> class List, class... Ts, class T>
struct contains_impl<List<Ts...>, T>
{
    constexpr static bool value = (is_same<Ts,T>::value || ...);
};

template <class List, class T>
using contains = contains_impl<List, T>;

template <class List, class T>
constexpr bool contains_v = contains<List,T>::value;


} //namespace utl

#endif //UTL_TYPE_LIST_HH_
