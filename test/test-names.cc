/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include "utl/test-types.hh"
#include "utl/utl.hh"
#include "utl/names.hh"
#include <utl/logger.hh>
#include <utl/string.hh>

TEST_GROUP(Names) {

void setup(void)
{
}

void teardown(void)
{
}

};

enum class foo {
    HEY,
    THERE
};

TEST(Names,ConstexprGetTypeName)
{
    constexpr utl::string_view foo{"hello"};
    constexpr auto name = utl::get_type_name(foo);
    CHECK(name.compare("utl::string_view"));
}

TEST(Names,GetTypeName)
{
    utl::string_view foo{"hello"};
    auto name = utl::get_type_name(foo);
    CHECK(name.compare("utl::string_view"));
}

TEST(Names,LValueReference)
{    
    utl::string_view foo{"hello"};
    utl::string_view& bar = foo;
    auto name = utl::get_type_name(bar);
    CHECK(name.compare("utl::string_view"));
}

TEST(Names,ConstexprLValueReference)
{    
    constexpr utl::string_view foo{"hello"};
    utl::string_view const& bar = foo;
    auto name = utl::get_type_name(bar);
    CHECK(name.compare("utl::string_view"));
}

TEST(Names,ConstLValueReference)
{    
    utl::string_view foo{"hello"};
    utl::string_view const& bar = foo;
    auto name = utl::get_type_name(bar);
    CHECK(name.compare("utl::string_view"));
}

TEST(Names,ConstexprRValueReference)
{    
    constexpr utl::string_view foo{"hello"};
    auto name = utl::get_type_name(std::move(foo));
    CHECK(name.compare("utl::string_view"));
}

TEST(Names,RValueReference)
{    
    utl::string_view foo{"hello"};
    auto name = utl::get_type_name(std::move(foo));
    CHECK(name.compare("utl::string_view"));
}

TEST(Names,ConstRValueReference)
{    
    utl::string_view foo{"hello"};
    auto name = utl::get_type_name(const_cast<const utl::string_view&&>(std::move(foo)));
    CHECK(name.compare("utl::string_view"));
}

TEST(Names,Noncopyable)
{
    awful::noncopyable foo{};
    auto name = utl::get_type_name(foo);
    CHECK(name.compare("awful::noncopyable"));
}

TEST(Names,GetEnumValueNameLiteral)
{
    constexpr auto name = utl::get_enum_name(foo::HEY);
    CHECK(name.compare("foo::HEY"));
}

TEST(Names,ConstexprGetEnumValueNameRuntime)
{
    constexpr auto value = foo::THERE;
    constexpr auto name = utl::get_enum_name(value);
    CHECK(name.compare("foo::THERE"));
}

TEST(Names,GetEnumValueNameRuntime)
{
    auto value = foo::THERE;
    auto name = utl::get_enum_name(value);
    CHECK(name.compare("foo::THERE"));
}

TEST(Names,GetEnumValueNameUnnamed)
{
    auto value = static_cast<foo>(10u);
    auto name = utl::get_enum_name(value);
    CHECK(name.compare(""));
}

TEST(Names,EnumCount)
{
    constexpr auto count = utl::enum_count<foo>;
    CHECK(count == 2);
}

TEST(Names,CopyIntoConstexprString)
{
    utl::string_view foo{"hello"};
    constexpr auto name = utl::get_type_name(foo);
    constexpr utl::string<20> bar{name.c_str()};
    utl::log("\n\"%s\", \"%s\", %x, %x\n", name.c_str(), bar.c_str(), name.data(), bar.data());
    CHECK(name.compare("utl::string_view"));
    CHECK(bar.compare("utl::string_view"));
}

TEST(Names,CopyIntoString)
{
    utl::string_view foo{"hello"};
    auto name = utl::get_type_name(foo);
    utl::string<20> bar{name.c_str()};
    utl::log("\n\"%s\", \"%s\", %x, %x\n", name.c_str(), bar.c_str(), name.data(), bar.data());
    CHECK(name.compare("utl::string_view"));
    CHECK(bar.compare("utl::string_view"));
}
