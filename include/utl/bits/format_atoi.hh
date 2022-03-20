#pragma once

#include <utl/string-view.hh>

namespace utl::fmt {

constexpr bool is_separator(char ch)
{
    return ch == '\'';
}

constexpr bool is_digit(char ch)
{
    return (ch >= '0') and (ch <= '9');
}

constexpr unsigned int digit_to_int(char c)
{
    return static_cast<unsigned int>(c - '0');
}

// internal ASCII string to unsigned integer conversion
template <typename T>
constexpr auto ascii_to_integer(utl::string_view ascii)
{
    T accumulator = 0u;
    for(char c : ascii) {
        if(is_separator(c)) continue;
        if(not is_digit(c)) break;
        accumulator *= 10; //NOLINT(cppcoreguidelines-avoid-magic-numbers)
        accumulator += digit_to_int(c);
    }

    return accumulator;
}


constexpr auto ascii_to_uint(utl::string_view ascii)
{
    return ascii_to_integer<unsigned int>(ascii);
}

constexpr auto ascii_to_ulong(utl::string_view ascii)
{
    return ascii_to_integer<unsigned long int>(ascii);
}

constexpr auto ascii_to_ulonglong(utl::string_view ascii)
{
    return ascii_to_integer<unsigned long long int>(ascii);
}

} //namespace utl::fmt
