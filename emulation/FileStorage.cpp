// FileStorage.cpp — desktop filesystem backend using ILogger
// Implementation details:
// - joinUnder(): avoid path concat bugs; std::filesystem handles separators portably.
// - writeAll(): temp write -> flush -> rename to final; best-effort cleanup on failure.
// - readAll(): sizes upfront; caller provides destination buffer to avoid backend allocation.
// - removeTree(): recursive delete; use carefully.
#include "FileStorage.h"
#include <filesystem>
#include <fstream>

// Bring in to call ILogger::logf.
#include "Logging.h" // defines ILogger and LogLevel

namespace fs = std::filesystem;

// Compose a path under base; std::filesystem handles separators portably.
static inline std::string joinUnder(const std::string &base, const char *rel)
{
    fs::path p = fs::path(base) / rel;
    return p.string();
}

void FileStorage::info(const char *msg)
{
    if (log)
        log->logf(LogLevel::Info, "[FileStorage] %s", msg);
}
void FileStorage::warn(const char *msg)
{
    if (log)
        log->logf(LogLevel::Warning, "[FileStorage] %s", msg);
}

void FileStorage::recoverTemp()
{
    const std::string tmp = joinUnder(baseDir, ".tmp_write");
    std::error_code ec;
    if (fs::exists(tmp, ec))
    {
        fs::remove(tmp, ec);
        if (!ec)
            info("cleaned stale temp");
    }
}

StorageResult FileStorage::init(const char *dir, ILogger *logger)
{
    log = logger;
    if (dir && *dir)
        baseDir = dir;

    // Ensure baseDir exists; create if missing.
    std::error_code ec;
    if (!fs::exists(baseDir, ec))
        fs::create_directories(baseDir, ec);
    if (ec)
    {
        warn("base dir create failed");
        return {StorageError::MountFailed, 0};
    }

    // Best-effort cleanup of a generic orphan temp.
    recoverTemp();

    info("ready");
    return {};
}

bool FileStorage::exists(const char *rel)
{
    std::error_code ec;
    return fs::exists(joinUnder(baseDir, rel), ec);
}

StorageResult FileStorage::writeAll(const char *rel, const void *src, size_t n)
{
    const std::string abs = joinUnder(baseDir, rel);
    const std::string tmp = joinUnder(baseDir, ".tmp_write");

    // 1) Write to temp file; RAII closes the stream even on early return.
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f)
        {
            warn("open temp failed");
            return {StorageError::OpenFailed, 0};
        }
        f.write((const char *)src, (std::streamsize)n);
        if (!f)
        {
            warn("write failed");
            return {StorageError::WriteFailed, 0};
        }
        f.flush(); // push data and metadata to the OS
        if (!f)
        {
            warn("flush failed");
            return {StorageError::SyncFailed, 0};
        }
    }

    // 2) Replace final via rename (same directory -> atomic on most filesystems).
    std::error_code ec;
    fs::rename(tmp, abs, ec);
    if (ec)
    {
        fs::remove(tmp, ec); // best-effort cleanup
        warn("rename failed");
        return {StorageError::RenameFailed, 0};
    }

    info("write ok");
    return {StorageError::None, (int32_t)n};
}

StorageResult FileStorage::readAll(const char *rel, void *dst, size_t cap)
{
    const std::string abs = joinUnder(baseDir, rel);
    std::ifstream f(abs, std::ios::binary); // RAII
    if (!f)
        return {StorageError::NotFound, 0};

    f.seekg(0, std::ios::end);
    size_t sz = (size_t)f.tellg();
    f.seekg(0);

    // Enforce caller-provided buffer limits; avoids hidden allocations.
    if (sz >= cap)
    {
        warn("buffer too small");
        return {StorageError::TooLarge, 0};
    }

    f.read((char *)dst, (std::streamsize)sz);
    if (!f)
    {
        warn("read failed");
        return {StorageError::ReadFailed, 0};
    }

    return {StorageError::None, (int32_t)sz};
}

StorageResult FileStorage::removeFile(const char *rel)
{
    std::error_code ec;
    fs::remove(joinUnder(baseDir, rel), ec);
    if (ec)
    {
        warn("remove failed");
        return {StorageError::RemoveFailed, 0};
    }
    info("removed");
    return {};
}

StorageResult FileStorage::removeTree(const char *relDir)
{
    std::error_code ec;
    fs::remove_all(joinUnder(baseDir, relDir), ec);
    if (ec)
    {
        warn("remove tree failed");
        return {StorageError::RemoveFailed, 0};
    }
    info("tree removed");
    return {};
}
