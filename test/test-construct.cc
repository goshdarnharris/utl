// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

// /* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

// #include "utl/construct.hh"
// #include <CppUTest/TestHarness.h>
// #include "packages/libawful/include/awful.hpp"
// #include "utl/test-types.hh"
// #include "utl/utl.hh"
// #include "utl/system-error.hh"

// TEST_GROUP(Construct) {

// void setup(void)
// {
// }

// void teardown(void)
// {
// }

// };

// struct normal_constructible {
//     int m_data;
//     bool m_valid;
    
// protected:
//     normal_constructible(int value, bool valid) : m_data{value}, m_valid{valid} {}
//     utl::result<void> validate() {
//         if(m_valid) {
//             return utl::success();
//         }
//         return utl::system_error::UNKNOWN;
//     }
// };

// struct public_validation {
//     int m_data;
//     bool m_valid;
    
// protected:
//     public_validation(int value, bool valid) : m_data{value}, m_valid{valid} {}

// public:
//     utl::result<void> validate() {
//         if(m_valid) {
//             return utl::success();
//         }
//         return utl::system_error::UNKNOWN;
//     }
// };

// struct tracked_constructible :public utl::tracked {
//     using utl::tracked::tracked;
//     bool m_valid;

//     tracked_constructible(bool valid) : utl::tracked{valid}, m_valid{valid} {}
//     tracked_constructible() : m_valid{true} {}

// protected:    
//     utl::result<void> validate() {
//         if(m_valid) return utl::success();
//         return utl::system_error::UNKNOWN;
//     }
// };

// TEST(Construct,Construction)
// {
//     auto obj = utl::construct<normal_constructible>{5, true};
//     CHECK(static_cast<bool>(obj));
//     CHECK(obj.value().m_data == 5);

//     auto obj2 = utl::construct<normal_constructible>{5, false};
//     CHECK(!static_cast<bool>(obj2));
// }

// TEST(Construct, InPlaceDefaultConstruction)
// {
//     utl::construct<tracked_constructible> obj{};

//     CHECK(static_cast<bool>(obj));
//     CHECK(obj.value().default_constructed());
//     CHECK(!obj.value().regular_constructed());
//     CHECK(!obj.value().copy_constructed());
//     CHECK(!obj.value().move_constructed());
//     CHECK(obj.value().copy_assigned() == 0);
//     CHECK(obj.value().move_assigned() == 0);
// }

// TEST(Construct, InPlaceRegularConstruction)
// {
//     utl::construct<tracked_constructible> obj{true};

//     CHECK(static_cast<bool>(obj));
//     CHECK(!obj.value().default_constructed());
//     CHECK(obj.value().regular_constructed());
//     CHECK(!obj.value().copy_constructed());
//     CHECK(!obj.value().move_constructed());
//     CHECK(obj.value().copy_assigned() == 0);
//     CHECK(obj.value().move_assigned() == 0);
// }

// TEST(Construct, InPlaceCopyConstruction)
// {
//     tracked_constructible a{};
//     utl::construct<tracked_constructible> obj{a};

//     CHECK(static_cast<bool>(obj));
//     CHECK(!obj.value().default_constructed());
//     CHECK(!obj.value().regular_constructed());
//     CHECK(!obj.value().move_constructed());
//     CHECK(obj.value().copy_constructed());
//     CHECK(obj.value().copy_assigned() == 0);
//     CHECK(obj.value().move_assigned() == 0);
// }

// TEST(Construct, InPlaceMoveConstruction)
// {
//     tracked_constructible a{};
//     utl::construct<tracked_constructible> obj{std::move(a)};

//     CHECK(static_cast<bool>(obj));
//     CHECK(!obj.value().default_constructed());
//     CHECK(!obj.value().regular_constructed());
//     CHECK(!obj.value().copy_constructed());
//     CHECK(obj.value().move_constructed());
//     CHECK(obj.value().copy_assigned() == 0);
//     CHECK(obj.value().move_assigned() == 0);
// }

// TEST(Construct, PublicValidation)
// {
//     auto obj = utl::construct<public_validation>{5, true};
//     CHECK(static_cast<bool>(obj));
//     CHECK(obj.value().m_data == 5);

