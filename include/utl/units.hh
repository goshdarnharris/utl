#ifndef UTL_UNITS_HH_
#define UTL_UNITS_HH_

#include <stdlib.h>

namespace utl::detail::unit {

//NOLINTNEXTLINE(misc-no-recursion)
constexpr int32_t ipow(int32_t base, int32_t exp, int32_t result = 1) {
    return exp < 1 ? result : ipow(base*base, exp/2, (exp % 2) ? result*base : result);
}
//TODO: make the underlying type configurable to allow for fixed point.
//be careful with making operations that generate units of a different type.
//don't want to lose sight that this is all fixed point.
//TODO: consider factoring the idea of "precision" out from the unit types.

inline constexpr size_t DECIMAL_ORDER = 10;
inline constexpr int ORDER_PICO = -12;
inline constexpr int ORDER_NANO = -9;
inline constexpr int ORDER_MICRO = -6;
inline constexpr int ORDER_MILLI = -3;
inline constexpr int ORDER_BASE = 0;
inline constexpr int ORDER_KILO = 3;
inline constexpr int ORDER_MEGA = 6;
inline constexpr int ORDER_GIGA = 9;
inline constexpr int ORDER_TERA = 12;

template <
    typename BaseUnit,
    int32_t Magnitude, 
    typename OverflowPolicy,
    typename TruncatePolicy
>
struct metric {
    using overflow_policy_t = OverflowPolicy;
    using truncate_policy_t = TruncatePolicy;
    using base_unit_t = BaseUnit;

    static constexpr uint32_t magnitude = Magnitude;
    uint32_t value;

    constexpr explicit metric(uint32_t v) : value{v} {}

    template <int32_t Magnitude_Other, typename OverflowPolicy_Other, typename TruncatePolicy_Other>
    constexpr metric(metric<base_unit_t, Magnitude_Other, OverflowPolicy_Other, TruncatePolicy_Other> const& other) 
        : value{other.value * ipow(DECIMAL_ORDER, Magnitude_Other - Magnitude)}
    {
        overflow_policy_t::check(value, Magnitude_Other, Magnitude);
        truncate_policy_t::check(value, Magnitude_Other, Magnitude);
    }

    template <int32_t Magnitude_Other, typename OverflowPolicy_Other, typename TruncatePolicy_Other>
    constexpr metric(metric<base_unit_t, Magnitude_Other, OverflowPolicy_Other, TruncatePolicy_Other> const&& other) 
        : value{other.value * ipow(DECIMAL_ORDER, Magnitude_Other - Magnitude)}
    {
        overflow_policy_t::check(other.value, Magnitude_Other, Magnitude);
        truncate_policy_t::check(other.value, Magnitude_Other, Magnitude);
    }

    template <int32_t Magnitude_Other, typename OverflowPolicy_Other, typename TruncatePolicy_Other>
    metric& operator=(metric<base_unit_t, Magnitude_Other, OverflowPolicy_Other, TruncatePolicy_Other> const& other)
    {
        value = other.value * ipow(DECIMAL_ORDER, Magnitude_Other - Magnitude);
        overflow_policy_t::check(other.value, Magnitude_Other, Magnitude);
        truncate_policy_t::check(other.value, Magnitude_Other, Magnitude);
    }

    template <int32_t Magnitude_Other, typename OverflowPolicy_Other, typename TruncatePolicy_Other>
    metric& operator=(metric<base_unit_t, Magnitude_Other, OverflowPolicy_Other, TruncatePolicy_Other>&& other)
    {
        value = other.value * ipow(DECIMAL_ORDER, Magnitude_Other - Magnitude);
        overflow_policy_t::check(other.value, Magnitude_Other, Magnitude);
        truncate_policy_t::check(other.value, Magnitude_Other, Magnitude);
    }

    metric& operator-=(metric const& other) {
        value -= other.value;
        return *this;
    }

    metric operator-(metric const& other) {
        return metric{this->value - other.value};
    }

    metric& operator+=(metric const& other) {
        value += other.value;
        return *this;
    }

    metric operator+(metric const& other) {
        return metric{this->value + other.value};
    }

    template <typename U>
    constexpr bool operator==(U const& rhs) const { return value == rhs.template to<metric>().value; }

    template <typename U>
    constexpr bool operator!=(U const& rhs) const { return value != rhs.template to<metric>().value; }

    template <typename U>
    constexpr bool operator<(U const& rhs) const { return value < rhs.template to<metric>().value; }

    template <typename U>
    constexpr bool operator<=(U const& rhs) const { return value <= rhs.template to<metric>().value; }

    template <typename U>
    constexpr bool operator>(U const& rhs) const { return value > rhs.template to<metric>().value; }

    template <typename U>
    constexpr bool operator>=(U const& rhs) const { return value >= rhs.template to<metric>().value; }

    template <typename T>
    T to() const {
        return {*this};
    }
};

}

