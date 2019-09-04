#ifndef UTL_RESULT_HH_
#define UTL_RESULT_HH_

#include <stdint.h>
#include <utility>
#include <bits/refwrap.h>
#include <type_traits>
#include "error.hh"

namespace utl {

struct value_tag {};
struct error_tag {};


template <typename T>
struct traits {
    static constexpr bool is_lvalue_ref = std::is_lvalue_reference_v<T>;
    static constexpr bool is_rvalue_ref = std::is_rvalue_reference_v<T>;    
    using storage_t = typename std::conditional_t<is_lvalue_ref, std::reference_wrapper<typename std::remove_reference_t<T>>, T>;
    using return_t = typename std::remove_reference_t<T>;
    static constexpr bool storage_is_trivially_destructible = std::is_trivially_destructible_v<storage_t>;
};

template <typename T, typename E>
struct storage_trivial {
    union {
        T m_value;
        E m_error;
    };
    bool m_has_value;

    storage_trivial() = delete;

    template <typename U>
    constexpr storage_trivial(value_tag, U&& value) 
        : m_value{std::forward<U>(value)}, m_has_value{true}
    {}

    template <typename U>
    constexpr storage_trivial(error_tag, U&& error) 
        : m_error{std::forward<U>(error)}, m_has_value{false}
    {}
};

//I also need to handle non-trivial copy/move constructor
//and non-trivial copy/move assignment operator cases.
//How do I abstract this better?
//well, for each of these a nontrivial functions,
//the equivalent will be deleted on the union type.
//so, for each that exists on one of the provided types,
//I need to provide an implementation that calls into
//the matching method on the contained type.
//woo.
//But I also need to take into account the case where
//the contained type doesn't have a corresponding method
//(it's been deleted). It'd be good to bubble up useful
//errors when that happens.

//product of:
// move - deleted, trivial, nontrivial
// copy - deleted, trivial, nontrivial
// destructor - deleted, trivial, nontrivial
// if it's trivial, it's automatic.
// if it's nontrivial, it'll be deleted on the union class and I need to provide it.
// if it's deleted, it'll be... deleted. I don't need to do anything.

//So how do I do this?
//CRTP classes that implement the different bits.
//     T           E            result<T,E>
//  trivial     trivial          trivial
//  trivial     nontrivial       nontrivial
//  trivial     deleted          deleted
//  nontrivial  trivial          nontrivial
//  nontrivial  nontrivial       nontrivial
//  nontrivial  deleted          deleted
//  deleted     trivial          deleted
//  deleted     nontrivial       deleted
//  deleted     deleted          deleted
//assuming I don't want to be making runtime decisions about this
//(and therefore throwing exceptions), I think if either is deleted
//I need to also delete.
//Why is this? Because at compile time I don't know which type
//is boxed. I could theoretically make a decision in a constexpr
//context... but that sounds like a pain in the ass.
//But, making it a bit configurable in that regard would be good.

//So, I need to be able to mix-and-match with copy & move.
//destruction comes from the base storage type.
//   copy =    move =    copy{}    move{}
//  trivial   trivial   trivial   trivial
//  trivial   trivial   trivial   nontrivial
//       ...

//Okay. For each of these, trivial is free.
//deleted is free.
//so, I just need to create a nontrivial version for each of these.

//But, I do need to specify default, delete, or an implementation
//for each of the special functions.

//So I do need to define a type that deletes each of these somewhere
//in the inheritance hierarchy.

template <typename Base>
struct storage_nontrivial_copy_construct : public Base {

};

template <typename T, typename E>
struct storage_nontrivial {
    using value_traits = traits<T>;
    using error_traits = traits<E>;

    union {
        T m_value;
        E m_error;
    };
    bool m_has_value;

    storage_nontrivial() = delete;

    template <typename U>
    constexpr storage_nontrivial(value_tag, U&& value) 
        : m_value{std::forward<U>(value)}, m_has_value{true}
    {}

    template <typename U>
    constexpr storage_nontrivial(error_tag, U&& error) 
        : m_error{std::forward<U>(error)}, m_has_value{false}
    {}

