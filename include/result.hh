#ifndef UTL_RESULT_HH_
#define UTL_RESULT_HH_

#include <stdint.h>
#include <utility>
#include <bits/refwrap.h>
#include <type_traits>
#include "error.hh"
#include "utl.hh"

namespace utl {

struct value_tag {};
struct error_tag {};
struct in_place_t {};

namespace detail {
    struct uninitialized_tag{};
    struct void_value_t{};
}

//TODO: maybe refactor the different constructors/operators & void support
//as policies that are automatically selected by the result type?
//FIXME: noexcept everything

template <typename T>
struct traits {
    static constexpr bool is_lvalue_ref = std::is_lvalue_reference_v<T>;
    static constexpr bool is_rvalue_ref = std::is_rvalue_reference_v<T>;
    static constexpr bool is_void = std::is_void_v<T>;
    using storage_t = std::conditional_t<is_void,    
        detail::void_value_t,
        std::conditional_t<
            is_lvalue_ref, 
            std::reference_wrapper<std::remove_reference_t<T>>, 
            T
    >>;
    static constexpr bool storage_is_trivially_destructible = std::is_trivially_destructible_v<storage_t>;
};

//This is the "trivial" case of the storage type. If
//both T and E have trivial destructors (basically, not
//user declared) then the union itself will also have
//a trivial destructor... and we don't need to do anything
//special to destruct the storage.
template <typename T, typename E>
struct storage_trivial_destruct {
    using value_t = T;
    using error_t = E;
    struct uninitialized_t {};

    union {
        T m_value;
        E m_error;
        uninitialized_t m_uninitialized;
    };
    bool m_has_value;

    storage_trivial_destruct() = delete;

    template <typename U>
    constexpr storage_trivial_destruct(value_tag, U&& value) 
        : m_value{std::forward<U>(value)}, m_has_value{true}
    {}

    constexpr storage_trivial_destruct(in_place_t, value_tag)
        : m_value{}, m_has_value{true}
    {}

    template <typename... Args>
    constexpr storage_trivial_destruct(in_place_t, value_tag, Args&&... args)
        : m_value{std::forward<Args>(args)...}, m_has_value{true}
    {}

    template <typename U>
    constexpr storage_trivial_destruct(error_tag, U&& error) 
        : m_error{std::forward<U>(error)}, m_has_value{false}
    {}

    constexpr storage_trivial_destruct(in_place_t, error_tag)
        : m_error{}, m_has_value{false}
    {}

    template <typename... Args>
    constexpr storage_trivial_destruct(in_place_t, error_tag, Args&&... args)
        : m_error{std::forward<Args>(args)...}, m_has_value{false}
    {}

    constexpr void destroy() {}

    template <typename... Args>
    void emplace(error_tag, Args&&... args) {
        destroy();
        m_has_value = false;
        new (&m_error) error_t{std::forward<Args>(args)...};
    }

    template <typename... Args>
    void emplace(value_tag, Args&&... args) {
        destroy();
        m_has_value = true;
        new (&m_value) value_t{std::forward<Args>(args)...};
    }

    //FIXME: these aren't quite there.
    // - support rvalues
    // - useful error messages if types aren't copyable/movable
    void set_error(E& error) {
        destroy();
        m_has_value = false;
        m_error = error;
    }

    void set_value(T& value) {
        destroy();
        m_has_value = true;
        m_value = value;
    }

protected:
    //The nontrival copy & move constructors can't initialize
    //the storage in their initializer lists, because we don't
    //know what the storage we're copying/moving from contains
    //until runtime. This constructor gives us a safer way to 
    //"default construct" a storage object, though it's protected
    //to prevent external use because it's semantically illegal
    //to leave a storage object in this uninitialized state
    //(if it was legal, the result type would have to handle
    //the case where the storage is uninitialized by either
    //"throwing" an "exception" or forcing the user to be
    //aware of this state).
    storage_trivial_destruct(detail::uninitialized_tag) : m_uninitialized{}, m_has_value{false} {}
};

//If either T or E has a nontrivial (user declared) destructor,
//then the union of T and E will have its destructor deleted.
//If we don't do anything about that, then the value/error held
//by the union will not have its destructor called when the
//storage is destructed. To handle it, this nontrivial storage
//type has a destructor that looks at which type is currently
//being held and calls its destructor directly if necessary.
template <typename T, typename E>
struct storage_nontrivial_destruct {
    using value_traits = traits<T>;
    using error_traits = traits<E>;
    using value_t = T;
    using error_t = E;
    struct uninitialized_t {};

