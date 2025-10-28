#include "ScoreData.h"
#include "Serializer.h"
#include "IStorage.h"
#include "Logging.h"

size_t ScoreData::toJSON(char *dst, size_t cap) const
{
    return Serializer::Score::toJSON(*this, dst, cap);
}

bool ScoreData::fromJSON(const char *src)
{
    return Serializer::Score::fromJSON(src, *this);
}

bool ScoreData::save(IStorage &storage, ILogger &log, const char *filename) const
{
    char json[kJsonCap]{};
    const size_t n = toJSON(json, sizeof(json));
    if (n == 0)
    {
        log.logf(LogLevel::Warning, "[%s] toJSON failed", kLogTag);
        return false;
    }
    if (storage.writeAll(filename, json, n))
    {
        log.logf(LogLevel::Debug, "Score Saved to '%s':\n%s", filename, json);
    }
    else
    {
        log.logf(LogLevel::Warning, "[%s] writeAll failed", kLogTag);
        return false;
    }
    log.logf(LogLevel::Info, "[%s] saved %u bytes to %s", kLogTag, static_cast<unsigned>(n), filename);
    return true;
}

bool ScoreData::load(IStorage &storage, ILogger &log, const char *filename)
{
    char buf[kReadCap]{};
    const auto r = storage.readAll(filename, buf, sizeof(buf));
    if (!r)
    {
        log.logf(LogLevel::Info, "[%s] %s not found or read failed", kLogTag, filename);
        return false;
    }
    if (fromJSON(buf))
    {
        log.logf(LogLevel::Debug, "Score Loaded from '%s':\n%s", filename, buf);
    }
    else
    {
        log.logf(LogLevel::Warning, "[%s] fromJSON failed", kLogTag);
        return false;
    }
    log.logf(LogLevel::Info, "[%s] loaded %d bytes from %s", kLogTag, r.bytes, filename);
    return true;
}
