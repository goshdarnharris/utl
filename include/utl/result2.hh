#pragma once

#include <concepts>
#include <utility>
#include <type_traits>
#include <utl/error.hh>
#include <utl/utl.hh>

namespace utl {

namespace detail {
template <class T> constexpr T& FUN(T& t) noexcept { return t; }
template <class T> void FUN(T&&) = delete;
}
 
template <typename T>
class reference_wrapper {
    T* m_ptr;
public:
    using type = T;
    
    //NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    constexpr reference_wrapper(std::convertible_to<T&> auto&& u)
        requires (not std::is_same_v<reference_wrapper, std::remove_cvref_t<decltype(u)>>)
        and requires() { detail::FUN<T>(u); }
      : m_ptr(std::addressof(detail::FUN<T>(std::forward<decltype(u)>(u)))) {}

    reference_wrapper(const reference_wrapper&) = default;

    // assignment
    reference_wrapper& operator=(const reference_wrapper&) = default;

    // access
    constexpr operator T& () const { return *m_ptr; }
    [[nodiscard]] constexpr T& get() const { return *m_ptr; }

    ~reference_wrapper() = default;
};
 
// deduction guides
template<class T>
reference_wrapper(T&) -> reference_wrapper<T>;



namespace trait {
template <typename T>
struct copy_ctor {
    static constexpr bool use_deleted = not (std::is_copy_constructible_v<T> or std::is_void_v<T>);
    static constexpr bool use_trivial = std::is_trivially_copy_constructible_v<T> or std::is_void_v<T>;
    static constexpr bool use_nontrivial = (not use_trivial) and (not use_deleted);
};

template <typename T>
struct move_ctor {
    static constexpr bool use_deleted = not (std::is_move_constructible_v<T> or std::is_void_v<T>);
    static constexpr bool use_trivial = std::is_trivially_move_constructible_v<T> or std::is_void_v<T>;
    static constexpr bool use_nontrivial = (not use_trivial) and (not use_deleted);
};

template <typename T>
struct copy_assign {
    static constexpr bool use_deleted = not (std::is_copy_assignable_v<T> or std::is_void_v<T>);
    static constexpr bool use_trivial = std::is_trivially_copy_assignable_v<T> or std::is_void_v<T>;
    static constexpr bool use_nontrivial = (not use_trivial) and (not use_deleted);
};

template <typename T>
struct move_assign {
    static constexpr bool use_deleted = not (std::is_move_assignable_v<T> or std::is_void_v<T>);
    static constexpr bool use_trivial = std::is_trivially_move_assignable_v<T> or std::is_void_v<T>;
    static constexpr bool use_nontrivial = (not use_trivial) and (not use_deleted);
};

template <typename T>
struct dtor {
    static constexpr bool use_deleted = not (std::is_destructible_v<T> or std::is_void_v<T>);
    static constexpr bool use_trivial = std::is_trivially_destructible_v<T> or std::is_void_v<T>;
    static constexpr bool use_nontrivial = (not use_trivial) and (not use_deleted);
};

template <typename T>
struct boxed {
    using type = T;
};

template <typename T>    
struct boxed<T&> {
    using type = reference_wrapper<T>;
};


struct void_placeholder {};

template <>
struct boxed<void> {
    using type = void_placeholder;
};

} //namespace trait

template <typename T>
using boxed_t = typename trait::boxed<T>::type;

inline constexpr struct {} value_tag{};
inline constexpr struct {} error_tag{};
using value_tag_t = decltype(value_tag);
using error_tag_t = decltype(error_tag);

// namespace workaround {

// // FIXME: once clang considers multiple destructors, these can go away.
// // https://bugs.llvm.org/show_bug.cgi?id=46269
// // Once that's fixed, remove this workaround namespace and uncomment the
// // destructors in result.

// template <typename T, typename E>
// struct dtor {
//     static constexpr bool use_deleted = 
//         trait::dtor<T>::use_deleted or trait::dtor<T>::use_deleted;
//     static constexpr bool use_trivial = 
//         trait::dtor<T>::use_trivial and trait::dtor<T>::use_trivial;
//     static constexpr bool use_nontrivial = (not use_deleted) and
//         (trait::dtor<T>::use_nontrivial or trait::dtor<T>::use_nontrivial);
// };

// template <typename R, bool Delete, bool Default, bool Implement>
// struct dtor_impl {};

// template <typename R>
// struct dtor_impl<R,true,false,false> { //NOLINT(cppcoreguidelines-special-member-functions)
//     ~dtor_impl() = delete;
// };

// template <typename R>
// struct dtor_impl<R,false,true,false> { //NOLINT(cppcoreguidelines-special-member-functions)
//     ~dtor_impl() = default;
// };

// template <typename R>
// struct dtor_impl<R,false,false,true> { //NOLINT(cppcoreguidelines-special-member-functions)
//     ~dtor_impl() {
//         static_cast<R*>(this)->destroy_boxed();
//     }
// };

// template <typename R, typename T, typename E>
// using dtor_impl_t = dtor_impl<R, dtor<T,E>::use_deleted, dtor<T,E>::use_trivial, dtor<T,E>::use_nontrivial>;

// } //namespace workaround

template <typename T, typename E = error_code>
class [[nodiscard]] result { //: public workaround::dtor_impl_t<result<T,E>,T,E> {
public:
    using this_t = result<T,E>;
    using value_t = T;
    using error_t = E;
    using value_return_t = std::add_lvalue_reference_t<value_t>;
    using value_const_return_t = std::add_lvalue_reference_t<const value_t>;
    using error_return_t = std::add_lvalue_reference_t<error_t>;
    using error_const_return_t = std::add_lvalue_reference_t<const error_t>;
private:
    // using boxed_t<value_t> = boxed_t<value_t>;
    // using boxed_t<error_t> = boxed_t<error_t>;
    struct uninitialized_t {};

