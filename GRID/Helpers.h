#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint>
#include <cmath>

// Type
using millis_t = uint32_t; // convenience alias for time in milliseconds

// These helpers are needed because the defs are missing for either platform
namespace Helpers
{
    // Clamp value v to the range [lo..hi]
    template <typename T>
    inline T clamp(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); } // Arduino lacks std::clamp
    // Provide a random int between 0..range
    inline int random(int range) { return static_cast<float>(rand()) * range / RAND_MAX; }
    inline int random() { return rand(); }
}

// Helpers for use by the emulation
#ifdef GRID_EMULATION

#include <SDL.h>

using byte = uint8_t; // Mimic the byte alias in Arduino-land

#else

#include <Arduino.h>
namespace Helpers
{
    inline char *dtostrf_shim(double val, signed char /*width*/, unsigned char prec, char *out)
    {
        char *p = out;

        // Special values
        if (isnan(val))
        {
            strcpy(out, "nan");
            return out;
        }
        if (isinf(val))
        {
            strcpy(out, (val < 0) ? "-inf" : "inf");
            return out;
        }

        // Sign
        bool neg = val < 0.0;
        if (neg)
            val = -val;

        // Precision cap for small buffers
        unsigned char effPrec = prec > 6 ? 6 : prec;

        // Split into integer and fractional, then round fractional at requested precision
        unsigned long ip = (unsigned long)val;
        double frac = val - (double)ip;

        // Scale fractional and round
        static const unsigned long pow10[] = {1ul, 10ul, 100ul, 1000ul, 10000ul, 100000ul, 1000000ul};
        unsigned long scale = pow10[effPrec];
        unsigned long fs = (unsigned long)(frac * (double)scale + 0.5);

        // Carry: if fs == scale, bump integer part and zero fractional
        if (fs >= scale)
        {
            fs -= scale;
            ++ip;
        }

        // Emit sign
        if (neg)
            *p++ = '-';

        // Emit integer part (ensure at least one digit)
        char ibuf[20];
        int i = 0;
        do
        {
            ibuf[i++] = char('0' + (ip % 10));
            ip /= 10;
        } while (ip && i < (int)sizeof(ibuf));
        for (int j = i - 1; j >= 0; --j)
            *p++ = ibuf[j];

        // Optional fractional part
        if (effPrec > 0)
        {
            *p++ = '.';
            // Emit exactly effPrec digits with leading zeros
            char fbuf[8];
            for (int j = (int)effPrec - 1; j >= 0; --j)
            {
                fbuf[j] = char('0' + (fs % 10));
                fs /= 10;
            }
            memcpy(p, fbuf, effPrec);
            p += effPrec;
        }

        *p = '\0';
        return out;
    }
}

#endif // GRID_EMULATION
#endif // HELPERS_H
