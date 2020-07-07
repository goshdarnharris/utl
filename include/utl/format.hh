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

static constexpr size_t MAX_SPEC_SIZE = 16;
static constexpr size_t MAX_FIELD_SIZE = 32;
static constexpr size_t MAX_FORMATTED_INT_SIZE = 64;

//from python:
// replacement_field ::=  "{" [field_name] ["!" conversion] [":" format_spec] "}"
// field_name        ::=  arg_name ("." attribute_name | "[" element_index "]")*
// arg_name          ::=  [identifier | digit+]
// attribute_name    ::=  identifier
// element_index     ::=  digit+ | index_string
// index_string      ::=  <any source character except "]"> +
// conversion        ::=  "r" | "s" | "a"
// format_spec       ::=  <described in the next section>

inline constexpr unsigned int HEXADECIMAL = 16;
inline constexpr unsigned int DECIMAL = 10;
inline constexpr unsigned int BINARY = 2;

namespace fmt {
    inline constexpr char error_char = '@';   

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

        auto iter = utl::ranges::begin(view);
        auto end = utl::ranges::end(view);
        size_t pos = 0;

        auto check = [&](auto it) { return it != end; };

        auto is_alignment = [](char c) { return (c == '<' or c == '>' or c == '^'); };        

        auto check_advance = [&]() {
            if(not check(iter)) return false;
            if(not check(iter++)) return false;
            pos++;
            return true;
        };        

        auto advance_get_digits = [&]() {
            size_t mark = pos;
            size_t count = 1;
            while(check_advance() and is_digit(*iter)) count++;
            return ascii_to_int(view.substr(mark,count));
        };
        
        if(not check(iter)) return spec;