    union {
        T m_value;
        E m_error;
        uninitialized_t m_uninitialized;
    };
    bool m_has_value;

    storage_nontrivial_destruct() = delete;

    template <typename U>
    constexpr storage_nontrivial_destruct(value_tag, U&& value) 
        : m_value{std::forward<U>(value)}, m_has_value{true}
    {}

    constexpr storage_nontrivial_destruct(in_place_t, value_tag)
        : m_value{}, m_has_value{true}
    {}

    template <typename... Args>
    constexpr storage_nontrivial_destruct(in_place_t, value_tag, Args&&... args)
        : m_value{std::forward<Args>(args)...}, m_has_value{true}
    {}

    template <typename U>
    constexpr storage_nontrivial_destruct(error_tag, U&& error) 
        : m_error{std::forward<U>(error)}, m_has_value{false}
    {}

    constexpr storage_nontrivial_destruct(in_place_t, error_tag)
        : m_error{}, m_has_value{false}
    {}

    template <typename... Args>
    constexpr storage_nontrivial_destruct(in_place_t, error_tag, Args&&... args)
        : m_error{std::forward<Args>(args)...}, m_has_value{false}
    {}

    constexpr void destroy() {
        if(m_has_value) {
            if constexpr(!value_traits::storage_is_trivially_destructible) {
                m_value.~T();
            }
        } else {
            if constexpr(!error_traits::storage_is_trivially_destructible) {
                m_error.~E();
            }
        }
    }

    template <typename... Args>
    void emplace(error_tag, Args&&... args) {
        destroy();
        m_has_value = false;
        new (&m_error) error_t{std::forward<Args>(args)...};
    }

    template <typename... Args>
    void emplace(value_tag, Args&&... args) {
        destroy();
        m_has_value = true;
        new (&m_value) value_t{std::forward<Args>(args)...};
    }

    //FIXME: these aren't quite there.
    // - support rvalues
    // - useful error messages if types aren't copyable/movable
    void set_error(E& error) {
        destroy();
        m_has_value = false;
        m_error = error;
    }

    void set_value(T& value) {
        destroy();
        m_has_value = true;
        m_value = value;
    }

    ~storage_nontrivial_destruct() {
        destroy();
    }

protected:
    storage_nontrivial_destruct(detail::uninitialized_tag) : m_uninitialized{}, m_has_value{false} {}
};

//If a union contains a type that has a nontrivial 
//(see e.g. https://en.cppreference.com/w/cpp/language/copy_constructor) 
// or deleted special function (destructor, copy constructor, copy 
//assignment operator, move constructor, or move assignment operator), 
//the corresponding special function will be deleted on the union itself.
//This means that if either T or E has a nontrivial special function,
//the storage class will not be able to use that function - and,
//therefore, result<T,E> will not be able to use it.
//We already handle the destructor with the two trivial/nontrivial
//storage classes, but we also need to handle the other functions.
//At a high level, we want the semantics of the result type to match
//the combined semantics of the result and error types it can hold:
//if both of them can be copy constructed then we should be able
//to copy construct the corresponding result type, even one of the
//contained types has a nontrivial copy constructor. The table below
//shows how we want this to behave:
//
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
//
//Unfortunately, this does mean that if either T or E has a deleted
//special function, result<T,E> must delete that function. This is
//because we don't know which one we're actually holding until runtime,
//and if we make the decision of whether or not we can use a special
//function at runtime then we can only handle failure by throwing an
//exception.
//Thankfully, we can take advantage of the fact that a deleted
//special function on either T or E will result in that special function
//being deleted on the storage's union - we'll already get a compiler
//error if we try to perform the corresponding operation.
//This means that we only need to handle the cases where result<T,E>
//needs to have a nontrivial special function.
//
//We implement these as classes that inherit from a templated Base class,
//each of which defines a nontrivial version of one of the special functions
//and sets the others to default (which will make them inherit from the
//Base class). By doing this, we can assemble a storage type piece by
//piece that has its special functions appropriately defaulted to the
//union's trivial implementation, defaulted to the union's deleted version, 
//or overridden with a nontrivial version that will call T or E's nontrivial
//version depending on which type we're currently holding.

template <typename Base>
struct storage_nontrivial_copy_construct : public Base {
    using Base::m_has_value;
    using Base::m_value;
    using Base::m_error;
    using Base::Base;
    using value_t = typename Base::value_t;
    using error_t = typename Base::error_t;

