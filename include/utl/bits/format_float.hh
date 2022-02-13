
#pragma once

#include <utl/bits/format_options.hh>
#include <utl/bits/format_output.hh>

namespace utl::fmt {

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

} //namespace utl::fmt
