/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/traits.hh>
#include <utl/logger.hh>

TEST_GROUP(Traits) {
    void setup(void) {}
    void teardown(void) {}
};

TEST(Traits,is_same)
{
    CHECK((utl::is_same_v<int,int>) == true);
    CHECK((utl::is_same_v<int,float>) == false);
    CHECK((utl::is_same_v<int&,int&>) == true);
    CHECK((utl::is_same_v<int&,int>) == false);
}


