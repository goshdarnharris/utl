
#ifndef UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_
#define UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_

#include "interface/driver/pin/output.hh"

namespace utl::driver::pin::interface {

class open_drain : public virtual output {
public:
    virtual ~open_drain(void) = default;
};

} // namespace utl::driver::pin::interface
#endif // UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_
