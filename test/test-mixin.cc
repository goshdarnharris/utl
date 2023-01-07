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
// #include <utl/mixin.hh>
#include <utl/logger.hh>
#include <utl/string-view.hh>
 
using namespace utl::literals;

TEST_GROUP(Mixin) {};

TEST(Mixin,Crash)
{
    CHECK("Including utl/mixin.hh causes clang 14 to crash during compilation."_sv == " "_sv);
}

// //some concept to use as a constraint
// template <typename T>
// concept thingable = requires(T& t) {
//     { t.do_the_thing() } -> std::same_as<void>;
// };

// //the constraint type, to "capture" the concept so we can
// //use it as a template parameter
// template <thingable T> struct thingable_c{};


// //some concept to use as a constraint
// template <typename T>
// concept has_some_value = requires(T& t) {
//     { t.some_value };
// };

// //the constraint type, to "capture" the concept so we can
// //use it as a template parameter
// template <has_some_value T> struct has_some_value_c{};

// //can't do the thing (doesn't satisfy the thingable concept)
// template <typename T>
// struct spi : T {
//     using some_trait_t = int;
//     uint8_t some_value = 0;
// };


// //can do the thing (does satisfy the thingable concept)
// template <typename T>
// struct dma : T {
//     //as_t allows access to other mixin types in this composition
//     // typename utl::mixin::as_t<has_some_value_c,dma>::some_trait_t some_other_value = 10;

//     void do_the_thing() { 

//         //also visitation
//         if constexpr (utl::mixin::has_one<has_some_value_c,dma>()) {            
//             auto& spi = utl::mixin::as<has_some_value_c>(this);
//             spi.some_value++;

//             utl::mixin::as<has_some_value_c>(this, [&](auto& s) {
//                 s.some_value++;
//             });
//         } else {
//             utl::mixin::for_each<has_some_value_c>(this, [](auto& s) {
//                 s.some_value++;
//                 s.some_value++;
//                 s.some_value++;
//                 s.some_value++;
//                 s.some_value++;
//             });
//         }
//     }
// };

// struct some_data_type {
//     uint8_t some_value = 0;
// };


// //utl::mixin::composed<utl::mixin::interface<spi>,dma,spi> also breaks in interesting ways
// //break it more!
// template <typename T>
// struct do_things : T  {
// // struct spi_dma : public utl::mixin::composed<utl::mixin::interface<dma>,dma,dma> {
//     void foo()
//     {
//         //and can access the appropriate mixin type by applying
//         //a constraint on it, rather than doing so explicitly
//         // utl::mixin::as<thingable_c>(this).do_the_thing();

//         utl::mixin::for_each<thingable_c>(this, [](auto& thing) {
//             thing.do_the_thing();
//         });
//     }
// };

// using test_mixin_t = utl::mixin::composed<utl::mixin::interface<do_things>,dma,dma,spi,spi>;

// TEST(Mixin,Basic)
// {
//     test_mixin_t instance{};
//     instance.foo();
// }

// TEST(Mixin,InPlaceConstruction)
// {
//     test_mixin_t instance{
//         utl::mixin::forward<do_things>(),
//         utl::mixin::forward<dma>(),
//         utl::mixin::forward<dma>(),
//         utl::mixin::forward<spi>(uint8_t{13}),
//         utl::mixin::forward<spi>()
//     };
//     // spi_dma instance {{
//     //     {13},{},{},{}
//     // }};
//     instance.foo();
// }

