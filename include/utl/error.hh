// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef UTL_ERROR_HH_
#define UTL_ERROR_HH_

#include "utl/utl.hh"
#include <utility>
#include <utl/string-view.hh>
#include <stdint.h>

namespace utl {

enum class errc : int32_t {
    success,
    out_of_bounds,
    unknown
};

class error_code;

namespace error {

    template <typename T>
    constexpr bool is_error_code_enum() { return false; }

    template <typename T>
    concept any_error_enum = is_error_code_enum<T>();

    template <any_error_enum T>
    [[nodiscard]] constexpr string_view message(T value); //FIXME: default should get the enum value name

    template <any_error_enum T>
    [[nodiscard]] constexpr string_view name(); //FIXME: default should get the enum's type name


    
    template <>
    constexpr bool is_error_code_enum<errc>() { return true; }

    template <>
    [[nodiscard]] constexpr string_view message<errc>(errc value)
    {
        switch(value) {
            case errc::success:
                return "success"_sv;
            case errc::out_of_bounds:
                return "index out of bounds"_sv;
            case errc::unknown:
            default:
                return "unknown generic error"_sv;
        }
    }

    template <>
    [[nodiscard]] constexpr string_view name<errc>()
    {
        return "errc"_sv;
    }
} //namespace error

struct error_condition;

struct error_domain {
    constexpr error_domain() = default;    
    [[nodiscard]] virtual constexpr string_view message(int32_t value) const = 0;
    [[nodiscard]] virtual constexpr string_view name() const = 0;
    // [[nodiscard]] virtual constexpr error_condition default_error_condition(int32_t code) const
    // {
    //     return error_condition{code,*this};
    // }
    // [[nodiscard]] constexpr bool equivalent(int32_t code, const error_condition& condition) const
    // {
    //     return default_error_condition(code) == condition;
    // }
    // [[nodiscard]] constexpr bool equivalent(const error_code& code, int32_t condition) const
    // {
    //     return *this == code.domain() && code.value() == condition;
    // }
    // [[nodiscard]] constexpr bool operator==(error_domain const& rhs) const
    // {
    //     return &rhs == this;
    // }
    // [[nodiscard]] constexpr std::strong_ordering operator<=>(error_domain const& rhs) const
    // {
    //     return this <=> &rhs;
    // }
    virtual constexpr ~error_domain() = default;
};

template <error::any_error_enum T>
struct error_domain_t : public error_domain {    
    [[nodiscard]] constexpr string_view message(int32_t value) const final { return error::message<T>(static_cast<T>(value)); }
    [[nodiscard]] constexpr string_view name() const final { return error::name<T>(); }
};

template <error::any_error_enum T>
/*constexpr FIXME: needs clang >= 15*/ error_domain const& get_error_domain()
{
    static /*constexpr FIXME: needs clang >= 15*/ error_domain_t<T> domain{};
    return domain;
}

class [[nodiscard]] error_code {
    int32_t value_;
    error_domain const * domain_;
public:
    // constructors
    constexpr error_code() : error_code{0,get_error_domain<errc>()} {}
    constexpr error_code(int val, error_domain const& dom) : value_{val}, domain_{&dom} {}
    constexpr error_code(error::any_error_enum auto e) : value_{static_cast<int32_t>(e)}, 
        domain_{&get_error_domain<std::decay_t<decltype(e)>>()} {}

    // modifiers
    void assign(int value, error_domain const& dom)
    {
        *this = error_code{value,dom};
    }
    
    error_code& operator=(error::any_error_enum auto value)
    {
        *this = error_code(value);
    }

    void clear()
    {
        *this = error_code{0, get_error_domain<errc>()};
    }

    // observers
    [[nodiscard]] constexpr auto value() const
    {
        return value_;
    }
    [[nodiscard]] constexpr error_domain const& domain() const
    {
        return *domain_;
    }
    [[nodiscard]] constexpr string_view message() const
    {
        return domain().message(value());
    }
    [[nodiscard]] explicit constexpr operator bool() const
    {
        return not (value() == 0);
    }
};

} //namespace utl

#endif //UTL_ERROR_HH_
