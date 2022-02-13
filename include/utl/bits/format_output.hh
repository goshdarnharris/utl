#pragma once

#include <utl/bits/format_options.hh>

namespace utl::fmt {

inline constexpr char error_char = '@';       

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"
struct output {
    constexpr virtual ~output() = default;
    constexpr virtual void operator()(char c) = 0;
    constexpr virtual void operator()(utl::string_view view) = 0;
};    
#pragma clang diagnostic pop

template <typename F>
struct output_t final : public virtual output {
    F& call;
    output_t(F& c) : call{c} {}
    void operator()(char c) final
    {
        call(c);
    }            
    void operator()(utl::string_view view) final
    {
        for(char c : view) call(c);
    }
};

template <typename F>
output_t(F&) -> output_t<F>;

// output the specified string. could be in reverse.
// if the string represents a number type, ignore precision (it either doesn't apply or has a different meaning)
// if it isn't a number type, precision is the maximum number of chars to take from the field value.
//NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
constexpr void align_pad_out(output& out, utl::string_view input, char fill, alignment align, size_t min_width, 
    size_t max_content_chars = utl::npos, bool reverse = false)
{
    size_t pad_chars = input.size() < min_width ? min_width - input.size() : 0;

    switch(align) {
        case alignment::LEFT:
            break;
        case alignment::CENTER:
            for(size_t i = 0; i < pad_chars/2; i++) {
                out(fill);
            }
            break;
        case alignment::RIGHT:
            for(size_t i = 0; i < pad_chars; i++) {
                out(fill);
            }
            break;
    }

    //FIXME: need a better way to express what default means for each type
    //FIXME: need a good way to express what "precision" means

    // reverse string
    if(reverse) {
        size_t count = 0;
        for(char c : utl::ranges::reverse(input)) {
            count++;
            out(c);
            if(count >= max_content_chars) break;
        }
    } else {
        out(input.substr(0,max_content_chars));
    }

    // append pad spaces up to given width
    switch(align) {
        case alignment::LEFT:
            for(size_t i = 0; i < pad_chars; i++) {
                out(fill);
            }
            break;
        case alignment::CENTER:
            for(size_t i = 0; i < (pad_chars/2) + (pad_chars%2); i++) {
                out(fill);
            }
            break;
        case alignment::RIGHT:               
            break;
    }
}

template <typename T, typename As>
static constexpr bool is_formattable_as_v = same_as<T,As>;

template <typename T, typename As>
concept formattable_as = is_formattable_as_v<T,As>;    

} //namespace utl::fmt
