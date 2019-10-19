
#ifndef UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_
#define UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_

#include "interface/driver/pin/base.hh"

namespace utl::driver::pin::interface {

class output : public virtual base {
public:
    virtual ~output(void) = default;
    virtual void set_direction(direction value) = 0;
    virtual void set_state(bool active) = 0;
};

} // namespace utl::driver::pin::interface
#endif // UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_
