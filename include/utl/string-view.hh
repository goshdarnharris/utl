#pragma once

#include <utl/utl.hh>
#include <utl/type-list.hh>

namespace utl {

struct string_view {
private:
    [[nodiscard]] constexpr char access(size_t index) const
    {
        return str[index]; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    [[nodiscard]] constexpr const char* address(size_t index) const
    {
        return &str[index]; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
public:
    const char* str;
    const size_t len;
    
    constexpr string_view(const char* s) : str{s}, len{s == nullptr ? 0 : __builtin_strlen(s)} {}
    constexpr string_view(const char* s, size_t l) : str{s}, len{s == nullptr ? 0 : l} {}    
    constexpr explicit string_view(std::nullptr_t) : str{nullptr}, len{0} {}
    
    [[nodiscard]] constexpr const char* data() const { return str; }
    [[nodiscard]] constexpr size_t size() const { return len; }    
    [[nodiscard]] constexpr size_t length() const { return size(); }

    [[nodiscard]] constexpr char operator[](size_t idx) const
    {
        return access(idx);
    }

    constexpr char at(size_t idx, char dfault)
    {
        if(idx < length()) {
            return access(idx);
        } else {
            return dfault;
        }
    }

    [[nodiscard]] constexpr const char* begin() const
    {
        return str;
    }

    [[nodiscard]] constexpr const char* end() const
    {
        return address(length());
    }

    [[nodiscard]] constexpr const char* rbegin() const
    {
        return address(length()-1);
    }

    [[nodiscard]] constexpr const char* rend() const
    {
        return address(0) - 1; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    [[nodiscard]] constexpr string_view substr(size_t pos, size_t count) const
    {
        if(length() == 0) return {"",0};
        if(pos >= length()) pos = length() - 1;
        if(count == npos) count = length() - pos;
        if(pos + count >= length()) count = length() - pos; 
        return {address(pos), count}; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    [[nodiscard]] constexpr const char* c_str() const
    {
        return data();
    }

    [[nodiscard]] constexpr bool starts_with(string_view v) const
    {
        if(v.length() > length()) { return false; }
        if(length() == 0 && v.length() == 0) { return true; }
        if(length() == 0) { return false; }
        if (std::is_constant_evaluated()) {
            if(access(0) == v.access(0)) {
                if(v.length() == 1) { return true; }
                return substr(1,npos).starts_with(v.substr(1, npos));
            } else {
                return false;
            }
        } else {
            for(size_t idx = 0; idx < v.length(); idx++) {
                if(access(idx) != v.access(idx)) return false;
            }
            return true;
        }
    }

    [[nodiscard]] constexpr size_t find(string_view v, size_t pos = 0) const
    {        
        if(length() == 0) { return npos; }
        if(v.length() > length()) { return npos; }
        if(pos >= length()) { return npos; }
        const size_t search_end_pos = length() - v.length();
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
        if(length() == 0) { return npos; }
        if(v.length() > length()) { return npos; }
        const size_t last_searchable_pos = length() - v.length();
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

    [[nodiscard]] constexpr bool compare(string_view v) const
    {
        if(v.length() != length()) return false;
        return starts_with(v);
    }
};

namespace literals {
inline constexpr string_view operator ""_sv(const char* str, size_t len) { return{str,len}; }
}

} //namespace utl
