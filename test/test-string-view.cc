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
#include "utl/string-view.hh"

TEST_GROUP(StringView) {};

TEST(StringView,Construction)
{
    const utl::string_view foo{"hello"};
    CHECK(foo.size() == 5);
}

TEST(StringView,Constexpr)
{
    constexpr utl::string_view foo{"hello"};
    CHECK(foo.size() == 5);
}

TEST(StringView,ConstexprSubstr)
{
    constexpr utl::string_view foo{"hello"};
    constexpr auto bar = foo.substr(1,2);
    CHECK(bar.size() == 2);

    constexpr auto baz = foo.substr(6,2);
    CHECK(baz.size() == 1);
    CHECK(baz.data() == &foo.data()[4]); //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

TEST(StringView,ConstexprStartsWith)
{
    constexpr utl::string_view foo{"hello"};
    constexpr bool a = foo.starts_with("he");
    constexpr bool b = foo.starts_with("llo");
    CHECK(a);
    CHECK(!b);
}

TEST(StringView,ConstexprFind)
{
    constexpr utl::string_view foo{"hello"};
    constexpr auto a = foo.find("he");
    constexpr auto b = foo.find("llo");
    constexpr auto c = foo.find("what?");
    CHECK(a == 0);
    CHECK(b == 2);
    CHECK(c == utl::npos);
}

TEST(StringView,ConstexprCompare)
{
    constexpr utl::string_view foo{"hello"};
    constexpr auto a = foo.compare("hello");
    constexpr auto b = foo.compare("he");
    constexpr auto c = foo.compare("llothere");
    CHECK(a);
    CHECK(!b);
    CHECK(!c);
}

// TEST(StringView,FormattedLengthDeduction)
// {
//     auto foo = utl::string{"hello",'a'};
//     CHECK(foo.size() == 1);
//     utl::maybe_unused(foo);
// }

