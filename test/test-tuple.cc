// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/tuple.hh>
#include <utl/logger.hh>
#include <type_traits>

TEST_GROUP(tuple) {};

namespace {

inline constexpr auto tup = utl::tuple{1.0f,10u,false};

inline constexpr float some_float = get<0>(tup);
inline constexpr unsigned int some_int = get<1>(tup);
inline constexpr bool some_bool = get<2>(tup);

inline constexpr auto tied = utl::tie(some_float,some_int,some_bool);
static_assert(std::is_same_v<decltype(tied), const utl::tuple<const float&, const unsigned int&, const bool&>>);


static auto forwarded = utl::forward_as_tuple(some_float,some_int,true);
static_assert(std::is_same_v<decltype(forwarded), utl::tuple<const float&, const unsigned int&, bool&&>>);

} //anonymous namespace

static constexpr float float_value = 1.1f;

TEST(tuple,create)
{
    using tuple_t = utl::tuple<int,float,bool>;
    tuple_t foo{0, float_value, false};
    utl::maybe_unused(foo);
}

TEST(tuple,get)
{
    using tuple_t = utl::tuple<int,float,bool>;
    tuple_t foo{0, float_value, false};
    
    CHECK(utl::get<0>(foo) == 0);
    CHECK(utl::get<1>(foo) == float_value);
    CHECK(utl::get<2>(foo) == false);
}

TEST(tuple,rvalue_tuple)
{
    using tuple_t = utl::tuple<int,float,bool>;    
    CHECK(utl::get<0>(tuple_t{0, float_value, false}) == 0);
    CHECK(utl::get<1>(tuple_t{0, float_value, false}) == float_value);
    CHECK(utl::get<2>(tuple_t{0, float_value, false}) == false);
    CHECK((utl::is_same_v<int&&  ,decltype(utl::get<0>(tuple_t{0, float_value, false}))>));
    CHECK((utl::is_same_v<float&&,decltype(utl::get<1>(tuple_t{0, float_value, false}))>));
    CHECK((utl::is_same_v<bool&& ,decltype(utl::get<2>(tuple_t{0, float_value, false}))>));
}

TEST(tuple,rvalue_contents_values)
{
    using tuple_t = utl::tuple<int&,float&&,bool>;    
    int a = 5; //NOLINT(cppcoreguidelines-avoid-magic-numbers)

    tuple_t foo{a,100.0f,false};

    CHECK(utl::get<0>(foo) == a);
    CHECK(utl::get<1>(foo) == 100.0f);
    CHECK(utl::get<2>(foo) == false);
}

TEST(tuple,rvalue_contents_types)
{
    using tuple_t = utl::tuple<int&,float&&,bool>;    
    int a = 5; //NOLINT(cppcoreguidelines-avoid-magic-numbers)

    tuple_t foo{a,100.0f,false};

    CHECK((utl::is_same_v<int&   ,decltype(utl::get<0>(foo))>));
    CHECK((utl::is_same_v<float& ,decltype(utl::get<1>(foo))>));
    CHECK((utl::is_same_v<bool&  ,decltype(utl::get<2>(foo))>));
}

TEST(tuple,get_types)
{
    using tuple_t = utl::tuple<int,float,bool>;
    tuple_t foo{0, float_value, false};
    
    CHECK((utl::is_same_v<int&  ,decltype(utl::get<0>(foo))>));
    CHECK((utl::is_same_v<float&,decltype(utl::get<1>(foo))>));
    CHECK((utl::is_same_v<bool& ,decltype(utl::get<2>(foo))>));
}

TEST(tuple,references)
{
    using tuple_t = utl::tuple<int&,float&,bool&>;

    int a = 5; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    float b = 3.4f; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    bool c = true;

    tuple_t foo{a,b,c};

    CHECK((utl::is_same_v<int&  ,decltype(utl::get<0>(foo))>));
    CHECK((utl::is_same_v<float&,decltype(utl::get<1>(foo))>));
    CHECK((utl::is_same_v<bool& ,decltype(utl::get<2>(foo))>));

    CHECK(utl::get<0>(foo) == a);
    CHECK(utl::get<1>(foo) == b);
    CHECK(utl::get<2>(foo) == c);
}

namespace {

struct wrapper {
    int a;
    float b;
    bool c;
};

static wrapper wrap_args(int a, float b, bool c)
{
    return {a,b,c};
}

} //anonymous namespace

TEST(tuple,apply)
{
    using tuple_t = utl::tuple<int&,float&,bool&>;

    int a = 5; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    float b = 3.4f; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    bool c = true;

    tuple_t foo{a,b,c};

    auto result = utl::apply(wrap_args, foo);
    CHECK_EQUAL(a, result.a);
    CHECK_EQUAL(b, result.b);
    CHECK_EQUAL(c, result.c);
}
