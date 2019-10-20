#ifndef UTL_ERROR_HH_
#define UTL_ERROR_HH_

#include "utl.hh"
#include <utility>
#include <string_view>
#include <stdint.h>

namespace utl {

class error_code;

template <typename T>
constexpr error_code make_error_code(T code);

template <typename T>
struct is_error_code_enum : std::false_type {};

using namespace std::literals;

struct error_category {
    constexpr error_category() {}    
    virtual const utl::string_view message(int32_t value) const;
    virtual const utl::string_view name() const;
    virtual ~error_category() = default;
};

class error_code {
    int32_t value;
    error_category const * category;
public:

    constexpr error_code(int32_t val, const error_category* cat) 
     : value{val}, category{cat} {}

    template <typename T, typename std::enable_if<is_error_code_enum<T>::value, int*>::type = nullptr>
    constexpr error_code(T code) : error_code{make_error_code(code)} {}

    const utl::string_view message() const { return category->message(value); }
    const utl::string_view category_name() const { return category->name(); }
};

} //namespace utl

#endif //UTL_ERROR_HH_
