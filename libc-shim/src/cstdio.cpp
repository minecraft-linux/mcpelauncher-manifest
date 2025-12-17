#include "no-fortify.h"
#include <cstdlib>
#include "cstdio.h"
#include "iorewrite.h"

#include "wchar.h"
#include <libgen.h>
#include "variadic.h"

using namespace shim;

bionic::FILE bionic::standard_files[3];
bionic::FILE* bionic::standard_input = &bionic::standard_files[0];
bionic::FILE* bionic::standard_output = &bionic::standard_files[1];
bionic::FILE* bionic::standard_error = &bionic::standard_files[2];
int bionic::io_isthreaded = 1;

void bionic::init_standard_files() {
    standard_files[0]._p = "stdin";
    standard_files[0].wrapped = stdin;
    standard_files[1]._p = "stdout";
    standard_files[1].wrapped = stdout;
    standard_files[2]._p = "stderr";
    standard_files[2].wrapped = stderr;
}

static bionic::FILE *wrap_file(::FILE *file) {
    if (!file)
        return nullptr;

    auto ret = new bionic::FILE;
    ret->wrapped = file;
    ret->_p = "Internal";
    ret->_r = ret->_w = 0;
#if defined(__LP64__)
    ret->_file = fileno(file);
#else
    ret->_file = (short) fileno(file);
#endif
    ret->_flags = 0;
    return ret;
}

bionic::FILE *shim::fopen(const char *filename, const char *mode) {
    return wrap_file(::fopen(filename, mode));
}

bionic::FILE* shim::fdopen(int fd, const char *mode) {
    return wrap_file(::fdopen(fd, mode));
}

bionic::FILE* shim::freopen(const char *filename, const char *mode, bionic::FILE *stream) {
    return wrap_file(::freopen(filename, mode, stream->wrapped));
}

bionic::FILE* shim::tmpfile() {
    return wrap_file(::tmpfile());
}

bionic::FILE* shim::popen(const char *command, const char *mode) {
    return wrap_file(::popen(command, mode));
}

int shim::fclose(bionic::FILE *file) {
    int ret = fclose(file->wrapped);
    if (file == &bionic::standard_files[0] ||
        file == &bionic::standard_files[1] ||
        file == &bionic::standard_files[2])
        return ret;
    delete file;
    return ret;
}

int shim::pclose(bionic::FILE *file) {
    int ret = ::pclose(file->wrapped);
    delete file;
    return ret;
}

size_t shim::fread(void *ptr, size_t size, size_t n, bionic::FILE *file) {
    auto ret = ::fread(ptr, size, n, file->wrapped);
    update_feof(file);
    return ret;
}

int shim::fprintf(bionic::FILE *fp, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vfprintf(fp->wrapped, fmt, args);
    va_end(args);
    return ret;
}

int shim::fscanf(bionic::FILE *fp, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vfscanf(fp->wrapped, fmt, args);
    va_end(args);
    update_feof(fp);
    return ret;
}

int shim::vfscanf(bionic::FILE *fp, const char *fmt, va_list va) {
    int ret = vfscanf(fp->wrapped, fmt, va);
    update_feof(fp);
    return ret;
}

int shim::fseeko(bionic::FILE *fp, bionic::off_t off, int whence) {
    return ::fseeko(fp->wrapped, (off_t) off, whence);
}

bionic::off_t shim::ftello(bionic::FILE *fp) {
    return (bionic::off_t) ::ftello(fp->wrapped);
}

int shim::__vsnprintf_chk(char *dst, size_t in_len, int, size_t max_len, const char *fmt, va_list va) {
    if (in_len > max_len) {
        fprintf(stderr, "detected write past buffer");
        abort();
    }
    return ::vsnprintf(dst, in_len, fmt, va);
}

int shim::__snprintf_chk(char *dst, size_t in_len, int flags, size_t max_len, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = ::__vsnprintf_chk(dst, in_len, flags, max_len, fmt, args);
    va_end(args);
    return ret;
}

char *shim::__fgets_chk(char *dst, int len, bionic::FILE *stream, size_t max_len) {
    if (len < 0 || (size_t) len > max_len) {
        fprintf(stderr, "detected write past buffer");
        abort();
    }
    return ::fgets(dst, len, stream->wrapped);
}

