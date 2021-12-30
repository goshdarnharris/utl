// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef UTL_CONSTRUCT_HH_
#define UTL_CONSTRUCT_HH_

#include "utl/result.hh"
#include "utl/system-error.hh"
#include <experimental/type_traits>
#include <stdio.h>

namespace utl {

//what is this type, really?
//it represents an operation that construct is to perform.
//in part, it represents a future reference, copy, or move.
//  so that should probably get split off.
//in part, it represents an unboxing operation.
// - check if there's a value
// - unbox that value
// - return that value with the correct type
template <typename T>
class try_t {
    static constexpr bool is_pointer = std::is_pointer_v<T>;
    static constexpr bool is_rvalue_ref = std::is_rvalue_reference_v<T>;
    static constexpr bool is_lvalue_ref = std::is_lvalue_reference_v<T>;

    struct pointer_tag {};
    struct rvalue_ref_tag {};
    struct lvalue_ref_tag {};

    using select_constructor = std::conditional_t<
        is_pointer, 
        pointer_tag, std::conditional_t<
            is_rvalue_ref,
            rvalue_ref_tag,
            lvalue_ref_tag
        >
    >;

    using unboxable_t = std::remove_reference_t<std::remove_pointer_t<T>>;
    using boxed_t = typename unboxable_t::value_t;
    static_assert(is_result_v<unboxable_t>, "try_t can only be used with types satisfying is_result<T>");  
    unboxable_t& m_unboxable;
public:
      
    using arg_t = std::conditional_t<is_pointer, boxed_t*, std::conditional_t<is_rvalue_ref, boxed_t&&, boxed_t&>>;

    constexpr try_t(lvalue_ref_tag, T unboxable) : m_unboxable{unboxable} {}
    constexpr try_t(pointer_tag, T unboxable) : m_unboxable{*unboxable} {}
    constexpr try_t(rvalue_ref_tag, T unboxable) : m_unboxable{unboxable} {}

    template <same_as<T> U>
    constexpr try_t(U&& unboxable) : try_t{select_constructor{}, std::forward<U>(unboxable)} {} //NOLINT(bugprone-forwarding-reference-overload)

    [[nodiscard]] constexpr bool unboxable_has_value() const { return m_unboxable.has_value(); }
    [[nodiscard]] constexpr arg_t get_unboxable_value() const 
    {
        if constexpr(is_pointer) {
            return &m_unboxable.value();
        } else if constexpr(is_rvalue_ref) {
            return std::move(m_unboxable.value());
        } else {
            return m_unboxable.value(); 
        }
    }
};

template <typename T>
try_t(T*) -> try_t<T*>;

template <typename T>
try_t(T&&) -> try_t<T&&>;

template <typename T>
try_t(T&) -> try_t<T&>;

namespace detail {

struct dummy { using validate = int; };
template <typename T>
struct test_validate_t : T, dummy
{
    template <typename U=test_validate_t, typename = typename U::validate>
    static std::false_type test(int);
    static std::true_type test(float);
};

template <typename T>
static constexpr bool has_validate = decltype(detail::test_validate_t<T>::test(0))::value;

// template <class T>
// using validate_t = decltype(T::validate);  

// template <class T>
// static constexpr bool asdf = std::experimental::is_detected<validate_t, T>::value;

template <class T>
struct accessor : public T {
    using underlying_t = T;
    static constexpr bool do_validation = has_validate<T>;

    constexpr accessor() : T{} {}

    template <typename... Args>
    constexpr accessor(Args&&... args) : T{std::forward<Args>(args)...} {}
    
    constexpr auto validate() { return T::validate(); } //this should be a policy and/or based on T::validate's actual return type
};

template <class T, class R>
struct cast_observer_policy {
    using return_t = R&;

    static constexpr R& get(T& storage) {
        return static_cast<R&>(storage);
    }

