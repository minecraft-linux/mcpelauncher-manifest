#pragma once

#include <libc_shim.h>
#include <cstdint>
#include <dirent.h>

namespace shim {

    namespace bionic {

        struct dirent {
            uint64_t d_ino;
            int64_t d_off;
            unsigned short d_reclen;
            unsigned char d_type;
            char d_name[256];
        };

        struct DIR {
            ::DIR *wrapped;
            dirent current;
        };

    }

    bionic::DIR *opendir(const char *name);

    bionic::DIR *fdopendir(int fd);

    void closedir(bionic::DIR *dir);

    bionic::dirent *readdir(bionic::DIR *dir);

    void rewinddir(bionic::DIR *dir);

    void seekdir(bionic::DIR *dir, long pos);

    long telldir(bionic::DIR *dir);

    int dirfd(bionic::DIR *dir);


    void add_dirent_shimmed_symbols(std::vector<shimmed_symbol> &list);

}