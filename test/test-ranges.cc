/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/ranges.hh>
#include <utl/logger.hh>

TEST_GROUP(Ranges) {
    void setup(void) {}
    void teardown(void) {}
};

TEST(Ranges,Enumerate)
{
    uint8_t arr[3] = {1,2,3};
    for(auto&& [idx,val] : utl::ranges::enumerate(arr)) {
        CHECK(idx == val - 1);
        CHECK(arr[idx] == val);
    }
}

// TEST(Ranges,Iterate)
// {
//     uint8_t arr[3] = {1,2,3};
//     for(auto&& [iter,val] : utl::ranges::iterate(arr)) {
//         CHECK(utl::get<1>(*iter) == val);        
//     }
// }

// TEST(Ranges,IterateEnumerate)
// {
//     uint8_t arr[3] = {1,2,3};
//     for(auto [iter, en] : utl::ranges::iterate(utl::ranges::enumerate(arr))) {
//         auto&& [idx,val] = en;
//         CHECK(utl::get<1>(*iter) == en);
//         CHECK(idx == val - 1);
//         CHECK(arr[idx] == val);
//     }
// }

