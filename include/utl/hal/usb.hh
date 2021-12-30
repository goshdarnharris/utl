// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0


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

namespace scancode {

struct code {
    uint8_t data = 0;
    bool modifier:1 = false;

    constexpr bool operator==(code const& other) {
        return modifier == other.modifier and data == other.data;
    }
};

static constexpr code lctrl{0x01, true};
static constexpr code lshift{0x02, true};
static constexpr code lalt{0x04, true};
static constexpr code lmeta{0x08, true};
static constexpr code rctrl{0x10, true};
static constexpr code rshift{0x20, true};
static constexpr code ralt{0x40, true};
static constexpr code rmeta{0x80, true};

static constexpr code none{0x00}; // No key pressed
static constexpr code err_ovf{0x01}; //  Keyboard Error Roll Over - used for all slots if too many keys are pressed ("Phantom key")
static constexpr code a{0x04}; // Keyboard a and A
static constexpr code b{0x05}; // Keyboard b and B
static constexpr code c{0x06}; // Keyboard c and C
static constexpr code d{0x07}; // Keyboard d and D
static constexpr code e{0x08}; // Keyboard e and E
static constexpr code f{0x09}; // Keyboard f and F
static constexpr code g{0x0a}; // Keyboard g and G
static constexpr code h{0x0b}; // Keyboard h and H
static constexpr code i{0x0c}; // Keyboard i and I
static constexpr code j{0x0d}; // Keyboard j and J
static constexpr code k{0x0e}; // Keyboard k and K
static constexpr code l{0x0f}; // Keyboard l and L
static constexpr code m{0x10}; // Keyboard m and M
static constexpr code n{0x11}; // Keyboard n and N
static constexpr code o{0x12}; // Keyboard o and O
static constexpr code p{0x13}; // Keyboard p and P
static constexpr code q{0x14}; // Keyboard q and Q
static constexpr code r{0x15}; // Keyboard r and R
static constexpr code s{0x16}; // Keyboard s and S
static constexpr code t{0x17}; // Keyboard t and T
static constexpr code u{0x18}; // Keyboard u and U
static constexpr code v{0x19}; // Keyboard v and V
static constexpr code w{0x1a}; // Keyboard w and W
static constexpr code x{0x1b}; // Keyboard x and X
static constexpr code y{0x1c}; // Keyboard y and Y
static constexpr code z{0x1d}; // Keyboard z and Z
static constexpr code num1{0x1e}; // Keyboard 1 and !
static constexpr code num2{0x1f}; // Keyboard 2 and @
static constexpr code num3{0x20}; // Keyboard 3 and #
static constexpr code num4{0x21}; // Keyboard 4 and $
static constexpr code num5{0x22}; // Keyboard 5 and %
static constexpr code num6{0x23}; // Keyboard 6 and ^
static constexpr code num7{0x24}; // Keyboard 7 and &
static constexpr code num8{0x25}; // Keyboard 8 and *
static constexpr code num9{0x26}; // Keyboard 9 and (
static constexpr code num0{0x27}; // Keyboard 0 and )
static constexpr code enter{0x28}; // Keyboard Return (ENTER)
static constexpr code esc{0x29}; // Keyboard ESCAPE
static constexpr code backspace{0x2a}; // Keyboard DELETE (Backspace)
static constexpr code tab{0x2b}; // Keyboard Tab
static constexpr code space{0x2c}; // Keyboard Spacebar
static constexpr code minus{0x2d}; // Keyboard - and _
static constexpr code equal{0x2e}; // Keyboard = and +
static constexpr code lbrace{0x2f}; // keyboard [ and {
static constexpr code rbrace{0x30}; // Keyboard ] and };
static constexpr code lslash{0x31}; // Keyboard \ and |
static constexpr code hashtilde{0x32}; // Keyboard Non-US # and ~
static constexpr code semicolon{0x33}; // Keyboard ; and :
static constexpr code apostrophe{0x34}; // Keyboard ' and "
static constexpr code grave{0x35}; // Keyboard ` and ~
static constexpr code comma{0x36}; // Keyboard , and <
static constexpr code dot{0x37}; // Keyboard . and >
static constexpr code slash{0x38}; // Keyboard / and ?
static constexpr code capslock{0x39}; // Keyboard Caps Lock
static constexpr code f1{0x3a}; // Keyboard F1
static constexpr code f2{0x3b}; // Keyboard F2
static constexpr code f3{0x3c}; // Keyboard F3
static constexpr code f4{0x3d}; // Keyboard F4
static constexpr code f5{0x3e}; // Keyboard F5
static constexpr code f6{0x3f}; // Keyboard F6
static constexpr code f7{0x40}; // Keyboard F7
static constexpr code f8{0x41}; // Keyboard F8
static constexpr code f9{0x42}; // Keyboard F9
static constexpr code f10{0x43}; // Keyboard F10
static constexpr code f11{0x44}; // Keyboard F11
static constexpr code f12{0x45}; // Keyboard F12
static constexpr code sysrq{0x46}; // Keyboard Print Screen
static constexpr code scrolllock{0x47}; // Keyboard Scroll Lock
static constexpr code pause{0x48}; // Keyboard Pause
static constexpr code insert{0x49}; // Keyboard Insert
static constexpr code home{0x4a}; // Keyboard Home
static constexpr code pageup{0x4b}; // Keyboard Page Up
static constexpr code del{0x4c}; // Keyboard Delete Forward
static constexpr code end{0x4d}; // Keyboard End
static constexpr code pagedown{0x4e}; // Keyboard Page Down
static constexpr code right{0x4f}; // Keyboard Right Arrow
static constexpr code left{0x50}; // Keyboard Left Arrow
static constexpr code down{0x51}; // Keyboard Down Arrow
static constexpr code up{0x52}; // Keyboard Up Arrow
static constexpr code numlock{0x53}; // Keyboard Num Lock and Clear
static constexpr code kpslash{0x54}; // Keypad /
static constexpr code kpasterisk{0x55}; // Keypad *
static constexpr code kpminus{0x56}; // Keypad -
static constexpr code kpplus{0x57}; // Keypad +
static constexpr code kpenter{0x58}; // Keypad ENTER
static constexpr code kp1{0x59}; // Keypad 1 and End
static constexpr code kp2{0x5a}; // Keypad 2 and Down Arrow
static constexpr code kp3{0x5b}; // Keypad 3 and PageDn
static constexpr code kp4{0x5c}; // Keypad 4 and Left Arrow
static constexpr code kp5{0x5d}; // Keypad 5
static constexpr code kp6{0x5e}; // Keypad 6 and Right Arrow
static constexpr code kp7{0x5f}; // Keypad 7 and Home
static constexpr code kp8{0x60}; // Keypad 8 and Up Arrow
static constexpr code kp9{0x61}; // Keypad 9 and Page Up
static constexpr code kp0{0x62}; // Keypad 0 and Insert
static constexpr code kpdot{0x63}; // Keypad . and Delete
static constexpr code iso_bslash{0x64}; // Keyboard Non-US \ and |
static constexpr code compose{0x65}; // Keyboard Application
static constexpr code power{0x66}; // Keyboard Power
static constexpr code kpequal{0x67}; // Keypad =
static constexpr code f13{0x68}; // Keyboard F13
static constexpr code f14{0x69}; // Keyboard F14
static constexpr code f15{0x6a}; // Keyboard F15
static constexpr code f16{0x6b}; // Keyboard F16
static constexpr code f17{0x6c}; // Keyboard F17
static constexpr code f18{0x6d}; // Keyboard F18
static constexpr code f19{0x6e}; // Keyboard F19
static constexpr code f20{0x6f}; // Keyboard F20
static constexpr code f21{0x70}; // Keyboard F21
static constexpr code f22{0x71}; // Keyboard F22
static constexpr code f23{0x72}; // Keyboard F23
static constexpr code f24{0x73}; // Keyboard F24
static constexpr code open{0x74}; // Keyboard Execute
static constexpr code help{0x75}; // Keyboard Help
static constexpr code props{0x76}; // Keyboard Menu
static constexpr code front{0x77}; // Keyboard Select
static constexpr code stop{0x78}; // Keyboard Stop
static constexpr code again{0x79}; // Keyboard Again
static constexpr code undo{0x7a}; // Keyboard Undo
static constexpr code cut{0x7b}; // Keyboard Cut
static constexpr code copy{0x7c}; // Keyboard Copy
static constexpr code paste{0x7d}; // Keyboard Paste
static constexpr code find{0x7e}; // Keyboard Find
static constexpr code mute{0x7f}; // Keyboard Mute
static constexpr code volumeup{0x80}; // Keyboard Volume Up
static constexpr code volumedown{0x81}; // Keyboard Volume Down
static constexpr code kpcomma{0x85}; // Keypad Comma
static constexpr code kpequal2{0x86}; // Keypad Equal Sign
static constexpr code ro{0x87}; // Keyboard International1
static constexpr code katakanahiragana{0x88}; // Keyboard International2
static constexpr code yen{0x89}; // Keyboard International3
static constexpr code henkan{0x8a}; // Keyboard International4
static constexpr code muhenkan{0x8b}; // Keyboard International5
static constexpr code kpjpcomma{0x8c}; // Keyboard International6
static constexpr code hangeul{0x90}; // Keyboard LANG1
static constexpr code hanja{0x91}; // Keyboard LANG2
static constexpr code katakana{0x92}; // Keyboard LANG3
static constexpr code hiragana{0x93}; // Keyboard LANG4
static constexpr code zenkakuhankaku{0x94}; // Keyboard LANG5
static constexpr code kpleftparen{0xb6}; // Keypad (
static constexpr code kprightparen{0xb7}; // Keypad )
static constexpr code media_playpause{0xe8};
static constexpr code media_stopcd{0xe9};
static constexpr code media_previoussong{0xea};
static constexpr code media_nextsong{0xeb};
static constexpr code media_ejectcd{0xec};
static constexpr code media_volumeup{0xed};
static constexpr code media_volumedown{0xee};
static constexpr code media_mute{0xef};
static constexpr code media_www{0xf0};
static constexpr code media_back{0xf1};
static constexpr code media_forward{0xf2};
static constexpr code media_stop{0xf3};
static constexpr code media_find{0xf4};
static constexpr code media_scrollup{0xf5};
static constexpr code media_scrolldown{0xf6};
static constexpr code media_edit{0xf7};
static constexpr code media_sleep{0xf8};
static constexpr code media_coffee{0xf9};
static constexpr code media_refresh{0xfa};
static constexpr code media_calc{0xfb};

} //namespace scancode

