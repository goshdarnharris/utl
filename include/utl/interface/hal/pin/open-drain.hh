
#ifndef UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_
#define UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_

#include "utl/interface/hal/pin/output.hh"

namespace utl::hal::pin::interface {

class open_drain : public virtual output {
public:
    virtual ~open_drain(void) = default;
};

} // utl::hal::pin::interface
#endif // UTL_INTERFACE_DRIVER_PIN_OUTPUT_HH_
