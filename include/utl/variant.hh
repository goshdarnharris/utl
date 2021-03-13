#ifndef UTL_VARIANT_HH_
#define UTL_VARIANT_HH_

#include <type_traits>
#include <utl/utl.hh>

namespace utl {

template <typename... Ts>
class variant {
    std::aligned_union_t<1,Ts...> m_storage;
    size_t m_index;

    template <bool Const, size_t N, typename F>
    void _accept(F&& visitor) const {
        if(N == m_index) {
            using value_t = utl::get_t<N,Ts...>;
            if constexpr(Const) {
                auto value_ptr = reinterpret_cast<value_t const*>(&m_storage);
                visitor(*value_ptr);
            } else {
                auto value_ptr = reinterpret_cast<value_t const*>(&m_storage);
                visitor(*const_cast<value_t*>(value_ptr)); //NOLINT(cppcoreguidelines-pro-type-const-cast)
            }
        } else {
            if constexpr(N == 0) {
                return;
            } else {
                _accept<Const,N-1,F>(std::forward<F>(visitor));
            }
        }
    }

public:

    template <typename T>
    requires utl::contains_v<T,Ts...> //NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    variant(T&& value) : m_storage{}, m_index{utl::get_type_index_v<T,Ts...>} {
        new (&m_storage) T{std::forward<T>(value)};
    }

    template <typename F>
    void accept(F&& visitor) {
        _accept<false, sizeof...(Ts) - 1, F>(std::forward<F>(visitor));
    }

    template <typename F>
    void accept(F&& visitor) const {
        _accept<true, sizeof...(Ts) - 1, F>(std::forward<F>(visitor));
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
