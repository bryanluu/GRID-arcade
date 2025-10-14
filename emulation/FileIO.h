// FileIO.h — desktop filesystem backend
#ifndef FILE_IO_H
#define FILE_IO_H

#include "IExternalIO.h"
#include <string>

class FileIO final : public IExternalIO
{
public:
    IOResult init(const char *baseDir, LogFn logger = nullptr) override;
    IOResult readAll(const char *relPath, void *dst, size_t cap) override;
    IOResult writeAll(const char *relPath, const void *src, size_t nbytes) override;
    bool exists(const char *relPath) override;
    IOResult removeFile(const char *relPath) override;
    IOResult removeTree(const char *relDir) override;
    const char *base() const override { return baseDir.c_str(); }

private:
    std::string baseDir = "/save";
    LogFn log = nullptr;

    void recoverTemp(); // promote leftover temp if present
};

#endif // FILE_IO_H
