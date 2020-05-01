#pragma once

#include <utl/type-list.hh>
#include <utl/string.hh>

namespace utl {

struct string_view {
public:
    static constexpr size_t npos = static_cast<size_t>(-1);
    const char* str;
    const size_t length;
    
    constexpr string_view(const char* s) : str{s}, length{s == nullptr ? 0 : __builtin_strlen(s)} {}
    constexpr string_view(const char* s, size_t l) : str{s}, length{s == nullptr ? 0 : l} {}
    
    constexpr explicit string_view(std::nullptr_t) : str{nullptr}, length{0} {}
    
    template <size_t N>
    constexpr string_view(string<N> const& s) : str{s.c_str()}, length{s.size()} {}
    
    constexpr const char* data() const { return str; }
    constexpr size_t size() const { return length; }
    constexpr string_view substr(size_t pos, size_t count) const
    {
        if(size() == 0) return {"",0};
        if(pos >= size()) pos = size() - 1;
        if(count == npos) count = size() - pos;
        if(pos + count >= size()) count = size() - pos; 
        return {&str[pos], count};
    }
    constexpr const char* c_str() const
    {
        return data();
    }
    constexpr bool starts_with(string_view v) const
    {
        if(v.size() > size()) { return false; }
        if(size() == 0 && v.size() == 0) { return true; }
        if(size() == 0) { return false; }
        if (std::is_constant_evaluated()) {
            if(str[0] == v.data()[0]) {
                if(v.size() == 1) { return true; }
                return substr(1,npos).starts_with(v.substr(1, npos));
            } else {
                return false;
            }
        } else {
            for(size_t idx = 0; idx < v.size(); idx++) {
                if(str[idx] != v.data()[idx]) return false;
            }
            return true;
        }
    }
    constexpr size_t find(string_view v, size_t pos = 0) const
    {        
        if(size() == 0) { return npos; }
        if(v.size() > size()) { return npos; }
        if(pos >= size()) { return npos; }
        const size_t search_end_pos = size() - v.size();
        if(pos > search_end_pos) { return npos; }

        auto check = [&](auto p) {
            if(substr(p,npos).starts_with(v)) { return true; }
            return false;
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
    constexpr size_t rfind(string_view v, size_t pos = npos) const
    {        
        if(size() == 0) { return npos; }
        if(v.size() > size()) { return npos; }
        const size_t last_searchable_pos = size() - v.size();
        if(pos > last_searchable_pos) { pos = last_searchable_pos; }

        auto check = [&](auto p) {
            if(substr(p,npos).starts_with(v)) { return true; }
            return false;
        };

        if (std::is_constant_evaluated()) {
            if(check(pos)) { return pos; }
            if(pos == 0) { return npos; }
            return find(v, pos - 1);
        } else {
            for(size_t idx = pos; idx >= 0; idx--) {
                if(check(pos)) { return pos; }
            }
            return npos;
        }
    }
    constexpr bool compare(string_view v) const
    {
        if(v.size() != size()) return false;
        return starts_with(v);
    }
};

namespace literals {
inline constexpr string_view operator ""_sv(const char* str, size_t len) { return{str,len}; }
}

} //namespace utl
