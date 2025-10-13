#ifndef ARDUINO_LOGGER_H
#define ARDUINO_LOGGER_H
#include "Logging.h"
#include "Helpers.h"
#include <Arduino.h>

// SerialSink: concrete sink that writes to Serial.
// - We ignore isErr and use a single serial stream.
// - flush() waits for outgoing data to be transmitted.
struct SerialSink final : ILogSink
{
    void write(const char *s, size_t n, bool /*isErr*/) override
    {
        Serial.write(reinterpret_cast<const uint8_t *>(s), n);
    }
    void flush() override { Serial.flush(); }
};

// Helper: pad_and_write
// - Pads with padChar to satisfy a minimum field width.
// - If leftAlign is true, pad on the right; otherwise pad on the left.
// - Returns total characters emitted (padding + body).
inline size_t pad_and_write(LoggerCore &core, LogLevel lvl,
                            const char *body, size_t len,
                            int width, bool leftAlign, char padChar)
{
    if (width <= 0 || (int)len >= width)
    {
        core.write(body, len, lvl == LogLevel::Warning);
        return len;
    }
    int pad = width - (int)len;
    if (!leftAlign)
    {
        for (int i = 0; i < pad; ++i)
            core.write(&padChar, 1, lvl == LogLevel::Warning);
        core.write(body, len, lvl == LogLevel::Warning);
    }
    else
    {
        core.write(body, len, lvl == LogLevel::Warning);
        for (int i = 0; i < pad; ++i)
            core.write(&padChar, 1, lvl == LogLevel::Warning);
    }
    return (size_t)width;
}

// ArduinoLogger: polymorphic ILogger implementation for Arduino
// - Uses LoggerCore for prefix, buffering, and sink I/O
// - Implements a small printf-like parser that supports:
//   %%, %s, %c, %d, %i, %u, %ld, %lu, %f
//   Optional flags (- + space 0 #), width, and precision are CONSUMED to keep va_list aligned.
//   Padding is implemented for %s, %c, integers, and %f.
//   Precision for %f is honored via dtostrf_shim. We ignore precision for integers to stay small.
// - Do NOT include '\n' in format strings; newline is appended by core_.end(lvl).
class ArduinoLogger final : public ILogger
{
public:
    explicit ArduinoLogger(Timing &t, ILogSink &s) : core_(t, s) {}
    void setRuntimeLevel(LogLevel lvl) override { core_.setRuntimeLevel(lvl); }

    void logf(LogLevel lvl, const char *fmt, ...) override
    {
        if (!core_.begin(lvl))
            return; // Early out if filtered by level
        va_list ap;
        va_start(ap, fmt);
        vformat_(lvl, fmt, ap); // Parse and emit formatted body
        va_end(ap);
        core_.end(lvl); // Append newline and finalize the line
    }