        //If the 2nd character is alignment, the first character is fill.
        if(is_alignment(view.at(1,'\0'))) {            
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

    class arg_spec {
    public:
        enum class modes : uint8_t {
            UNKNOWN,
            MANUAL,
            AUTOMATIC
        };
    private:
        modes m_mode{modes::AUTOMATIC};
        size_t m_id{};
    public:
        constexpr arg_spec(utl::string_view view)
          : m_mode{view.length() > 0 and is_digit(view[0]) ? modes::MANUAL : modes::AUTOMATIC},
            m_id{m_mode == modes::MANUAL ? ascii_to_int(view) : 0}
        {}
        constexpr arg_spec(size_t id_) : m_mode{modes::MANUAL}, m_id{id_} {}
        constexpr arg_spec() = default;
        [[nodiscard]] constexpr modes mode() const { return m_mode; }
        [[nodiscard]] constexpr auto id() const { return m_id; }
    };

    struct field {
        arg_spec id{};
        utl::string<MAX_SPEC_SIZE> spec{};
    };

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
    struct output {
        constexpr virtual ~output() = default;
        constexpr virtual void operator()(char c) = 0;
        constexpr virtual void operator()(utl::string_view view) = 0;
    };    
    #pragma clang diagnostic pop

    // output the specified string. could be in reverse.
    // if the string represents a number type, ignore precision (it either doesn't apply or has a different meaning)
    // if it isn't a number type, precision is the maximum number of chars to take from the field value.
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

    enum class bases : uint8_t {
        BINARY = 2,
        OCTAL = 8,
        DECIMAL = 10,
        HEXADECIMAL = 16
    };

    constexpr bool is_uppercase(char c) {
        return (c >= 'A' && c <= 'Z');
    }

    // internal itoa format, formerly _ntoa_format
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

    // internal itoa for 'long' type
    static constexpr void format_ulong(output& out, unsigned long value, 
        bool negative, bases base, format_options options)
    {
        utl::array<char,MAX_FORMATTED_INT_SIZE> working{};
        size_t working_pos = 0u;
        size_t digit_pos = 0u;
        
        // if(value) {
            do {
                const auto digit_value = static_cast<unsigned int>(value % static_cast<unsigned int>(base));
                auto digit = digit_value < DECIMAL ? '0' + digit_value : (is_uppercase(options.presentation) ? 'A' : 'a') + digit_value - DECIMAL;
                
                digit_pos++;
                working[working_pos++] = static_cast<char>(digit);
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
            } while((value > 0) and (working_pos < working.size()));
        // }

        reverse_integer_decorate(out, working, working_pos, negative, base, options);
    }

    constexpr bases get_int_spec_base(char c)
    {
        switch(c) {
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

    template <typename T, typename As>
    concept formattable_as = same_as<T,As>;

    namespace detail {
        struct error{};    

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

        namespace {
            struct varg {
                constexpr varg() = default;
                constexpr virtual ~varg() = default;
                constexpr virtual void format(output& out, field const& f) const = 0;
            };
        }

        template <typename T>
        struct arg_t final : public virtual varg {
            T const& value;
            arg_t(T& v) : value{v} {}
            void format(output& out, field const& f) const final
            {
                _format(value, out, f);
            }
        };

        template <typename... Ts>
        struct arg_storage {
            static constexpr size_t n_args = sizeof...(Ts);
            using arg_storage_t = utl::tuple<arg_t<Ts>...>;
            using varg_storage_t = utl::array<varg const* const,n_args>;

            const arg_storage_t args;
            const varg_storage_t vargs;

            template <size_t... N>
            constexpr arg_storage(index_sequence<N...>, Ts&&... args_)
            : args{std::forward<Ts>(args_)...},
                vargs{&utl::get<N>(args)...}
            {}

            constexpr arg_storage(Ts&&... args_) 
            : arg_storage{utl::make_index_sequence<n_args>{}, std::forward<Ts>(args_)...}
            {}
        };

        template <typename... Ts>
        arg_storage(Ts&&...) -> arg_storage<Ts...>;

        struct arglist {
            using arg_view_t = utl::span<varg const* const>;
            struct next_arg_tag{};

            const arg_view_t view;
            size_t next_arg;

            template <typename... Ts>
            arglist(arg_storage<Ts...> const& s) : view{s.vargs.data(),s.vargs.size()}, next_arg{0} {}

            varg const& consume_next() { return *view[next_arg++]; }
            [[nodiscard]] varg const& get(size_t idx) const { return *view[idx]; }
            [[nodiscard]] bool valid_id(size_t idx) const { return idx < view.size(); }
            [[nodiscard]] bool valid_id(next_arg_tag) const { return next_arg < view.size(); }
        };
        
        template <typename... Ts>
        constexpr auto wrap_args(Ts&&... vs)
        {
            return arg_storage{std::forward<Ts>(vs)...};
        }

        template <typename... Ts>
        constexpr auto erase_args(arg_storage<Ts...> const& s)
        {
            return arglist{s};
        }

    } //namespace detail

    void vformat(output& out, utl::string_view format, detail::arglist& args, 
        arg_spec::modes id_mode = arg_spec::modes::UNKNOWN);

    inline auto make_field(utl::string_view view, detail::arglist& args)
    {
        constexpr utl::string_view field_spec_start = ":";
        if(view.length() == 0) return field{};

        size_t pos = 0;
        utl::string<MAX_SPEC_SIZE> formatted{};
        auto write_buffer = [&] (char c) {
            if(pos < formatted.size()) formatted[pos++] = c;
        };
        detail::output_t into_buffer{write_buffer};

        const size_t mark = view.find(field_spec_start);

        if(mark != utl::npos and mark+1 < view.length()) {
            auto spec_view = view.substr(mark+1,utl::npos);
            if(spec_view.length() > 0 and spec_view.length() < MAX_SPEC_SIZE) {
                vformat(into_buffer, view.substr(mark+1,utl::npos), args);
            }
        }

        return field{arg_spec{view.substr(0,mark)}, formatted};
    }

    inline void vformat(output& out, utl::string_view format, detail::arglist& args, 
        arg_spec::modes id_mode)
    {
        //FIXME: id_mode isn't being passed around properly.
        struct format_state {
            enum states {
                ECHO,
                ESCAPED_ENTRY,
                ESCAPED_EXIT,
                PROCESS_FIELD
            };
            states active = ECHO;
        };
        
        constexpr auto field_entry = '{';
        constexpr auto field_exit = '}';
        size_t field_inner_depth = 0;
        auto state = format_state{};
        size_t mark{};

        auto process_field = [&](field f) {
            if(id_mode == arg_spec::modes::UNKNOWN) {
                id_mode = f.id.mode();
            }

            if(f.id.mode() != id_mode) {            
                //switching field numbering modes isn't allowed.
                out(error_char);
                return;
            }

            if(f.id.mode() == arg_spec::modes::AUTOMATIC) {
                if(args.valid_id(detail::arglist::next_arg_tag{})) {
                    args.consume_next().format(out,f);
                }
            } else if(args.valid_id(f.id.id())) {
                args.get(f.id.id()).format(out,f);
            } else {
                out(error_char);
            }
        };

        for(const auto&& [pos,c] : utl::ranges::enumerate(format)) {
            // utl::log(" vf: %d, %c, %d", pos, c, state.active);
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
                    if(c == '{') {
                        //we support fields in fields
                        field_inner_depth++;
                    } else if(c == '}' and field_inner_depth > 0) {
                        field_inner_depth--;
                    } else if(c == '}') {
                        //FIXME: need to know if formatting the field yielded an error.
                        auto f = make_field(format.substr(mark, pos - mark), args);
                        process_field(f);
                        state.active = format_state::ECHO;
                    }
                    break;
            }
        }
    }

    template <typename T>
    concept formattable = requires(T const& arg, output& out, field& spec) {
        _format(arg, out, spec);
    };
} //namespace fmt

template <size_t N, fmt::formattable... Args>
constexpr auto format(utl::string_view format, Args&&... args)
{
    array<char,N> buffer{};
    format_into(buffer, format, std::forward<Args>(args)...);
    return utl::string<N>{buffer.data()};
}

template <typename T, fmt::formattable... Args>
    requires requires(T&& v) {
        v[size_t{}] = char{};
        { v.size() } -> utl::convertible_to<size_t>;
    }
constexpr size_t format_into(T&& buffer, utl::string_view format, Args&&... args)
{
    size_t pos = 0;
    auto buffer_out = [&] (char c) {
        if(pos < buffer.size()) buffer[pos++] = c;
    };
    format_to(buffer_out, format, std::forward<Args>(args)...);
    return pos;
}

template <callable<void,char> F, fmt::formattable... Args>
constexpr void format_to(F&& out, utl::string_view format, Args&&... args)
{
    const auto arg_storage = fmt::detail::wrap_args(std::forward<Args>(args)...);
    auto arg_view = fmt::detail::erase_args(arg_storage);
    fmt::detail::output_t call{std::forward<F>(out)};
    fmt::vformat(call,format,arg_view);
}

template <fmt::formattable... Args>
constexpr void format_to(fmt::output& out, utl::string_view format, Args&&... args)
{
    //FIXME: add is_formattable trait, if constexpr to throw a static assert asap if
    // a type isn't supported!
    const auto arg_storage = fmt::detail::wrap_args(std::forward<Args>(args)...);
    auto arg_view = fmt::detail::erase_args(arg_storage);
    fmt::vformat(out,format,arg_view);
}


namespace fmt {
    constexpr format_options default_int_options{ 
        .fill = ' ', 
        .align = alignment::RIGHT,
        .sign = format_options::signs::NEGATIVE_ONLY,
        .width = 0,
        .presentation = 'd'
    };

    constexpr void _format(formattable_as<short> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, get_int_spec_base(options.presentation), options);
    }

    constexpr void _format(formattable_as<int> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, get_int_spec_base(options.presentation), options);
    }

