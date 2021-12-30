// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0


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
