#ifndef UTL_CONSTRUCT_HH_
#define UTL_CONSTRUCT_HH_

#include <result.hh>

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

template <class T>
struct construct : public detail::result_t<T> {
    using detail::result_t<T>::value;
    using detail::result_t<T>::m_storage; 

    //FIXME: add static_asserts around visibility of constructors & validate   
    //enforce constexpr on constructors?

    constexpr construct()
        : detail::result_t<T>{in_place_t{}, value_tag{}}
    {}

    template <typename... Args>
    constexpr construct(Args&&... args) 
        : detail::result_t<T>{in_place_t{}, value_tag{}, std::forward<Args>(args)...}
    {
        auto res = m_storage.m_value.validate();
        if(!res) {
            m_storage.set_error(res.error());
        }
    }
};

} //namespace utl

#endif //UTL_CONSTRUCT_HH_