struct keyboard_report {
    static constexpr uint8_t n_codes = 6;

    union {
        struct {
            union {
                uint8_t data;
                struct {
                    uint8_t LEFT_CTRL:1;
                    uint8_t LEFT_SHIFT:1;
                    uint8_t LEFT_ALT:1;
                    uint8_t LEFT_GUI:1;
                    uint8_t RIGHT_CTRL:1;
                    uint8_t RIGHT_SHIFT:1;
                    uint8_t RIGHT_ALT:1;
                    uint8_t RIGHT_GUI:1;
                };
            } modifiers;
            uint8_t _reserved;
            uint8_t codes[n_codes];
        };
        uint8_t data[n_codes + 2];
    };
    static_assert(sizeof(data) == 8, "keyboard report size isn't correct");
    
    constexpr keyboard_report() :
        modifiers{0x0}, _reserved{0x0},
        codes{0,0,0,0,0,0}
    {}

    // template <typename... Codes>
    // constexpr keyboard_report(Codes... codes) : 
    //     modifiers{0x0}, _reserved{0x0}, 
    //     codes{codes.data...}
    // {}

    bool add(scancode::code code) {
        if(code == scancode::none) return true;
        if(code.modifier) {
            modifiers.data |= code.data;
            return true;
        }
        for(uint8_t i=0; i<n_codes; i++) {
            if(codes[i] == scancode::none.data) {
                codes[i] = code.data;
                return true;
            }
        }
        return false;
    }

    void remove(scancode::code code) {
        if(code.modifier) {
            modifiers.data &= ~code.data;
            return;
        }
        for(uint8_t i=0; i<n_codes; i++) {
            if(codes[i] == code.data) codes[i] = scancode::none.data;
        }
    }
};

} //namespace hid

} //namespace utl::hal::usb::device

#endif //UTL_HAL_USB_HH_
