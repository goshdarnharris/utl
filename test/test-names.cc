/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/names.hh>
#include <utl/logger.hh>
#include <utl/string.hh>
#include "test-support.hh"

TEST_GROUP(Names) {};

using namespace utl::literals;

enum class demo : uint8_t {
    HEY,
    THERE
};

TEST(Names,ConstexprGetTypeName)
{
    constexpr utl::string_view foo{"hello"};
    constexpr auto name = utl::get_type_name(foo);
    CHECK_EQUAL("utl::string_view"_sv, name);
}

TEST(Names,GetTypeName)
{
    utl::string_view foo{"hello"};
    auto name = utl::get_type_name(foo);
    CHECK_EQUAL("utl::string_view"_sv, name);
}

TEST(Names,LValueReference)
{    
    utl::string_view foo{"hello"};
    utl::string_view& bar = foo;
    auto name = utl::get_type_name(bar);
    CHECK_EQUAL("utl::string_view"_sv, name);
}

TEST(Names,ConstexprLValueReference)
{    
    constexpr utl::string_view foo{"hello"};
    utl::string_view const& bar = foo;
    auto name = utl::get_type_name(bar);
    CHECK_EQUAL("utl::string_view"_sv, name);
}

TEST(Names,ConstLValueReference)
{    
    utl::string_view foo{"hello"};
    utl::string_view const& bar = foo;
    auto name = utl::get_type_name(bar);
    CHECK_EQUAL("utl::string_view"_sv, name);
}

TEST(Names,ConstexprRValueReference)
{    
    constexpr utl::string_view foo{"hello"};
    auto name = utl::get_type_name(std::move(foo));
    CHECK_EQUAL("utl::string_view"_sv, name);
}

TEST(Names,RValueReference)
{    
    utl::string_view foo{"hello"};
    auto name = utl::get_type_name(std::move(foo));
    CHECK_EQUAL("utl::string_view"_sv, name);
}

TEST(Names,ConstRValueReference)
{    
    utl::string_view foo{"hello"};
    auto name = utl::get_type_name(const_cast<const utl::string_view&&>(std::move(foo)));
    CHECK_EQUAL("utl::string_view"_sv, name);
}

TEST(Names,Noncopyable)
{
    awful::noncopyable foo{};
    auto name = utl::get_type_name(foo);
    CHECK_EQUAL("awful::noncopyable"_sv, name);
}

TEST(Names,GetEnumValueNameLiteral)
{
    constexpr auto name = utl::get_enum_name(demo::HEY);
    CHECK_EQUAL("demo::HEY"_sv, name);
}

TEST(Names,ConstexprGetEnumValueNameRuntime)
{
    constexpr auto value = demo::THERE;
    constexpr auto name = utl::get_enum_name(value);
    CHECK_EQUAL("demo::THERE"_sv, name);
}

TEST(Names,GetEnumValueNameRuntime)
{
    auto value = demo::THERE;
    auto name = utl::get_enum_name(value);
    CHECK_EQUAL("demo::THERE"_sv, name);
}

TEST(Names,GetEnumValueNameUnnamed)
{
    auto value = static_cast<demo>(10u);
    auto name = utl::get_enum_name(value);
    CHECK_EQUAL(""_sv, name);
}

TEST(Names,EnumCount)
{
    constexpr auto count = utl::enum_count<demo>;
    CHECK_EQUAL(2, count);
}

TEST(Names,CopyIntoConstexprString)
{
    utl::string_view foo{"hello"};
    constexpr auto name = utl::get_type_name(foo);
    auto bar = static_cast<utl::string<20>>(name);
    CHECK_EQUAL("utl::string_view"_sv, name);
    CHECK_EQUAL("utl::string_view"_sv, bar);
}

TEST(Names,CopyIntoString)
{
    utl::string_view foo{"hello"};
    auto name = utl::get_type_name(foo);
    auto bar = static_cast<utl::string<20>>(name);
    CHECK_EQUAL("utl::string_view"_sv, name);
    CHECK_EQUAL("utl::string_view"_sv, bar);
}

TEST(Names,CustomFormatEnum)
{
    auto value = demo::THERE;
    auto name = utl::format<15>("{}", value);
    CHECK_EQUAL("demo::THERE"_sv, name);
}