namespace utl::unit {

struct permissive_overflow_policy {
    template <typename Value, typename Magnitude>
    static constexpr bool check(Value v, Magnitude from, Magnitude to) {
        maybe_unused(v,from,to);
        return true;
    }
};

struct restrictive_overflow_policy {
    template <typename Value, typename Magnitude>
    static constexpr bool check(Value v, Magnitude from, Magnitude to) {
        maybe_unused(v,from,to);
        //FIXME: actually check whether or not we'll overflow.
        return true;
    }
};

struct permissive_truncate_policy {
    template <typename Value, typename Magnitude>
    static constexpr bool check(Value v, Magnitude from, Magnitude to) {
        maybe_unused(v,from,to);
        return true;
    }
};

struct restrictive_truncate_policy {
    template <typename Value, typename Magnitude>
    static constexpr bool check(Value v, Magnitude from, Magnitude to) {
        maybe_unused(v,from,to);
        //FIXME: actually check whether or not we'll overflow.
        return true;
    }
};

using default_overflow_policy_t = restrictive_overflow_policy;
using default_truncate_policy_t = restrictive_truncate_policy;

namespace duration {

struct seconds_t {};

//default policies for now.
using picoseconds = detail::unit::metric<seconds_t, detail::unit::ORDER_PICO, 
    default_overflow_policy_t, default_truncate_policy_t>;
using nanoseconds = detail::unit::metric<seconds_t, detail::unit::ORDER_NANO, 
    default_overflow_policy_t, default_truncate_policy_t>;
using microseconds = detail::unit::metric<seconds_t, detail::unit::ORDER_MICRO, 
    default_overflow_policy_t, default_truncate_policy_t>;
using milliseconds = detail::unit::metric<seconds_t, detail::unit::ORDER_MILLI, 
    default_overflow_policy_t, default_truncate_policy_t>;
using seconds = detail::unit::metric<seconds_t, detail::unit::ORDER_BASE, 
    default_overflow_policy_t, default_truncate_policy_t>;

using ps = picoseconds;
using ns = nanoseconds;
using us = microseconds;
using ms = milliseconds;
using s = seconds;

}

namespace frequency {

struct hertz_t {};

using millihertz = detail::unit::metric<hertz_t, detail::unit::ORDER_MILLI, 
    default_overflow_policy_t, default_truncate_policy_t>;
using hertz = detail::unit::metric<hertz_t, detail::unit::ORDER_BASE, 
    default_overflow_policy_t, default_truncate_policy_t>;
using kilohertz = detail::unit::metric<hertz_t, detail::unit::ORDER_KILO, 
    default_overflow_policy_t, default_truncate_policy_t>;
using megahertz = detail::unit::metric<hertz_t, detail::unit::ORDER_MEGA, 
    default_overflow_policy_t, default_truncate_policy_t>;
using gigahertz = detail::unit::metric<hertz_t, detail::unit::ORDER_GIGA, 
    default_overflow_policy_t, default_truncate_policy_t>;

using mHz = millihertz;
using Hz = hertz;
using KHz = kilohertz;
using MHz = megahertz;
using GHz = gigahertz;

}
namespace freq = frequency;

} //namespace utl::unit


namespace utl::literals {

constexpr unit::duration::picoseconds operator ""_ps(unsigned long long value) { return unit::duration::picoseconds{static_cast<uint32_t>(value)}; }
constexpr unit::duration::nanoseconds operator ""_ns(unsigned long long value) { return unit::duration::nanoseconds{static_cast<uint32_t>(value)}; }
constexpr unit::duration::microseconds operator ""_us(unsigned long long value) { return unit::duration::microseconds{static_cast<uint32_t>(value)}; }
constexpr unit::duration::milliseconds operator ""_ms(unsigned long long value) { return unit::duration::milliseconds{static_cast<uint32_t>(value)}; }
constexpr unit::duration::seconds operator ""_s(unsigned long long value) { return unit::duration::seconds{static_cast<uint32_t>(value)}; }

constexpr unit::frequency::millihertz operator ""_mHz(unsigned long long value) { return unit::frequency::millihertz{static_cast<uint32_t>(value)}; }
constexpr unit::frequency::hertz operator ""_Hz(unsigned long long value) { return unit::frequency::hertz{static_cast<uint32_t>(value)}; }
constexpr unit::frequency::kilohertz operator ""_KHz(unsigned long long value) { return unit::frequency::kilohertz{static_cast<uint32_t>(value)}; }
constexpr unit::frequency::megahertz operator ""_MHz(unsigned long long value) { return unit::frequency::megahertz{static_cast<uint32_t>(value)}; }
constexpr unit::frequency::gigahertz operator ""_GHz(unsigned long long value) { return unit::frequency::gigahertz{static_cast<uint32_t>(value)}; }

} //namespace literals

#endif //UTL_UNITS_HH_
