#ifndef UTL_COLOR_HH_
#define UTL_COLOR_HH_

namespace utl::color {

struct hsv {
    unsigned char h;
    unsigned char s;
    unsigned char v;
};

struct rgb {
    union {
        struct {
            uint8_t b;
            uint8_t r;
            uint8_t g;
            uint8_t _;
        };
        uint32_t data;
    };
    constexpr rgb(uint8_t r, uint8_t g, uint8_t b) : g{g}, r{r}, b{b}, _{} {}
    constexpr rgb() : g{0}, r{0}, b{0}, _{} {}

    rgb(hsv in) {
        if (in.s == 0)
        {
            r = in.v;
            g = in.v;
            b = in.v;
            return;
        }

        const uint8_t region = in.h / 43;
        const uint8_t remainder = (in.h - (region * 43)) * 6; 

        const uint8_t p = (in.v * (255 - in.s)) >> 8;
        const uint8_t q = (in.v * (255 - ((in.s * remainder) >> 8))) >> 8;
        const uint8_t t = (in.v * (255 - ((in.s * (255 - remainder)) >> 8))) >> 8;

        switch (region)
        {
            case 0:
                r = in.v; g = t; b = p;
                break;
            case 1:
                r = q; g = in.v; b = p;
                break;
            case 2:
                r = p; g = in.v; b = t;
                break;
            case 3:
                r = p; g = q; b = in.v;
                break;
            case 4:
                r = t; g = p; b = in.v;
                break;
            default:
                r = in.v; g = p; b = q;
                break;
        }
    }
};

static constexpr rgb red = {0xff,0,0};
static constexpr rgb green = {0,0xff,0};
static constexpr rgb blue = {0,0,0xff};

} //namespace utl::color

#endif //UTL_COLOR_HH_
