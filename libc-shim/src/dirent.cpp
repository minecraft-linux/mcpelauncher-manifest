#include "no-fortify.h"
#include "dirent.h"
#include "iorewrite.h"

#include <cstring>
#include <unistd.h>

using namespace shim;

static bionic::DIR *wrap_dir(::DIR *dir) {
    if (dir == nullptr)
        return nullptr;
    return new bionic::DIR{dir, {}};
}

bionic::DIR *shim::opendir(const char *name) {
    return wrap_dir(::opendir(name));
}

bionic::DIR* shim::fdopendir(int fd) {
    return wrap_dir(::fdopendir(fd));
}

void shim::closedir(bionic::DIR *dir) {
    if (!dir)
        return;
    ::closedir(dir->wrapped);
    delete dir;
}

bionic::dirent* shim::readdir(bionic::DIR *dir) {
    auto hent = ::readdir(dir->wrapped);
    if (!hent)
        return nullptr;

    auto &ent = dir->current;
    ent.d_ino = hent->d_ino;
#ifndef __APPLE__
    ent.d_off = hent->d_off;
#else
    ent.d_off = ::telldir(dir->wrapped);
#endif
    ent.d_reclen = sizeof(bionic::dirent);
    ent.d_type = hent->d_type;
    strncpy(ent.d_name, hent->d_name, sizeof(ent.d_name));
    ent.d_name[sizeof(ent.d_name) - 1] = '\0';

    return &dir->current;
}

void shim::rewinddir(bionic::DIR *dir) {
    ::rewinddir(dir->wrapped);
}

void shim::seekdir(bionic::DIR *dir, long pos) {
    ::seekdir(dir->wrapped, pos);
}

long shim::telldir(bionic::DIR *dir) {
    return ::telldir(dir->wrapped);
}

int shim::dirfd(bionic::DIR *dir) {
    int fd = ::dirfd(dir->wrapped);
#ifdef __APPLE__
    if (fd != -1) {
        // On macOS, dirfd returns a file descriptor that is owned by the directory stream and is closed by closedir.
        // We need to duplicate it to ensure it can be used independently.
        return dup(fd);
    }
#endif
    return fd;
}

void shim::add_dirent_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"opendir", IOREWRITE1(opendir)},
        {"fdopendir", fdopendir},
        {"closedir", closedir},
        {"readdir", readdir},
        {"rewinddir", rewinddir},
        {"seekdir", seekdir},
        {"telldir", telldir},
        {"dirfd", dirfd},
    });
}