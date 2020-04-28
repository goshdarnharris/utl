/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include "utl/test-types.hh"
#include "utl/utl.hh"
#include "utl/string.hh"

TEST_GROUP(String) {

void setup(void)
{
}

void teardown(void)
{
}

};

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

TEST(String,LiteralStringLengthDeduction)
{
    utl::string foo{"hello"};
    CHECK(foo.size() == 5);
    utl::maybe_unused(foo);
}

TEST(String,ConstantStringLengthDeduction)
{
    const char str[6] = "hello";
    utl::string foo{str};
    CHECK(foo.size() == 5);
    utl::maybe_unused(foo);
}

// TEST(String,FormattedLengthDeduction)
// {
//     auto foo = utl::string{"hello",'a'};
//     CHECK(foo.size() == 1);
//     utl::maybe_unused(foo);
// }

