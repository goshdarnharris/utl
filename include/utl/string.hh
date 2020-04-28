#ifndef UTL_STRING_HH_
#define UTL_STRING_HH_

#include <string.h>

namespace utl {

template <size_t N, typename char_t = char>
class string {
    struct literal_string_tag {};
    char_t m_elements[N+1];
public:
    constexpr string() : m_elements{'\0'} {}
    
    constexpr string(const char_t (&str)[N+1]) : string(literal_string_tag{},str,make_index_sequence<N+1>{}) 
    {}

    template <size_t... Indexes>
    constexpr string(literal_string_tag, const char_t (&str)[N+1], index_sequence<Indexes...>) 
      : m_elements{str[Indexes]...} {}

    template <typename... Args>
    string(const char_t (&fmt)[N+1], Args&&... args) {
        snprintf(m_elements, N+1, fmt, std::forward<Args>(args)...);
        m_elements[N+1] = '\0';
    }

    constexpr size_t size() { return N; }
};

template <size_t N, typename char_t, const char_t(&String)[N]>
static constexpr size_t get_length_v = N-1;

template <size_t N, typename char_t>
string(const char_t (&)[N]) -> string<N-1,char_t>;

//This doesn't work. I don't know how to make it recognize that the string is a constexpr.
//Well, apparently you can't use strings as template arguments anyway.
// template <size_t N, typename char_t>
// string(const char_t (&String)[N]) -> string<get_length_v<N,char_t,String>,char_t>;

//Now... compile time string format length calculation!
//this sounds like a mess. but, maybe try with a simple case
//like %c first.

} //namespace utl

#endif //UTL_STRING_HH_