//     auto obj2 = utl::construct<public_validation>{5, false};
//     CHECK(!static_cast<bool>(obj2));
// }

// TEST(Construct, NoValidation)
// {
//     utl::construct<utl::tracked> obj{};

//     CHECK(static_cast<bool>(obj));
//     CHECK(obj.value().default_constructed());
//     CHECK(!obj.value().regular_constructed());
//     CHECK(!obj.value().copy_constructed());
//     CHECK(!obj.value().move_constructed());
//     CHECK(obj.value().copy_assigned() == 0);
//     CHECK(obj.value().move_assigned() == 0);
// }

// template <typename T>
// struct dependent_constructible : tracked_constructible {
//     using type_t = std::remove_reference_t<std::remove_pointer_t<T>>;
//     type_t dependency;
// protected:
//     utl::result<void> validate() { return utl::success(); }
//     constexpr dependent_constructible(type_t& dep) : tracked_constructible{true}, dependency{dep} {}
//     constexpr dependent_constructible(type_t&& dep) : tracked_constructible{true}, dependency{std::move(dep)} {}
//     constexpr dependent_constructible(type_t* dep) : tracked_constructible{true}, dependency{*dep} {}
// };

// TEST(Construct, TryValue)
// {
//     using dependent_t = dependent_constructible<tracked_constructible>;
//     utl::construct<tracked_constructible> dependency{true};
//     utl::construct<dependent_t> dependent{utl::try_t{dependency}};

//     CHECK(static_cast<bool>(dependency));
//     CHECK(static_cast<bool>(dependent));
//     CHECK(dependency.value().copy_assigned() == 0);
//     CHECK(dependency.value().move_assigned() == 0);
//     CHECK(dependency.value().moved_from() == 0);
//     CHECK(dependent.value().regular_constructed());
//     CHECK(dependent.value().copy_assigned() == 0);
//     CHECK(dependent.value().move_assigned() == 0);
// }

// TEST(Construct, TryLValueRef)
// {
//     using dependent_t = dependent_constructible<tracked_constructible&>;
//     utl::construct<tracked_constructible> dependency{true};
//     utl::construct<dependent_t> dependent{utl::try_t{dependency}};

//     CHECK(static_cast<bool>(dependency));
//     CHECK(static_cast<bool>(dependent));
//     CHECK(dependency.value().copy_assigned() == 0);
//     CHECK(dependency.value().move_assigned() == 0);
//     CHECK(dependency.value().moved_from() == 0);
//     CHECK(dependent.value().regular_constructed());
//     CHECK(dependent.value().copy_assigned() == 0);
//     CHECK(dependent.value().move_assigned() == 0);
// }

// TEST(Construct, TryRValueRef)
// {
//     using dependent_t = dependent_constructible<tracked_constructible&&>;
//     utl::construct<tracked_constructible> dependency{true};
//     utl::construct<dependent_t> dependent{utl::try_t{std::move(dependency)}};

//     CHECK(static_cast<bool>(dependent));
//     CHECK(dependency.value().copy_assigned() == 0);
//     CHECK(dependency.value().move_assigned() == 0);
//     CHECK(dependency.value().moved_from() == 1);
//     CHECK(dependent.value().regular_constructed());
//     CHECK(dependent.value().copy_assigned() == 0);
//     CHECK(dependent.value().move_assigned() == 0);    
// }

// TEST(Construct, TryPointer)
// {
//     using dependent_t = dependent_constructible<tracked_constructible*>;
//     utl::construct<tracked_constructible> dependency{true};
//     utl::construct<dependent_t> dependent{utl::try_t{&dependency}};

//     CHECK(static_cast<bool>(dependent));
//     CHECK(dependency.value().copy_assigned() == 0);
//     CHECK(dependency.value().move_assigned() == 0);
//     CHECK(dependency.value().moved_from() == 0);
//     CHECK(dependent.value().regular_constructed());
//     CHECK(dependent.value().copy_assigned() == 0);
//     CHECK(dependent.value().move_assigned() == 0);    
// }
// //TODO: test all the things for try_t.
// // - pointers
// // - non-pointers
// // - no copy, no move
// // - actual construction occurring
// // - errors being returned appropriately
