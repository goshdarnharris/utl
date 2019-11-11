
#ifndef UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_
#define UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_

#include "utl/interface/hal/pin/base.hh"

namespace utl::hal::pin::interface {

class output : public virtual base {
public:
    virtual ~output(void) = default;
    virtual void set_direction(direction value) = 0;
    virtual void set_state(bool active) = 0;
};

} // utl::hal::pin::interface
#endif // UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_