    static constexpr R const& get(T const& storage) {
        return static_cast<T const&>(storage);
    }
};

template <class T>
using accessor_observer_policy = cast_observer_policy<T, typename T::underlying_t>;

template <class T>
using result_t = result<T>;

template <typename T>
static constexpr bool is_try = false;

template <typename T>
static constexpr bool is_try<try_t<T>> = true;

template <typename... Ts>
static constexpr bool try_unboxing = (is_try<Ts> or ...);


template <class T, std::enable_if_t<!try_unboxing<T>,int*> = nullptr>
constexpr bool is_value_or_unboxable(T& value) {
    utl::maybe_unused(value);
    return true;
}

template <class T, std::enable_if_t<!try_unboxing<T>,int*> = nullptr>
constexpr bool is_value_or_unboxable(T&& value) {
    utl::maybe_unused(value);
    return true;
}

template <class T, std::enable_if_t<!try_unboxing<T>,int*> = nullptr>
constexpr bool is_value_or_unboxable(T* value) {
    utl::maybe_unused(value);
    return true;
}

template <class T, std::enable_if_t<try_unboxing<T>,int*> = nullptr>
constexpr bool is_value_or_unboxable(T&& try_v) {
    return try_v.unboxable_has_value();
}


template <class T, std::enable_if_t<!try_unboxing<T>,int*> = nullptr>
constexpr auto get_value(typename std::remove_reference_t<T>& value) -> T&&
{
    return static_cast<T&&>(value);
}


template <class T, std::enable_if_t<!try_unboxing<T>,int*> = nullptr>
constexpr auto get_value(typename std::remove_reference_t<T>&& value) -> T&&
{
    return static_cast<T&&>(value);
}

template <class T, std::enable_if_t<!try_unboxing<T>,int*> = nullptr>
constexpr auto get_value(T* value) -> T*
{
    return value;
}

template <class T, std::enable_if_t<try_unboxing<T>,int*> = nullptr>
constexpr auto get_value(T&& try_v)-> typename T::arg_t
{
    return try_v.get_unboxable_value();
}

template <typename T>
using unboxed_t = std::result_of<decltype(get_value<T>)(T)>;

template <typename... Ts>
using select_constructor_t = std::conditional_t<try_unboxing<Ts...>, error_tag_t, value_tag_t>;

} //namespace detailpling

//TODO: would it be possible to make this type a set of policies on result?
template <class T>
class construct : public result<T> {

    static constexpr bool do_validation = requires(T&& v) { v.validate() -> result<void>; };

    template <typename... Args>
    constexpr construct(value_tag_t, Args&&... args)
        requires requires() { T{std::forward<Args>(args)...}; }
        : result<T>{std::in_place, value_tag, std::forward<Args>(args)...}
    {}

    template <typename... Args>
    constexpr construct(error_tag_t, Args&&... args)
        // requires requires() { T{detail::get_value<Args>(std::forward<Args>(args))...}; }
        : result<T>{std::in_place, error_tag, utl::system_error::UNKNOWN}
    { 
        //FIXME: can probably refactor this as a collection of constexpr_if.
        //FIXME: can probably discover the arguments of the constructor and compare
        //   to decide whether or not to try to unbox a result. this would eliminate
        //   the type_t tag.
        bool can_construct = (detail::is_value_or_unboxable<Args>(std::forward<Args>(args)) and ...);
        //uh... constructability policy? what would that even do?
        //would that be a step in decou try_t from construct completely?
        if(!can_construct) {
            return;
        }

        result<T>::emplace(value_tag, detail::get_value<Args>(std::forward<Args>(args))...);    
    }

public:

    constexpr construct()
        : result<T>{std::in_place, value_tag}
    {
        //FIXME: validate-or-not should probably be a policy, and I should probably have
        //  two separate default types representing validate and do-not-validate.
        //Or... does it make more sense to decide based on the argument? If this type
        //  enforces one or the other, then the construct type could be placing
        //  extra invariants on the boxed type. Which would be bad.
        if constexpr(do_validation) {
            auto res = result<T>::value().validate();
            if(!res) {
                result<T>::emplace(error_tag, res.error());
            }
        }
    }

    template <typename... Args>
    constexpr construct(Args&&... args)
        : construct{detail::select_constructor_t<Args...>{}, std::forward<Args>(args)...}
    {
        if(result<T>::is_error()) return; //should invoke an error policy

        //FIXME: validate-or-not should probably be a policy, and I should probably have
        //  two separate default types representing validate and do-not-validate.
        //Or... does it make more sense to decide based on the argument? If this type
        //  enforces one or the other, then the construct type could be placing
        //  extra invariants on the boxed type. Which would be bad.
        if constexpr(do_validation) {
            auto res = result<T>::value().validate();
            if(!res) {
                result<T>::emplace(error_tag, res.error());
            }
        }
    }
};

template <typename T>
struct is_result<construct<T>> : std::true_type {};

} //namespace utl

#endif //UTL_CONSTRUCT_HH_
