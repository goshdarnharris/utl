
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

    constexpr bool operator==(keycode const& other) {
        return data == other.data;
    }
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

    template <typename... Codes>
    constexpr keyboard_report(Codes... codes) : 
        modifiers{0x0}, _reserved{0x0}, 
        keycodes{codes...}
    {}
};

struct keycode_buffer {
    static constexpr size_t max_codes = 6;
    keycode codes[max_codes];

    bool add(keycode code) {
        for(uint8_t i=0; i<max_codes; i++) {
            if(codes[i] == keycode{0x00}) {
                codes[i] = code;
                return true;
            }
        }
        return false;
    }

    void remove(keycode code) {
        for(uint8_t i=0; i<max_codes; i++) {
            if(codes[i] == code) codes[i] = keycode{0x00};
        }
    }

    keyboard_report report() const {
        return keyboard_report{codes[0], codes[1], codes[2],
            codes[3], codes[4], codes[5]};
    }
};

} //namespace hid

} //namespace utl::hal::usb::device

#endif //UTL_HAL_USB_HH_
