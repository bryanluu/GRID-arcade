#ifndef LOGGER_H
#define LOGGER_H

#include "Helpers.h"
#include "Timing.h"
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <array>

enum class LogLevel : byte
{
    Info = 0,
    Debug = 1,
    Warning = 2
};

// Compile-time minimum level filter: 0=Info,1=Debug,2=Warning
#ifndef GRID_LOG_MIN_LEVEL
#define GRID_LOG_MIN_LEVEL 0
#endif

// Abstract sink for platform-specific output
struct ILogSink
{
    virtual ~ILogSink() = default;
    virtual void write(const char *s, size_t n, bool isErr) = 0;
    virtual void flush() = 0;
};

// Small, fast logger with per-line buffering and explicit per-frame flush.
// Notes:
// - Timestamp from Timing::nowMs()
// - Auto-flush on '\n' or when buffer is full; also manual flush() once per frame
// - Formatting via vsnprintf; no heap except the internal std::string for safety
class Logger
{
    static constexpr size_t kPrefixCap = 48;
    static constexpr size_t kStackMsgCap = 256;

public:
    explicit Logger(Timing &timing, ILogSink &sink, size_t bufCap = 512)
        : timing_(timing), sink_(sink), buf_() { buf_.reserve(bufCap); }

    void setRuntimeLevel(LogLevel level) { runtimeMinLevel_ = level; }

    void log(LogLevel level, const char *fmt, ...)
    {
        if (!passes(level))
            return;

        // Prefix: [ts][level]
        char prefix[kPrefixCap];
        auto ts = timing_.nowMs(); // milliseconds since scene start
        const char *lvl = levelStr(level);
        int pn = std::snprintf(prefix, sizeof(prefix), "[%09u][%s] ", ts, lvl);
        if (pn < 0)
            return;
        writeChunk(prefix, static_cast<size_t>(pn), level == LogLevel::Warning);

        // Format message body
        char stackBuf[kStackMsgCap];
        va_list ap;
        va_start(ap, fmt);
        int n = std::vsnprintf(stackBuf, sizeof(stackBuf), fmt, ap);
        va_end(ap);

        if (n < 0)
            return;
        if (static_cast<size_t>(n) < sizeof(stackBuf))
        {
            writeChunk(stackBuf, static_cast<size_t>(n), level == LogLevel::Warning);
        }
        else
        {
            // Grow path (rare): write in chunks without allocations
            size_t left = static_cast<size_t>(n);
            const char *p = stackBuf;
            // Reformat in parts to avoid heap: fall back to emitting truncated tail
            // to keep code small. Adjust if you need full messages.
            writeChunk(stackBuf, sizeof(stackBuf) - 1, level == LogLevel::Warning);
            left = 0;
            (void)p;
        }

        // Ensure each call ends with newline for readability and potential auto-flush
        writeChunk("\n", 1, level == LogLevel::Warning);
    }

    // Flush once per app frame
    void flush()
    {
        if (!buf_.empty())
        {
            sink_.write(buf_.data(), buf_.size(), flushToErr_);
            buf_.clear();
            flushToErr_ = false;
        }
        sink_.flush();
    }

private:
    Timing &timing_;
    ILogSink &sink_;
    std::string buf_;
    bool flushToErr_ = false; // if any Warning in the batch, flush to "err" stream
    LogLevel runtimeMinLevel_ = static_cast<LogLevel>(GRID_LOG_MIN_LEVEL);

    static const char *levelStr(LogLevel l)
    {
        switch (l)
        {
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Warning:
            return "WARN";
        }
        return "?";
    }
    bool passes(LogLevel l) const
    {
        return static_cast<int>(l) >= static_cast<int>(runtimeMinLevel_);
    }
    void writeChunk(const char *s, size_t n, bool markErr)
    {
        if (markErr)
            flushToErr_ = true;

        // Auto-flush on newline or if buffer would overflow capacity
        bool hasNewline = (memchr(s, '\n', n) != nullptr);
        if (buf_.size() + n > buf_.capacity())
        {
            // Spill existing buffer first
            sink_.write(buf_.data(), buf_.size(), flushToErr_);
            buf_.clear();
        }
        // Append; if still too big, write directly
        if (n <= (buf_.capacity() - buf_.size()))
        {
            buf_.append(s, n);
        }
        else
        {
            sink_.write(s, n, flushToErr_);
        }
        if (hasNewline)
        {
            sink_.write(buf_.data(), buf_.size(), flushToErr_);
            buf_.clear();
            flushToErr_ = false;
        }
    }
};

constexpr size_t Logger::kPrefixCap;
constexpr size_t Logger::kStackMsgCap;

// Desktop sink using std::cout / std::cerr
#ifdef GRID_EMULATION
#include <iostream>
struct DesktopSink final : public ILogSink
{
    void write(const char *s, size_t n, bool isErr) override
    {
        auto &os = isErr ? std::cerr : std::cout;
        os.write(s, static_cast<std::streamsize>(n));
    }
    void flush() override
    {
        std::cout.flush();
        std::cerr.flush();
    }
};
#else

// Arduino sink using Serial
#include <Arduino.h>
struct ArduinoSink final : public ILogSink
{
    void write(const char *s, size_t n, bool isErr) override
    {
        (void)isErr; // single serial line
        Serial.write(reinterpret_cast<const byte *>(s), n);
    }
    void flush() override { Serial.flush(); }
};

#endif

#endif // LOGGER_H