    ~storage_nontrivial_copy_construct() = default;
    constexpr storage_nontrivial_copy_construct(storage_nontrivial_copy_construct const& that) 
        : Base{detail::uninitialized_tag{}}
    {
        m_has_value = that.m_has_value;
        //TODO: use std::is_constant_evaluated to make this more constexpr
        if(m_has_value) {
            new (&m_value) typename Base::value_t{that.m_value};
        } else {
            new (&m_error) typename Base::error_t{that.m_error};
        }
    }
    constexpr storage_nontrivial_copy_construct& operator=(storage_nontrivial_copy_construct const& that) = default;
    constexpr storage_nontrivial_copy_construct(storage_nontrivial_copy_construct&& that) = default;
    constexpr storage_nontrivial_copy_construct& operator=(storage_nontrivial_copy_construct&& that) = default;    
};

template <typename Base>
struct storage_nontrivial_copy_assign : public Base {
    using Base::m_has_value;
    using Base::m_value;
    using Base::m_error;
    using Base::destroy;
    using Base::Base;
    using value_t = typename Base::value_t;
    using error_t = typename Base::error_t;

    ~storage_nontrivial_copy_assign() = default;
    constexpr storage_nontrivial_copy_assign(storage_nontrivial_copy_assign const& that) = default;
    constexpr storage_nontrivial_copy_assign& operator=(storage_nontrivial_copy_assign const& that)
    {
        //TODO: use std::is_constant_evaluated to make this more constexpr
        if(m_has_value != that.m_has_value) {
            destroy();
            m_has_value = that.m_has_value;
            if(m_has_value) {
                new (&m_value) typename Base::value_t{that.m_value};
            } else {
                new (&m_error) typename Base::error_t{that.m_error};
            }
        } else {
            if(m_has_value) {
                m_value = that.m_value;
            } else {
                m_error = that.m_error;
            }
        }
        return *this;
    }
    constexpr storage_nontrivial_copy_assign(storage_nontrivial_copy_assign&& that) = default;
    constexpr storage_nontrivial_copy_assign& operator=(storage_nontrivial_copy_assign&& that) = default;
};

template <typename Base>
struct storage_nontrivial_move_construct : public Base {
    using Base::m_has_value;
    using Base::m_value;
    using Base::m_error;
    using Base::Base;
    using value_t = typename Base::value_t;
    using error_t = typename Base::error_t;

    ~storage_nontrivial_move_construct() = default;
    constexpr storage_nontrivial_move_construct(storage_nontrivial_move_construct const& that) = default;
    constexpr storage_nontrivial_move_construct& operator=(storage_nontrivial_move_construct const& that) = default;
    constexpr storage_nontrivial_move_construct(storage_nontrivial_move_construct&& that) 
        : Base{detail::uninitialized_tag{}}
    {
        m_has_value = std::move(that.m_has_value);
        //TODO: use std::is_constant_evaluated to make this more constexpr
        if(m_has_value) {
            new (&m_value) typename Base::value_t{std::move(that.m_value)};
        } else {
            new (&m_error) typename Base::error_t{std::move(that.m_error)};
        }
    }
    constexpr storage_nontrivial_move_construct& operator=(storage_nontrivial_move_construct&& that) = default;
};

template <typename Base>
struct storage_nontrivial_move_assign : public Base {
    using Base::m_has_value;
    using Base::m_value;
    using Base::m_error;
    using Base::destroy;
    using Base::Base;
    using value_t = typename Base::value_t;
    using error_t = typename Base::error_t;

