// EmulationLogger.h
#ifndef EMULATION_LOGGER_H
#define EMULATION_LOGGER_H

#include "Logging.h"

struct StdoutSink final : ILogSink
{
    void write(const char *s, size_t n, bool isErr) override
    {
        FILE *f = isErr ? stderr : stdout;
        (void)fwrite(s, 1, n, f);
    }
    void flush() override
    {
        fflush(stdout);
        fflush(stderr);
    }
};

class EmulationLogger final : public ILogger
{
public:
    EmulationLogger(Timing &t, ILogSink &s) : core_(t, s) {}
    void setRuntimeLevel(LogLevel lvl) override { core_.setRuntimeLevel(lvl); }

    void logf(LogLevel lvl, const char *fmt, ...) override
    {
        if (!core_.begin(lvl))
            return;
        char buf[256];
        va_list ap;
        va_start(ap, fmt);
        int n = std::vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        if (n > 0)
            core_.write(buf, size_t(n < int(sizeof(buf)) ? n : int(sizeof(buf)) - 1), lvl == LogLevel::Warning);
        core_.end(lvl);
    }
    void flush() override { core_.flush(); }

private:
    LoggerCore core_;
};

#endif // EMULATION_LOGGER_H
