#pragma once
#include <utl/array.hh>

namespace utl {

template <typename T>
class span {
    T* m_container;
    size_t m_size;
public:
    template <typename U, 
        typename = utl::enable_if_t<utl::is_same_v<U,T*> ,U>>
    constexpr span(U container, size_t size) : m_container{container}, m_size{size} {}

    template <size_t N>
    constexpr span(T (&arr)[N]) : m_container{arr}, m_size{N} {}

    template <size_t N>
    constexpr span(array<T,N>& arr) : m_container{arr.data()}, m_size{N} {}

    constexpr T& operator[](size_t index) const
    {
        return m_container[index];
    }
    
    constexpr T* data() const { return m_container; }
    constexpr size_t size() const { return m_size; }

    constexpr T* begin() const { return m_container; }
    constexpr T* end() const { return m_container + m_size; }
};

} //namespace utl
