// FileStorage.cpp — desktop filesystem backend
// Implementation details:
// - joinUnder(): avoids path concatenation bugs and platform differences.
// - writeAll(): temp write -> flush -> rename to final, then cleanup on error.
// - readAll(): sizes upfront; caller provides destination buffer to avoid backend allocation.
// - removeTree(): recursive deletion; use carefully.
#include "FileStorage.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// Compose a path under base; std::filesystem handles separators portably.
static inline std::string joinUnder(const std::string &base, const char *rel)
{
    fs::path p = fs::path(base) / rel;
    return p.string();
}

void FileStorage::recoverTemp()
{
    const std::string tmp = joinUnder(baseDir, ".tmp_write");
    std::error_code ec;
    if (fs::exists(tmp, ec))
        fs::remove(tmp, ec);
}

StorageResult FileStorage::init(const char *dir, StorageLogFn logger)
{
    log = logger;
    if (dir && *dir)
        baseDir = dir;

    // Ensure base directory exists; create if needed.
    std::error_code ec;
    if (!fs::exists(baseDir, ec))
        fs::create_directories(baseDir, ec);
    if (ec)
        return {StorageError::MountFailed, 0};

    // Best-effort cleanup of a generic orphan temp.
    recoverTemp();

    if (log)
        log("[FileStorage] ready");
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

    // 1) Write to temp file
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc); // RAII close on scope exit
        if (!f)
            return {StorageError::OpenFailed, 0};
        f.write((const char *)src, (std::streamsize)n);
        if (!f)
            return {StorageError::WriteFailed, 0};
        f.flush(); // push data and metadata; still OS-buffered but durable enough for our scope
        if (!f)
            return {StorageError::SyncFailed, 0};
    }

    // 2) Replace final via rename (same directory -> atomic on most filesystems)
    std::error_code ec;
    fs::rename(tmp, abs, ec);
    if (ec)
    {
        fs::remove(tmp, ec); // best-effort cleanup
        return {StorageError::RenameFailed, 0};
    }
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
        return {StorageError::TooLarge, 0};

    f.read((char *)dst, (std::streamsize)sz);
    if (!f)
        return {StorageError::ReadFailed, 0};

    return {StorageError::None, (int32_t)sz};
}

StorageResult FileStorage::removeFile(const char *rel)
{
    std::error_code ec;
    fs::remove(joinUnder(baseDir, rel), ec);
    return ec ? StorageResult{StorageError::RemoveFailed, 0} : StorageResult{};
}

StorageResult FileStorage::removeTree(const char *relDir)
{
    std::error_code ec;
    fs::remove_all(joinUnder(baseDir, relDir), ec);
    return ec ? StorageResult{StorageError::RemoveFailed, 0} : StorageResult{};
}
