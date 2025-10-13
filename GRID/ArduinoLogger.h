#ifndef ARDUINO_LOGGER_H
#define ARDUINO_LOGGER_H
#include "Logging.h"
#include "Helpers.h"
#include <Arduino.h>

struct SerialSink final : ILogSink
{
    void write(const char *s, size_t n, bool /*isErr*/) override
    {
        Serial.write(reinterpret_cast<const uint8_t *>(s), n);
    }
    void flush() override { Serial.flush(); }
};

class ArduinoLogger final : public ILogger
{
public:
    ArduinoLogger(Timing &t, ILogSink &s) : core_(t, s) {}
    void setRuntimeLevel(LogLevel lvl) override { core_.setRuntimeLevel(lvl); }

    void logf(LogLevel lvl, const char *fmt, ...) override
    {
        if (!core_.begin(lvl))
            return;
        va_list ap;
        va_start(ap, fmt);
        vformat_(lvl, fmt, ap);
        va_end(ap);
        core_.end(lvl);
    }
    void flush() override { core_.flush(); }

private:
    // Minimal printf-style parser and formatter for Arduino.
    // Supported: %%, %s, %c, %d, %i, %u, %ld, %lu, %f with optional flags, width, and precision.
    // Notes:
    // - We CONSUME flags and width to keep va_list aligned, but IGNORE padding in output.
    // - Precision for %f is honored, width is ignored.
    // - Unknown specifiers are emitted literally to avoid data loss.
    // - Output goes through LoggerCore::write to reuse buffering and newline handling.
    void vformat_(LogLevel lvl, const char *fmt, va_list ap)
    {
        char nbuf[32]; // scratch for integers
        char fbuf[24]; // scratch for floats

        while (*fmt)
        {
            // Fast path: ordinary characters → copy one byte and continue
            if (*fmt != '%')
            {
                core_.write(fmt, 1, lvl == LogLevel::Warning);
                ++fmt;
                continue;
            }

            // Remember where this conversion started; useful if we need to emit literally
            const char *start = fmt++;

            // "%%" → literal '%'
            if (*fmt == '%')
            {
                core_.write("%", 1, lvl == LogLevel::Warning);
                ++fmt;
                continue;
            }

            // --- Parse a tiny subset of printf format: [%][flags]*[width][.prec][length]spec ---

            // 1) Flags: we accept but ignore them for output; consuming keeps va_list alignment correct.
            // Recognized flags: '-', '+', ' ', '0', '#'
            while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '0' || *fmt == '#')
            {
                ++fmt;
            }

            // 2) Width: consume digits; we don’t use width for padding, but must consume to align subsequent reads.
            int width = -1;
            if (*fmt >= '0' && *fmt <= '9')
            {
                int w = 0;
                while (*fmt >= '0' && *fmt <= '9')
                {
                    w = w * 10 + (*fmt - '0');
                    ++fmt;
                }
                width = w; // intentionally unused
            }

            // 3) Precision: default to 3 for %f; for integers we ignore precision for simplicity.
            int prec = 3;
            if (*fmt == '.')
            {
                ++fmt;
                int p = 0;
                while (*fmt >= '0' && *fmt <= '9')
                {
                    p = p * 10 + (*fmt - '0');
                    ++fmt;
                }
                prec = p;
            }

            // 4) Length: we support only 'l' (long) for %ld/%lu.
            bool longFlag = false;
            if (*fmt == 'l')
            {
                longFlag = true;
                ++fmt;
            }

            // 5) Specifier: choose how to format and CONSUME the correct type from va_list.
            char spec = *fmt ? *fmt++ : '\0';
            switch (spec)
            {
            case 's':
            {
                // String: print literal or "(null)"
                const char *s = va_arg(ap, const char *);
                if (!s)
                    s = "(null)";
                core_.write(s, strlen(s), lvl == LogLevel::Warning);
            }
            break;

            case 'c':
            {
                // Char is promoted to int in varargs
                char ch = (char)va_arg(ap, int);
                core_.write(&ch, 1, lvl == LogLevel::Warning);
            }
            break;

            case 'd':
            case 'i':
            {
                // Signed integers: %d, %i, %ld
                if (longFlag)
                {
                    long v = va_arg(ap, long);
                    int n = snprintf(nbuf, sizeof nbuf, "%ld", v);
                    if (n > 0)
                        core_.write(nbuf, (size_t)n, lvl == LogLevel::Warning);
                }
                else
                {
                    int v = va_arg(ap, int);
                    int n = snprintf(nbuf, sizeof nbuf, "%d", v);
                    if (n > 0)
                        core_.write(nbuf, (size_t)n, lvl == LogLevel::Warning);
                }
            }
            break;

            case 'u':
            {
                // Unsigned integers: %u, %lu
                if (longFlag)
                {
                    unsigned long v = va_arg(ap, unsigned long);
                    int n = snprintf(nbuf, sizeof nbuf, "%lu", v);
                    if (n > 0)
                        core_.write(nbuf, (size_t)n, lvl == LogLevel::Warning);
                }
                else
                {
                    unsigned int v = va_arg(ap, unsigned int);
                    int n = snprintf(nbuf, sizeof nbuf, "%u", v);
                    if (n > 0)
                        core_.write(nbuf, (size_t)n, lvl == LogLevel::Warning);
                }
            }
            break;

            case 'f':
            {
                // Floats are passed as double in varargs. We honor precision and ignore width.
                double v = va_arg(ap, double);
                Helpers::dtostrf_shim(v, 0, (unsigned char)prec, fbuf);
                core_.write(fbuf, strlen(fbuf), lvl == LogLevel::Warning);
            }
            break;

            default:
            {
                // Unsupported or malformed specifier: emit the raw format sequence to avoid losing information.
                core_.write(start, (size_t)(fmt - start), lvl == LogLevel::Warning);
            }
            break;
            }
        }
    }

    LoggerCore core_;
};

#endif // ARDUINO_LOGGER_H