    union box_t {
        boxed_t<value_t> value;
        boxed_t<error_t> error;
        uninitialized_t _;

        constexpr box_t() : _{} {}

        constexpr box_t(value_tag_t, std::convertible_to<boxed_t<value_t>> auto&& v) 
          : value{std::forward<decltype(v)>(v)}
        {}

        constexpr box_t(value_tag_t)
            requires std::is_default_constructible_v<boxed_t<value_t>>
          : value{}
        {}

        constexpr box_t(std::in_place_t, value_tag_t)
            requires std::is_default_constructible_v<boxed_t<value_t>>
          : value{}
        {}

        constexpr box_t(std::in_place_t, value_tag_t, auto&&... args)
          : value{std::forward<decltype(args)>(args)...}
        {}

        constexpr box_t(error_tag_t, std::convertible_to<boxed_t<error_t>> auto&& e) 
          : error{std::forward<decltype(e)>(e)}
        {}

        constexpr box_t(error_tag_t)
            requires std::is_default_constructible_v<boxed_t<error_t>>
          : error{}
        {}

        constexpr box_t(std::in_place_t, error_tag_t)
            requires std::is_default_constructible_v<boxed_t<error_t>>
          : error{}
        {}

        constexpr box_t(std::in_place_t, error_tag_t, auto&&... args)
          : error{std::forward<decltype(args)>(args)...}
        {}
    };

    // Union types have special behaviour with regards to
    // their special member functions, as described at
    // https://en.cppreference.com/w/cpp/language/union
    // and copied below.
    //
    // If a union contains a non-static data member with a 
    // non-trivial special member function (copy/move 
    // constructor, copy/move assignment, or destructor), 
    // that function is deleted by default in the union 
    // and needs to be defined explicitly by the programmer.
    //
    // Here's a table that describes what the result type
    // does for each possible combination of the value &
    // error types' special member functions. The traits
    // below implement this table for each of result's
    // special member functions.
    //
    //     T           E            result<T,E>
    //  trivial     trivial          use default
    //  trivial     nontrivial       define
    //  trivial     deleted          use delete
    //  nontrivial  trivial          define
    //  nontrivial  nontrivial       define
    //  nontrivial  deleted          use delete
    //  deleted     trivial          use delete
    //  deleted     nontrivial       use delete
    //  deleted     deleted          use delete

    struct copy_ctor {
        static constexpr bool use_deleted = 
            trait::copy_ctor<value_t>::use_deleted or trait::copy_ctor<error_t>::use_deleted;
        static constexpr bool use_trivial = 
            trait::copy_ctor<value_t>::use_trivial and trait::copy_ctor<error_t>::use_trivial;
        static constexpr bool use_nontrivial = (not use_deleted) and
            (trait::copy_ctor<value_t>::use_nontrivial or trait::copy_ctor<error_t>::use_nontrivial);
    };

