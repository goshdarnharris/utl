/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include "utl/result.hh"
#include "utl/system-error.hh"
#include <CppUTest/TestHarness.h>
#include "packages/libawful/include/awful.hpp"

constexpr utl::result<uint32_t> foo(bool fail, uint32_t val) {
    if(fail) return utl::system_error::UNKNOWN;
    return val;
}

struct Foo {
    uint32_t a;

    Foo(uint32_t val) : a{val} {}

    utl::result<uint32_t&> get_a(bool fail) {
        if(fail) return utl::system_error::UNKNOWN;
        return a;
    }

    utl::result<uint32_t> move_a(bool fail) {
        if(fail) return utl::system_error::UNKNOWN;
        return std::move(a); //NOLINT(performance-move-const-arg)
    }
};

struct nontrivial_destructor {
    nontrivial_destructor(nontrivial_destructor const& that) = default;
    nontrivial_destructor& operator=(nontrivial_destructor const& that) = default;
    nontrivial_destructor(nontrivial_destructor&& that) = default;
    nontrivial_destructor& operator=(nontrivial_destructor&& that) = default;
    nontrivial_destructor() = default;

    ~nontrivial_destructor() //NOLINT(modernize-use-equals-default)
    {
        
    }
};

struct nontrivial_copy_assign {
    uint8_t data;

    nontrivial_copy_assign(nontrivial_copy_assign const& that) = default;
    nontrivial_copy_assign& operator=(nontrivial_copy_assign const& that) //NOLINT(modernize-use-equals-default)
    {
        data = that.data;
        return *this;
    }
    nontrivial_copy_assign(nontrivial_copy_assign&& that) = default;
    nontrivial_copy_assign& operator=(nontrivial_copy_assign&& that) = default;
    nontrivial_copy_assign() = default;
    ~nontrivial_copy_assign() = default;
};

struct nontrivial_copy_construct {
    uint8_t data;

    //NOLINTNEXTLINE(modernize-use-equals-default)
    nontrivial_copy_construct(nontrivial_copy_construct const& that) : data{that.data} {}
    nontrivial_copy_construct& operator=(nontrivial_copy_construct const& that) = default;
    nontrivial_copy_construct(nontrivial_copy_construct&& that) = default;
    nontrivial_copy_construct& operator=(nontrivial_copy_construct&& that) = default;
    nontrivial_copy_construct() = default;
    ~nontrivial_copy_construct() = default;
};

struct nontrivial_move_assign {
    uint8_t data;
    
    nontrivial_move_assign(nontrivial_move_assign const& that) = default;
    nontrivial_move_assign& operator=(nontrivial_move_assign const& that) = default;
    nontrivial_move_assign(nontrivial_move_assign&& that) = default;
    nontrivial_move_assign& operator=(nontrivial_move_assign&& that) 
    {
        data = std::move(that.data); //NOLINT(performance-move-const-arg)
        return *this;
    }
    nontrivial_move_assign() = default;
    ~nontrivial_move_assign() = default;
};

struct nontrivial_move_construct {
    uint8_t data;

    nontrivial_move_construct(nontrivial_move_construct const& that) = default;
    nontrivial_move_construct& operator=(nontrivial_move_construct const& that) = default;
    //NOLINTNEXTLINE(performance-move-const-arg)
    nontrivial_move_construct(nontrivial_move_construct&& that) : data{std::move(that.data)} {}
    nontrivial_move_construct& operator=(nontrivial_move_construct&& that) = default;
    nontrivial_move_construct() = default;
    ~nontrivial_move_construct() = default;
};

TEST_GROUP(Result) {};

TEST(Result,ValueReturn)
{
    utl::result<uint32_t> res{10u}; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    CHECK(static_cast<bool>(res));
    CHECK(res.has_value());
    CHECK(res.value() == 10u);
}

TEST(Result,ErrorReturn)
{
    utl::result<uint32_t> res{utl::system_error::UNKNOWN};
    CHECK(!static_cast<bool>(res));
    CHECK(!res.has_value());
    // CHECK(res.error() == utl::system_error::UNKNOWN);
}