    ~storage_nontrivial() {
        if(m_has_value) {
            if constexpr(!value_traits::storage_is_trivially_destructible) {
                m_value.~T();
            }
        } else {
            if constexpr(!value_traits::storage_is_trivially_destructible) {
                m_error.~E();
            }
        }
    }
};

template <typename T, typename E>
struct predicate {
    static constexpr bool value_storable = !traits<T>::is_rvalue_ref;
    static constexpr bool error_storable = !traits<E>::is_rvalue_ref;
    static constexpr bool value_error_convertible = std::is_convertible<T,E>::value
        or std::is_convertible<E,T>::value;
    static constexpr bool can_use_trivial_storage = traits<T>::storage_is_trivially_destructible 
        and traits<E>::storage_is_trivially_destructible;
    static constexpr bool enable_implicit_construction = !value_error_convertible;
};

template <typename T, typename E = error_code>
class [[nodiscard]] result {
    using value_traits = traits<T>;
    using error_traits = traits<E>;
    using value_storage_t = typename value_traits::storage_t;
    using error_storage_t = typename error_traits::storage_t;
    using value_return_t = typename value_traits::return_t;
    using error_return_t = typename error_traits::return_t;  

    using predicate = predicate<T, E>;

    static_assert(predicate::value_storable, "cannot store value type");
    static_assert(predicate::error_storable, "cannot store error type");

    using storage_class_t = std::conditional_t<predicate::can_use_trivial_storage, 
        storage_trivial<value_storage_t, error_storage_t>, 
        storage_nontrivial<value_storage_t, error_storage_t>>;

    storage_class_t m_storage;

public:
    template <typename U>
    constexpr result(value_tag, U&& value) : m_storage{value_tag{}, std::forward<U>(value)}
    {}

    template <typename U>
    constexpr result(error_tag, U&& error) : m_storage{error_tag{}, std::forward<U>(error)}
    {}

    template <typename U, typename std::enable_if<predicate::enable_implicit_construction && std::is_convertible_v<U,T>, int*>::type = nullptr>
    constexpr result(U&& value) : m_storage{value_tag{}, std::forward<U>(value)} 
    {}

    template <typename U, typename std::enable_if<predicate::enable_implicit_construction && std::is_convertible_v<U,E>, int*>::type = nullptr>
    constexpr result(U&& error) : m_storage{error_tag{}, std::forward<U>(error)}
    {}

    constexpr explicit operator bool() const { return has_value(); }

    constexpr bool has_value() const { return m_storage.m_has_value; }
    constexpr bool is_error() const { return !has_value(); }

    constexpr value_return_t& value() & {
        // this doesn't seem to be available?
        // if constexpr(!std::is_constant_evaluated()) {
        //     if(!has_value()) printf("oh no");
        // }

        if constexpr(value_traits::is_lvalue_ref) {
            return m_storage.m_value.get();
        } else {
            return m_storage.m_value; 
        }
    }

    constexpr error_return_t& error() & { 
        // if constexpr(!std::is_constant_evaluated()) {
        //     if(has_value()) printf("oh no");
        // }

        if constexpr(error_traits::is_lvalue_ref) {
            return m_storage.m_error.get();
        } else {
            return m_storage.m_error; 
        }
    }

    constexpr const value_return_t& value() const& { 
        // if constexpr(!std::is_constant_evaluated()) {
        //     if(!has_value()) printf("oh no");
        // }

        if constexpr(value_traits::is_lvalue_ref) {
            return m_storage.m_value.get();
        } else {
            return m_storage.m_value; 
        }
    }

    constexpr const error_return_t& error() const& { 
        // if constexpr(!std::is_constant_evaluated()) {
        //     if(has_value()) printf("oh no");
        // }

        if constexpr(error_traits::is_lvalue_ref) {
            return m_storage.m_error.get();
        } else {
            return m_storage.m_error; 
        }
    }

    //Don't allow value/error unboxing on rvalue results.
    //The rationale here is that you can't both examine
    //and unbox a temporary result; in order to do
    //both you have to actually give it a name.
    constexpr value_return_t&& value() && = delete;
    constexpr error_return_t&& error() && = delete;
};

template <typename T, typename E = error_code>
static constexpr auto failure(T&& error) -> result<T,E>&& {
    return {error_tag{}, std::forward<T>(error)};
}

template <typename T, typename E = error_code>
static constexpr auto success(T&& value) -> result<T,E>&& {
    return {value_tag{}, std::forward<T>(value)};
}

} //namespace utl

#endif //UTL_RESULT_HH_
