#ifndef I_EXTERNAL_IO_H
#define I_EXTERNAL_IO_H

#include <stddef.h>
#include <stdint.h>

enum class IOError
{
    None = 0,
    NotInitialized,
    PathError,
    OpenFailed,
    ReadFailed,
    WriteFailed,
    SyncFailed,
    RenameFailed,
    RemoveFailed,
    NotFound,
    TooLarge,
    MountFailed
};

struct IOResult
{
    IOError err = IOError::None;
    int32_t bytes = 0; // bytes read/written when applicable
    explicit operator bool() const { return err == IOError::None; }
};

using LogFn = void (*)(const char *);

class IExternalIO
{
public:
    virtual ~IExternalIO() = default;

    // May perform temp-file recovery (promote leftover temp to final).
    virtual IOResult init(const char *baseDir, LogFn logger = nullptr) = 0;

    // Whole-file operations relative to baseDir.
    virtual IOResult readAll(const char *relPath, void *dst, size_t cap) = 0;
    virtual IOResult writeAll(const char *relPath, const void *src, size_t nbytes) = 0;

    virtual bool exists(const char *relPath) = 0;
    virtual IOResult removeFile(const char *relPath) = 0;
    virtual IOResult removeTree(const char *relDir) = 0;

    virtual const char *base() const = 0;
};

#endif // I_EXTERNAL_IO_H
