#ifndef UTL_VARIANT_HH_
#define UTL_VARIANT_HH_

#include <type_traits>
#include <utl/utl.hh>

namespace utl {

template <typename... Ts>
class variant {
    std::aligned_union_t<1,Ts...> m_storage;
    size_t m_index;

    template <size_t N, typename F>
    void _accept(F&& visitor) {
        if(N == m_index) {
            apply<N,F>(std::forward<F>(visitor));
        } else {
            if constexpr(N == 0) {
                return;
            } else {
                _accept<N-1,F>(std::forward<F>(visitor));
            }
        }
    }

    template <size_t N, typename F>
    void apply(F&& visitor) {
        using value_t = utl::get_t<N,Ts...>;
        visitor(*reinterpret_cast<value_t*>(&m_storage));
    }
public:

    template <typename T>
    variant(T&& value) : m_storage{}, m_index{utl::get_type_index_v<T,Ts...>} {
        static_assert(utl::contains_v<T,Ts...>, "variant cannot hold type");
        new (&m_storage) T{std::forward<T>(value)};
    }

    template <typename F>
    void accept(F&& visitor) {
        _accept<sizeof...(Ts) - 1, F>(std::forward<F>(visitor));
    }

    template <typename T>
    variant& operator=(T&& value) {
        static_assert(utl::contains_v<T,Ts...>, "variant cannot hold type");
        new (&m_storage) T{std::forward<T>(value)};
        m_index = utl::get_type_index_v<T,Ts...>;
        return *this;
    }
};


} //namespace utl

#endif //UTL_VARIANT_HH_
