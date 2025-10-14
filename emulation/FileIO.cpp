// FileIO.cpp — desktop filesystem backend
#include "FileIO.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static inline std::string joinUnder(const std::string &base, const char *rel)
{
    fs::path p = fs::path(base) / rel;
    return p.string();
}

void FileIO::recoverTemp()
{
    // Generic single-temp name; if you have multiple files, you can scan directory.
    const std::string tmp = joinUnder(baseDir, ".tmp_write");
    // No-op if not present; if present but final missing, caller will handle per-file.
    // Desktop code keeps it simple: cleanup leftover tmp.
    std::error_code ec;
    if (fs::exists(tmp, ec))
        fs::remove(tmp, ec);
}

IOResult FileIO::init(const char *dir, LogFn logger)
{
    log = logger;
    if (dir && *dir)
        baseDir = dir;
    std::error_code ec;
    if (!fs::exists(baseDir, ec))
        fs::create_directories(baseDir, ec);
    if (ec)
        return {IOError::MountFailed, 0};
    recoverTemp();
    if (log)
        log("[FileIO] ready");
    return {};
}

bool FileIO::exists(const char *rel)
{
    std::error_code ec;
    return fs::exists(joinUnder(baseDir, rel), ec);
}

IOResult FileIO::writeAll(const char *rel, const void *src, size_t n)
{
    const std::string abs = joinUnder(baseDir, rel);
    const std::string tmp = joinUnder(baseDir, ".tmp_write");

    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc); // RAII close
        if (!f)
            return {IOError::OpenFailed, 0};
        f.write((const char *)src, (std::streamsize)n);
        if (!f)
            return {IOError::WriteFailed, 0};
        f.flush();
        if (!f)
            return {IOError::SyncFailed, 0};
    }
    std::error_code ec;
    fs::rename(tmp, abs, ec);
    if (ec)
    {
        fs::remove(tmp, ec);
        return {IOError::RenameFailed, 0};
    }
    return {IOError::None, (int32_t)n};
}

IOResult FileIO::readAll(const char *rel, void *dst, size_t cap)
{
    const std::string abs = joinUnder(baseDir, rel);
    std::ifstream f(abs, std::ios::binary); // RAII
    if (!f)
        return {IOError::NotFound, 0};
    f.seekg(0, std::ios::end);
    size_t sz = (size_t)f.tellg();
    f.seekg(0);
    if (sz >= cap)
        return {IOError::TooLarge, 0};
    f.read((char *)dst, (std::streamsize)sz);
    if (!f)
        return {IOError::ReadFailed, 0};
    return {IOError::None, (int32_t)sz};
}

IOResult FileIO::removeFile(const char *rel)
{
    std::error_code ec;
    fs::remove(joinUnder(baseDir, rel), ec);
    return ec ? IOResult{IOError::RemoveFailed, 0} : IOResult{};
}

IOResult FileIO::removeTree(const char *relDir)
{
    std::error_code ec;
    fs::remove_all(joinUnder(baseDir, relDir), ec);
    return ec ? IOResult{IOError::RemoveFailed, 0} : IOResult{};
}
