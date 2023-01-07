// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef UTL_LOGGER_HH_
#define UTL_LOGGER_HH_

#include <utl/string-view.hh>
#include <utl/type-list.hh>
#include <utl/construct.hh>
#include <utl/error.hh>
#include <utl/format.hh>
#include <stdio.h>
#include <string.h>
#include <utility>
#include "utl-platform.hh"

namespace utl {

namespace logger {

namespace detail {

struct output_base {
    virtual ~output_base() = default;
    virtual result<void> write(utl::string_view const& s) const;
};

detail::output_base const * get_global_output();

} //namespace detail

struct push_output {
    push_output(detail::output_base const* output);
    // template <typename T, std::enable_if_t<utl::is_result_v<std::remove_reference_t<T>>, int*> = nullptr>
    // push_output(T&& maybe_output) 
    //     : push_output{utl::unwrap_pointer(maybe_output)}
    // {}
    push_output(push_output const&) = delete;
    push_output& operator=(push_output const&) = delete;
    ~push_output();
private:
    detail::output_base const * m_previous_output;
};

//FIXME: it'd be great if this could take a lambda, and rvalues.
//FIXME: implement some unit tests for this.
template <typename T>
struct output : detail::output_base {
    T const * const writer;

    output(T const& w) : writer{&w} {}

    result<void> write(utl::string_view const& s) const final {
        if(writer != nullptr) return writer->write(s);
        return errc::unknown;
    }
};

} //namespace logger

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
#pragma clang diagnostic ignored "-Wformat-nonliteral"


//TODO: automatically convert error_codes to their strings.
template <typename... Args>
void log(utl::string_view format, Args&&... args) {
    if(format.size() == 0 or format.size() == npos) return;
    static_assert(utl::platform::config::use_float || 
        (!contains_v<type_list<Args...>,float> && !contains_v<type_list<Args...>,double>),
        "floating point printing is disabled!");

    constexpr size_t buffer_size = 512;
    auto buffer = utl::format<buffer_size>(format,std::forward<Args>(args)...);
    //FIXME: switch to using utl::format
    // constexpr size_t size = 512; 
    // char buffer[size] = {0}; //NOLINT(cppcoreguidelines-avoid-c-arrays)
    // uint32_t length{};

    // if constexpr(utl::platform::config::use_float) {
    //     //Auto convert things to double?
    //     length = static_cast<uint32_t>(snprintf(buffer, size, format.data(), std::forward<Args>(args)...));
    // } else {
    //     length = static_cast<uint32_t>(sniprintf(buffer, size, format.data(), std::forward<Args>(args)...));
    // }

    //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    auto res = logger::detail::get_global_output()->write(buffer);
    ignore_result(res);
    ignore_result(logger::detail::get_global_output()->write("\r\n"_sv));
}

void log(utl::string_view const& str);

// template <typename... Args>
// void log(string_view format, Args&&... args) {
//     log(format, std::forward<Args>(args)...);
// }

#pragma clang diagnostic pop

} //namespace utl

#endif //UTL_LOGGER_HH_
