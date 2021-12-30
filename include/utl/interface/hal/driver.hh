// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef UTL_INTERFACE_DRIVER_HH_
#define UTL_INTERFACE_DRIVER_HH_

namespace utl::interface::hal {

class driver {
public:
    driver(driver const& that) = delete;
    driver& operator=(const driver&) = delete;
    driver(driver&& that) = delete;
    driver& operator=(driver&&) = delete;
    driver() = default;
};

}

#endif