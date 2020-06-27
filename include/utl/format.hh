#pragma once

#include <utl/string-view.hh>
#include <utl/string.hh>
#include <utl/concepts.hh>
#include <utl/result.hh>
#include <utl/tuple.hh>
#include <utl/array.hh>
#include <utl/ranges.hh>
#include <utl/span.hh>
#include <utl/logger.hh>

namespace utl {

static constexpr size_t MAX_FIELD_SIZE = 32;
static constexpr size_t MAX_FORMATTED_INT_SIZE = 32;

//from python:
// replacement_field ::=  "{" [field_name] ["!" conversion] [":" format_spec] "}"
// field_name        ::=  arg_name ("." attribute_name | "[" element_index "]")*
// arg_name          ::=  [identifier | digit+]
// attribute_name    ::=  identifier
// element_index     ::=  digit+ | index_string
// index_string      ::=  <any source character except "]"> +
// conversion        ::=  "r" | "s" | "a"
// format_spec       ::=  <described in the next section>


// format_spec     ::=  [[fill]align][sign][#][0][width][grouping_option][.precision][type]
// fill            ::=  <any character>
// align           ::=  "<" | ">" | "=" | "^"
// sign            ::=  "+" | "-" | " "
// width           ::=  digit+
// grouping_option ::=  "_" | ","
// precision       ::=  digit+
// type            ::=  "b" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G" | "n" | "o" | "s" | "x" | "X" | "%"

inline constexpr unsigned int HEXADECIMAL = 16;
inline constexpr unsigned int DECIMAL = 10;
inline constexpr unsigned int BINARY = 2;


constexpr bool is_digit(char ch)
{
    return (ch >= '0') and (ch <= '9');
}

constexpr unsigned int digit_to_int(char c)
{
    return static_cast<unsigned int>(c - '0');
}

// internal ASCII string to unsigned int conversion
constexpr unsigned int ascii_to_int(utl::string_view ascii)
{
    unsigned int value = 0u;
    for(char c : ascii) {
        if(not is_digit(c)) break;
        value *= DECIMAL;
        value += digit_to_int(c);
    }

    return value;
}

struct format_flags {
    enum flag {
        ZEROPAD     = 0b000000000001,
        LEFT        = 0b000000000010,
        PLUS        = 0b000000000100,
        SPACE       = 0b000000001000,
        HASH        = 0b000000010000,
        UPPERCASE   = 0b000000100000,
        CHAR        = 0b000001000000,
        SHORT       = 0b000010000000,
        LONG        = 0b000100000000,
        LONG_LONG   = 0b001000000000,
        PRECISION   = 0b010000000000,
        ADAPT_EXP   = 0b100000000000
    };
    uint32_t value = 0;
    constexpr void set(flag f) { value |= static_cast<uint32_t>(f); }
    constexpr void clear(flag f) { value &= ~static_cast<uint32_t>(f); }

    template <same_as<flag>... Args>
    constexpr bool check(Args&&... args) { 
        return ((value & static_cast<uint32_t>(args)) && ...); 
    }

