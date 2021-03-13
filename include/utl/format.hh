#pragma once

#include <utl/string-view.hh>
#include <utl/string.hh>
#include <utl/concepts.hh>
#include <utl/result.hh>
#include <utl/tuple.hh>
#include <utl/array.hh>
#include <utl/ranges.hh>
#include <utl/span.hh>
#include <utility>

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

    inline constexpr bases get_int_spec_base(char c)
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

    // internal itoa for 'long' type
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
                auto digit = digit_value < DECIMAL ? '0' + digit_value : (is_uppercase(options.presentation) ? 'A' : 'a') + digit_value - DECIMAL;
                
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

#if defined(PRINTF_SUPPORT_FLOAT)
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
    // forward declaration so that _ftoa can switch to exp notation for values > PRINTF_MAX_FLOAT
    static size_t format_exponential(out_fct_type out, char *buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags);
#endif

    // internal ftoa for fixed decimal floating point
    static size_t format_float(out_fct_type out, char *buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags)
    {
        utl::array<char,MAX_FORMATTED_FLOAT_SIZE> working;
        size_t len = 0U;
        double diff = 0.0;

        // powers of 10
        static constexpr double pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

        // test for special values
        if (value != value) {
            align_pad_out(out, "nan", options.fill, options.align, options.width, options.precision);
            return;
        }
        if (value < -DBL_MAX) {
            align_pad_out(out, "-inf", options.fill, options.align, options.width, options.precision);
            return;
        }
        if (value > DBL_MAX) {
            switch(options.sign) {
                case format_options::signs::BOTH:
                    align_pad_out(out, "+inf", options.fill, options.align, options.width, options.precision);
                    return;
                case format_options::signs::SPACE:
                    align_pad_out(out, " inf", options.fill, options.align, options.width, options.precision);
                    return;
                case format_options::signs::NEGATIVE_ONLY:
                    align_pad_out(out, "inf", options.fill, options.align, options.width, options.precision);
                    return;
            }
        }
        // test for very large values
        // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
        if ((value > PRINTF_MAX_FLOAT) || (value < -PRINTF_MAX_FLOAT))
        {
#if defined(PRINTF_SUPPORT_EXPONENTIAL)
            return _etoa(out, buffer, idx, maxlen, value, prec, width, flags);
#else
            return 0U;
#endif
        }

        // test for negative
        bool negative = false;
        if (value < 0)
        {
            negative = true;
            value = 0 - value;
        }

        // set default precision, if not set explicitly
        if (!(flags & FLAGS_PRECISION))
        {
            prec = PRINTF_DEFAULT_FLOAT_PRECISION;
        }
        // limit precision to 9, cause a prec >= 10 can lead to overflow errors
        while ((len < PRINTF_FTOA_BUFFER_SIZE) && (prec > 9U))
        {
            working[len++] = '0';
            prec--;
        }

        int whole = (int)value;
        double tmp = (value - whole) * pow10[prec];
        unsigned long frac = (unsigned long)tmp;
        diff = tmp - frac;

        if (diff > 0.5)
        {
            ++frac;
            // handle rollover, e.g. case 0.99 with prec 1 is 1.0
            if (frac >= pow10[prec])
            {
                frac = 0;
                ++whole;
            }
        }
        else if (diff < 0.5)
        {
        }
        else if ((frac == 0U) || (frac & 1U))
        {
            // if halfway, round up if odd OR if last digit is 0
            ++frac;
        }

        if (prec == 0U)
        {
            diff = value - (double)whole;
            if ((!(diff < 0.5) || (diff > 0.5)) && (whole & 1))
            {
                // exactly 0.5 and ODD, then round up
                // 1.5 -> 2, but 2.5 -> 2
                ++whole;
            }
        }
        else
        {
            unsigned int count = prec;
            // now do fractional part, as an unsigned number
            while (len < PRINTF_FTOA_BUFFER_SIZE)
            {
                --count;
                working[len++] = (char)(48U + (frac % 10U));
                if (!(frac /= 10U))
                {
                    break;
                }
            }
            // add extra 0s
            while ((len < PRINTF_FTOA_BUFFER_SIZE) && (count-- > 0U))
            {
                working[len++] = '0';
            }
            if (len < PRINTF_FTOA_BUFFER_SIZE)
            {
                // add decimal
                working[len++] = '.';
            }
        }

        // do whole part, number is reversed
        while (len < PRINTF_FTOA_BUFFER_SIZE)
        {
            working[len++] = (char)(48 + (whole % 10));
            if (!(whole /= 10))
            {
                break;
            }
        }

        // pad leading zeros
        if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD))
        {
            if (width && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))))
            {
                width--;
            }
            while ((len < width) && (len < PRINTF_FTOA_BUFFER_SIZE))
            {
                working[len++] = '0';
            }
        }

        if (len < PRINTF_FTOA_BUFFER_SIZE)
        {
            if (negative)
            {
                working[len++] = '-';
            }
            else if (flags & FLAGS_PLUS)
            {
                working[len++] = '+'; // ignore the space if the '+' exists
            }
            else if (flags & FLAGS_SPACE)
            {
                working[len++] = ' ';
            }
        }

        return _out_rev(out, buffer, idx, maxlen, working, len, width, flags);
    }

