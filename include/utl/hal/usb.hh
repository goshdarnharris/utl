
#ifndef UTL_HAL_USB_HH_
#define UTL_HAL_USB_HH_

namespace utl::hal::usb::device {

enum class state {
    UNKNOWN,
    DEFAULT,
    ADDRESSED,
    CONFIGURED,
    SUSPENDED
};

}

namespace utl::hal {
namespace usbd = usb::device;
}

#endif //UTL_HAL_USB_HH_
