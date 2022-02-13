// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include <utl/string-view.hh>
#include <utl/string.hh>
#include <utl/concepts.hh>
#include <utl/result.hh>
#include <utl/tuple.hh>
#include <utl/array.hh>
#include <utl/ranges.hh>
#include <utl/span.hh>
#include <utility>

#include <utl/bits/format_vformat.hh>
#include <utl/bits/format_options.hh>
#include <utl/bits/format_itoa.hh>
#include <utl/bits/format_output.hh>

namespace utl {



//from python:
// replacement_field ::=  "{" [field_name] ["!" conversion] [":" format_spec] "}"
// field_name        ::=  arg_name ("." attribute_name | "[" element_index "]")*
// arg_name          ::=  [identifier | digit+]
// attribute_name    ::=  identifier
// element_index     ::=  digit+ | index_string
// index_string      ::=  <any source character except "]"> +
// conversion        ::=  "r" | "s" | "a"
// format_spec       ::=  <described in the next section>

template <size_t N, fmt::formattable... Args>
constexpr auto format(utl::string_view format, Args&&... args)
{
    array<char,N> buffer{};
    format_into(buffer, format, std::forward<Args>(args)...);
    return utl::string<N>{buffer.data()};
}

template <fmt::formattable... Args>
constexpr auto format_into(utl::ranges::output_iterable<char> auto&& buffer, utl::string_view format, 
    Args&&... args)
{
    auto iter = utl::ranges::begin(buffer);
    auto end = utl::ranges::end(buffer);
    auto buffer_out = [&] (char c) {
        if(iter != end) *iter++ = c;
    };
    format_to(buffer_out, format, std::forward<Args>(args)...);
    return iter;
}

template <callable<void,char> F, fmt::formattable... Args>
constexpr void format_to(F&& out, utl::string_view format, Args&&... args)
{
    const auto arg_storage = fmt::detail::wrap_args(std::forward<Args>(args)...);
    auto arg_view = fmt::detail::erase_args(arg_storage);
    // auto [arg_view, arg_storage] = fmt::detail::make_args(std::forward<Args>(args)...);
    fmt::output_t call{std::forward<F>(out)};
    fmt::vformat(call,format,arg_view);
}

template <fmt::formattable... Args>
constexpr void format_to(fmt::output& out, utl::string_view format, Args&&... args)
{
    //FIXME: add is_formattable trait, if constexpr to throw a static assert asap if
    // a type isn't supported!
    const auto arg_storage = fmt::detail::wrap_args(std::forward<Args>(args)...);
    auto arg_view = fmt::detail::erase_args(arg_storage);
    // auto [arg_view, arg_storage] = fmt::detail::make_args(std::forward<Args>(args)...);
    fmt::vformat(out,format,arg_view);
}


namespace fmt {

    //FIXME: need to account for rvalues & lvalues in these, and decay pointers.

    template <typename T> //specialization for char types
    static constexpr bool is_formattable_as_v<T,const char> = contains_v<T, char, const char, 
        signed char, const signed char>;

    template <typename T> //specialization for unsigned integer types
    static constexpr bool is_formattable_as_v<T,const unsigned long> = contains_v<T, 
        unsigned short, const unsigned short, unsigned int, const unsigned int,
        unsigned long, const unsigned long>;

    template <typename T> //specialization for signed integer types
    static constexpr bool is_formattable_as_v<T,const long long> = contains_v<T, 
        short, const short, int, const int, long, const long, long long,
        const long long>;

    template <typename T> //specialization for cstring types
    static constexpr bool is_formattable_as_v<T,const char*> = contains_v<std::decay_t<T>, 
        char*, const char*, signed char*, const signed char*>;

    template <typename T, typename As> //specialization for pointers
    static constexpr bool is_formattable_as_v<T,As*> = is_same_v<T,As*>;

    template <typename T> //specialization for unsigned char pointers
    static constexpr bool is_formattable_as_v<T,const unsigned char*> = contains_v<std::decay_t<T>, 
        unsigned char*, const unsigned char*>;



    constexpr void format_arg(formattable_as<const long long> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        auto arg_long = static_cast<long>(arg);
        format_ulong(out, static_cast<const unsigned long>(arg_long < 0 ? -arg_long : arg_long), arg < 0, options);
    }

    constexpr void format_arg(formattable_as<const unsigned long> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<const unsigned long>(arg), false, options);
    }

    constexpr void format_arg(formattable_as<float> auto arg, output& out, field const& f)
    {
        utl::maybe_unused(arg, f);
        out("{:f}");
    }

    constexpr void format_arg(formattable_as<double> auto arg, output& out, field const& f)
    {
        utl::maybe_unused(arg, f);
        out("{:f}");
    }

    constexpr void format_arg(formattable_as<long double> auto arg, output& out, field const& f)
    {
        utl::maybe_unused(arg, f);
        out("{:f}");
    }


    //********************** Pointer types

    inline void format_arg(const void* arg, output& out, field const& f)
    {
        auto defaults = default_int_options;
        defaults.presentation = 'p';
        auto options = parse_format_options(f.spec, defaults);
        format_ulong(out, reinterpret_cast<unsigned long>(arg), false, options);
    }

    inline void format_arg(std::nullptr_t, output& out, field const& f)
    {
        format_arg(reinterpret_cast<void*>(0),out,f);
    }


    //********************** String types

    inline constexpr void format_arg(utl::string_view arg, output& out, field const& f)
    {      
        auto options = parse_format_options(f.spec, {
            .fill = ' ',
            .align = alignment::LEFT,
            .presentation = 's'
        });
        fmt::align_pad_out(out, arg, options.fill, options.align, options.width, options.precision);
    }

    template<size_t N>
    constexpr void format_arg(utl::same_as<utl::string<N>> auto arg, output& out, field const& f)
    {
        format_arg(utl::string_view{arg.data(),arg.length()},out,f);
    }

    constexpr void format_arg(formattable_as<const char*> auto arg, output& out, field const& f)
    {
        format_arg(utl::string_view{reinterpret_cast<const char*>(arg)},out,f);
    }

    inline void format_arg(formattable_as<const unsigned char*> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, {.presentation = 'p'});
        if(options.presentation == 's') {
            format_arg(utl::string_view{reinterpret_cast<const char*>(arg)},out,f);
        } else {
            format_arg(reinterpret_cast<const void*>(arg),out,f);
        }        
    }


    //********************** Character types

    constexpr void format_arg(formattable_as<const char> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_char_options);
        switch(options.presentation) {
            case 'c':
                format_arg(utl::string_view{&arg,1},out,f);
                break;
            default:                
                format_ulong(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, options);
                break;
        }        
    }

    constexpr void format_arg(formattable_as<unsigned char> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_unsigned_char_options);
        switch(options.presentation) {
            case 'c':
                format_arg(static_cast<const char>(arg),out,f);
                break;
            default:                
                format_ulong(out, static_cast<unsigned long>(arg), false, options);
                break;
        }
    }

    constexpr void format_arg(formattable_as<bool> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, {.presentation = 's'});
        switch(options.presentation) {
            case 's':
                format_arg(utl::string_view{arg ? "true" : "false"},out,f);
                break;
            default:
                format_arg(static_cast<unsigned int>(arg),out,f);
                break;
        }
    }
} //namespace fmt

} //namespace utl
