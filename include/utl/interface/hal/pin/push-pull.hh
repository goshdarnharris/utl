
#ifndef UTL_INTERFACE_DRIVER_PIN_PUSH_PULL_HH_
#define UTL_INTERFACE_DRIVER_PIN_PUSH_PULL_HH_

#include "utl/interface/hal/pin/output.hh"

namespace utl::hal::pin::interface {

class push_pull : public virtual output {
public:
    virtual ~push_pull(void) = default;
};

} // utl::hal::pin::interface
#endif // UTL_INTERFACE_DRIVER_PIN_PUSH_PULL_HH_
