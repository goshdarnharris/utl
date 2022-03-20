
#pragma once

namespace utl::fmt {

enum class alignment {
    LEFT,
    CENTER,
    RIGHT
};

struct format_options {

    enum class signs : uint8_t {
        BOTH,
        NEGATIVE_ONLY,
        SPACE
    };

    enum class grouping_options : uint8_t {
        NONE,
        SEP_COMMA,
        SEP_USCORE
    };

    char fill = ' ';
    alignment align = alignment::LEFT;
    signs sign = signs::NEGATIVE_ONLY; 
    bool alternate_form = false;
    bool zero_pad = false;
    size_t width = 0;
    grouping_options grouping_option = grouping_options::NONE;
    bool has_precision = false;
    size_t precision = utl::npos;
    char presentation = '\0';
};     

//NOLINTNEXTLINE(readability-function-cognitive-complexity)
constexpr format_options parse_format_options(utl::string_view view, format_options defaults = {})
{
    // format_spec     ::=  [[fill]align][sign][#][0][width][grouping_option][.precision][type]
    // fill            ::=  <any character>
    // align           ::=  "<" | ">" | "=" | "^"
    // sign            ::=  "+" | "-" | " "
    // width           ::=  digit+
    // grouping_option ::=  "_" | ","
    // precision       ::=  digit+
    // type            ::=  "b" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G" | "n" | "o" | "s" | "x" | "X" | "%"
    format_options spec = defaults;

    const auto* iter = utl::ranges::begin(view);
    const auto* end = utl::ranges::end(view);
    size_t pos = 0;

    auto check = [&](auto it) { return it != end; };

    auto is_alignment = [](char c) { return (c == '<' or c == '>' or c == '^'); };        

    auto check_advance = [&]() {
        if(not check(iter)) return false;
        //NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if(not check(iter++)) return false;
        pos++;
        return true;
    };        

    auto advance_get_digits = [&]() {
        size_t mark = pos;
        size_t count = 1;
        while(check_advance() and is_digit(*iter)) count++;
        return ascii_to_uint(view.substr(mark,count));
    };
    
    if(not check(iter)) return spec;

    //If the 2nd character is alignment, the first character is fill.
    if(is_alignment(view.at(1))) {            
        if(*iter == '{' or *iter == '}') {
            //reject
        } else {      
            spec.fill = *iter;
        }

        if(not check_advance()) return spec;
    }

    //align
    switch(*iter) {
        case '<':
            spec.align = alignment::LEFT;
            if(not check_advance()) return spec;
            break;
        case '>':
            spec.align = alignment::RIGHT;
            if(not check_advance()) return spec;
            break;
        case '^':                
            spec.align = alignment::CENTER;
            if(not check_advance()) return spec;
            break;
        default:
            break;
    }

    //sign
    switch(*iter) {
        case '+':
            spec.sign = format_options::signs::BOTH;
            if(not check_advance()) return spec;
            break;
        case '-':
            spec.sign = format_options::signs::NEGATIVE_ONLY;
            if(not check_advance()) return spec;
            break;
        case ' ':
            spec.sign = format_options::signs::SPACE;
            if(not check_advance()) return spec;
            break;
        default:
            break;
    }

    //alternate form
    if(*iter == '#') {
        //do the thing with it
        spec.alternate_form = true;
        if(not check_advance()) return spec;
    }

    //zero padding
    if(*iter == '0') {
        spec.zero_pad = true;
        if(not check_advance()) return spec;
    }

    //width
    if(is_digit(*iter)) {
        spec.width = advance_get_digits();
        if(not check(iter)) return spec;
    }

    //grouping option
    switch(*iter) {
        case '_':
            spec.grouping_option = format_options::grouping_options::SEP_USCORE;
            if(not check_advance()) return spec;
            break;
        case ',':
            spec.grouping_option = format_options::grouping_options::SEP_COMMA;
            if(not check_advance()) return spec;
            break;
        default:
            break;
    }

    //precision
    if(*iter == '.') {
        if(not check_advance()) return spec;
        
        if(is_digit(*iter)) {
            spec.has_precision = true;
            spec.precision = advance_get_digits();
            if(not check(iter)) return spec;
        }
    }
    
    spec.presentation = *iter;

    return spec;
}

inline constexpr format_options default_int_options{ 
    .fill = ' ', 
    .align = alignment::RIGHT,
    .sign = format_options::signs::NEGATIVE_ONLY,
    .width = 0,
    .presentation = 'd'
};

inline constexpr format_options default_pointer_options{ 
    .fill = ' ', 
    .align = alignment::RIGHT,
    .sign = format_options::signs::NEGATIVE_ONLY,
    .width = 0,
    .presentation = 'x'
};

inline constexpr format_options default_char_options{ 
    .fill = ' ', 
    .align = alignment::RIGHT,
    .sign = format_options::signs::NEGATIVE_ONLY,
    .width = 0,
    .presentation = 'c'
};

inline constexpr format_options default_unsigned_char_options{ 
    .fill = ' ', 
    .align = alignment::RIGHT,
    .sign = format_options::signs::NEGATIVE_ONLY,
    .width = 0,
    .presentation = 'd'
};

} //namespace utl::fmt