    constexpr void _format(formattable_as<long> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, get_int_spec_base(options.presentation), options);
    }

    constexpr void _format(formattable_as<long long> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, get_int_spec_base(options.presentation), options);
    }

    constexpr void _format(formattable_as<unsigned char> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, get_int_spec_base(options.presentation), options);
    }

    constexpr void _format(formattable_as<unsigned short> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, get_int_spec_base(options.presentation), options);
    }

    constexpr void _format(formattable_as<unsigned int> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<unsigned long>(arg), false, get_int_spec_base(options.presentation), options);
    }

    constexpr void _format(formattable_as<unsigned long> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, arg, false, get_int_spec_base(options.presentation), options);
    }

    template <typename T>
    void _format(T* arg, output& out, field const& f)
    {
        auto defaults = default_int_options;
        defaults.alternate_form = true;
        defaults.presentation = 'x';
        auto options = parse_format_options(f.spec, defaults);
        format_ulong(out, reinterpret_cast<unsigned long>(arg), false, get_int_spec_base(options.presentation), options);
    }

    constexpr void _format(formattable_as<float> auto arg, output& out, field const& f)
    {
        utl::maybe_unused(arg, f);
        out("{:f}");
    }

    constexpr void _format(formattable_as<double> auto arg, output& out, field const& f)
    {
        utl::maybe_unused(arg, f);
        out("{:f}");
    }

    constexpr void _format(formattable_as<long double> auto arg, output& out, field const& f)
    {
        utl::maybe_unused(arg, f);
        out("{:f}");
    }

    constexpr void _format(utl::string_view arg, output& out, field const& f)
    {      
        auto options = parse_format_options(f.spec, {
            .fill = ' ',
            .align = alignment::LEFT,
            .presentation = 's'
        });
        fmt::align_pad_out(out, arg, options.fill, options.align, options.width, options.precision);
    }

    template<size_t N>
    constexpr void _format(utl::string<N>& arg, output& out, field const& f)
    {
        utl::maybe_unused(f);
        _format(static_cast<utl::string_view>(arg),out,f);
    }

    constexpr void _format(const char* arg, output& out, field const& f)
    {
        _format(utl::string_view{arg},out,f);
    }

    constexpr void _format(char* arg, output& out, field const& f)
    {
        _format(utl::string_view{arg},out,f);
    }

    inline void _format(std::nullptr_t, output& out, field const& f)
    {
        _format(reinterpret_cast<void*>(0),out,f);
    }

    constexpr void _format(formattable_as<char> auto arg, output& out, field const& f)
    {
        _format(utl::string_view{&arg,1},out,f);
    }

    constexpr void _format(formattable_as<bool> auto arg, output& out, field const& f)
    {
        _format(utl::string_view{arg ? "true" : "false"},out,f);
    }

} //namespace fmt

} //namespace utl