    constexpr bool zeropad() { return check(ZEROPAD); }
    constexpr bool left() { return check(LEFT); }
    constexpr bool plus() { return check(PLUS); }
    constexpr bool space() { return check(SPACE); }
    constexpr bool hash() { return check(HASH); }
    constexpr bool uppercase() { return check(UPPERCASE); }
    constexpr bool character() { return check(CHAR); }
    constexpr bool long_int() { return check(LONG); }
    constexpr bool long_long_int() { return check(LONG_LONG); }
    constexpr bool precision() { return check(PRECISION); }
    constexpr bool adapt_exp() { return check(ADAPT_EXP); }
};

// output the specified string in reverse, taking care of any zero-padding
//formerly _out_rev
constexpr void out_pad_reverse(callable<void,char> auto&& out, utl::span<char> input, 
    unsigned int output_width, format_flags flags)
{
    size_t out_chars = 0;
    // pad spaces up to given width
    if(not flags.left() and not flags.zeropad()) {

        for(size_t i = input.size(); i < output_width; i++) {
            out(' ');
            out_chars++;
        }
    }

    // reverse string
    for(char c : utl::ranges::reverse(input)) {
        out(c);
        out_chars++;
    }

    // append pad spaces up to given width
    if(flags.left()) {
        while(out_chars++ < output_width) out(' ');
    }
}

// static size_t _out_rev(out_fct_type out, char* buffer, size_t idx, 
//     size_t maxlen, const char* buf, size_t len, unsigned int width, 
//     unsigned int flags)
// out -> char output functor
// buffer -> output buffer (goes away)
// idx -> output index (goes away)
// maxlen -> maximum output length (goes away)
// buf -> input buffer
// len -> input buffer length
// width -> output width
// flags -> format_flags


// internal itoa format, formerly _ntoa_format
static constexpr void format_and_blit(callable<void,char> auto&& out, utl::span<char> working, size_t& working_pos, 
    bool negative, unsigned int base, unsigned int prec, unsigned int width, format_flags flags)
{

  // pad leading zeros
    if(not flags.left()) {
        if(width and flags.zeropad() and (negative or (flags.plus() and flags.space()))) {
            width--;
        }
        while((working_pos < prec) and (working_pos < working.size())) {
            working[working_pos++] = '0';
        }
        while(flags.zeropad() and (working_pos < width) and (working_pos < working.size())) {
            working[working_pos++] = '0';
        }
    }

    // handle hash
    if(flags.hash()) {
        if((not flags.precision()) and working_pos and ((working_pos == prec) || (working_pos == width))) {
            working_pos--;
            if(working_pos and (base == HEXADECIMAL)) {
                working_pos--;
            }
        }
        if((base == HEXADECIMAL) and (not flags.uppercase()) and (working_pos < working.size())) {
            working[working_pos++] = 'x';
        } else if((base == HEXADECIMAL) and flags.uppercase() and (working_pos < working.size())) {
            working[working_pos++] = 'X';
        } else if((base == BINARY) and (working_pos < working.size())) {
            working[working_pos++] = 'b';
        }
        if(working_pos < working.size()) {
            working[working_pos++] = '0';
        }
    }

    if(working_pos < working.size()) {
        if(negative) {
            working[working_pos++] = '-';
        } else if(flags.plus()) {
            working[working_pos++] = '+';  // ignore the space if the '+' exists
        } else if(flags.space()) {
            working[working_pos++] = ' ';
        }
    }

    out_pad_reverse(out, {working.data(), working_pos}, width, flags);
}

// internal itoa for 'long' type
static constexpr void ulong_to_ascii(callable<void,char> auto&& out, unsigned long value, 
    bool negative, unsigned long base, unsigned int prec, unsigned int width, format_flags flags)
{
    utl::array<char,MAX_FORMATTED_INT_SIZE> working{};
    size_t working_pos = 0u;

    // no hash for 0 values
    if(!value) {
        flags.clear(format_flags::HASH);
    }
    // write if precision != 0 and value is != 0
    if(not flags.precision() or value) {
        do {
            const auto digit_value = static_cast<unsigned int>(value % base);
            auto digit = digit_value < DECIMAL ? '0' + digit_value : (flags.uppercase() ? 'A' : 'a') + digit_value - DECIMAL;
            working[working_pos++] = static_cast<char>(digit);
            value /= base;
        } while(value && (working_pos < working.size()));
    }

    format_and_blit(out, working, working_pos, negative, static_cast<unsigned int>(base), prec, width, flags);
}

struct field {
    using arg_id_t = size_t;

    enum modes : uint8_t {
        UNKNOWN,
        AUTOMATIC_NUMBERING,
        MANUAL_NUMBERING
    };
    arg_id_t arg_index = 0;
    modes mode = modes::AUTOMATIC_NUMBERING;
    utl::string_view format_spec = "";
};

namespace detail {

