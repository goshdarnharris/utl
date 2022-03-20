#pragma once

#include <utl/bits/format_atoi.hh>

namespace utl::fmt {

static constexpr size_t MAX_SPEC_SIZE = 16;
static constexpr size_t MAX_FIELD_SIZE = 32;
static constexpr size_t MAX_FORMATTED_INT_SIZE = 64;

class arg_spec {
public:
    enum class modes : uint8_t {
        UNKNOWN,
        MANUAL,
        AUTOMATIC
    };
private:
    modes m_mode{modes::AUTOMATIC};
    size_t m_id{};
public:
    constexpr arg_spec(utl::string_view view)
        : m_mode{view.length() > 0 and is_digit(view[0]) ? modes::MANUAL : modes::AUTOMATIC},
        m_id{m_mode == modes::MANUAL ? ascii_to_uint(view) : 0}
    {}
    constexpr arg_spec(size_t id_) : m_mode{modes::MANUAL}, m_id{id_} {}
    constexpr arg_spec() = default;
    [[nodiscard]] constexpr modes mode() const { return m_mode; }
    [[nodiscard]] constexpr auto id() const { return m_id; }
};

struct field {
    arg_spec id{};
    utl::string<MAX_SPEC_SIZE> spec{};
};

} //namespace utl::fmt
