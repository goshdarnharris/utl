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

struct foo {};


constexpr void _format(foo const& arg, utl::callable<void,char> auto&& out, utl::field const& f)
{
    utl::maybe_unused(arg,f);
    out('^');
}

TEST(Format,Develop)
{
    // utl::string<30> test{};
    // size_t pos = 0;
    // auto do_it = [&](char c) {
    //     test[pos++] = c;
    // };
    // utl::_inner(do_it, "{1}, {}, {}", 10u, 1.0f, foo{});
    constexpr utl::string_view bar = "hi there!";
    auto test = utl::format<60>("{3}, {4}, {2}, {1}, {{, }}, {{{0}}}", 10u, -5, 1.0f, foo{}, bar);
    utl::log(test);
}