    struct move_ctor {
        static constexpr bool use_deleted = 
            trait::move_ctor<value_t>::use_deleted or trait::move_ctor<error_t>::use_deleted;
        static constexpr bool use_trivial = 
            trait::move_ctor<value_t>::use_trivial and trait::move_ctor<error_t>::use_trivial;
        static constexpr bool use_nontrivial = (not use_deleted) and
            (trait::move_ctor<value_t>::use_nontrivial or trait::move_ctor<error_t>::use_nontrivial);
    };

    struct copy_assign {
        static constexpr bool use_deleted = 
            trait::copy_assign<value_t>::use_deleted or trait::copy_assign<error_t>::use_deleted;
        static constexpr bool use_trivial = 
            trait::copy_assign<value_t>::use_trivial and trait::copy_assign<error_t>::use_trivial;
        static constexpr bool use_nontrivial = (not use_deleted) and
            (trait::copy_assign<value_t>::use_nontrivial or trait::copy_assign<error_t>::use_nontrivial);
    };

    struct move_assign {
        static constexpr bool use_deleted = 
            trait::move_assign<value_t>::use_deleted or trait::move_assign<error_t>::use_deleted;
        static constexpr bool use_trivial = 
            trait::move_assign<value_t>::use_trivial and trait::move_assign<error_t>::use_trivial;
        static constexpr bool use_nontrivial = (not use_deleted) and
            (trait::move_assign<value_t>::use_nontrivial or trait::move_assign<error_t>::use_nontrivial);
    };

    struct dtor {
        static constexpr bool use_deleted = 
            trait::dtor<value_t>::use_deleted or trait::dtor<error_t>::use_deleted;
        static constexpr bool use_trivial = 
            trait::dtor<value_t>::use_trivial and trait::dtor<error_t>::use_trivial;
        static constexpr bool use_nontrivial = (not use_deleted) and
            (trait::dtor<value_t>::use_nontrivial or trait::dtor<error_t>::use_nontrivial);
    };


    box_t m_box{};
    bool m_has_value{false};

    constexpr void destroy_boxed() requires dtor::use_deleted {}
    constexpr void destroy_boxed() requires dtor::use_trivial {}
    constexpr void destroy_boxed() requires dtor::use_nontrivial
    {   
        if(m_has_value) {
            if constexpr(trait::dtor<value_t>::use_nontrivial) {
                m_box.value.~value_t();
            }
        } else {
            if constexpr(trait::dtor<error_t>::use_nontrivial) {
                m_box.error.~error_t();
            }
        }
    }

protected:
    void emplace(error_tag_t, auto&&... args)
    {
        destroy_boxed();
        m_has_value = false;
        new (&m_box.error) error_t{std::forward<decltype(args)>(args)...};
    }

    void emplace(value_tag_t, auto&&... args)
    {
        destroy_boxed();
        m_has_value = true;
        new (&m_box.value) value_t{std::forward<decltype(args)>(args)...};
    }

public:

    //FIXME: consider visibility of special functions

    constexpr result() requires (not std::is_void_v<value_t>) = delete;
    constexpr result() requires std::is_void_v<value_t>
      : m_box{value_tag}, m_has_value{true} //NOLINT(modernize-use-default-member-init)
    {}
    constexpr result(value_tag_t) requires std::is_void_v<value_t>
      : result{}
    {}

    constexpr result(value_tag_t) requires std::is_default_constructible_v<value_t>
      : m_box{value_tag}, m_has_value{true}
    {}
    constexpr result(value_tag_t, std::convertible_to<value_t> auto&& value)
      : m_box{value_tag, std::forward<decltype(value)>(value)}, m_has_value{true}
    {}
    constexpr result(std::in_place_t, value_tag_t)
        requires std::is_default_constructible_v<value_t>
      : m_box{std::in_place, value_tag}, m_has_value{true} 
    {}    
    constexpr result(std::in_place_t, value_tag_t, auto&&... args) 
        requires requires() { boxed_t<value_t>{std::forward<decltype(args)>(args)...}; }
      : m_box{std::in_place, value_tag, std::forward<decltype(args)>(args)...},
        m_has_value{true}
    {}

