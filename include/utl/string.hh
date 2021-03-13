#ifndef UTL_STRING_HH_
#define UTL_STRING_HH_

#include <utl/type-list.hh>
#include <string.h>
#include <utl/string-view.hh>
#include <utl/ranges.hh>
#include <utility>

namespace utl {

template <size_t N, typename char_t = char>
class string {
    [[nodiscard]] constexpr char_t* address()
    {
        return static_cast<char_t*>(m_elements);
    }

    [[nodiscard]] constexpr const char_t* address() const
    {
        return static_cast<const char_t*>(m_elements);
    }

    [[nodiscard]] constexpr char_t& access(size_t index)
    {
        //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-bounds-constant-array-index)
        return m_elements[index];
    }

    [[nodiscard]] constexpr char_t const& access(size_t index) const
    {
        //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-bounds-constant-array-index)
        return m_elements[index];
    }

    struct impl_tag{};

    constexpr string(impl_tag, const char_t* str, size_t length = N)
    {
        if(length != 0) __builtin_memcpy(address(), str, length <= N ? length : N);
        access(N) = '\0';
    }    

    [[nodiscard]] constexpr string_view to_sv() const
    {
        return {data(),length()};
    }

    char_t m_elements[N+1]{}; //NOLINT(cppcoreguidelines-avoid-c-arrays)

public:
    constexpr string() = default;
    
    constexpr string(const char_t* str) : string{impl_tag{},str,__builtin_strlen(str)}
    {}

    explicit constexpr string(string_view view) : string{impl_tag{},view.data(),view.length()}
    {}
    
    template <size_t M>
        requires (M >= N)
    constexpr string(string<M>& other) : string{impl_tag{},other.data(),N} 
    {}

    constexpr string(size_t count, char_t ch) : string{}
    {
        if(count > N) count = N;
        for(size_t pos=0; pos < count; pos++) access(pos) = ch;
        access(N) = '\0';
    }

    [[nodiscard]] constexpr const char_t* data() const
    {
        return address();
    }

    [[nodiscard]] constexpr const char_t* c_str() const
    {
        return data();
    }

    [[nodiscard]] constexpr size_t length() const
    { return __builtin_strlen(address()); }

    [[nodiscard]] constexpr size_t size() const
    { return N; }

    constexpr char_t& operator[](size_t idx)
    {
        return access(idx);
    }

    constexpr char_t const& operator[](size_t idx) const
    {
        return access(idx);
    }

    template <size_t M>
    constexpr bool operator==(string<M> const& other) const
    {
        return to_sv() == other.to_sv();
    }

    constexpr bool operator==(string_view const& other) const
    {
        return to_sv() == other;
    }

    // template <size_t M>
    // constexpr auto operator+(string<M> const& other) const
    // {
    //     string<N+M> res
    // }

    constexpr char_t at(size_t idx, char_t dfault)
    {
        if(idx < length()) {
            return access(idx);
        } else {
            return dfault;
        }
    }

    constexpr operator string_view() const
    {
        return string_view{c_str(),length()};
    }

    char_t* begin() {
        return &access(0);
    }
    [[nodiscard]] const char_t* begin() const
    {
        return &access(0);
    }

    char_t* end()
    {
        return &access(N);
    }    
    [[nodiscard]] const char_t* end() const
    {
        return &access(N);
    }

    [[nodiscard]] constexpr string<N> substr(size_t pos, size_t count) const
    {
        if(size() == 0) return {""};
        if(pos >= size()) pos = size() - 1;
        if(count == npos) count = size() - pos;
        if(pos + count >= size()) count = size() - pos; 
        return {impl_tag{}, &access(pos), count}; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    [[nodiscard]] constexpr bool starts_with(string_view v) const
    {
        if(v.size() > size()) { return false; }
        if(size() == 0 && v.size() == 0) { return true; }
        if(size() == 0) { return false; }
        if (std::is_constant_evaluated()) {
            if(access(0) == v[0]) {
                if(v.size() == 1) { return true; }
                return substr(1,npos).starts_with(v.substr(1, npos));
            } else {
                return false;
            }
        } else {
            for(size_t idx = 0; idx < v.size(); idx++) {
                if(access(idx) != v[idx]) return false;
            }
            return true;
        }
    }

    [[nodiscard]] constexpr size_t find(string_view v, size_t pos = 0) const
    {        
        if(size() == 0) { return npos; }
        if(v.size() > size()) { return npos; }
        if(pos >= size()) { return npos; }
        const size_t search_end_pos = size() - v.size();
        if(pos > search_end_pos) { return npos; }

        auto check = [&](auto p) {
            return substr(p,npos).starts_with(v);
        };

        if (std::is_constant_evaluated()) {
            if(check(pos)) { return pos; }
            return find(v, pos + 1);
        } else {
            for(size_t idx = pos; idx <= search_end_pos; idx++) {
                if(check(idx)) { return idx; }
            }
            return npos;
        }
    }

    [[nodiscard]] constexpr size_t rfind(string_view v, size_t pos = npos) const
    {        
        if(size() == 0) { return npos; }
        if(v.size() > size()) { return npos; }
        const size_t last_searchable_pos = size() - v.size();
        if(pos > last_searchable_pos) { pos = last_searchable_pos; }

        auto check = [&](auto p) {
            return substr(p,npos).starts_with(v);
        };

        if (std::is_constant_evaluated()) {
            if(check(pos)) { return pos; }
            if(pos == 0) { return npos; }
            return find(v, pos - 1);
        } else {
            for(size_t idx = pos; idx > 0u; idx--) {
                if(check(pos)) { return pos; }
            }
            if(check(0u)) { return 0u; }
            return npos;
        }
    }

    [[nodiscard]] constexpr int compare(string_view v) const
    {
        for(const auto [cursor,c] : ranges::enumerate(v)) {
            if(cursor >= size()) break;
            const auto this_c = this->operator[](cursor);
            if(this_c < c) return -1;
            if(this_c > c) return 1;
        }
        if(size() < v.size()) return -1;
        if(size() > v.size()) return 1;
        return 0;
    }
};

//NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
template <size_t N, typename char_t, const char_t(&String)[N]>
static constexpr size_t get_length_v = N-1;

template <size_t N, typename char_t>
string(const char_t (&)[N]) -> string<N-1,char_t>; //NOLINT(cppcoreguidelines-avoid-c-arrays)

//This doesn't work. I don't know how to make it recognize that the string is a constexpr.
//Well, apparently you can't use strings as template arguments anyway.
// template <size_t N, typename char_t>
// string(const char_t (&String)[N]) -> string<get_length_v<N,char_t,String>,char_t>;

//Now... compile time string format length calculation!
//this sounds like a mess. but, maybe try with a simple case
//like %c first.

} //namespace utl

#endif //UTL_STRING_HH_
