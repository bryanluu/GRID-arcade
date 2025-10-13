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
    // Minimal formatter: %%, %s, %c, %d, %i, %u, %ld, %lu, %f with optional %.Nf
    void vformat_(LogLevel lvl, const char *fmt, va_list ap)
    {
        char nbuf[32], fbuf[24];
        while (*fmt)
        {
            if (*fmt != '%')
            {
                core_.write(fmt, 1, lvl == LogLevel::Warning);
                ++fmt;
                continue;
            }
            const char *start = fmt++;
            if (*fmt == '%')
            {
                core_.write("%", 1, lvl == LogLevel::Warning);
                ++fmt;
                continue;
            }
            bool longFlag = false;
            int prec = 3;
            if (*fmt == 'l')
            {
                longFlag = true;
                ++fmt;
            }
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
            char spec = *fmt ? *fmt++ : '\0';
            switch (spec)
            {
            case 's':
            {
                const char *s = va_arg(ap, const char *);
                if (!s)
                    s = "(null)";
                core_.write(s, strlen(s), lvl == LogLevel::Warning);
            }
            break;
            case 'c':
            {
                char ch = (char)va_arg(ap, int);
                core_.write(&ch, 1, lvl == LogLevel::Warning);
            }
            break;
            case 'd':
            case 'i':
            {
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
                double v = va_arg(ap, double);
                char fbuf[24];
                Helpers::dtostrf_shim(v, 0, (unsigned char)prec, fbuf);
                core_.write(fbuf, strlen(fbuf), lvl == LogLevel::Warning);
            }
            break;
                break;
            default:
            { // emit unknown as-is
                core_.write(start, size_t(fmt - start), lvl == LogLevel::Warning);
            }
            break;
            }
        }
    }

    LoggerCore core_;
};

#endif // ARDUINO_LOGGER_H
