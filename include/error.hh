#ifndef UTL_ERROR_HH_
#define UTL_ERROR_HH_

#include <utility>
#include <stdint.h>

namespace utl {

struct error_code;

template <typename T>
constexpr error_code make_error_code(T code);

template <typename T>
struct is_error_code_enum : std::false_type {};


class error_category {
public:
    //a couple of virtual methods to do things with
    //the error code
};

static constexpr error_category system_error_category{};

//the standard library uses:
// - is_error_code, specialized by the developer for their error code to return true
// - make_error_code, specialized to make an error_code object given a typed code
//   (which makes its association with a domain automagic)
// so, that's nice. but maybe a lot of typing?

struct error_code {
    int32_t value;
    const error_category* category;

    //This needs to use make_error_code on the templated
    //enum type.
    constexpr error_code(int32_t val, const error_category* cat) 
     : value{val}, category{cat} {}

    template <typename T, typename std::enable_if<is_error_code_enum<T>::value, int*>::type = nullptr>
    constexpr error_code(T code) : error_code{make_error_code(code)} {}
};

enum class system_error {
    OK,
    UNKNOWN
};

template <>
struct is_error_code_enum<system_error> : std::true_type {};

template <>
constexpr error_code make_error_code<system_error>(system_error code) {
    return {static_cast<int32_t>(code), &system_error_category};
}


} //namespace utl

#endif //UTL_ERROR_HH_
