
#ifndef UTL_INTERFACE_DRIVER_PIN_PUSH_PULL_HH_
#define UTL_INTERFACE_DRIVER_PIN_PUSH_PULL_HH_

#include "interface/driver/pin/output.hh"

namespace utl::driver::pin::interface {

class push_pull : public virtual output {
public:
    virtual ~push_pull(void) = default;
};

} // namespace utl::driver::pin::interface
#endif // UTL_INTERFACE_DRIVER_PIN_PUSH_PULL_HH_
