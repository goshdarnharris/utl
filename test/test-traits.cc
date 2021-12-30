// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/traits.hh>
#include <utl/logger.hh>

TEST_GROUP(Traits) {};

TEST(Traits,is_same)
{
    CHECK((utl::is_same_v<int,int>) == true);
    CHECK((utl::is_same_v<int,float>) == false);
    CHECK((utl::is_same_v<int&,int&>) == true);
    CHECK((utl::is_same_v<int&,int>) == false);
}