#if defined(PRINTF_SUPPORT_EXPONENTIAL)
    // internal ftoa variant for exponential floating-point type, contributed by Martijn Jasperse <m.jasperse@gmail.com>
    static size_t _etoa(out_fct_type out, char *buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags)
    {
        // check for NaN and special values
        if ((value != value) || (value > DBL_MAX) || (value < -DBL_MAX))
        {
            return format_float(out, buffer, idx, maxlen, value, prec, width, flags);
        }

        // determine the sign
        const bool negative = value < 0;
        if (negative)
        {
            value = -value;
        }

        // default precision
        if (!(flags & FLAGS_PRECISION))
        {
            prec = PRINTF_DEFAULT_FLOAT_PRECISION;
        }

        // determine the decimal exponent
        // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
        union
        {
            uint64_t U;
            double F;
        } conv;

        conv.F = value;
        int exp2 = (int)((conv.U >> 52U) & 0x07FFU) - 1023;          // effectively log2
        conv.U = (conv.U & ((1ULL << 52U) - 1U)) | (1023ULL << 52U); // drop the exponent so conv.F is now in [1,2)
        // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
        int expval = (int)(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
        // now we want to compute 10^expval but we want to be sure it won't overflow
        exp2 = (int)(expval * 3.321928094887362 + 0.5);
        const double z = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
        const double z2 = z * z;
        conv.U = (uint64_t)(exp2 + 1023) << 52U;
        // compute exp(z) using continued fractions, see https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
        conv.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
        // correct for rounding errors
        if (value < conv.F)
        {
            expval--;
            conv.F /= 10;
        }

        // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
        unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;

        // in "%g" mode, "prec" is the number of *significant figures* not decimals
        if (flags & FLAGS_ADAPT_EXP)
        {
            // do we want to fall-back to "%f" mode?
            if ((value >= 1e-4) && (value < 1e6))
            {
                if ((int)prec > expval)
                {
                    prec = (unsigned)((int)prec - expval - 1);
                }
                else
                {
                    prec = 0;
                }
                flags |= FLAGS_PRECISION; // make sure _ftoa respects precision
                // no characters in exponent
                minwidth = 0U;
                expval = 0;
            }
            else
            {
                // we use one sigfig for the whole part
                if ((prec > 0) && (flags & FLAGS_PRECISION))
                {
                    --prec;
                }
            }
        }

        // will everything fit?
        unsigned int fwidth = width;
        if (width > minwidth)
        {
            // we didn't fall-back so subtract the characters required for the exponent
            fwidth -= minwidth;
        }
        else
        {
            // not enough characters, so go back to default sizing
            fwidth = 0U;
        }
        if ((flags & FLAGS_LEFT) && minwidth)
        {
            // if we're padding on the right, DON'T pad the floating part
            fwidth = 0U;
        }

        // rescale the float value
        if (expval)
        {
            value /= conv.F;
        }

        // output the floating part
        const size_t start_idx = idx;
        idx = format_float(out, buffer, idx, maxlen, negative ? -value : value, prec, fwidth, flags & ~FLAGS_ADAPT_EXP);

        // output the exponent part
        if (minwidth)
        {
            // output the exponential symbol
            out((flags & FLAGS_UPPERCASE) ? 'E' : 'e', buffer, idx++, maxlen);
            // output the exponent value
            idx = format_long(out, buffer, idx, maxlen, (expval < 0) ? -expval : expval, expval < 0, 10, 0, minwidth - 1, FLAGS_ZEROPAD | FLAGS_PLUS);
            // might need to right-pad spaces
            if (flags & FLAGS_LEFT)
            {
                while (idx - start_idx < width)
                    out(' ', buffer, idx++, maxlen);
            }
        }
        return idx;
    }
#endif // PRINTF_SUPPORT_EXPONENTIAL
#endif // PRINTF_SUPPORT_FLOAT

    template <typename T, typename As>
    static constexpr bool is_formattable_as_v = same_as<T,As>;

    template <typename T, typename As>
    concept formattable_as = is_formattable_as_v<T,As>;

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
                // constexpr varg() = default;
                constexpr virtual ~varg() = default;
                constexpr virtual void format(output& out, field const& f) const = 0;
            };
        }

        template <typename T>
        struct arg_t final : public virtual varg {
            using type = T;
            const T value;
            arg_t(T v) : value{v} {}            
            // arg_t(T& v) : value{v} {}
            void format(output& out, field const& f) const final
            {
                _format(value, out, f);
            }
        };

        template <typename T>
        struct arg_t<T&> final : public virtual varg {
            using type = T;
            T const& value;
            arg_t(T const& v) : value{v} {}
            void format(output& out, field const& f) const final
            {
                _format(value, out, f);
            }
        };

        template <typename T>
        arg_t(T) -> arg_t<T>;

        // template <typename T>
        // arg_t(T&) -> arg_t<T&>;

        template <typename... Ts>
        struct arg_storage {
            static constexpr size_t n_args = sizeof...(Ts);
            using arg_storage_t = utl::tuple<arg_t<Ts>...>;
            using varg_storage_t = utl::array<varg const* const,n_args>;

            const arg_storage_t args;
            const varg_storage_t vargs;

            template <size_t... N>
            constexpr arg_storage(std::index_sequence<N...>, std::convertible_to<Ts>auto&&... args_)
            : args{arg_t<Ts>{std::forward<decltype(args_)>(args_)}...},
                vargs{&utl::get<N>(args)...}
            {}

            constexpr arg_storage(std::convertible_to<Ts> auto&&... args_) 
            : arg_storage{std::make_index_sequence<n_args>{}, std::forward<decltype(args_)>(args_)...}
            {}
        };

        template <typename... Ts>
        arg_storage(Ts...) -> arg_storage<Ts...>;

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

        template <typename... Ts>
        constexpr utl::tuple<arglist,arg_storage<Ts...>> make_args(Ts&&... vs)
        {
            auto storage = arg_storage{std::forward<Ts>(vs)...};
            return {arglist{storage},storage};
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

template <fmt::formattable... Args>
constexpr auto format_into(utl::ranges::output_iterable<char> auto&& buffer, utl::string_view format, 
    Args&&... args)
{
    auto iter = utl::ranges::begin(buffer);
    auto end = utl::ranges::end(buffer);
    auto buffer_out = [&] (char c) {
        if(iter != end) *iter++ = c;
    };
    format_to(buffer_out, format, std::forward<Args>(args)...);
    return iter;
}

template <callable<void,char> F, fmt::formattable... Args>
constexpr void format_to(F&& out, utl::string_view format, Args&&... args)
{
    const auto arg_storage = fmt::detail::wrap_args(std::forward<Args>(args)...);
    auto arg_view = fmt::detail::erase_args(arg_storage);
    // auto [arg_view, arg_storage] = fmt::detail::make_args(std::forward<Args>(args)...);
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
    // auto [arg_view, arg_storage] = fmt::detail::make_args(std::forward<Args>(args)...);
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

    constexpr format_options default_pointer_options{ 
        .fill = ' ', 
        .align = alignment::RIGHT,
        .sign = format_options::signs::NEGATIVE_ONLY,
        .width = 0,
        .presentation = 'x'
    };

    constexpr format_options default_char_options{ 
        .fill = ' ', 
        .align = alignment::RIGHT,
        .sign = format_options::signs::NEGATIVE_ONLY,
        .width = 0,
        .presentation = 'c'
    };

    constexpr format_options default_unsigned_char_options{ 
        .fill = ' ', 
        .align = alignment::RIGHT,
        .sign = format_options::signs::NEGATIVE_ONLY,
        .width = 0,
        .presentation = 'd'
    };

    //FIXME: need to account for rvalues & lvalues in these, and decay pointers.

    template <typename T> //specialization for char types
    static constexpr bool is_formattable_as_v<T,const char> = contains_v<T, char, const char, 
        signed char, const signed char>;

    template <typename T> //specialization for unsigned integer types
    static constexpr bool is_formattable_as_v<T,const unsigned long> = contains_v<T, 
        unsigned short, const unsigned short, unsigned int, const unsigned int,
        unsigned long, const unsigned long>;

    template <typename T> //specialization for signed integer types
    static constexpr bool is_formattable_as_v<T,const long long> = contains_v<T, 
        short, const short, int, const int, long, const long, long long,
        const long long>;

    template <typename T> //specialization for cstring types
    static constexpr bool is_formattable_as_v<T,const char*> = contains_v<std::decay_t<T>, 
        char*, const char*, signed char*, const signed char*>;

    template <typename T, typename As> //specialization for pointers
    static constexpr bool is_formattable_as_v<T,As*> = is_same_v<T,As*>;

    template <typename T> //specialization for unsigned char pointers
    static constexpr bool is_formattable_as_v<T,const unsigned char*> = contains_v<std::decay_t<T>, 
        unsigned char*, const unsigned char*>;

    
    //********************** Number types

    constexpr void _format(formattable_as<const long long> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        auto arg_long = static_cast<long>(arg);
        format_ulong(out, static_cast<const unsigned long>(arg_long < 0 ? -arg_long : arg_long), arg < 0, options);
    }

    constexpr void _format(formattable_as<const unsigned long> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_int_options);
        format_ulong(out, static_cast<const unsigned long>(arg), false, options);
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


    //********************** Pointer types

    inline void _format(const void* arg, output& out, field const& f)
    {
        auto defaults = default_int_options;
        defaults.presentation = 'p';
        auto options = parse_format_options(f.spec, defaults);
        format_ulong(out, reinterpret_cast<unsigned long>(arg), false, options);
    }

    inline void _format(std::nullptr_t, output& out, field const& f)
    {
        _format(reinterpret_cast<void*>(0),out,f);
    }

    
    //********************** String types

    inline constexpr void _format(utl::string_view arg, output& out, field const& f)
    {      
        auto options = parse_format_options(f.spec, {
            .fill = ' ',
            .align = alignment::LEFT,
            .presentation = 's'
        });
        fmt::align_pad_out(out, arg, options.fill, options.align, options.width, options.precision);
    }

    template<size_t N>
    constexpr void _format(utl::same_as<utl::string<N>> auto arg, output& out, field const& f)
    {
        _format(utl::string_view{arg.data(),arg.length()},out,f);
    }

    constexpr void _format(formattable_as<const char*> auto arg, output& out, field const& f)
    {
        _format(utl::string_view{reinterpret_cast<const char*>(arg)},out,f);
    }

    inline void _format(formattable_as<const unsigned char*> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, {.presentation = 'p'});
        if(options.presentation == 's') {
            _format(utl::string_view{reinterpret_cast<const char*>(arg)},out,f);
        } else {
            _format(reinterpret_cast<const void*>(arg),out,f);
        }        
    }

    
    //********************** Character types

    constexpr void _format(formattable_as<const char> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_char_options);
        switch(options.presentation) {
            case 'c':
                _format(utl::string_view{&arg,1},out,f);
                break;
            default:                
                format_ulong(out, static_cast<unsigned long>(arg < 0 ? -arg : arg), arg < 0, options);
                break;
        }        
    }

    constexpr void _format(formattable_as<unsigned char> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, default_unsigned_char_options);
        switch(options.presentation) {
            case 'c':
                _format(static_cast<const char>(arg),out,f);
                break;
            default:                
                format_ulong(out, static_cast<unsigned long>(arg), false, options);
                break;
        }
    }

    constexpr void _format(formattable_as<bool> auto arg, output& out, field const& f)
    {
        auto options = parse_format_options(f.spec, {.presentation = 's'});
        switch(options.presentation) {
            case 's':
                _format(utl::string_view{arg ? "true" : "false"},out,f);
                break;
            default:
                _format(static_cast<unsigned int>(arg),out,f);
                break;
        }
    }

} //namespace fmt

} //namespace utl