    ~storage_nontrivial_move_assign() = default;
    constexpr storage_nontrivial_move_assign(storage_nontrivial_move_assign const& that) = default;
    constexpr storage_nontrivial_move_assign& operator=(storage_nontrivial_move_assign const& that) = default;
    constexpr storage_nontrivial_move_assign(storage_nontrivial_move_assign&& that) = default;
    constexpr storage_nontrivial_move_assign& operator=(storage_nontrivial_move_assign&& that)
    {
        //TODO: use std::is_constant_evaluated to make this more constexpr
        if(m_has_value != that.m_has_value) {
            destroy();
            m_has_value = std::move(that.m_has_value);
            if(m_has_value) {
                new (&m_value) typename Base::value_t{std::move(that.m_value)};
            } else {
                new (&m_error) typename Base::error_t{std::move(that.m_error)};
            }
        } else {
            if(m_has_value) {
                m_value = std::move(that.m_value);
            } else {
                m_error = std::move(that.m_error);
            }
        }
        return *this;
    }    
};

template <typename T, typename E>
struct storage_predicate {
    static constexpr bool value_storable = !traits<T>::is_rvalue_ref;
    static constexpr bool error_storable = !traits<E>::is_rvalue_ref and !traits<E>::is_void;
    static constexpr bool value_error_convertible = std::is_convertible<T,E>::value
        or std::is_convertible<E,T>::value;
    static constexpr bool enable_implicit_construction = !value_error_convertible;
    static constexpr bool is_trivially_destructible = std::is_trivially_destructible_v<T> and std::is_trivially_destructible_v<E>;
    static constexpr bool is_trivially_copy_constructible = std::is_trivially_copy_constructible_v<T> and std::is_trivially_copy_constructible_v<E>;
    static constexpr bool is_copy_constructible = std::is_copy_constructible_v<T> and std::is_copy_constructible_v<E>;
    static constexpr bool is_trivially_copy_assignable = std::is_trivially_copy_assignable_v<T> and std::is_trivially_copy_assignable_v<E>;
    static constexpr bool is_copy_assignable = std::is_copy_assignable_v<T> and std::is_copy_assignable_v<E>;
    static constexpr bool is_trivially_move_constructible = std::is_trivially_move_constructible_v<T> and std::is_trivially_move_constructible_v<E>;
    static constexpr bool is_move_constructible = std::is_move_constructible_v<T> and std::is_move_constructible_v<E>;
    static constexpr bool is_trivially_move_assignable = std::is_trivially_move_assignable_v<T> and std::is_trivially_move_assignable_v<E>;
    static constexpr bool is_move_assignable = std::is_move_assignable_v<T> and std::is_move_assignable_v<E>;

    static constexpr bool use_nontrivial_destruct = !is_trivially_destructible;
    static constexpr bool use_nontrivial_copy_assign = is_copy_assignable and !is_trivially_copy_assignable;
    static constexpr bool use_nontrivial_copy_construct = is_copy_constructible and !is_trivially_copy_constructible;
    static constexpr bool use_nontrivial_move_assign = is_move_assignable and !is_trivially_move_assignable;
    static constexpr bool use_nontrivial_move_construct = is_move_constructible and !is_trivially_move_constructible;

    static constexpr bool use_void_value_storage = std::is_same_v<T,void>;
};

//Finally, we have a series of templates that will select each intermediate
//class based on the characteristics of T and E (as defined in the predicate
//type above). At each stage, we look at whether or not a nontrivial version
//of the special function is required (it is only required if neither T nor
//E's version is deleted, and either T or E has a nontrivial version). If it
//is, we use the corresponding storage type at that stage in the inheritance.
//If it isn't, we don't.

template <typename T, typename E>
using select_storage_destruct_t = std::conditional_t<
    storage_predicate<T,E>::use_nontrivial_destruct,
    storage_nontrivial_destruct<T,E>,
    storage_trivial_destruct<T,E>
>;

template <typename T, typename E>
using select_storage_copy_assign_t = std::conditional_t<
    storage_predicate<T,E>::use_nontrivial_copy_assign,
    storage_nontrivial_copy_assign<select_storage_destruct_t<T,E>>,
    select_storage_destruct_t<T,E>
>;

template <typename T, typename E>
using select_storage_copy_construct_t = std::conditional_t<
    storage_predicate<T,E>::use_nontrivial_copy_construct,
    storage_nontrivial_copy_construct<select_storage_copy_assign_t<T,E>>,
    select_storage_copy_assign_t<T,E>
>;

template <typename T, typename E>
using select_storage_move_assign_t = std::conditional_t<
    storage_predicate<T,E>::use_nontrivial_move_assign,
    storage_nontrivial_move_assign<select_storage_copy_construct_t<T,E>>,
    select_storage_copy_construct_t<T,E>
>;

template <typename T, typename E>
using select_storage_move_construct_t = std::conditional_t<
    storage_predicate<T,E>::use_nontrivial_move_construct,
    storage_nontrivial_move_construct<select_storage_move_assign_t<T,E>>,
    select_storage_move_assign_t<T,E>
>;

//This one's slightly different than the rest. It helps us handle
//the case where T or E are lvalue reference types (e.g. int&). It's
//illegal for a union to hold a reference type, so if T or E is a
//reference then we need to wrap it in something else so we can hold it.
//This template does that for us if necessary, then goes on to build
//up the storage type.
template <typename T, typename E>
using select_union_types_t = select_storage_move_construct_t<
    typename traits<T>::storage_t,
    typename traits<E>::storage_t
>;

template <typename T, typename E>
using select_storage_t = select_union_types_t<T,E>;


template <class T>
struct default_observer_policy {
    using return_t = T&;