    constexpr result(error_tag_t) requires std::is_default_constructible_v<error_t>
      : m_box{error_tag}
    {}
    constexpr result(error_tag_t, std::convertible_to<error_t> auto&& error)
      : m_box{error_tag, std::forward<decltype(error)>(error)}
    {}
    constexpr result(std::in_place_t, error_tag_t)
      : m_box{std::in_place, error_tag}
    {}    
    constexpr result(std::in_place_t, error_tag_t, auto&&... args) 
      : m_box{std::in_place, error_tag, std::forward<decltype(args)>(args)...}
    {}

    static constexpr bool enable_converting_constructor =
        (not std::is_convertible_v<value_t,error_t>) and 
        (not std::is_convertible_v<error_t,value_t>);

    //NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    constexpr result(std::convertible_to<value_t> auto&& value)
        requires enable_converting_constructor
      : m_box{value_tag, std::forward<decltype(value)>(value)}, m_has_value{true}
    {}

    //NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    constexpr result(std::convertible_to<error_t> auto&& error)
        requires enable_converting_constructor
      : m_box{error_tag, std::forward<decltype(error)>(error)}
    {}

    constexpr result(result const&) requires copy_ctor::use_deleted = delete;
    constexpr result(result const&) requires copy_ctor::use_trivial = default;
    constexpr result(result const& that) requires copy_ctor::use_nontrivial
      : m_has_value{that.m_has_value}
    {
        if(m_has_value) {
            new (&m_box.value) value_t{that.m_box.value};
        } else {
            new (&m_box.error) error_t{that.m_box.error};
        }
    }

    constexpr result(result&&) requires move_ctor::use_deleted = delete;
    constexpr result(result&&) requires move_ctor::use_trivial = default;
    constexpr result(result&& that) requires move_ctor::use_nontrivial
      : m_has_value{std::move(that.m_has_value)}
    {
        if(m_has_value) {
            new (&m_box.value) value_t{std::move(that.m_box.value)};
        } else {
            new (&m_box.error) error_t{std::move(that.m_box.error)};
        }
    }

    constexpr result& operator=(result const&) requires copy_assign::use_deleted = delete;
    constexpr result& operator=(result const&) requires copy_assign::use_trivial = default;
    constexpr result& operator=(result const& that) requires copy_assign::use_nontrivial
    {
        if(m_has_value != that.m_has_value) {
            //FIXME: should this unconditionally destroy the boxed value/error?
            destroy_boxed();
            m_has_value = that.m_has_value;
            if(m_has_value) {
                new (&m_box.value) value_t{that.m_box.value};
            } else {
                new (&m_box.error) error_t{that.m_box.error};
            }
        } else {
            if(m_has_value) {
                m_box.value = that.m_box.value;
            } else {
                m_box.error = that.m_box.error;
            }
        }
        return *this;
    }

    constexpr result& operator=(result&&) requires move_assign::use_deleted = delete;
    constexpr result& operator=(result&&) requires move_assign::use_trivial = default;
    constexpr result& operator=(result&& that) requires move_assign::use_nontrivial
    {
        if(m_has_value != that.m_has_value) {
            destroy_boxed();
            m_has_value = std::move(that.m_has_value);
            if(m_has_value) {
                new (&m_box.value) value_t{std::move(that.m_box.value)};
            } else {
                new (&m_box.error) error_t{std::move(that.m_box.error)};
            }
        } else {
            if(m_has_value) {
                m_box.value = std::move(that.m_box.value);
            } else {
                m_box.error = std::move(that.m_box.error);
            }
        }
        return *this;
    }

    //FIXME: consider virtual destructors
    // constexpr ~result() requires dtor::use_deleted = delete;
    // constexpr ~result() requires dtor::use_trivial = default;
    // constexpr ~result() requires dtor::use_nontrivial
    // {
    //     destroy_boxed();
    // }
    // Remove this destructor when uncommenting the ones above.
    // using dtor_impl_t::~dtor_impl_t;
    constexpr ~result()
    { 
        if constexpr(dtor::use_nontrivial)
        {
            destroy_boxed();
        }
    }
    


    [[nodiscard]] constexpr explicit operator bool() const { return has_value(); }
    [[nodiscard]] constexpr bool has_value() const { return m_has_value; }
    [[nodiscard]] constexpr bool is_error() const { return not has_value(); }



