/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include "construct.hh"
#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include "test-types.hh"
#include "utl.hh"
#include "system-error.hh"

TEST_GROUP(Construct) {

void setup(void)
{
}

void teardown(void)
{
}

};

struct construct_me {
    int m_data;
    bool m_valid;
    
protected:
    construct_me(bool valid) : m_data{10}, m_valid{valid} {}
    utl::result<void> validate() {
        if(m_valid) {
            return utl::success();
        }
        return utl::system_error::UNKNOWN;
    }
};

struct tracked_constructible : utl::tracked {
    using utl::tracked::tracked;

    utl::result<void> validate() {
        return {};
    }
};

TEST(Construct,Construction)
{
    auto obj = utl::construct<construct_me>{true};
    CHECK(static_cast<bool>(obj));

    auto obj2 = utl::construct<construct_me>{false};
    CHECK(!static_cast<bool>(obj2));
}

TEST(Construct, InPlaceDefaultConstruction)
{
    utl::construct<tracked_constructible> obj{};

    CHECK(static_cast<bool>(obj));
    CHECK(obj.value().default_constructed());
    CHECK(!obj.value().regular_constructed());
    CHECK(!obj.value().copy_constructed());
    CHECK(!obj.value().move_constructed());
    CHECK(obj.value().copy_assigned() == 0);
    CHECK(obj.value().move_assigned() == 0);
}

TEST(Construct, InPlaceRegularConstruction)
{
    utl::construct<tracked_constructible> obj{1,2,3};

    CHECK(static_cast<bool>(obj));
    CHECK(!obj.value().default_constructed());
    CHECK(obj.value().regular_constructed());
    CHECK(!obj.value().copy_constructed());
    CHECK(!obj.value().move_constructed());
    CHECK(obj.value().copy_assigned() == 0);
    CHECK(obj.value().move_assigned() == 0);
}

TEST(Construct, InPlaceCopyConstruction)
{
    tracked_constructible a{};
    utl::construct<tracked_constructible> obj{a};

    CHECK(static_cast<bool>(obj));
    CHECK(!obj.value().default_constructed());
    CHECK(!obj.value().regular_constructed());
    CHECK(!obj.value().move_constructed());
    CHECK(obj.value().copy_constructed());
    CHECK(obj.value().copy_assigned() == 0);
    CHECK(obj.value().move_assigned() == 0);
}

TEST(Construct, InPlaceMoveConstruction)
{
    tracked_constructible a{};
    utl::construct<tracked_constructible> obj{std::move(a)};

    CHECK(static_cast<bool>(obj));
    CHECK(!obj.value().default_constructed());
    CHECK(!obj.value().regular_constructed());
    CHECK(!obj.value().copy_constructed());
    CHECK(obj.value().move_constructed());
    CHECK(obj.value().copy_assigned() == 0);
    CHECK(obj.value().move_assigned() == 0);
}
