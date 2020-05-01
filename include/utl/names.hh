#pragma once
#include <utl/string-view.hh>
#include <utl/logger.hh>

namespace utl {

namespace detail {

namespace types {

struct probe_type {};
static constexpr string_view probe_name = "utl::detail::types::probe_type"_sv;

template <typename T>
constexpr string_view get_function_name()
{
    constexpr string_view name{__PRETTY_FUNCTION__};
    return name;
}

static constexpr auto probed_name = get_function_name<probe_type>();
static constexpr auto param_name_start_pos = probed_name.find(probe_name);
static constexpr auto param_name_end_rpos = probed_name.size() - (param_name_start_pos + probe_name.size());

template <typename T>
constexpr string_view get_template_param_name()
{
    constexpr auto full_name = get_function_name<T>();
    constexpr auto param_name_size = full_name.size() - param_name_end_rpos - param_name_start_pos;
    return full_name.substr(param_name_start_pos, param_name_size);
}

} //namespace types
} //namespace detail

template <typename T>
static constexpr string_view type_name = detail::types::get_template_param_name<T>();

template <typename T>
constexpr string_view get_type_name(const T& v)
{
    utl::maybe_unused(v);
    return type_name<T>;
}

template <typename T>
constexpr string_view get_type_name(const T&& v)
{
    utl::maybe_unused(v);
    return type_name<T>;
}

namespace detail {

namespace enums {

template <auto V> 
struct enum_v {
    using enum_t = decltype(V);
    static constexpr enum_t value = V;
};

enum class probe_enum {
    HELLO
};

static constexpr auto probed_name = type_name<enum_v<probe_enum::HELLO>>;
static constexpr auto value_name_start_pos = probed_name.find("enum_v<") + "enum_v<"_sv.size();
static constexpr auto value_name_end_rpos = probed_name.size() - (probed_name.find("HELLO") + "HELLO"_sv.size());

template <auto V>
constexpr string_view get_enum_name()
{
    constexpr auto enum_v_name = type_name<enum_v<V>>;
    constexpr auto name_size = enum_v_name.size() - value_name_start_pos - value_name_end_rpos;
    return enum_v_name.substr(value_name_start_pos, name_size);
}

constexpr bool is_numeric_char(char c)
{
    return c >= '0' and c <= '9';
}

template <typename E, size_t Cursor = 0>
constexpr size_t get_enum_count()
{
    constexpr auto name = get_enum_name<static_cast<E>(Cursor)>();
    if constexpr (is_numeric_char(name.data()[0])) {
        return Cursor;
    } else {
        return get_enum_count<E,Cursor+1>();
    }
}

} //namespace enums
} //namespace detail

template <typename E>
static constexpr size_t enum_count = detail::enums::get_enum_count<E>();

template <auto E>

static constexpr string_view enum_name = detail::enums::get_enum_name<E>();

template <typename E, size_t Cursor = 0>
    requires is_enum_v<remove_reference_t<E>>
constexpr string_view get_enum_name(E&& v)
{
    using enum_t = std::remove_reference_t<E>;
    if constexpr (Cursor == enum_count<enum_t>) {
        return "";
    } else {
        if(static_cast<size_t>(v) == Cursor) {
            return enum_name<static_cast<enum_t>(Cursor)>;
        } else {
            return get_enum_name<E,Cursor+1>(std::forward<E>(v));
        }
    }
}



} //namespace utl
