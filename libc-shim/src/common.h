#pragma once

#include <cstdint>
#include <cstddef>
#include <sys/types.h>
#include <sys/socket.h>
#include "argrewrite.h"
#include "mallinfo.h"
#include <stdarg.h>
#include <ctime>

namespace shim {

    namespace bionic {

        enum class clock_type : uint32_t {
            REALTIME = 0,
            MONOTONIC = 1,
            BOOTTIME = 7
        };

        clockid_t to_host_clock_type(clock_type type);

        enum class mmap_flags : int {
            SHARED = 1,
            PRIVATE = 2,
            FIXED = 0x10,
            ANON = 0x20,
            NORESERVE = 0x4000,
            GROWSDOWN = 0x0100,
            DENYWRITE = 0x0800,
            EXECUTABLE = 0x1000,
            LOCKED = 0x2000,
            // UNINITIALIZED = 0x80000000,
            STACK = 0x20000, // not used in bionic, but used in musl
            GROWSDOWN_STACK = 0x10000 // not used in bionic, but used in musl
        };

        int to_host_mmap_flags(mmap_flags flags);

        enum class rlimit_resource : int {
            NOFILE = 7
        };

        int to_host_rlimit_resource(rlimit_resource r);

        struct rlimit {
            unsigned long int rlim_cur, rlim_max;
        };

        enum class prctl_num {
            SET_NAME = 15
        };


        extern uintptr_t stack_chk_guard;

        void on_stack_chk_fail();

        struct timeval {
            long tv_sec, tv_usec;
        };

#if defined(__LP64__)
        using off_t = ::off_t;
#else
        using off_t = int32_t;
#endif

        size_t strlcpy(char *dst, const char *src, size_t siz);

    }

    int gettid();
    void assert_impl(const char* file, int line, const char* msg);
    void assert2_impl(const char* file, int line, const char* function, const char* msg);

    void android_set_abort_message(const char *msg);

    size_t strlen_chk(const char *str, size_t max_len);
    const char* strchr_chk(const char* __s, int __ch, size_t __n);
    const char* strrchr_chk(const char* __s, int __ch, size_t __n);

#ifndef __LP64__
    /* Bionic uses a 32-bit off_t; this doesn't match up on Darwin so let's
     * overkill and apply it on all 32-bit platforms. */

    int ftruncate(int fd, bionic::off_t len);

    ssize_t pread(int fd, void *buf, size_t len, bionic::off_t off);

    ssize_t pwrite(int fd, const void *buf, size_t len, bionic::off_t off);
#endif

    bionic::mallinfo mallinfo();

    void *memalign(size_t alignment, size_t size);

    void *mmap(void *addr, size_t length, int prot, bionic::mmap_flags flags, int fd, bionic::off_t offset);

    void *mremap(void *old_addr, size_t old_length, size_t new_length, int flags, ...);

    int getrusage(int who, void *usage);

    int getrlimit(bionic::rlimit_resource res, bionic::rlimit *info);

    int clock_gettime(bionic::clock_type clock, struct timespec *ts);

    int prctl(bionic::prctl_num opt, unsigned long a2, unsigned long a3, unsigned long a4, unsigned long a5);

    int sendfile(int src, int dst, bionic::off_t *offset, size_t count);

    void *__memcpy_chk(void *dst, const void *src, size_t size, size_t max_len);

    void *__memmove_chk(void *dst, const void *src, size_t size, size_t max_len);

    void *__memset_chk(void *dst, int c, size_t size, size_t max_len);

    int __vsprintf_chk(char* dst, int flags, size_t dst_len_from_compiler, const char* format, va_list va);

    char* __strcpy_chk(char* dst, const char* src, size_t dst_len);

    char* __strcat_chk(char *dst, const char *src, size_t dst_len);

    char* __strncat_chk(char *dst, const char * src, size_t n, size_t dst_len);

    char* __strncpy_chk(char* dst, const char* src, size_t len, size_t dst_len);

    char* __strncpy_chk2(char* dst, const char* src, size_t n, size_t dst_len, size_t src_len);

    size_t ctype_get_mb_cur_max();

    int gettimeofday(bionic::timeval *tv, void *p);

    ssize_t __read_chk(int fd, void* buf, size_t count, size_t buf_size);

    ssize_t __recvfrom_chk(int socket, void* buf, size_t len, size_t buf_size,
                        int flags, sockaddr* src_addr, socklen_t* addrlen);

    ssize_t __sendto_chk(int socket, const void* buf, size_t len, size_t buflen,
                        int flags, const struct sockaddr* dest_addr,
                        socklen_t addrlen);

    ssize_t __write_chk(int fd, const void* buf, size_t count, size_t buf_size);

#ifdef __APPLE__
    int fdatasync(int fildes);
#endif
#if defined(__APPLE__) || defined(__FreeBSD__)
    int __cmsg_nxthdr();
#endif

    long fakesyscall(long sysno, ...);

    ssize_t getrandom(void *buf, size_t len, unsigned int flags);

    int unlinkat(int dirfd, const char *pathname, int flags);

    int isnan(double d);

    void arc4random_buf(void* buf, size_t len);

    uint32_t arc4random();

    int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);

    void add_common_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_stdlib_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_malloc_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_ctype_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_math_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_time_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_wait_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_sched_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_unistd_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_signal_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_string_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_wchar_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_mman_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_resource_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_prctl_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_locale_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_setjmp_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_misc_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_fnmatch_shimmed_symbols(std::vector<shim::shimmed_symbol> &list);

    void add_socket_shimmed_symbols(std::vector<shim::shimmed_symbol> &list);

    namespace detail {

        template <class T>
        struct tm_detail {
            tm loc;
            using source = T;

            source before(source src) {
                if(src) {
                    loc = *src;
                    switch(loc.tm_isdst) {
                        case 1: 
                            loc.tm_isdst = 0;
                            break;
                        case 0: 
                            loc.tm_isdst = 1;
                            break;
                        default:
                            break;
                    } 
                    return &loc;
                }
                return nullptr;
            }
            void after(const tm* src) {

            }

            void after(tm* src) {
                if(src) {
                    switch(loc.tm_isdst) {
                        case 1:
                            loc.tm_isdst = 0;
                            break;
                        case 0:
                            loc.tm_isdst = 1;
                            break;
                        default:
                            break;
                    } 
                    *src = loc;
                }
            }
        };

        template <>
        struct arg_rewrite<tm *> : tm_detail<tm*> {};
        template <>
        struct arg_rewrite<const tm *> : tm_detail<const tm*> {};

    }

}
