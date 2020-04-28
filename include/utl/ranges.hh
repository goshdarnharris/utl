#pragma once

#include <utility>

namespace utl::ranges {

// auto && __range = range_expression ;
// auto __begin = begin_expr ;
// auto __end = end_expr ;
// for ( ; __begin != __end; ++__begin) {
// range_declaration = *__begin;
// loop_statement
// }

//I want to be able to customize what happens
//when * is dereferenced.

namespace detail {

template <typename T, typename R>
struct marker {
    T position;
    R& range;
    marker& operator++()
    {
        position++;
        range.advance(position);
        return *this;
    }
    bool operator!=(marker& other) const
    {
        return position != other.position;
    }
    typename R::enumeration& operator*()
    {
        return range.transform(position);
    }
};

template <typename T, typename R>
marker(T,R&) -> marker<T,R>;


} //namespace detail

//FIXME: doing this with tuple-based bindings would be better; I can
// pass reference types through more directly that way without changing them.
//FIXME: this needs tests. badly.
//TODO: consider using templight to see what's going on there in more detail.

template <typename R>
class enumerate {
    using value_t = decltype(*std::declval<R>().begin());
public:
    struct enumeration {
        size_t index;
        value_t value;
    };
private:
    R& m_iterable;
    enumeration m_enumeration;
public:
    enumerate(R& iterable) 
      : m_iterable{iterable}, m_enumeration{0,*m_iterable.begin()}
    {}
    constexpr decltype(auto) begin() { return detail::marker{m_iterable.begin(), *this}; }
    constexpr decltype(auto) end() { return detail::marker{m_iterable.end(), *this}; }
    template <typename T>
    void advance(T&& position) { m_enumeration.index++; m_enumeration.value = *position; }
    template <typename T>
    enumeration& transform(T&& position) { utl::maybe_unused(position); return m_enumeration; }
};

template <typename R>
enumerate(R&&) -> enumerate<R>;


} //namespace utl
