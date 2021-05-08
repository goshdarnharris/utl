#pragma once

#include <utility>
#include <utl/tuple.hh>
#include <type_traits>

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
//I want to be able to customize what happens
//when the iterator is incremented.





template <typename T>
constexpr auto* begin(T&& container) { return container.begin(); }

template <typename T, size_t N>
constexpr auto* begin(T (&container)[N]) { return &container[0]; } //NOLINT(cppcoreguidelines-avoid-c-arrays)

template <typename T>
constexpr auto* end(T&& container) { return container.end(); }

template <typename T, size_t N>
constexpr auto* end(T (&container)[N]) { return &container[N-1]; } //NOLINT(cppcoreguidelines-avoid-c-arrays)

//FIXME: these concepts need a lot of refinement.

template <typename C>
concept iterable = requires(C&& container) {
    begin(container);
    end(container);
    requires requires(decltype(begin(container))&& iter) {
        iter++;
        iter != end(container);
    };
};

template <typename C, typename T>
concept output_iterable = iterable<C> and requires(C&& container, T&& v) {
    requires requires(decltype(begin(container)) iter) {
        *iter++ = v;
    };
};

template <iterable T>
struct range_traits {
    using begin_t = decltype(begin(std::declval<T>()));
    using end_t = decltype(end(std::declval<T>()));
    using value_t = decltype(*begin(std::declval<T>()));
};

template <iterable T>
constexpr auto enumerate(T&& container)
{
    struct iterator {
        using container_begin_t = typename range_traits<T>::begin_t;
        size_t index;
        container_begin_t iter;

        constexpr void operator ++() { iter++; index++; }
        constexpr bool operator !=(iterator const& other) const { return iter != other.iter; }
        constexpr auto operator *() const { return utl::tie(index, *iter); }
    }; 
    struct enumerator {
        decltype(container) c;
        constexpr auto begin() { return iterator{0, utl::ranges::begin(c)}; }
        constexpr auto end() { return iterator{0, utl::ranges::end(c)}; }
    };
    return enumerator{std::forward<T>(container)};
};


template <typename T>
concept is_reversable_iterator = requires(T& v) {
    v--;
    *v;
};

template <is_reversable_iterator T>
struct reverse_iterator {
    T active;
    constexpr auto operator *() const { return *active; }
    constexpr void operator++() { active--; }
    constexpr bool operator!=(reverse_iterator const& other) const { return active != other.active; }
};

template <is_reversable_iterator T>
reverse_iterator(T&&) -> reverse_iterator<T>;

template <iterable T>
constexpr auto reverse(T&& container)
{
    struct reversed {
        T container;
        constexpr auto begin() { return reverse_iterator{container.rbegin()}; }
        constexpr auto end() { return reverse_iterator{container.rend()}; }
    };
    return reversed{container};
}


// template <iterable T>
// constexpr auto iterate(T&& container)
// {
//     struct iterator {
//         using container_begin_t = typename range_traits<T>::begin_t;
//         using container_end_t = typename range_traits<T>::end_t;
//         container_begin_t iter;
//         container_end_t end;

//         void operator ++() { if(iter != end) iter++; }
//         bool operator !=(iterator const& other) const { return iter != other.iter; }
//         auto operator *() const { return utl::tie(*this, *iter); }
//         void next() { operator++(); }
//     }; 
//     struct iterator_facade {
//         T container;
//         auto begin() { return iterator{utl::ranges::begin(container),utl::ranges::end(container)}; }
//         auto end() { return iterator{utl::ranges::end(container),utl::ranges::end(container)}; }
//     };
//     return iterator_facade{std::forward<T>(container)};
// }

} //namespace utl::ranges