    void flush() override { core_.flush(); } // Call once per frame if batching

private:
    // vformat_: small, safe subset of printf for Arduino
    // - Consumes flags, width, precision, and 'l' length to keep va_list aligned
    // - Emits unknown specifiers literally (so logs don't silently drop content)
    // - Uses snprintf for integers (keeps code small) and dtostrf_shim for floats
    void vformat_(LogLevel lvl, const char *fmt, va_list ap)
    {
        char nbuf[32]; // integer scratch
        char fbuf[24]; // float scratch

        while (*fmt)
        {
            // Fast path: copy ordinary characters
            if (*fmt != '%')
            {
                core_.write(fmt, 1, lvl == LogLevel::Warning);
                ++fmt;
                continue; // Important: continue, do not return
            }

            const char *start = fmt++; // Points at '%'

            // "%%" → literal '%'
            if (*fmt == '%')
            {
                core_.write("%", 1, lvl == LogLevel::Warning);
                ++fmt;
                continue;
            }

            // Parse minimal printf grammar: [%][flags][-+ 0#]* [width]* [.prec]* [length] spec

            // 1) Flags
            bool leftAlign = false;
            bool zeroPad = false;
            while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '0' || *fmt == '#')
            {
                if (*fmt == '-')
                    leftAlign = true;
                if (*fmt == '0')
                    zeroPad = true;
                ++fmt;
            }

            // 2) Width (digits)
            int width = -1;
            if (*fmt >= '0' && *fmt <= '9')
            {
                int w = 0;
                while (*fmt >= '0' && *fmt <= '9')
                {
                    w = w * 10 + (*fmt - '0');
                    ++fmt;
                }
                width = w;
            }

            // 3) Precision (for %f only; default 3)
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

            // 4) Length (we support only 'l' for long / unsigned long)
            bool longFlag = false;
            if (*fmt == 'l')
            {
                longFlag = true;
                ++fmt;
            }

            // 5) Specifier
            char spec = *fmt ? *fmt++ : '\0';
            switch (spec)
            {
            case 's':
            {
                // String (promoted to const char*)
                const char *s = va_arg(ap, const char *);
                if (!s)
                    s = "(null)";
                size_t len = strlen(s);
                pad_and_write(core_, lvl, s, len, width, leftAlign, ' ');
            }
            break;

            case 'c':
            {
                // Char (promoted to int in varargs)
                char ch = (char)va_arg(ap, int);
                pad_and_write(core_, lvl, &ch, 1, width, leftAlign, ' ');
            }
            break;

            case 'd':
            case 'i':
            {
                // Signed integer (int or long)
                if (longFlag)
                {
                    long v = va_arg(ap, long);
                    int n = snprintf(nbuf, sizeof nbuf, "%ld", v);
                    if (n > 0)
                        pad_and_write(core_, lvl, nbuf, (size_t)n, width,
                                      leftAlign, (zeroPad && !leftAlign) ? '0' : ' ');
                }
                else
                {
                    int v = va_arg(ap, int);
                    int n = snprintf(nbuf, sizeof nbuf, "%d", v);
                    if (n > 0)
                        pad_and_write(core_, lvl, nbuf, (size_t)n, width,
                                      leftAlign, (zeroPad && !leftAlign) ? '0' : ' ');
                }
            }
            break;

            case 'u':
            {
                // Unsigned integer (unsigned int or unsigned long)
                if (longFlag)
                {
                    unsigned long v = va_arg(ap, unsigned long);
                    int n = snprintf(nbuf, sizeof nbuf, "%lu", v);
                    if (n > 0)
                        pad_and_write(core_, lvl, nbuf, (size_t)n, width,
                                      leftAlign, (zeroPad && !leftAlign) ? '0' : ' ');
                }
                else
                {
                    unsigned int v = va_arg(ap, unsigned int);
                    int n = snprintf(nbuf, sizeof nbuf, "%u", v);
                    if (n > 0)
                        pad_and_write(core_, lvl, nbuf, (size_t)n, width,
                                      leftAlign, (zeroPad && !leftAlign) ? '0' : ' ');
                }
            }
            break;

            case 'f':
            {
                // Float (as double in varargs); precision honored, width handled with padding
                double v = va_arg(ap, double);
                Helpers::dtostrf_shim(v, 0, (unsigned char)prec, fbuf);
                size_t len = strlen(fbuf);

                // Handle zero-pad for negative numbers so zeros come after the sign: -001.23
                if (zeroPad && !leftAlign && width > 0 && fbuf[0] == '-')
                {
                    const char sign = '-';
                    const char *rest = fbuf + 1;
                    size_t restLen = len - 1;
                    int pad = width - 1 - (int)restLen;
                    if (pad > 0)
                    {
                        // Sign first
                        core_.write(&sign, 1, lvl == LogLevel::Warning);
                        // Zero padding
                        for (int i = 0; i < pad; ++i)
                        {
                            char ch = '0';
                            core_.write(&ch, 1, lvl == LogLevel::Warning);
                        }
                        // Remainder
                        core_.write(rest, restLen, lvl == LogLevel::Warning);
                        break;
                    }
                    // If no pad needed, fall through to generic padding below
                }

                // Generic left/right pad
                pad_and_write(core_, lvl, fbuf, len, width,
                              leftAlign, (zeroPad && !leftAlign) ? '0' : ' ');
            }
            break;

            default:
            {
                // Unknown/unsupported → emit literal format sequence so content isn't lost
                core_.write(start, (size_t)(fmt - start), lvl == LogLevel::Warning);
            }
            break;
            }
        }
    }

private:
    LoggerCore core_;
};

#endif // ARDUINO_LOGGER_H