    struct error{};

    struct format_state {
        utl::string_view format;
        size_t pos;
    };

    template <typename T, callable<void,char> F>
    static void format_erased(const void* arg, F&& out, field const& f)
    {
        _format(*static_cast<const T*>(arg), std::forward<F>(out), f);
    }

    template <callable<void,char> F>
    using format_erased_t = void(*)(const void*,F&&,field const&);

    template <callable<void,char> F>
    struct arg {
        const void* value;
        format_erased_t<F> do_format;

        template <typename T>
        constexpr arg(T* a)
        : value{static_cast<const void*>(a)}, do_format{&format_erased<T,F>}
        {}

        constexpr auto format(same_as<F> auto&& out, field const& f)
        {
            do_format(value, std::forward<F>(out), f);
        }
    };

    template <callable<void,char> F>
    struct arglist {
        utl::span<arg<F>> args;
        size_t next_arg = 0;
        arg<F>& consume_next() { return args[next_arg++]; }
    };

    template <callable<void,char> F, typename... Args>
    constexpr utl::array<arg<F>,sizeof...(Args)> erase_args(Args&&... args)
    {
        return {{{&args}...}};
    }

    

    constexpr field parse_field(utl::string_view view)
    {    
        constexpr utl::string_view field_spec_start = ":";

        if(view.size() == 0) return field{};

        const size_t mark = view.find(field_spec_start);

        field::arg_id_t arg_index{};
        auto mode = field::modes::AUTOMATIC_NUMBERING;

        //for now, just numerical ids.
        auto arg_id_view = view.substr(0,mark);
        if(arg_id_view.length() > 0 and is_digit(arg_id_view[0])) {
            arg_index = ascii_to_int(arg_id_view);
            mode = field::modes::MANUAL_NUMBERING;
        }

        if(mark != utl::npos and mark + 1 < view.size()) {
            auto spec = view.substr(mark+1, utl::npos);
            return {arg_index,mode,spec};
        } 
        return {arg_index,mode};
    }

