/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include "utl/test-types.hh"
#include "utl/utl.hh"
#include "utl/names.hh"
#include <utl/logger.hh>
#include <utl/format.hh>
#include <utl/string.hh>

TEST_GROUP(Format) {

void setup(void)
{
}

void teardown(void)
{
}

};


using namespace utl::literals;

struct foo {
    int value;
};

constexpr void _format(foo const& arg, utl::fmt::output& out, utl::fmt::field const& f)
{
    utl::maybe_unused(f);
    utl::format_to(out, "{:#06x}", arg.value);
}

TEST(Format,AutomaticNumbering)
{
    constexpr utl::string_view bar = "hi there!";
    auto test = utl::format<60>("{}, {}, {:{}}", bar, 1.0f, 5, 7);
    CHECK("hi there!, {:f},     7"_sv.compare(test));
}

TEST(Format,ManualNumbering)
{
    constexpr utl::string_view bar = "hi there!";
    auto test = utl::format<60>("{3}, {4}, {2}, {1}, {{, }}, {{{0}}}, {", 10u, -5, 1.0f, foo{42}, bar);
    CHECK("0x002a, hi there!, {:f}, -5, {, }, {10}, "_sv.compare(test));
}

TEST(Format,Alignment)
{
    auto test = utl::format<60>("{:_^16,}", 1234567);
    CHECK("___1,234,567____"_sv.compare(test));
}

TEST(Format,String)
{
    utl::string<10> str = "hello";
    auto test = utl::format<60>("{}", str);
    CHECK("\"hello     \""_sv.compare(test));
}
