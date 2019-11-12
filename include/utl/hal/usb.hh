
#ifndef UTL_HAL_USB_HH_
#define UTL_HAL_USB_HH_

namespace utl::hal::usb {

enum class state {
    UNKNOWN,
    DEFAULT,
    ADDRESSED,
    CONFIGURED,
    SUSPENDED
};


namespace hid {

struct keycode {
    uint8_t data;
};

struct keyboard_report {
    union {
        struct {
            struct {
                uint8_t LEFT_CTRL:1;
                uint8_t LEFT_SHIFT:1;
                uint8_t LEFT_ALT:1;
                uint8_t LEFT_GUI:1;
                uint8_t RIGHT_CTRL:1;
                uint8_t RIGHT_SHIFT:1;
                uint8_t RIGHT_ALT:1;
                uint8_t RIGHT_GUI:1;
            } modifiers;
            uint8_t _reserved;
            keycode keycodes[6];
        };
        uint8_t data[8];
    };

    keyboard_report() = default;
    constexpr keyboard_report(keycode code) : 
        modifiers{0x0}, _reserved{0x0}, 
        keycodes{code,0x0,0x0,0x0,0x0,0x0}
    {}
};

} //namespace hid

} //namespace utl::hal::usb::device

#endif //UTL_HAL_USB_HH_
