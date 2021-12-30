// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include "utl/test-types.hh"
#include "utl/utl.hh"
#include "utl/array.hh"
#include <utl/logger.hh>

TEST_GROUP(array) {};

TEST(array,Construction)
{
    const utl::array<int,5> foo{{1,2,3,4,5}};
    utl::maybe_unused(foo);
}

TEST(array,Constexpr)
{
    constexpr utl::array<int,5> foo{{1,2,3,4,5}};
    utl::maybe_unused(foo);
}

TEST(array,Iteration)
{    
    const utl::array<int,5> foo{{1,2,3,4,5}};
    auto count = 0u;
    for(auto const& v : foo) { 
        utl::maybe_unused(v);
        count++; 
    }
    CHECK(count == 5);
}
