#pragma once

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <libc_shim.h>
#include <sys/statvfs.h>

namespace shim {
    struct vfs {
        /** Block size. */
        unsigned long f_bsize;
        /** Fragment size. */
        unsigned long f_frsize;
        /** Total size of filesystem in `f_frsize` blocks. */
        fsblkcnt_t f_blocks;
        /** Number of free blocks. */
        fsblkcnt_t f_bfree;
        /** Number of free blocks for non-root. */
        fsblkcnt_t f_bavail;
        /** Number of inodes. */
        fsfilcnt_t f_files;
        /** Number of free inodes. */
        fsfilcnt_t f_ffree;
        /** Number of free inodes for non-root. */
        fsfilcnt_t f_favail;
        /** Filesystem id. */
        unsigned long f_fsid;
        /** Mount flags. (See `ST_` constants.) */
        unsigned long f_flag;
        /** Maximum filename length. */
        unsigned long f_namemax;
#if defined(__LP64__)
        uint32_t __f_reserved[6];
#endif
    };
    int statvfs(const char *path, struct vfs * buf);

    void add_statvfs_shimmed_symbols(std::vector<shimmed_symbol> &list);
}