    template <callable<void,char> F>
    constexpr auto make_field(utl::string_view view, arglist<F>& args)
    {
        // size_t pos = 0;
        // utl::array<char,MAX_FIELD_SIZE> formatted{};
        // auto buffer_out = [&] (char c) {
        //     if(pos < formatted.size()) formatted[pos++] = c;
        // };
        //balls. the args we're passing around are templated on
        //a different output function than the one we want to give
        //to _vformat.
        //FIXME: the note above this. for now, no field formatting.
        // _vformat(buffer_out, view, args);
        utl::maybe_unused(args);
        //FIXME: need to know if the formatting had an error.
        return parse_field(view);
    }

} //namespace detail




template <typename T, typename As>
concept formattable_as = same_as<T,As>;

template <typename T>
constexpr void _format(T&&, callable<void,char> auto&&, field const&)
{
    static_assert(not std::is_same_v<T,T>, "to format your own types, implement a _format function: "
        "constexpr void _format(T& arg, utl::callable<void,char> auto&& out, utl::field const& f)");
}

constexpr void _format(detail::error e, callable<void,char> auto&& out, field const& f)
{
    //TODO: actually implement formatters
    utl::maybe_unused(f,e);
    out('@');
}

constexpr void _format(formattable_as<int> auto arg, callable<void,char> auto&& out, field const& f)
{
    utl::maybe_unused(f);
    ulong_to_ascii(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, DECIMAL, 0, 0, {});
}

constexpr void _format(formattable_as<unsigned int> auto arg, callable<void,char> auto&& out, field const& f)
{
    utl::maybe_unused(f);    
    ulong_to_ascii(out, static_cast<unsigned long>(arg), false, DECIMAL, 0, 0, {});
}

constexpr void _format(formattable_as<float> auto arg, callable<void,char> auto&& out, field const& f)
{
    utl::maybe_unused(arg, f);
    out('f');
}

constexpr void _format(utl::string_view arg, callable<void,char> auto&& out, field const& f)
{
    utl::maybe_unused(f);
    for(auto c : arg) out(c);
}



template <size_t N, typename... Args>
constexpr auto format(utl::string_view format, Args&&... args)
{
    array<char,N> buffer;
    format_into(buffer, format, std::forward<Args>(args)...);
    return utl::string<N>{buffer.data()};
}

template <typename T, typename... Args>
    requires requires(T&& v) {
        v[size_t{}] = char{};
    }
constexpr void format_into(T&& buffer, utl::string_view format, Args&&... args)
{
    size_t pos = 0;
    auto buffer_out = [&] (char c) {
        if(pos < buffer.size()) buffer[pos++] = c;
    };
    format_to(buffer_out, format, std::forward<Args>(args)...);  
}

template <callable<void,char> F, typename... Args>
constexpr void format_to(F&& out, utl::string_view format, Args&&... args)
{
    auto erased = detail::erase_args<F>(args...);
    auto list = detail::arglist<F>{erased};
    _vformat(std::forward<F>(out),format,list);
}



template <callable<void,char> F>
constexpr void _vformat(F&& out, utl::string_view format, detail::arglist<F>& args)
{
    struct format_state {
        enum states {
            ECHO,
            ESCAPED_ENTRY,
            ESCAPED_EXIT,
            PROCESS_FIELD
        };
        states active = ECHO;
        size_t next_arg = 0;
        field::modes mode = field::modes::UNKNOWN;
        size_t consume_arg() { return next_arg++; }
    };
    //FIXME: there's internal _vformat state for stepping through the machine
    //then, there's maybe a separate, larger formatting context that gets passed
    //around internally? Specifically, we want to be able to format a field
    //while keeping track of what numbering mode we're in and which arguments
    //have been consumed, if any
    
    constexpr auto field_entry = '{';
    constexpr auto field_exit = '}';
    auto state = format_state{};
    size_t mark{};

    auto process_field = [&](field f) {
        if(state.mode == field::modes::UNKNOWN) {
            state.mode = f.mode;
        }

        if(f.mode != state.mode) {            
            //switching field numbering modes isn't allowed.
            _format(detail::error{}, std::forward<F>(out), f);
            return;
        }

        if(f.mode == field::modes::AUTOMATIC_NUMBERING) {
            auto arg = args.consume_next();
            arg.format(std::forward<F>(out), f);
        } else {                
            args.args[f.arg_index].format(std::forward<F>(out), f);
        }
    };

    for(const auto&& [pos,c] : utl::ranges::enumerate(format)) {
        switch(state.active) {
            case format_state::ECHO:
                if(c == field_entry) {
                    state.active = format_state::ESCAPED_ENTRY;
                } else if(c == field_exit) {
                    state.active = format_state::ESCAPED_EXIT;
                    continue;
                } else {
                    out(c);
                }
                break;
            case format_state::ESCAPED_ENTRY:   
                if(c == field_entry) {
                    //escaped for output
                    out(c);
                    state.active = format_state::ECHO;
                } else if(c == field_exit) {
                    //you are standing in an empty field...
                    process_field(field{});
                    state.active = format_state::ECHO;
                    break;
                } else {
                    mark = pos;
                    state.active = format_state::PROCESS_FIELD;
                }             
                break;
            case format_state::ESCAPED_EXIT:
                if(c == field_entry) {
                    state.active = format_state::ESCAPED_ENTRY;
                } else {
                    out(c);
                    state.active = format_state::ECHO;
                }
                break;
            case format_state::PROCESS_FIELD:
                if(c == '}') {
                    //FIXME: need to know if formatting the field yielded an error.
                    auto f = detail::make_field(format.substr(mark, pos - mark), args);
                    process_field(f);
                    state.active = format_state::ECHO;
                }
                break;
        }
    }
}

} //namespace utl
