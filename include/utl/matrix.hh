﻿// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef UTL_MATRIX_HH_
#define UTL_MATRIX_HH_

#include "utl/utl.hh"

namespace utl {

namespace detail {

template <typename T>
struct iterator {
    using value_t = typename T::value_t;
    using index_t = typename T::index_t;
    T& matrix;
    index_t index;

    value_t& operator*() {
        return matrix[index];
    }
    value_t const& operator*() const {
        return matrix[index];
    }

    iterator& operator++() {
        auto next_row = index[1] + 1;
        if(next_row == matrix.height()) {
            index[0]++;
            index[1] = 0;
        } else {
            index[1]++;
        }
        return *this;
    }

    bool operator==(iterator const& rhs) const
    {
        return (&matrix == &rhs.matrix) && (index[0] == rhs.index[0])
            && (index[1] == rhs.index[1]);
    }
    
    bool operator!=(iterator const& rhs) const
    {
        return !(*this == rhs);
    }
};

template <typename T>
iterator(T,typename T::index_t) -> iterator<T>;

} //namespace detail


struct matrix_index_t : utl::array<size_t,2> {
    size_t& col;
    size_t& row;

    constexpr matrix_index_t(size_t c, size_t r)
      : utl::array<size_t,2>{{c,r}}, 
        col{this->data()[0]}, row{this->data()[1]}
    {}

    constexpr bool operator==(matrix_index_t const& other) const {
        return other.col == col and other.row == row;
    }
};

template <typename T, size_t W, size_t H>
struct matrix {
    using value_t = T;
    using index_t = matrix_index_t;

    T _storage[W][H];

    constexpr T& operator[](const index_t idx) {
        return _storage[idx[0]][idx[1]];
    }

    constexpr T const& operator[](const index_t idx) const {
        return _storage[idx[0]][idx[1]];
    }

    constexpr size_t size() const {
        return W*H;
    }

    constexpr size_t width() const {
        return W;
    }

    constexpr size_t height() const {
        return H;
    }

    auto begin() {
        return detail::iterator{*this, {0,0}};
    }

    auto begin() const {
        return detail::iterator{*this, {0,0}};
    }

    auto end() {
        return detail::iterator{*this, {W-1,H-1}};
    }

    auto end() const {
        return detail::iterator{*this, {W-1,H-1}};
    }

    constexpr T* data(void)
    {
        return _storage;
    }

    constexpr T const* data(void) const
    {
        return _storage;
    }
};


} // namespace utl

#endif // UTL_MATRIX_HH_
