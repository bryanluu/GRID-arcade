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
        // Minimal, no padding; handles NaN/Inf; rounds to 'prec' digits
        char *p = out;
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

        if (val < 0)
        {
            *p++ = '-';
            val = -val;
        }

        // integer part
        unsigned long ip = static_cast<unsigned long>(val);
        char ibuf[20];
        int i = 0;
        do
        {
            ibuf[i++] = char('0' + (ip % 10));
            ip /= 10;
        } while (ip && i < (int)sizeof(ibuf));
        for (int j = i - 1; j >= 0; --j)
            *p++ = ibuf[j];

        // fractional part
        if (prec == 0)
        {
            *p = '\0';
            return out;
        }
        *p++ = '.';
        const unsigned long pow10_table[] = {1ul, 10ul, 100ul, 1000ul, 10000ul, 100000ul, 1000000ul};
        unsigned char effPrec = prec > 6 ? 6 : prec; // keep it small
        unsigned long scale = pow10_table[effPrec];
        double frac = val - static_cast<unsigned long>(val);
        unsigned long f = static_cast<unsigned long>(frac * scale + 0.5); // rounded

        // write 'effPrec' digits with leading zeros
        char fbuf[8];
        for (int j = effPrec - 1; j >= 0; --j)
        {
            fbuf[j] = char('0' + (f % 10));
            f /= 10;
        }
        memcpy(p, fbuf, effPrec);
        p += effPrec;
        *p = '\0';
        return out;
    }
}

#endif // GRID_EMULATION
#endif // HELPERS_H
