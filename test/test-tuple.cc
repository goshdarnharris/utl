/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/tuple.hh>
#include <utl/logger.hh>

TEST_GROUP(tuple) {};

TEST(tuple,create)
{
    using tuple_t = utl::tuple<int,float,bool>;
    tuple_t foo{0, 1.1f, false};
    utl::maybe_unused(foo);
}

TEST(tuple,get)
{
    using tuple_t = utl::tuple<int,float,bool>;
    tuple_t foo{0, 1.1f, false};
    
    CHECK(utl::get<0>(foo) == 0);
    CHECK(utl::get<1>(foo) == 1.1f);
    CHECK(utl::get<2>(foo) == false);
}

TEST(tuple,rvalue_tuple)
{
    using tuple_t = utl::tuple<int,float,bool>;    
    CHECK(utl::get<0>(tuple_t{0, 1.1f, false}) == 0);
    CHECK(utl::get<1>(tuple_t{0, 1.1f, false}) == 1.1f);
    CHECK(utl::get<2>(tuple_t{0, 1.1f, false}) == false);
    CHECK((utl::is_same_v<int&&  ,decltype(utl::get<0>(tuple_t{0, 1.1f, false}))>));
    CHECK((utl::is_same_v<float&&,decltype(utl::get<1>(tuple_t{0, 1.1f, false}))>));
    CHECK((utl::is_same_v<bool&& ,decltype(utl::get<2>(tuple_t{0, 1.1f, false}))>));
}

TEST(tuple,rvalue_contents_values)
{
    using tuple_t = utl::tuple<int&,float&&,bool>;    
    int a = 5;

    tuple_t foo{a,100.0f,false};

    CHECK(utl::get<0>(foo) == a);
    CHECK(utl::get<1>(foo) == 100.0f);
    CHECK(utl::get<2>(foo) == false);
}

TEST(tuple,rvalue_contents_types)
{
    using tuple_t = utl::tuple<int&,float&&,bool>;    
    int a = 5;

    tuple_t foo{a,100.0f,false};

    CHECK((utl::is_same_v<int&   ,decltype(utl::get<0>(foo))>));
    CHECK((utl::is_same_v<float&&,decltype(utl::get<1>(foo))>));
    CHECK((utl::is_same_v<bool&  ,decltype(utl::get<2>(foo))>));
}

TEST(tuple,get_types)
{
    using tuple_t = utl::tuple<int,float,bool>;
    tuple_t foo{0, 1.1f, false};
    
    CHECK((utl::is_same_v<int&  ,decltype(utl::get<0>(foo))>));
    CHECK((utl::is_same_v<float&,decltype(utl::get<1>(foo))>));
    CHECK((utl::is_same_v<bool& ,decltype(utl::get<2>(foo))>));
}

TEST(tuple,references)
{
    using tuple_t = utl::tuple<int&,float&,bool&>;

    int a = 5;
    float b = 3.4f;
    bool c = true;

    tuple_t foo{a,b,c};

    CHECK((utl::is_same_v<int&  ,decltype(utl::get<0>(foo))>));
    CHECK((utl::is_same_v<float&,decltype(utl::get<1>(foo))>));
    CHECK((utl::is_same_v<bool& ,decltype(utl::get<2>(foo))>));

    CHECK(utl::get<0>(foo) == a);
    CHECK(utl::get<1>(foo) == b);
    CHECK(utl::get<2>(foo) == c);
}

struct wrapper {
    int a;
    float b;
    bool c;
};

static wrapper wrap_args(int a, float b, bool c)
{
    return {a,b,c};
}

TEST(tuple,apply)
{
    using tuple_t = utl::tuple<int&,float&,bool&>;

    int a = 5;
    float b = 3.4f;
    bool c = true;

    tuple_t foo{a,b,c};

    auto result = utl::apply(wrap_args, foo);
    CHECK_EQUAL(a, result.a);
    CHECK_EQUAL(b, result.b);
    CHECK_EQUAL(c, result.c);
}
