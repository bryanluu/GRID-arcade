#ifndef LOGGER_CORE_H
#define LOGGER_CORE_H

// Logging.h
#pragma once
#include "Timing.h"
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

// Higher means more logs
enum class LogLevel : uint8_t
{
    Info = 0,
    Warning = 1,
    Debug = 2
};

struct ILogSink
{
    virtual ~ILogSink() = default;
    virtual void write(const char *s, size_t n, bool isErr) = 0;
    virtual void flush() = 0;
};

// Polymorphic API for AppContext
struct ILogger
{
    virtual ~ILogger() = default;
    virtual void setRuntimeLevel(LogLevel) = 0;
    // printf-style. Do not include a trailing '\n' (a newline is added automatically)
    virtual void logf(LogLevel level, const char *fmt, ...) = 0;
    virtual void flush() = 0;
};

// Non-virtual core that handles timestamps, buffering, and sink I/O
class LoggerCore
{
public:
    static constexpr size_t kPrefixCap = 48;
    explicit LoggerCore(Timing &timing, ILogSink &sink, size_t bufCap = 512)
        : timing_(timing), sink_(sink) { buf_.reserve(bufCap); }

    void setRuntimeLevel(LogLevel lvl) { maxLevel_ = lvl; }
    bool passes(LogLevel lvl) const { return int(lvl) <= int(maxLevel_); }

    bool begin(LogLevel lvl)
    {
        if (!passes(lvl))
            return false;
        char prefix[kPrefixCap];
        unsigned ts = timing_.nowMs();
        const char *L = levelStr(lvl);
        int pn = std::snprintf(prefix, sizeof(prefix), "[%09u][%s] ", ts, L);
        if (pn <= 0)
            return false;
        write(prefix, size_t(pn), lvl == LogLevel::Warning);
        return true;
    }
    void end(LogLevel lvl) { write("\n", 1, lvl == LogLevel::Warning); }

    void write(const char *s, size_t n, bool markErr)
    {
        if (markErr)
            flushToErr_ = true;
        bool hasNL = (memchr(s, '\n', n) != nullptr);
        if (buf_.size() + n > buf_.capacity())
        {
            sink_.write(buf_.data(), buf_.size(), flushToErr_);
            buf_.clear();
        }
        if (n <= (buf_.capacity() - buf_.size()))
            buf_.append(s, n);
        else
            sink_.write(s, n, flushToErr_);
        if (hasNL)
        {
            sink_.write(buf_.data(), buf_.size(), flushToErr_);
            buf_.clear();
            flushToErr_ = false;
        }
    }
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
    Timing &timing_;
    ILogSink &sink_;
    std::string buf_;
    bool flushToErr_{false};
    LogLevel maxLevel_{LogLevel(0)};
};

#endif // LOGGER_CORE_H