    static constexpr T& get(T& storage) {
        return storage;
    }

    static constexpr T const& get(T const& storage) {
        return storage;
    }
};

template <class T>
struct default_observer_policy<T&> {
    using return_t = T&;

    static constexpr T& get(std::reference_wrapper<T>& storage) {
        return storage.get();
    }

    static constexpr T const& get(std::reference_wrapper<T> const& storage) {
        return storage.get();
    }
};

template <>
struct default_observer_policy<void> {
    using return_t = void;

    static constexpr void get(detail::void_value_t) {}
};

//Finally we have the result type.
template <typename T, typename E = error_code, 
    template<class> typename value_observer = default_observer_policy,
    template<class> typename error_observer = default_observer_policy
>
class [[nodiscard]] result {
protected:
    using value_traits = traits<T>;
    using error_traits = traits<E>;
    using value_observer_t = value_observer<T>; 
    using error_observer_t = error_observer<E>;

    using storage_predicate = storage_predicate<T, E>;

    static_assert(storage_predicate::value_storable, "cannot store value type");
    static_assert(storage_predicate::error_storable, "cannot store error type");

    using storage_class_t = select_storage_t<T,E>;
    storage_class_t m_storage;

public:
    using value_t = typename storage_class_t::value_t;
    using error_t = typename storage_class_t::error_t;
    //A result is only default constructible if its value type is void.
    //In that case, calling result<void,E>{} will give you a result that's
    //"holding" a void value. This is useful if you have a void function
    //and you want to indicate success/failure in a way that's consistent
    //with other functions. We allow this because we do not allow E to be
    //void.
    template <typename U = T, std::enable_if_t<traits<U>::is_void, int*> = nullptr>
    result() : m_storage{value_tag{}, detail::void_value_t{}} {}

    template <typename U = T, std::enable_if_t<traits<U>::is_void, int*> = nullptr>
    constexpr result(value_tag) : m_storage{value_tag{}, detail::void_value_t{}}
    {}
    
    template <typename U = T, std::enable_if_t<!traits<U>::is_void, int*> = nullptr>
    constexpr result(value_tag, U&& value) : m_storage{value_tag{}, std::forward<U>(value)}
    {}

    template <typename U = T, std::enable_if_t<!traits<U>::is_void, int*> = nullptr>
    constexpr result(in_place_t, value_tag)
        : m_storage{in_place_t{}, value_tag{}} {}
    
    template <typename... Args>
    constexpr result(in_place_t, value_tag, Args&&... args) 
        : m_storage{in_place_t{}, value_tag{}, std::forward<Args>(args)...} {}

    template <typename U>
    constexpr result(error_tag, U&& error) : m_storage{error_tag{}, std::forward<U>(error)}
    {}

    constexpr result(in_place_t, error_tag)
        : m_storage{in_place_t{}, error_tag{}} {}    

    template <typename... Args>
    constexpr result(in_place_t, error_tag, Args&&... args) 
        : m_storage{in_place_t{}, error_tag{}, std::forward<Args>(args)...} {}

    //We only want to enable this and the next constructor if T and E cannot be
    //converted between each other (that is, if it is valid to say T foo{E} or E foo{T},
    //we want to disable these constructors). If we don't, then saying result<T,E>{T}
    //will be ambiguous - you could end up with a result that's holding a T, or a result
    //that's holding an E. Instead of allowing that ambiguity, we make it illegal.
    template <typename U, std::enable_if_t<storage_predicate::enable_implicit_construction and std::is_convertible_v<U,T>, int*> = nullptr>
    constexpr result(U&& value) : m_storage{value_tag{}, std::forward<U>(value)} 
    {}

