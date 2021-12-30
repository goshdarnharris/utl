// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0


#pragma once

#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include "packages/libawful/include/awful.hpp"
#include "utl/test-types.hh"
#include <utl/string.hh>

inline SimpleString StringFrom (utl::string_view view)
{
    return {{view.data(),view.length()}};
}

template <size_t N>
inline SimpleString StringFrom (utl::string<N> const& s)
{
    return {{s.data(),s.length()}};
}
