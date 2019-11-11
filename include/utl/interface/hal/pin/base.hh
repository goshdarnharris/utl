
#ifndef UTL_INTERFACE_DRIVER_PIN_BASE_HH_
#define UTL_INTERFACE_DRIVER_PIN_BASE_HH_

#include "utl/interface/hal/driver.hh"

namespace utl::hal::pin {

enum class direction {
    input,
    output
};

enum class active_level {
    high,
    low
};

namespace interface {

class base : public virtual utl::interface::hal::driver {
public:
    virtual ~base(void) = default;
    virtual bool get_state(void) const = 0;
    virtual void set_active_level(active_level level) = 0;
    virtual active_level get_active_level(void) const = 0;
    virtual direction get_direction(void) const = 0;
};

} // namespace interface
} // utl::hal::pin
#endif // UTL_INTERFACE_DRIVER_PIN_BASE_HH_
