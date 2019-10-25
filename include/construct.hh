#ifndef UTL_CONSTRUCT_HH_
#define UTL_CONSTRUCT_HH_

#include "result.hh"
#include "system-error.hh"
#include <experimental/type_traits>
#include <stdio.h>

namespace utl {

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

    template <typename U = T>
    constexpr try_t(U&& unboxable) : try_t{select_constructor{}, std::forward<U>(unboxable)} {}

    constexpr bool unboxable_has_value() const { return m_unboxable.has_value(); }
    constexpr arg_t get_unboxable_value() const 
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
using result_t = result<
    accessor<T>,
    error_code,
    accessor_observer_policy,
    default_observer_policy
>;

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

template <typename... Ts>
using select_constructor_t = std::conditional_t<try_unboxing<Ts...>, error_tag, value_tag>;

} //namespace detail

//TODO: would it be possible to make this type a set of policies on result?
template <class T>
class construct : public detail::result_t<T> {
    using detail::result_t<T>::m_storage; 
    using accessor_t = typename detail::result_t<T>::value_t;

    template <typename... Args>
    constexpr construct(value_tag, Args&&... args)
        : detail::result_t<T>{in_place_t{}, value_tag{}, std::forward<Args>(args)...}
    {}

    template <typename... Args>
    constexpr construct(error_tag, Args&&... args)
        : detail::result_t<T>{in_place_t{}, error_tag{}, utl::system_error::UNKNOWN}
    { 
        bool can_construct = (detail::is_value_or_unboxable<Args>(std::forward<Args>(args)) and ...);
        if(!can_construct) {
            return;
        }

        m_storage.emplace(value_tag{}, detail::get_value<Args>(std::forward<Args>(args))...);    
    }

public:
    using detail::result_t<T>::value;
    using value_t = typename accessor_t::underlying_t;
    using error_t = typename detail::result_t<T>::error_t;

    constexpr construct()
        : detail::result_t<T>{in_place_t{}, value_tag{}}
    {}

    template <typename... Args>
    constexpr construct(Args&&... args)
        : construct{detail::select_constructor_t<Args...>{}, std::forward<Args>(args)...}
    {
        if(detail::result_t<T>::is_error()) return;

        if constexpr(accessor_t::do_validation) {
            auto res = m_storage.m_value.validate();
            if(!res) {
                m_storage.emplace(error_tag{}, res.error());
            }
        }
    }
};

template <typename T>
struct is_result<construct<T>> : std::true_type {};

} //namespace utl

#endif //UTL_CONSTRUCT_HH_