    [[nodiscard]] constexpr value_return_t value() &
        requires (not std::is_void_v<value_t>)
    {
        return m_box.value;
    }

    [[nodiscard]] constexpr value_const_return_t value() const&
        requires (not std::is_void_v<value_t>)
    {        
        return m_box.value;
    }

    //disallow value unboxing on an rvalue result
    constexpr auto value() && requires (not std::is_void_v<value_t>) = delete;

    [[nodiscard]] constexpr value_return_t value_or(std::same_as<value_t> auto& dfault) &
        requires (not std::is_void_v<value_t>)
    {
        if(not has_value()) return dfault;
        return m_box.value;
    }

    [[nodiscard]] constexpr value_const_return_t value_or(std::same_as<value_t> auto& dfault) const&
        requires (not std::is_void_v<value_t>)
    {
        if(not has_value()) return dfault;
        return m_box.value;
    }

    [[nodiscard]] constexpr error_return_t error() &
    {
        return m_box.error;
    }

    [[nodiscard]] constexpr error_const_return_t error() const&
    {
        return m_box.error;
    }

    //disallow error unboxing on an rvalue result
    constexpr auto error() && = delete;

    template <typename F>
    static constexpr bool invocable_with_value = std::invocable<F,std::result_of<decltype(&this_t::value)(void)>>;

    auto accept(auto&& visitor)
        requires (not std::is_void_v<value_t> and std::invocable<decltype(visitor),value_return_t>)
    {
        if(has_value()) {
            if constexpr(std::is_same_v<std::result_of<decltype(visitor)>, void>) {
                visitor(value());
            } else {
                return visitor(value());
            }
        }
    }

    auto accept(std::invocable auto&& visitor)
        requires std::is_void_v<value_t>
    {
        if(has_value()) {
            if constexpr(std::is_same_v<std::result_of<decltype(visitor)>, void>) {
                visitor();
            } else {
                return visitor();
            }
        }
    }

    // auto accept(auto&& visitor) const
    //     requires (not std::is_void_v<value_t> and std::invocable<decltype(visitor),value_const_return_t>)
    // {
    //     if(has_value()) {
    //         if constexpr(std::is_same_v<std::result_of<decltype(visitor)>, void>) {
    //             visitor(value());
    //         } else {
    //             return visitor(value());
    //         }
    //     }
    // }

    // auto accept(std::invocable auto&& visitor) const
    //     requires std::is_void_v<value_t>
    // {
    //     if(has_value()) {
    //         if constexpr(std::is_same_v<std::result_of<decltype(visitor)>, void>) {
    //             visitor();
    //         } else {
    //             return visitor();
    //         }
    //     }
    // }

};


template <typename T>
struct is_result : std::false_type {};

template <typename T, typename E>
struct is_result<result<T,E>> : std::true_type {};

template <typename T, typename E>
struct is_result<result<T,E>&> : std::true_type {};

template <typename T, typename E>
struct is_result<result<T,E> const&> : std::true_type {};

template <typename T, typename E>
struct is_result<result<T,E>&&> : std::true_type {};

template <typename T>
static constexpr bool is_result_v = is_result<T>::value;

template <typename T>
concept any_result = requires { requires is_result_v<T>; };

//Finally, these functions are a convenience for cases where
//result<T,E>'s converting constructors from T & E are disabled.
//Instead of forcing the user to say 
//"return result<T,E>{value_tag{},T}", 
//for example, these allow them to say "return success(T);"
template <typename T, typename E = error_code>
inline constexpr auto failure(E&& error) -> result<T,E> {
    return {error_tag, std::forward<E>(error)};
}

template <typename T, typename E = error_code>
inline constexpr auto success(T&& value) -> result<T,E> {
    return {value_tag, std::forward<T>(value)};
}

//And a version to handle the case where T is void.
template <typename E = error_code>
inline constexpr auto success() -> result<void,E> {
    return {value_tag};
}

//TODO: need a way to disable use of this on an arbitrary result type.
inline constexpr void ignore_result(any_result auto&& res) {
    utl::maybe_unused(res);
}

inline constexpr auto unwrap_pointer(any_result auto&& res) -> std::remove_reference_t<decltype(res.value())>* {
    return res ? &res.value() : nullptr;
}

} //namespace utl