    template <typename U, std::enable_if_t<storage_predicate::enable_implicit_construction and std::is_convertible_v<U,E>, int*> = nullptr>
    constexpr result(U&& error) : m_storage{error_tag{}, std::forward<U>(error)}
    {}

    constexpr explicit operator bool() const { return has_value(); }

    constexpr bool has_value() const { return m_storage.m_has_value; }
    constexpr bool is_error() const { return !has_value(); }

    constexpr auto& value() & {
        //fixme: if constant evaluated, do a constexpr check of has_value.
        return value_observer_t::get(m_storage.m_value);
    }

    constexpr auto& value_or(value_t& dfault) & {
        //fixme: if constant evaluated, do a constexpr check of has_value.
        if(!has_value()) return dfault;

        return value_observer_t::get(m_storage.m_value);
    }

    constexpr auto& error() & { 
        //fixme: if constant evaluated, do a constexpr check of has_value.
        return error_observer_t::get(m_storage.m_error);
    }

    constexpr auto& value() const& { 
        //fixme: if constant evaluated, do a constexpr check of has_value.
        return value_observer_t::get(m_storage.m_value);
    }

    constexpr auto& value_or(value_t& dfault) const& {
        //fixme: if constant evaluated, do a constexpr check of has_value.
        if(!has_value()) return dfault;

        return value_observer_t::get(m_storage.m_value);
    }

    constexpr auto& error() const& { 
        //fixme: if constant evaluated, do a constexpr check of has_value.
        return error_observer_t::get(m_storage.m_error);
    }

    //Don't allow value/error unboxing on rvalue results. An rvalue is
    //any value that doesn't have a name yet - for example, the return
    //value of foo in the expression foo().value(). As much as possible,
    //we want to do what we can to force the user to check whether a
    //result contains a value before trying to extract something for it -
    //and if a user is calling value() or error() on a result, they
    //definitely haven't checked whether it contains a value or not. Put
    //another way, it's impossible to check if a result contains
    //something _and_ extract the value if it's an rvalue.
    constexpr auto value() && -> const typename value_observer_t::return_t = delete;
    constexpr auto error() && -> const typename error_observer_t::return_t = delete;

    template <typename F>
    using visitor_invoke_result_t = std::invoke_result_t<F, value_t&>;
    //Visitation is always legal since it is checked.
    template <typename F>
    auto visit(F visitor) -> visitor_invoke_result_t<F> {
        if(has_value()) {
            if constexpr (std::is_same_v<visitor_invoke_result_t<F>, void>) {
                visitor(value());
            } else {
                return visitor(value());
            }
        }
    }

    template <typename F>
    auto visit(const F visitor) const -> visitor_invoke_result_t<F> {
        if(has_value()) {
            if constexpr (std::is_same_v<visitor_invoke_result_t<F>, void>) {
                visitor(value());
            } else {
                return visitor(value());
            }
        }
    }

    //I could extend this with try_t to do visitation conditioned on a collection of maybe-results.
    //Free function that examines its argument list for try's, attempts to unwrap them, and calls
    //the callable if everything is unwrappable.
    //oooOOOOoooooo


};

//Finally, these functions are a convenience for cases where
//result<T,E>'s converting constructors from T & E are disabled.
//Instead of forcing the user to say 
//"return result<T,E>{value_tag{},T}", 
//for example, these allow them to say "return success(T);"
template <typename T, typename E = error_code>
static constexpr auto failure(T&& error) -> result<T,E> {
    return {error_tag{}, std::forward<T>(error)};
}

template <typename T, typename E = error_code>
static constexpr auto success(T&& value) -> result<T,E> {
    return {value_tag{}, std::forward<T>(value)};
}

//And a version to handle the case where T is void.
template <typename E = error_code>
static constexpr auto success() -> result<void,E> {
    return {value_tag{}};
}

template <typename T, typename E>
static constexpr void ignore_result(result<T,E> res) {
    utl::maybe_unused(res);
}


template <typename T>
struct is_result : std::false_type {};

template <typename T, typename E>
struct is_result<result<T,E>> : std::true_type {};

template <typename T>
static constexpr bool is_result_v = is_result<T>::value;
//TODO: implement some compile-time tests here with static asserts.

} //namespace utl

#endif //UTL_RESULT_HH_
