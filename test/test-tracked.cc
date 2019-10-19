/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include <utility>
#include "test-types.hh"
#include "common.hh"

TEST_GROUP(Tracked) {

void setup(void)
{
}

void teardown(void)
{
}

};

TEST(Tracked,DefaultConstruction)
{
    utl::tracked obj{};
    
    CHECK(obj.default_constructed());
    CHECK(!obj.regular_constructed());
    CHECK(!obj.copy_constructed());
    CHECK(!obj.move_constructed());
    CHECK(obj.copy_assigned() == 0);
    CHECK(obj.move_assigned() == 0);
}

TEST(Tracked, RegularConstruction)
{
    utl::tracked obj{1,2,3};
    
    CHECK(!obj.default_constructed());
    CHECK(obj.regular_constructed());
    CHECK(!obj.copy_constructed());
    CHECK(!obj.move_constructed());
    CHECK(obj.copy_assigned() == 0);
    CHECK(obj.move_assigned() == 0);
}

TEST(Tracked, CopyConstruction)
{
    utl::tracked a{};
    utl::tracked b{a};

    CHECK(!b.default_constructed());
    CHECK(!b.regular_constructed());
    CHECK(b.copy_constructed());
    CHECK(!b.move_constructed());
    CHECK(b.copy_assigned() == 0);
    CHECK(b.move_assigned() == 0);
} 

TEST(Tracked, MoveConstruction)
{
    utl::tracked a{};
    utl::tracked b{std::move(a)};
    
    CHECK(!b.default_constructed());
    CHECK(!b.regular_constructed());
    CHECK(!b.copy_constructed());
    CHECK(b.move_constructed());
    CHECK(b.copy_assigned() == 0);
    CHECK(b.move_assigned() == 0);
}

TEST(Tracked, CopyAssignment)
{
    utl::tracked a{};
    utl::tracked b{};

    a = b;

    CHECK(a.copy_assigned() == 1);
    CHECK(a.move_assigned() == 0);
    CHECK(b.copy_assigned() == 0);
    CHECK(b.move_assigned() == 0);
}

TEST(Tracked, MoveAssignment)
{
    utl::tracked a{};
    utl::tracked b{};

    a = std::move(b);

    CHECK(a.copy_assigned() == 0);
    CHECK(a.move_assigned() == 1);
}
