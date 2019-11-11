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