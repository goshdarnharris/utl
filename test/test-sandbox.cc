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
#include <utl/string-view.hh>
#include <utl/tuple.hh>
#include <utl/ranges.hh>
#include <utl/logger.hh>

TEST_GROUP(Sandbox) {};

template <typename T>
struct S {
    static constexpr auto value() { }
};

constexpr size_t do_it(utl::string_view sv)
{
    size_t s = sv.size();
    return s;
}

TEST(Sandbox,ConstexprStringViews)
{
    constexpr size_t s = do_it("constant string");
    utl::maybe_unused(s);    
}