TEST(Result,ConstexprResult)
{
    constexpr utl::result<uint32_t> res = foo(false,10u);
    constexpr bool has_value = static_cast<bool>(res);
    constexpr uint32_t value = res.value();
    CHECK(static_cast<bool>(res));
    CHECK(has_value);
    CHECK(res.has_value());
    CHECK(res.value() == 10u);
    CHECK(value == 10u);
}

TEST(Result,ReferenceResult)
{
    Foo obj{10u}; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    utl::result<uint32_t&> res = obj.get_a(false);

    CHECK(static_cast<bool>(res));
    CHECK(res.has_value());
    CHECK(res.value() == 10u);

    //Assign through the reference
    res.value() = 20u; //NOLINT(cppcoreguidelines-avoid-magic-numbers)

    CHECK(res.value() == 20u);
    CHECK(res.value() == obj.a);
}

TEST(Result,NoncopyableResult)
{
    utl::result<awful::noncopyable> res{awful::noncopyable{}};
    CHECK(res.has_value());

    awful::noncopyable& value = res.value();
    (void)(value);
}

TEST(Result,NontrivialCopyConstruct)
{
    utl::result<nontrivial_copy_construct> res{nontrivial_copy_construct{}};
    utl::result<nontrivial_copy_construct> res2{res};
    CHECK(res.has_value());
    CHECK(res2.has_value());
}

TEST(Result,NontrivialCopyAssign)
{
    utl::result<nontrivial_copy_assign> res{nontrivial_copy_assign{}};
    utl::result<nontrivial_copy_assign> res2{nontrivial_copy_assign{}};
    res2 = res;
    CHECK(res.has_value());
    CHECK(res.has_value());
}

TEST(Result,NontrivialMoveConstruct)
{
    utl::result<nontrivial_move_construct> res{nontrivial_move_construct{}};
    utl::result<nontrivial_move_construct> res2{std::move(res)};
    CHECK(res2.has_value());
}

TEST(Result,NontrivialMoveAssign)
{
    utl::result<nontrivial_move_assign> res{nontrivial_move_assign{}};
    utl::result<nontrivial_move_assign> res2{nontrivial_move_assign{}};
    res2 = std::move(res);
    CHECK(res2.has_value());
    //FIXME: in these, need to do some static_asserts on the availability of
    //the related functions?
    //FIXME: maybe do something in the test value types to track whether
    //the special constructor was actually called.
    //FIXME: test with nontrivial error types?
    //FIXME: test deleted special functions.
}

TEST(Result,NonVoidNotDefaultConstructible)
{
    CHECK(!std::is_default_constructible_v<utl::result<int>>)
}

//FIXME: add tests for:
// - default constructible
// - various kinds of constructibility/assignability with in-place

TEST(Result,VoidValueWithValue)
{
    utl::result<void> res{};
    CHECK(res.has_value());
}

TEST(Result,VoidValueWithTag)
{
    utl::result<void> res{utl::value_tag};
    CHECK(res.has_value());
}

TEST(Result,VoidValueWithError)
{
    utl::result<void> res{utl::system_error::UNKNOWN};
    CHECK(!res.has_value());
}

TEST(Result,InPlaceValue)
{
    utl::result<Foo> res{std::in_place, utl::value_tag, 10u}; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
    CHECK(res.has_value());
    CHECK(res.value().a == 10);
}

TEST(Result,InPlaceValueNoncopyable)
{
    utl::result<awful::noncopyable> res{std::in_place, utl::value_tag, awful::noncopyable{}};
    CHECK(res.has_value());
}

TEST(Result,InPlaceError)
{
    utl::result<Foo> res{std::in_place, utl::error_tag, utl::system_error::UNKNOWN};
    CHECK(!res.has_value());
}

// { //rvalue reference
//     Foo obj{};
//     if(auto res = obj.move_a(false)) {
//         printf("rvalue reference succeeded %d\n", res.value());
//     } else {
//         printf("rvalue reference failed\n");
//     }
// }

// {
//     //nontrivial value type
//     if(auto res = nontrivial_destructor_test()) {
//         printf("nontrivial return value is a thing\n");
//     }        
// }