void shim::add_cstdio_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    bionic::init_standard_files();

    list.insert(list.end(), {
        {"__sF", (void*) &bionic::standard_files},
        {"__isthreaded", (void*) &bionic::io_isthreaded},

        {"fopen", IOREWRITE1(fopen)},
        {"fdopen", fdopen},
        {"freopen", IOREWRITE1(freopen)},
        {"tmpfile", tmpfile},
        {"popen", popen},
        {"fclose", fclose},
        {"pclose", pclose},
        {"clearerr", AutoArgRewritten(::clearerr)},
        {"feof", AutoArgRewritten(::feof)},
        {"fseek", AutoArgRewritten(::fseek)},
        {"ftell", AutoArgRewritten(::ftell)},
        {"fseeko", fseeko},
        {"ftello", ftello},
        {"ferror", AutoArgRewritten(::ferror)},
        {"ferror", AutoArgRewritten(::ferror)},
        {"fflush", AutoArgRewritten(::fflush)},
        {"fgetc", AutoArgRewritten(::fgetc)},
        {"fgets", AutoArgRewritten(::fgets)},
        {"__fgets_chk", __fgets_chk},
        {"fputc", AutoArgRewritten(::fputc)},
        {"fputs", AutoArgRewritten(::fputs)},
        {"fread", fread},
        {"fwrite", AutoArgRewritten(::fwrite)},
#if LIBC_SHIM_DEFINE_VARIADIC
        {"fprintf", fprintf},
        {"vfprintf", AutoArgRewritten(::vfprintf)},
        {"fscanf", fscanf},
        {"vfscanf", vfscanf},
#endif
        {"getc", AutoArgRewritten(::getc)},
        {"getc_unlocked", AutoArgRewritten(::getc_unlocked)},
        {"getdelim", AutoArgRewritten(::getdelim)},
        {"getline", AutoArgRewritten(::getline)},
        {"putc", AutoArgRewritten(::putc)},
        {"putc_unlocked", AutoArgRewritten(::putc_unlocked)},
        {"rewind", AutoArgRewritten(::rewind)},
        {"setbuf", AutoArgRewritten(::setbuf)},
        {"setvbuf", +[]() {} /*AutoArgRewritten(::setvbuf) crash in 1.20.71.01*/},
        {"setbuffer", AutoArgRewritten(::setbuffer)},
        {"setlinebuf", AutoArgRewritten(::setlinebuf)},
        {"ungetc", AutoArgRewritten(::ungetc)},
        {"fileno", AutoArgRewritten(::fileno)},
        {"pclose", AutoArgRewritten(::pclose)},
        {"flockfile", AutoArgRewritten(::flockfile)},
        {"ftrylockfile", AutoArgRewritten(::ftrylockfile)},
        {"funlockfile", AutoArgRewritten(::funlockfile)},
        {"getw", AutoArgRewritten(::getw)},
        {"putw", AutoArgRewritten(::putw)},

        {"remove", IOREWRITE1(::remove)},
        {"rename", IOREWRITE2(::rename)},

        {"putchar", ::putchar},
        {"puts", ::puts},
#if LIBC_SHIM_DEFINE_VARIADIC
        {"printf", ::printf},
        {"vprintf", ::vprintf},
        {"sprintf", ::sprintf},
        {"asprintf", ::asprintf},
        {"vasprintf", ::vasprintf},
        {"snprintf", ::snprintf},
        {"vsprintf", ::vsprintf},
        {"vsnprintf", ::vsnprintf},
        {"__vsnprintf_chk", ::__vsnprintf_chk},
        {"__snprintf_chk", ::__snprintf_chk},
        {"scanf", ::scanf},
        {"sscanf", ::sscanf},
        {"vscanf", ::vscanf},
        {"vsscanf", ::vsscanf},
#endif
        {"perror", ::perror},

        {"stdin", &bionic::standard_input},
        {"stdout", &bionic::standard_output},
        {"stderr", &bionic::standard_error},
        {"__register_atfork", (void*)+[]() {

        }},
        { "getauxval", (void*)+[](unsigned long) -> unsigned long {
            return 0;
        }},
        { "tcgetattr", (void*)+[](int, void*) -> int {
            return 0;
        }},
        { "tcsetattr", (void*)+[](int, int, void*) -> int {
            return 0;
        }},
        {"getpwuid_r", (void*)+[](int) -> void* {
            return nullptr;
        }},
        { "basename", (void*)basename},
        /* wchar.h */
        {"getwc", AutoArgRewritten(getwc)},
        {"ungetwc", AutoArgRewritten(ungetwc)},
        {"putwc", AutoArgRewritten(putwc)},
    });
}
