#pragma once

#include <utl/span.hh>
#include <utl/bits/format_options.hh>
#include <utl/bits/format_output.hh>

namespace utl::fmt {

enum class bases : uint8_t {
    BINARY = 2,
    OCTAL = 8,
    DECIMAL = 10,
    HEXADECIMAL = 16
};

constexpr bool is_uppercase(char character) {
    return (character >= 'A' && character <= 'Z');
}

// internal itoa format, formerly _ntoa_format
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
constexpr void reverse_integer_decorate(output& out, utl::span<char> working, size_t& working_pos, 
    bool negative, bases base, format_options options)
{
    auto width = options.width;
    // pad leading zeros
    if(options.align != alignment::LEFT) {
        bool leading_char = negative or options.sign == format_options::signs::BOTH
            or options.sign == format_options::signs::SPACE;
        if(width and options.zero_pad and leading_char) {
            width--;
        }
        if(options.alternate_form and base != bases::DECIMAL) {
            width --;
            if(working_pos and (base == bases::HEXADECIMAL || base == bases::BINARY)) {
                width--;
            }
        }
        while(options.zero_pad and (working_pos < width) and (working_pos < working.size())) {
            working[working_pos++] = '0';
        }
    }

    if(options.alternate_form) {
        switch(base) {
            case bases::HEXADECIMAL:
                if(is_uppercase(options.presentation) and (working_pos < working.size())) {
                    working[working_pos++] = 'X';
                } else if(working_pos < working.size()) {
                    working[working_pos++] = 'x';
                }
                break;
            case bases::DECIMAL:
                [[fallthrough]];
            case bases::OCTAL:
                break;
            case bases::BINARY:
                if(is_uppercase(options.presentation) and (working_pos < working.size())) {
                    working[working_pos++] = 'B';
                } else if(working_pos < working.size()) {
                    working[working_pos++] = 'b';
                }
                break;
        }
        if(base != bases::DECIMAL and working_pos < working.size()) {
            working[working_pos++] = '0';
        }
    }

    if(working_pos < working.size()) {
        if(negative) {
            working[working_pos++] = '-';
        } else if(options.sign == format_options::signs::BOTH) {
            working[working_pos++] = '+';
        } else if(options.sign == format_options::signs::SPACE) {
            working[working_pos++] = ' ';
        }
    }

    align_pad_out(out, {working.data(), working_pos}, options.fill, options.align, options.width, working_pos, true);
}

inline constexpr bases get_int_spec_base(char character)
{
    switch(character) {
        case 'B':
            [[fallthrough]];
        case 'b':
            return bases::BINARY;
        case 'o':
            return bases::OCTAL;
        case 'X':
            [[fallthrough]];
        case 'x':
            return bases::HEXADECIMAL;
        case 'd':
            [[fallthrough]];
        default:
            return bases::DECIMAL;
    }
}

// internal itoa for 'long' type
//NOLINTNEXTLINE(readability-function-cognitive-complexity)
inline constexpr void format_ulong(output& out, unsigned long value, 
    bool negative, format_options options)
{
    utl::array<char,MAX_FORMATTED_INT_SIZE> working{};
    size_t working_pos = 0u;
    size_t digit_pos = 0u;

    auto base = get_int_spec_base(options.presentation);

    switch(options.presentation)
    {
        case 'p':                
            options.alternate_form = true;
            options.presentation = 'x';
            base = bases::HEXADECIMAL;
            break;
    }
    
    if(value) {
        while((value > 0) and (working_pos < working.size())) {
            const auto digit_value = static_cast<unsigned int>(value % static_cast<unsigned int>(base));
            //NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
            auto digit = digit_value < 10 ? '0' + digit_value : (is_uppercase(options.presentation) ? 'A' : 'a') + digit_value - 10;
            
            digit_pos++;
            working[working_pos++] = static_cast<char>(digit);
            if(working_pos == working.size()) break;
            value /= static_cast<unsigned int>(base);    

            
            if(digit_pos % 3 == 0) {
                switch(options.grouping_option) {
                    case format_options::grouping_options::SEP_COMMA:
                        working[working_pos++] = ',';
                        if(working_pos == working.size()) break;
                        break;
                    case format_options::grouping_options::SEP_USCORE:
                        working[working_pos++] = '_';
                        if(working_pos == working.size()) break;
                        break;
                    case format_options::grouping_options::NONE:
                        break;
                }
            }
        }
    } else if(not options.alternate_form or base != bases::OCTAL) {
        working[working_pos++] = '0';
    }

    reverse_integer_decorate(out, working, working_pos, negative, base, options);
}

} //namespace utl::fmt
