// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef UTL_ALGORITHM_HH_
#define UTL_ALGORITHM_HH_

namespace utl {

template <typename T>
constexpr T const& max(T const& a, T const& b) {
    return a > b ? a : b;
}




} //namespace utl

#endif //UTL_ALGORITHM_HH_
