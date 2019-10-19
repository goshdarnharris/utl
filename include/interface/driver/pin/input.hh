
#ifndef UTL_INTERFACE_DRIVER_PIN_INPUT_HH_
#define UTL_INTERFACE_DRIVER_PIN_INPUT_HH_

#include "interface/driver/pin/base.hh"

namespace utl::driver::pin::interface {

class input : public virtual base {
public:
    virtual ~input(void) = default;
};

} // namespace utl::driver::pin::interface
#endif // UTL_INTERFACE_DRIVER_PIN_INPUT_HH_
