/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include "utl/variant.hh"
#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include "utl/test-types.hh"
#include "utl/utl.hh"
#include "utl/system-error.hh"

TEST_GROUP(Variant) {};


TEST(Variant,Construction)
{
    auto obj = utl::variant<int32_t,float,uint8_t>{0};
    utl::maybe_unused(obj);
}

TEST(Variant,Visitation)
{
    auto obj = utl::variant<int32_t,float,uint8_t>{1.0f};
    bool visited = false;
    
    obj.accept([&](auto& value) {
        visited = true;
        constexpr auto is_same = utl::is_same_v<decltype(value),float&>;
        CHECK(is_same);
    });

    CHECK(visited);
}

TEST(Variant,VisitationFirst)
{
    const auto obj = utl::variant<int32_t,float>{-10};
    bool visited = false;
    
    obj.accept([&](auto& value) {
        visited = true;
        // constexpr auto is_same = utl::is_same_v<decltype(value),int32_t&>;
        CHECK_EQUAL(-10, value);
    });

    CHECK(visited);
}

TEST(Variant,VisitationLast)
{
    auto obj = utl::variant<int32_t,float>{1.0f};
    bool visited = false;
    
    obj.accept([&](auto& value) {
        visited = true;
        constexpr auto is_same = utl::is_same_v<decltype(value),float&>;
        CHECK(is_same);
    });

    CHECK(visited);
}

TEST(Variant,Assignment)
{
    auto obj = utl::variant<int32_t,float>{1.0f};
    
    obj = -10; //NOLINT(cppcoreguidelines-avoid-magic-numbers);
    
    bool visited = false;
    obj.accept([&](auto& value) {
        visited = true;
        constexpr auto is_same = utl::is_same_v<decltype(value),int32_t&>;
        CHECK(is_same);
        CHECK_EQUAL(value, -10); //NOLINT(cppcoreguidelines-avoid-magic-numbers);
    });

    CHECK(visited);
}
