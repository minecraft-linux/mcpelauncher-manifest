#pragma once

#include <libc_shim.h>
#include <cstdint>
#include <stdio.h>
#include "argrewrite.h"
#include "common.h"

namespace shim {

    namespace bionic {

        struct FILE {
            const char* _p;
            int	_r, _w;
#if defined(__LP64__)
            int	_flags, _file;
#else
            short _flags, _file;
#endif
            ::FILE *wrapped;

#if defined(__LP64__)
            char filler[152 - 0x24];
#else
            char filler[0x54 - 0x14];
#endif
        };

#if defined(__LP64__)
        static_assert(sizeof(FILE) == 152, "FILE must be ? bytes big");
#else
        static_assert(sizeof(FILE) == 0x54, "FILE must be 0x54 bytes big");
#endif

        template <>
        inline auto to_host<FILE>(FILE const *m) { return m->wrapped; }

        extern bionic::FILE standard_files[3];
        extern bionic::FILE* standard_input;
        extern bionic::FILE* standard_output;
        extern bionic::FILE* standard_error;
        extern int io_isthreaded;

        void init_standard_files();

        inline void update_feof(bionic::FILE *file) {
            file->_flags = (short) (feof_unlocked(file->wrapped) ? 0x0020 : 0);
        }

    }

    bionic::FILE *fopen(const char *filename, const char *mode);

    bionic::FILE *fdopen(int fd, const char *mode);

    bionic::FILE *freopen(const char *filename, const char *mode, bionic::FILE *stream);

    bionic::FILE *tmpfile();

    bionic::FILE *popen(const char *command, const char *mode);

    int fclose(bionic::FILE *file);

    int pclose(bionic::FILE *file);

    size_t fread(void *ptr, size_t size, size_t n, bionic::FILE *file);

    int fprintf(bionic::FILE* fp, const char *fmt, ...);

    int fscanf(bionic::FILE* fp, const char *fmt, ...);

    int vfscanf(bionic::FILE* fp, const char *fmt, va_list va);

    int fseeko(bionic::FILE* fp, bionic::off_t off, int whence);

    bionic::off_t ftello(bionic::FILE* fp);

    int __vsnprintf_chk(char *dst, size_t in_len, int flags, size_t max_len, const char *fmt, va_list va);

    int __snprintf_chk(char *dst, size_t in_len, int flags, size_t max_len, const char* fmt, ...);

    char *__fgets_chk(char *dst, int len, bionic::FILE *stream, size_t max_len);

    void add_cstdio_shimmed_symbols(std::vector<shimmed_symbol> &list);

    namespace detail {

        template <>
        struct arg_rewrite<::FILE *> : bionic_ptr_rewriter<typename ::FILE *, bionic::FILE *> {};

    }
}
