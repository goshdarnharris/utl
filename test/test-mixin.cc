/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"
#include <utl/test-types.hh>
#include <utl/utl.hh>
#include <utl/mixin.hh>
#include <utl/logger.hh>

TEST_GROUP(Mixin) {};


//some concept to use as a constraint
template <typename T>
concept thingable = requires(T& t) {
    { t.do_the_thing() } -> std::same_as<void>;
};

//the constraint type, to "capture" the concept so we can
//use it as a template parameter
template <thingable T> struct thingable_c{};


//some concept to use as a constraint
template <typename T>
concept has_some_value = requires(T& t) {
    { t.some_value };
};

//the constraint type, to "capture" the concept so we can
//use it as a template parameter
template <has_some_value T> struct has_some_value_c{};

//can't do the thing (doesn't satisfy the thingable concept)
template <typename T>
struct spi : T {
    using some_trait_t = int;
    uint8_t some_value = 0;
};


//can do the thing (does satisfy the thingable concept)
template <typename T>
struct dma : T {
    //as_t allows access to other mixin types in this composition
    typename utl::mixin::as_t<T,has_some_value_c>::some_trait_t some_other_value = 10;

    void do_the_thing() { 
        auto& spi = utl::mixin::as<has_some_value_c>(this);
        spi.some_value++;

        //also visitation
        if(utl::mixin::has<has_some_value_c>(this)) {
            utl::mixin::as<has_some_value_c>(this, [&](auto& s) {
                s.some_value++;
            });
        }
    }
};

struct some_data_type {
    uint8_t some_value = 0;
};


//this type has the spi & dma mixins
struct spi_dma : public utl::mixin::composed<utl::mixin::interface<spi>,dma> {
    void foo()
    {
        //and can access the appropriate mixin type by applying
        //a constraint on it, rather than doing so explicitly
        utl::mixin::as<thingable_c>(this).do_the_thing();

        utl::mixin::for_each<thingable_c>(this, [](auto& thing) {
            thing.do_the_thing();
        });
    }
};



TEST(Mixin,Basic)
{
    spi_dma instance{};
    instance.foo();
}

