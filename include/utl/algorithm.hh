#ifndef UTL_ALGORITHM_HH_
#define UTL_ALGORITHM_HH_

namespace utl {

template <typename T>
constexpr T const& max(T const& a, T const& b) {
    return a > b ? a : b;
}




} //namespace utl

#endif //UTL_ALGORITHM_HH_
