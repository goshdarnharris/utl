﻿// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once
#include <utl/array.hh>

namespace utl {

template <typename T>
class span {
    T* m_container;
    size_t m_size;

    [[nodiscard]] constexpr T& access(size_t index) const
    {
        return m_container[index]; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    [[nodiscard]] constexpr T* address(size_t index) const
    {
        return &m_container[index]; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
public:
    constexpr span(T* container, size_t size) : m_container{container}, m_size{size} {}

    template <size_t N>
    constexpr span(T (&arr)[N]) : m_container{arr}, m_size{N} {} //NOLINT(cppcoreguidelines-avoid-c-arrays)

    template <size_t N>
    constexpr span(array<T,N>& arr) : m_container{arr.data()}, m_size{N} {}
    template <size_t N>
    constexpr span(array<T,N> const& arr) : m_container{arr.data()}, m_size{N} {}

    constexpr T& operator[](size_t index) const
    {
        return access(index);
    }
    
    constexpr auto size() { return m_size; }
    
    [[nodiscard]] constexpr T* data() const { return m_container; }
    [[nodiscard]] constexpr size_t size() const { return m_size; }

    [[nodiscard]] constexpr T* begin() const { return address(0); }
    [[nodiscard]] constexpr T* end() const
    { 
        return address(m_size-1) + 1; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    [[nodiscard]] constexpr T* rbegin() const { return address(size()-1); }
    [[nodiscard]] constexpr T* rend() const 
    {
        return address(0) - 1; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    [[nodiscard]] constexpr span subspan(size_t offset, size_t length) const
    {        
        if(offset >= size()) offset = size();
        if(size() == 0) return {offset,0};
        if(offset + length >= size()) length = size() - offset; 
        return {offset,length};
    }
};

template <typename T, size_t N>
span(T (&)[N]) -> span<T>; //NOLINT(cppcoreguidelines-avoid-c-arrays)

template <typename T, size_t N>
span(array<T,N>&) -> span<T>;

} //namespace utl
