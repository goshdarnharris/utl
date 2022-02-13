#pragma once

#include <utl/string-view.hh>

namespace utl::fmt {

constexpr bool is_digit(char ch)
{
    return (ch >= '0') and (ch <= '9');
}

constexpr unsigned int digit_to_int(char c)
{
    return static_cast<unsigned int>(c - '0');
}

// internal ASCII string to unsigned int conversion
constexpr unsigned int ascii_to_int(utl::string_view ascii)
{
    unsigned int value = 0u;
    for(char c : ascii) {
        if(not is_digit(c)) break;
        value *= 10; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
        value += digit_to_int(c);
    }

    return value;
}

} //namespace utl::fmt
