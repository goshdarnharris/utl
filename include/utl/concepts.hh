#pragma once

#include <traits.hh>

namespace utl {

// namespace detail {
//     template< class T, class U >
//     concept SameHelper = std::is_same_v<T, U>;
// }
 
// template< class T, class U >
// concept same_as = detail::SameHelper<T, U> && detail::SameHelper<U, T>;

template< class T, class U >
concept same_as = is_same_v<T,U>;




} //namespace utl
