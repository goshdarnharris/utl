/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#include "result.hh"
#include <CppUTest/TestHarness.h>

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
        return std::move(a);
    }
};

struct nontrivial_destructor {
    // nontrivial_destructor(nontrivial_destructor const& that) = delete;
    // nontrivial_destructor& operator=(nontrivial_destructor const& that) = delete;
    // nontrivial_destructor(nontrivial_destructor&& that) = delete;
    // nontrivial_destructor& operator=(nontrivial_destructor&& that) = delete;
    // nontrivial_destructor() = default;

    ~nontrivial_destructor() {
        // printf("nontrivial destructor called\n");
    }
};

utl::result<nontrivial_destructor> nontrivial_destructor_test() {
    return nontrivial_destructor{};
}

TEST_GROUP(Result) {

void setup(void)
{
}

void teardown(void)
{
}

};

TEST(Result,ValueReturn)
{
    auto res = utl::result<uint32_t>{10u};
    CHECK(static_cast<bool>(res));
    CHECK(res.has_value());
    CHECK(res.value() == 10u);
}

TEST(Result,ErrorReturn)
{
    auto res = utl::result<uint32_t>{utl::system_error::UNKNOWN};
    CHECK(!static_cast<bool>(res));
    CHECK(!res.has_value());
    // CHECK(res.error() == utl::system_error::UNKNOWN);
}

TEST(Result,ConstexprResult)
{
    constexpr auto res = foo(false,10u);
    CHECK(static_cast<bool>(res));
    CHECK(res.has_value());
    CHECK(res.value() == 10u);
}

TEST(Result,ReferenceResult)
{
    Foo obj{10u};
    auto res = obj.get_a(false);

    CHECK(static_cast<bool>(res));
    CHECK(res.has_value());
    CHECK(res.value() == 10u);

    //Assign through the reference
    res.value() = 20u;

    CHECK(res.value() == 20u);
    CHECK(res.value() == obj.a);
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
