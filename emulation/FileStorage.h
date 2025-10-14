// FileStorage.h — desktop filesystem backend using ILogger
// Purpose: Implement IStorage via std::filesystem and std::fstream.
// Guarantees/behavior:
// - Atomic-ish update using temp file + rename (within the same directory).
// - Best-effort cleanup of a generic temp file during init().
// - Uses ILogger for informative and warning messages (helpers add the “[FileStorage]” prefix).
#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#include "IStorage.h"
#include <string>

class FileStorage final : public IStorage
{
public:
    // Defaults used if caller passes nullptr or empty string to init()
    static constexpr const char *kDefaultBaseDir = "save";
    static constexpr const char *kTempName = ".tmp_write";

    StorageResult init(const char *baseDir, ILogger *logger = nullptr) override;
    StorageResult readAll(const char *relPath, void *dst, size_t cap) override;
    StorageResult writeAll(const char *relPath, const void *src, size_t nbytes) override;
    bool exists(const char *relPath) override;
    StorageResult removeFile(const char *relPath) override;
    StorageResult removeTree(const char *relDir) override;
    const char *base() const override { return baseDir.c_str(); }

private:
    std::string baseDir = kDefaultBaseDir;
    ILogger *log = nullptr; // non-owning

    // Remove a leftover generic temp file (e.g., from abrupt termination).
    void recoverTemp();

    // Logging helpers — include “[FileStorage]” prefix here to keep call sites clean.
    void info(const char *msg);
    void warn(const char *msg);
};

#endif // FILESTORAGE_H
