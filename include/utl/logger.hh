#ifndef UTL_LOGGER_HH_
#define UTL_LOGGER_HH_

#include <utl/string-view.hh>
#include <utl/type-list.hh>
#include <utl/construct.hh>
#include <utl/system-error.hh>
#include <stdio.h>
#include <string.h>
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
    template <typename T, std::enable_if_t<utl::is_result_v<std::remove_reference_t<T>>, int*> = nullptr>
    push_output(T&& maybe_output) 
        : push_output{utl::unwrap_pointer(maybe_output)}
    {}
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
        return system_error::UNKNOWN;
    }
};

} //namespace logger

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
#pragma clang diagnostic ignored "-Wformat-nonliteral"

//TODO: automatically convert error_codes to their strings.
template <typename... Args>
void log(utl::string_view const& format, Args&&... args) {
    if(format.size() == 0 or format.size() == string_view::npos) return;
    static_assert(utl::platform::config::use_float || 
        (!contains_v<type_list<Args...>,float> && !contains_v<type_list<Args...>,double>),
        "floating point printing is disabled!");

    char buffer[512] = {0};
    uint32_t length;

    if constexpr(utl::platform::config::use_float) {
        //Auto convert things to double?
        length = static_cast<uint32_t>(snprintf(buffer, 512, format.data(), std::forward<Args>(args)...));
    } else {
        length = static_cast<uint32_t>(sniprintf(buffer, 512, format.data(), std::forward<Args>(args)...));
    }

    auto res = logger::detail::get_global_output()->write({buffer,length});
    ignore_result(res);
    ignore_result(logger::detail::get_global_output()->write("\r\n"_sv));
}

void log(utl::string_view const& str);

template <typename... Args>
void log(const char * format, Args&&... args) {
    log({format,strlen(format)}, std::forward<Args>(args)...);
}

#pragma clang diagnostic pop

} //namespace utl

#endif //UTL_LOGGER_HH_
