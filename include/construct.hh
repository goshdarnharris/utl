#ifndef UTL_CONSTRUCT_HH_
#define UTL_CONSTRUCT_HH_

#include "result.hh"
#include "system-error.hh"
#include <experimental/type_traits>

namespace utl {

//TODO list
// - enforce that all constructors & a validate method are protected on T.
// - implement policies on result
// - support default constructible objects


namespace detail {
    template <class T>
    struct accessor : public T {
        using underlying_t = T;

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
}


template <typename T>
struct try_t {
    static_assert(is_result_v<T>, "try_t can only be used with types satisfying is_result<T>");
    using value_t = typename T::value_t;
    T& _dep;
    try_t(T& dep) : _dep{dep} {}
};

struct dependent_t {};

template <typename T>
constexpr bool is_try_v = false;

template <typename T>
constexpr bool is_try_v<try_t<T>> = true;

template <class T>
using validate_t = decltype(std::declval<T>().validate());


template <class T>
constexpr bool is_valid(T value) {
    utl::maybe_unused(value);
    return true;
}

template <class T>
constexpr bool is_valid(T&& value) {
    utl::maybe_unused(value);
    return true;
}

template <class T, std::enable_if<is_try_v<T>,int*> = nullptr>
constexpr bool is_valid(T&& value) {
    return value._dep.has_value();
}

template <class T>
constexpr T get_value(T value) {
    return value;
}

template <class T>
constexpr auto get_value(T&& value) -> T&&
{
    return std::forward<T>(value);
}

template <class T, std::enable_if<is_try_v<T>,int*> = nullptr>
constexpr auto get_value(T& value) -> typename T::value_t&
{
    return value._dep.value();
}

//TODO: would it be possible to make this type a set of policies on result?
template <class T>
struct construct : public detail::result_t<T> {
    using detail::result_t<T>::value;
    using detail::result_t<T>::m_storage; 

    static constexpr bool has_validate = std::experimental::is_detected<validate_t,T>::value;

    //FIXME: add static_asserts around visibility of constructors & validate   
    //enforce constexpr on constructors?

    constexpr construct()
        : detail::result_t<T>{in_place_t{}, value_tag{}}
    {}

    template <typename... Args>
    constexpr construct(Args&&... args) 
        : detail::result_t<T>{in_place_t{}, value_tag{}, std::forward<Args>(args)...}
    {
        if constexpr(has_validate) {
            auto res = m_storage.m_value.validate();
            if(!res) {
                m_storage.set_error(res.error());
            }
        }
    }

    template <typename... Args>
    constexpr construct(dependent_t, Args&&... args)
        : detail::result_t<T>{in_place_t{}, error_tag{}, utl::system_error::UNKNOWN}
    {
        bool can_construct = (is_valid<Args>(args) or ...);
        if(!can_construct) {
            m_storage.emplace(error_tag{}, utl::system_error::UNKNOWN);
            return;
        }

        m_storage.emplace(value_tag{}, get_value<Args>(args)...);

        if constexpr(has_validate) {
            auto res = m_storage.m_value.validate();
            if(!res) {
                m_storage.emplace(error_tag{}, res.error());
            }
        }
    }
    // template <typename... Args>
    // constexpr construct(dependent_t, Args&&... args) 
    //     : detail::result_t<T>{in_place_t{}, error_tag{}, utl::system_error::UNKNOWN}
    // {        
    //     //Step 0: construct this with an error.
    //     //  I think this implies a new error category. This way I can have a
    //     //  "dependent construction failed" error.
    //     //Step 1: go through the list, and test each type to see:
    //     //  is it a construct? if not, okay.
    //     //  is the corresponding argument the unwrapped type? if not, we can short circuit the rest of this.
    //     //    ...how do I test this? maybe I need to wrap individual args?
    //     //    that might make more sense. force the user to wrap constructs that need unwrapping in a utl::dependent{}
    //     //    type or something, and we only attempt to unwrap those.
    //     //    then, this constructor can be enabled if any of the args are of that type.
    //     //  if it is a construct, does it contain a value? if so, okay.
    //     //  if either isn't okay, we will contain the first error code we come across.
    //     //  or maybe there's a new kind of error that can wrap multiple codes. I dunno.
    //     //Step 2: unwrap the constructs and emplace the value
    //     //Step 3: validate as normal.

    //     //Should this be a separate type?

    //     //this needs:
    //     // - a way to tell if a type is a construct (result would be more general)
    //     // - emplace for results

    //     auto res = m_storage.m_value.validate();
    //     if(!res) {
    //         m_storage.set_error(res.error());
    //     }
    // }
};

template <typename T>
struct is_result<construct<T>> : std::true_type {};

} //namespace utl

#endif //UTL_CONSTRUCT_HH_
