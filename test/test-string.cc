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
#include "utl/string.hh"

TEST_GROUP(String) {};

TEST(String,Construction)
{
    const utl::string<5> foo{"hello"};
    utl::maybe_unused(foo);
}

TEST(String,Constexpr)
{
    constexpr utl::string<5> foo{"hello"};
    utl::maybe_unused(foo);
}

TEST(String,ConstexprArray)
{
    //NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    constexpr const char str[6] = "hello";
    //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    constexpr utl::string foo{str};
    CHECK(foo.size() == 5);
    utl::maybe_unused(foo);
}

TEST(String,ConstexprPointer)
{
    constexpr const char* str = "hello";
    constexpr utl::string<5> foo{str};
    CHECK(foo.size() == 5);
}

TEST(String,LiteralStringLengthDeduction)
{
    utl::string foo{"hello"};
    CHECK(foo.size() == 5);
    utl::maybe_unused(foo);
}

TEST(String,ConstantStringLengthDeduction)
{
    //NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    const char str[6] = "hello";
    //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    utl::string foo{str};
    CHECK(foo.size() == 5);
    utl::maybe_unused(foo);
}

TEST(String,ConstPointer)
{
    const char* str = "hello";
    utl::string<5> foo{str}; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    CHECK(foo.size() == 5);
}

TEST(String,StringViewConversion)
{
    utl::string foo{"hello"};
    utl::string_view bar{foo};
    CHECK(bar.data() == foo.data());
    CHECK_EQUAL(0, foo.compare(bar));
}

TEST(String,Substr)
{
    utl::string foo{"hello"};
    auto bar = foo.substr(1,2);
    CHECK(bar.length() == 2);

    auto baz = foo.substr(6,2); //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    CHECK(baz.length() == 1);
    CHECK(strcmp(baz.data(), "o") == 0);
}

TEST(String,ConstexprSubstr)
{
    constexpr utl::string foo{"hello"};
    constexpr auto bar = foo.substr(1,2);
    CHECK(bar.length() == 2);

    constexpr auto baz = foo.substr(6,2);
    CHECK(baz.length() == 1);
    CHECK(strcmp(baz.data(), "o") == 0);
}

TEST(String,StartsWith)
{
    utl::string foo{"hello"};
    bool a = foo.starts_with("he");
    bool b = foo.starts_with("llo");
    CHECK(a);
    CHECK(!b);
}

TEST(String,ConstexprStartsWith)
{
    constexpr utl::string foo{"hello"};
    constexpr bool a = foo.starts_with("he");
    constexpr bool b = foo.starts_with("llo");
    CHECK(a);
    CHECK(!b);
}

TEST(String,Find)
{
    utl::string foo{"hello"};
    auto a = foo.find("he");
    auto b = foo.find("llo");
    auto c = foo.find("what?");
    CHECK(a == 0);
    CHECK(b == 2);
    CHECK(c == utl::npos);
}

TEST(String,ConstexprFind)
{
    constexpr utl::string foo{"hello"};
    constexpr auto a = foo.find("he");
    constexpr auto b = foo.find("llo");
    constexpr auto c = foo.find("what?");
    CHECK(a == 0);
    CHECK(b == 2);
    CHECK(c == utl::npos);
}

TEST(String,Compare)
{
    utl::string foo{"hello"};
    auto a = foo.compare("hello");
    auto b = foo.compare("he");
    auto c = foo.compare("llothere");
    CHECK(a == 0);
    CHECK(b > 0);
    CHECK(c < 0);
}

TEST(String,ConstexprCompare)
{
    constexpr utl::string foo{"hello"};
    constexpr auto a = foo.compare("hello");
    constexpr auto b = foo.compare("he");
    constexpr auto c = foo.compare("llothere");
    CHECK(a == 0);
    CHECK(b > 0);
    CHECK(c < 0);
}
