#include "no-fortify.h"
#include <libc_shim.h>

#include <log.h>
#include <stdexcept>
#include "common.h"
#include "pthreads.h"
#include "semaphore.h"
#include "network.h"
#include "dirent.h"
#include "cstdio.h"
#include "errno.h"
#include "ctype_data.h"
#include "stat.h"
#include "file_misc.h"
#include "sysconf.h"
#include "system_properties.h"
#include "sched.h"
#include <cmath>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <cwctype>
#include <csignal>
#include <cstring>
#ifdef __APPLE__
#include <sys/fcntl.h>
#endif
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <csetjmp>
#include <clocale>
#include <locale.h>
#include <cerrno>
#include <utime.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <pthread.h>
#include <malloc/malloc.h>
#include <sys/socket.h>
#else
#include <malloc.h>
#endif
#include <sys/uio.h>
#include <syslog.h>
#if !defined(__APPLE__) && !defined(__FreeBSD__)
#include <sys/prctl.h>
#include <sys/auxv.h>
#include <sys/utsname.h>
#else
#include <xlocale.h>
#endif
#ifdef __APPLE__
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101200
// for macOS 10.10 - 10.11
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#endif
#include <inttypes.h>
#ifdef _WIN32
#include <Processthreadsapi.h>
#elif defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__)
#include <sys/syscall.h>
#endif
#ifdef __linux__
#include <sys/sendfile.h>
#include <fcntl.h>
#endif
#include "fakesyscall.h"
#include "iorewrite.h"
#include "armhfrewrite.h"
#include "statvfs.h"
#include "variadic.h"
#include <chrono>
#include <unistd.h>

#ifdef __linux__
// glibc introduced arc4random_buf in 2.36
#if defined(__GLIBC__) && !(__GLIBC__ < 2 || __GLIBC__ == 2 && __GLIBC_MINOR__ < 36)
#define HAS_ARC4RANDOM_BUF 1
#else
#include <random>
#endif
#else
// OSX has arc4random_buf
#define HAS_ARC4RANDOM_BUF 1
#endif

using namespace shim;

void shim::handle_runtime_error(const char *fmt, ...) {
    char buf[255];
    va_list l;
    va_start(l, fmt);
    ::vsnprintf(buf, sizeof(buf), fmt, l);
    va_end(l);
    ::fprintf(::stderr, "libc-shim runtime error: %s\ns", buf);
    ::fflush(::stderr);
    throw std::runtime_error(std::string(buf));
};

#if __i386__ || defined(__arm__)
extern "C" unsigned long __umoddi3(unsigned long a, unsigned long b);
extern "C" unsigned long __udivdi3(unsigned long a, unsigned long b);
extern "C" long __divdi3(long a, long b);
#endif

extern "C" int __cxa_atexit(void (*)(void*), void*, void*);
extern "C" void __cxa_finalize(void * d);

#ifdef USE_BIONIC_SETJMP
extern "C" void bionic_setjmp();
extern "C" void bionic_longjmp();
#endif

uintptr_t bionic::stack_chk_guard = []() {
#if !defined(__APPLE__) && !defined(__FreeBSD__)
    char* buf = reinterpret_cast<char*>(getauxval(AT_RANDOM));
    uintptr_t res = 0;
    memcpy(&res, buf, sizeof(res));
    return res;
#else
    return 0;
#endif
}();

clockid_t bionic::to_host_clock_type(bionic::clock_type type) {
    switch (type) {
        case clock_type::REALTIME: return CLOCK_REALTIME;
        case clock_type::MONOTONIC: return CLOCK_MONOTONIC;
#ifdef __APPLE__
        case clock_type::BOOTTIME: return CLOCK_MONOTONIC;
#else
        case clock_type::BOOTTIME: return CLOCK_BOOTTIME;
#endif
        default: handle_runtime_error("Unexpected clock type %d", (int)type);
    }
}

int bionic::to_host_mmap_flags(bionic::mmap_flags flags) {
    if (((uint32_t) flags & ~((uint32_t) mmap_flags::FIXED | (uint32_t) mmap_flags::ANON |
        (uint32_t) mmap_flags::NORESERVE | (uint32_t) mmap_flags::PRIVATE | (uint32_t) mmap_flags::SHARED |
        (uint32_t) mmap_flags::STACK | (uint32_t)mmap_flags::GROWSDOWN )) != 0)
        handle_runtime_error("Used unsupported mmap flags %d", (int)flags);

    int ret = 0;
    if ((uint32_t) flags & (uint32_t) mmap_flags::PRIVATE)
        ret |= MAP_PRIVATE;
    if ((uint32_t) flags & (uint32_t) mmap_flags::FIXED)
        ret |= MAP_FILE;
    if ((uint32_t) flags & (uint32_t) mmap_flags::ANON)
        ret |= MAP_ANONYMOUS;
    if ((uint32_t) flags & (uint32_t) mmap_flags::SHARED)
        ret |= MAP_SHARED;
#ifdef MAP_NORESERVE
    if ((uint32_t) flags & (uint32_t) mmap_flags::NORESERVE)
        ret |= MAP_NORESERVE;
#endif
#ifdef MAP_GROWSDOWN
    if ((uint32_t) flags & (uint32_t) mmap_flags::GROWSDOWN)
        ret |= MAP_GROWSDOWN;
#endif
#ifdef MAP_STACK
    if ((uint32_t) flags & (uint32_t) mmap_flags::STACK)
        ret |= MAP_STACK;
#endif
    return ret;
}

int bionic::to_host_rlimit_resource(shim::bionic::rlimit_resource r) {
    switch (r) {
        case rlimit_resource::NOFILE: return RLIMIT_NOFILE;
        default: handle_runtime_error("Unknown rlimit resource %d", (int)r);
    }
}

void bionic::on_stack_chk_fail() {
    fprintf(stderr, "stack corruption has been detected\n");
    abort();
}

void shim::assert_impl(const char *file, int line, const char *msg) {
    fprintf(stderr, "assert failed: %s:%i: %s\n", file, line, msg);
    abort();
}

void shim::assert2_impl(const char *file, int line, const char *function, const char *msg) {
    fprintf(stderr, "assert failed: %s:%i %s: %s\n", file, line, function, msg);
    abort();
}

void shim::android_set_abort_message(const char *msg) {
    fprintf(stderr, "abort message: %s\n", msg);
}

size_t shim::strlen_chk(const char *str, size_t max_len) {
    auto ret = strlen(str);
    if (ret >= max_len) {
        fprintf(stderr, "strlen_chk: string longer than expected\n");
        abort();
    }
    return ret;
}

const char* shim::strchr_chk(const char* __s, int __ch, size_t __n) {
    return strchr(__s, __ch);
}

const char* shim::strrchr_chk(const char* __s, int __ch, size_t __n) {
    return strrchr(__s, __ch);
}

#ifndef __LP64__
int shim::ftruncate(int fd, bionic::off_t len) {
    return ::ftruncate(fd, (::off_t) len);
}

ssize_t shim::pread(int fd, void *buf, size_t len, bionic::off_t off) {
    return ::pread(fd, buf, len, (::off_t) off);
}

ssize_t shim::pwrite(int fd, const void *buf, size_t len, bionic::off_t off) {
    return ::pwrite(fd, buf, len, (::off_t) off);
}
#endif

static int my_clock_getres(bionic::clock_type clock, struct timespec *ts) {
    return ::clock_getres(bionic::to_host_clock_type(clock), ts);
}

int shim::clock_gettime(bionic::clock_type clock, struct timespec *ts) {
#if defined(__APPLE__) && __MAC_OS_X_VERSION_MIN_REQUIRED < 101200
    if(::clock_gettime != NULL) {
#endif
    return ::clock_gettime(bionic::to_host_clock_type(clock), ts);
#if defined(__APPLE__) && __MAC_OS_X_VERSION_MIN_REQUIRED < 101200
    } else {
        // fallback if weak symbol is nullptr < macOS 10.12
        clock_serv_t cclock;
        mach_timespec_t mts;
        if (host_get_clock_service(mach_host_self(), clock == bionic::clock_type::MONOTONIC ? SYSTEM_CLOCK : CALENDAR_CLOCK, &cclock) != KERN_SUCCESS) {
            return -1;
        }
        kern_return_t r = clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        if (r != KERN_SUCCESS) {
            return -1;
        }
        ts->tv_sec = mts.tv_sec;
        ts->tv_nsec = mts.tv_nsec;
        return 0;
    }
#endif
}

bionic::mallinfo shim::mallinfo() {
    return { .ordblks = 8000000, .usmblks= 8000000, .fordblks= 8000000 };
}

void* shim::memalign(size_t alignment, size_t size) {
    void* ret;
    if (posix_memalign(&ret, alignment, size) != 0)
        return nullptr;
    return ret;
}

void *shim::mmap(void *addr, size_t length, int prot, bionic::mmap_flags flags, int fd, bionic::off_t offset) {
    return ::mmap(addr, length, prot, bionic::to_host_mmap_flags(flags), fd, (::off_t) offset);
}

void *shim::mremap(void *old_addr, size_t old_length, size_t new_length, int flags, ...) {
// TODO: Implement this someday, at least on linux
    return MAP_FAILED;
}

int shim::getrusage(int who, void *usage) {
    Log::warn("Shim/Common", "getrusage is unsupported");
    return -1;
}

int shim::getrlimit(bionic::rlimit_resource res, bionic::rlimit *info) {
    ::rlimit hinfo {};
    int ret = ::getrlimit(bionic::to_host_rlimit_resource(res), &hinfo);
    info->rlim_cur = hinfo.rlim_cur;
    info->rlim_max = hinfo.rlim_max;
    return ret;
}

int shim::prctl(bionic::prctl_num opt, unsigned long a2, unsigned long a3, unsigned long a4, unsigned long a5) {
#ifdef __linux__
    return ::prctl((int) opt, a2, a3, a4, a5);
#else
    switch (opt) {
        case bionic::prctl_num::SET_NAME:
#ifdef __FreeBSD__
            return ::pthread_setname_np(::pthread_self(), (const char *) a2);
#else
            return ::pthread_setname_np((const char *) a2);
#endif
        default:
            Log::error("Shim/Common", "Unexpected prctl: %i", opt);
            return EINVAL;
    }
#endif
}

void* shim::__memcpy_chk(void *dst, const void *src, size_t size, size_t max_len) {
    if (size > max_len) {
        fprintf(stderr, "detected copy past buffer size");
        abort();
    }
    return ::memcpy(dst, src, size);
}

void* shim::__memmove_chk(void *dst, const void *src, size_t size, size_t max_len) {
    if (size > max_len) {
        fprintf(stderr, "detected copy past buffer size");
        abort();
    }
    return ::memmove(dst, src, size);
}

void* shim::__memset_chk(void *dst, int c, size_t size, size_t max_len) {
    if (size > max_len) {
        fprintf(stderr, "detected set past buffer size");
        abort();
    }
    return ::memset(dst, c, size);
}

int shim::__vsprintf_chk(char* dst, int flags, size_t dst_len_from_compiler, const char* format, va_list va) {
    return vsprintf(dst, format, va);
}

char* shim::__strcpy_chk(char* dst, const char* src, size_t dst_len) {
  return strcpy(dst, src);
}

char* shim::__strcat_chk(char *dst, const char *src, size_t dst_len) {
    return strcat(dst, src);
}

char* shim::__strncat_chk(char *dst, const char *src, size_t n, size_t dst_len) {
    return strncat(dst, src, n);
}

char* shim::__strncpy_chk(char* dst, const char* src, size_t len, size_t dst_len) {
  return strncpy(dst, src, len);
}

char* shim::__strncpy_chk2(char* dst, const char* src, size_t n, size_t dst_len, size_t src_len) {
    return strncpy(dst, src, n);
}

int shim::sendfile(int src, int dst, bionic::off_t *offset, size_t count) {
    off_t c = offset ? (off_t)offset : 0;
#ifdef __APPLE__
    auto ret = ::sendfile(src, dst, c, offset ? &c : nullptr, nullptr, 0);
#elif defined(__FreeBSD__)
    auto ret = ::sendfile(src, dst, c, count, nullptr, 0, 0);
#else
    auto ret = ::sendfile(src, dst, offset ? &c : nullptr, count);
#endif
    if(offset) {
        *offset = (bionic::off_t)c;
    }
    return ret;
}

size_t shim::ctype_get_mb_cur_max() {
    return MB_CUR_MAX;
}

int shim::gettimeofday(bionic::timeval *tv, void *p) {
    if (p)
        handle_runtime_error("gettimeofday adtimezone is not supported");
    timeval htv {};
    int ret = ::gettimeofday(&htv, nullptr);
    tv->tv_sec = htv.tv_sec;
    tv->tv_usec = htv.tv_usec;
    return ret;
}

ssize_t shim::__read_chk(int fd, void *buf, size_t count, size_t buf_size) {
    return read(fd, buf, count);
}

ssize_t shim::__recvfrom_chk(int socket, void* buf, size_t len, size_t buf_size,
                       int flags, sockaddr* src_addr, socklen_t* addrlen) {
  return recvfrom(socket, buf, len, flags, src_addr, addrlen);
}

ssize_t shim::__sendto_chk(int socket, const void* buf, size_t len, size_t buflen,
                     int flags, const struct sockaddr* dest_addr,
                     socklen_t addrlen) {
  return sendto(socket, buf, len, flags, dest_addr, addrlen);
}

ssize_t shim::__write_chk(int fd, const void* buf, size_t count, size_t buf_size) {
  return write(fd, buf, count);
}

#ifdef __APPLE__
int shim::fdatasync(int fildes) {
    return ::fcntl(fildes, F_FULLFSYNC);
}
#endif
#if defined(__APPLE__) || defined(__FreeBSD__)
int shim::__cmsg_nxthdr() {
    return 0;
}
#endif

int shim::gettid() {
#ifdef __APPLE__
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return (long&)tid;
#elif defined(SYS_gettid)
    return syscall(SYS_gettid);
#elif defined(SYS_thr_self)
    return syscall(SYS_thr_self);
#else
    return pthread_getthreadid_np();
#endif
}

ssize_t shim::getrandom(void *buf, size_t len, unsigned int flags) {
#ifdef __linux__
        return (ssize_t)syscall(SYS_getrandom, buf, len, flags);
#elif defined(_WIN32)
        // TODO Implement if needed
        // this insecure stub works for Minecraft
        return (ssize_t)len;
#else
        // TODO do we need look at flags?
        // Should work for bsd and macOS
        ::arc4random_buf(buf, len);
        return (ssize_t)len;
#endif
}

long shim::fakesyscall(long sysno, ...) {
    if (sysno == FAKE_SYS_gettid) {
        return shim::gettid();
    } else if (sysno == FAKE_SYS_getrandom) {
        va_list l;
        va_start(l, sysno);
        auto buf = va_arg(l, char*);
        auto len = va_arg(l, size_t);
        auto flags = va_arg(l, unsigned int);
        long res = (long)shim::getrandom(buf, len, flags);
        va_end(l);
        return res;
    }
    return ENOSYS;
}

int shim::unlinkat(int dirfd, const char *pathname, int flags) {
    int ret = ::unlinkat(dirfd, iorewrite0(pathname).data(), flags);
    bionic::update_errno();
    return ret;
}

int shim::isnan(double d) {
	return std::isnan(d);
}

void shim::arc4random_buf(void* buf, size_t len) {
#ifdef HAS_ARC4RANDOM_BUF
    ::arc4random_buf(buf, len);
#else
    // Using SYS_getrandom is a bit complex since it can be interrupted
    // or not supported. Even if we used SYS_getrandom, we would have to
    // fall back to an alternative strategy or error out if it failed.
    // Instead rely on std::random_device to fill buf, allowing
    // std::random_device to handle the complex stuff.
    std::random_device get_rand;
    while(len > 0) {
        unsigned int u = get_rand();
        size_t n = std::min(len, sizeof(unsigned int));
        ::memcpy(buf, &u, n);
        buf = (uint8_t*)buf + n;
        len -= n;
    }
#endif
}

uint32_t shim::arc4random() {
    uint32_t u;
    shim::arc4random_buf(&u, sizeof(u));
    return u;
}

void shim::add_common_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"__errno", bionic::get_errno},
        {"__set_errno", bionic::set_errno},

        {"__stack_chk_fail", &bionic::on_stack_chk_fail},
        {"__stack_chk_guard", &bionic::stack_chk_guard},

        {"__assert", assert_impl},
        {"__assert2", assert2_impl},

        {"android_set_abort_message", android_set_abort_message},

        {"__cxa_atexit", ::__cxa_atexit},
        {"__cxa_finalize", ::__cxa_finalize},

        {"setpriority", setpriority}
    });
}

void shim::add_stdlib_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"abort", abort},
        {"atexit", atexit},
        {"exit", exit},
        {"_Exit", _Exit},

        {"system", system},

        {"getenv", getenv},
        {"putenv", WithErrnoUpdate(putenv)},
        {"setenv", WithErrnoUpdate(setenv)},
        {"unsetenv", WithErrnoUpdate(unsetenv)},


        {"random", random},
        {"srandom", srandom},
        {"initstate", initstate},
        {"setstate", WithErrnoUpdate(setstate)},

        {"rand", rand},
        {"srand", srand},
        {"rand_r", rand_r},
        {"drand48", drand48},
        {"erand48", erand48},
        {"lrand48", lrand48},
        {"nrand48", nrand48},
        {"mrand48", mrand48},
        {"jrand48", jrand48},
        {"srand48", srand48},
        {"seed48", seed48},
        {"lcong48", lcong48},

#if defined(__arm__) && defined(__linux__)
        { "isnanf", ARMHFREWRITE(isnanf) },
#endif
        { "isnan", ARMHFREWRITE(isnan) },

        {"atof", ARMHFREWRITE(atof)},
        {"atoi", atoi},
        {"atol", atol},
        {"atoll", atoll},
        {"strtod", ARMHFREWRITE(WithErrnoUpdate(strtod))},
        {"strtof", ARMHFREWRITE(WithErrnoUpdate(strtof))},
        {"strtold", ARMHFREWRITE(WithErrnoUpdate(strtold))},
        {"strtol", WithErrnoUpdate(strtol)},
        {"strtoul", WithErrnoUpdate(strtoul)},
        {"strtoul_l", WithErrnoUpdate(strtoul_l)},
        {"strtoq", WithErrnoUpdate(strtoq)},
        {"strtouq", WithErrnoUpdate(strtouq)},
        {"strtoll", WithErrnoUpdate(strtoll)},
        {"strtoll_l", WithErrnoUpdate(strtoll_l)},
        {"strtoull", WithErrnoUpdate(strtoull)},
        {"strtoull_l", WithErrnoUpdate(strtoull_l)},
        {"strtof_l", ARMHFREWRITE(WithErrnoUpdate(strtof_l))},
        {"strtold_l", ARMHFREWRITE(WithErrnoUpdate(strtold_l))},
        {"strtoumax", WithErrnoUpdate(strtoumax)},
        {"strtoimax", WithErrnoUpdate(strtoimax)},

        {"realpath", realpath},
        {"bsearch", bsearch},
        {"qsort", qsort},
        {"mblen", mblen},
        {"mbtowc", mbtowc},
        {"wctomb", wctomb},
        {"mbstowcs", mbstowcs},
        {"wcstombs", wcstombs},
        {"wcsrtombs", wcsrtombs}
    });
}

void shim::add_malloc_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"malloc", ::malloc},
#ifdef __APPLE__
        {"malloc_usable_size", ::malloc_size},
#else
        {"malloc_usable_size", ::malloc_usable_size},
#endif
        {"mallinfo", mallinfo},
        {"free", ::free},
        {"calloc", ::calloc},
        {"realloc", ::realloc},
        {"valloc", ::valloc},
        {"memalign", memalign},
        {"posix_memalign", ::posix_memalign},
        {"_Znwj", (void *(*)(size_t)) ::operator new},
        {"_ZdlPv", (void (*)(void *)) ::operator delete},
    });
}

void shim::add_ctype_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"_tolower_tab_", &_tolower_tab_},
        {"_toupper_tab_", &_toupper_tab_},
        {"_ctype_", &_ctype_},
        {"isalnum", isalnum},
        {"isalpha", isalpha},
        {"isblank", isblank},
        {"iscntrl", iscntrl},
        {"isdigit", isdigit},
        {"isgraph", isgraph},
        {"islower", islower},
        {"isprint", isprint},
        {"ispunct", ispunct},
        {"isspace", isspace},
        {"isupper", isupper},
        {"isxdigit", isxdigit},
        {"isascii", isascii},
        {"isxdigit_l", ::isxdigit_l},
        {"isdigit_l", ::isxdigit_l},

        {"tolower", ::tolower},
        {"toupper", ::toupper},

        {"__ctype_get_mb_cur_max", ctype_get_mb_cur_max}
    });
}

void shim::add_math_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
#if defined(__i386__) || defined(__arm__)
        {"__umoddi3", __umoddi3},
        {"__udivdi3", __udivdi3},
        {"__divdi3", __divdi3},
#endif
        {"ldexp", ARMHFREWRITE(((double(&)(double, int)) ::ldexp))},
    });
}

void shim::add_time_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    // Europe/Dublin xal sdk problem, clear out region
    tm t;
    time_t cur = time(nullptr);
    localtime_r(&cur, &t);
    std::chrono::seconds s(t.tm_gmtoff >= 0 ? t.tm_gmtoff : -t.tm_gmtoff);
    auto m = std::chrono::duration_cast<std::chrono::minutes>(s);
    s -= m;
    auto h = std::chrono::duration_cast<std::chrono::hours>(m);
    m -= h;
    static char buf[20] = { '\0' };
    sprintf(buf, "<%.5s>%c%02d:%02d:%02d", t.tm_zone, t.tm_gmtoff > 0 ? '-' : '+', (int)h.count(), (int)m.count(), (int)s.count());
    setenv("TZ", buf, true);

    tzset();
    localtime_r(&cur, &t);

    list.insert(list.end(), {
        /* sys/time.h */
        {"gettimeofday", gettimeofday},

        /* time.h */
        {"clock", ::clock},
        {"time", ::time},
        {"difftime", ::difftime},
        {"mktime", ::mktime},
        {"strftime", ::strftime},
        {"strptime", ::strptime},
        {"strftime_l", ::strftime_l},
        {"strptime_l", ::strptime_l},
        {"gmtime", ::gmtime},
        {"gmtime_r", ::gmtime_r},
        {"localtime", ::localtime},
        {"localtime_r", ::localtime_r},
        {"asctime", ::asctime},
        {"ctime", ::ctime},
        {"asctime_r", ::asctime_r},
        {"ctime_r", ::ctime_r},
        {"tzname", ::tzname},
        {"tzset", ::tzset},
#ifndef HAS_ARC4RANDOM_BUF
        {"arc4random_buf", shim::arc4random_buf},
#else
        {"arc4random_buf", ::arc4random_buf},
#endif
#ifndef __FreeBSD__
        {"daylight", &::daylight},
#endif
        {"timezone", &::timezone},
        {"nanosleep", ::nanosleep},
        {"clock_gettime", clock_gettime},
        {"clock_getres", (void*)my_clock_getres},
    });
}

void shim::add_wait_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"waitpid", ::waitpid},
    });
}

void shim::add_sched_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"sched_yield", ::sched_yield},
        {"sched_setaffinity", sched_setaffinity},
        {"sched_getaffinity", sched_getaffinity},
    });
}

void shim::add_fnmatch_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"fnmatch", ::fnmatch},
    });

}

static off_t _lseek64(int fd, off_t off, int dir) {
    return ::lseek(fd, off, dir);
}

static void stub() {
    abort();
}

void shim::add_unistd_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"access", WithErrnoUpdate(IOREWRITE1(::access))},
        {"lseek", WithErrnoUpdate(::lseek)},
        {"lseek64", WithErrnoUpdate(::_lseek64)},
        {"close", WithErrnoUpdate(::close)},
        {"read", WithErrnoUpdate(::read)},
        {"__read_chk", __read_chk},
        {"__write_chk", __write_chk},
        {"__recvfrom_chk", __recvfrom_chk},
        {"__sendto_chk", __sendto_chk},
        {"write", WithErrnoUpdate(::write)},
        {"pipe", WithErrnoUpdate(::pipe)},
        {"alarm", WithErrnoUpdate(::alarm)},
        {"sleep", WithErrnoUpdate(::sleep)},
        {"usleep", WithErrnoUpdate(::usleep)},
        {"pause", WithErrnoUpdate(::pause)},
        {"chown", WithErrnoUpdate(IOREWRITE1(::chown))},
        {"fchown", WithErrnoUpdate(::fchown)},
        {"lchown", WithErrnoUpdate(IOREWRITE1(::lchown))},
        {"chdir", WithErrnoUpdate(IOREWRITE1(::chdir))},
        {"fchdir", WithErrnoUpdate(::fchdir)},
        {"getcwd", WithErrnoUpdate(::getcwd)},
        {"dup", WithErrnoUpdate(::dup)},
        {"dup2", WithErrnoUpdate(::dup2)},
        {"execv", WithErrnoUpdate(::execv)},
        {"execle", ::execle},
        {"execl", ::execl},
        {"execvp", ::execvp},
        {"execlp", ::execlp},
        {"nice", WithErrnoUpdate(::nice)},
        {"_exit", ::_exit},
        {"getchar", getchar},
        {"getuid", WithErrnoUpdate(::getuid)},
        {"getpid", WithErrnoUpdate(::getpid)},
        {"getgid", WithErrnoUpdate(::getgid)},
        {"getppid", WithErrnoUpdate(::getppid)},
        {"getpgrp", WithErrnoUpdate(::getpgrp)},
        {"geteuid", WithErrnoUpdate(::geteuid)},
        {"getegid", WithErrnoUpdate(::getegid)},
        {"fork", WithErrnoUpdate(::fork)},
        {"vfork", WithErrnoUpdate(::vfork)},
        {"isatty", WithErrnoUpdate(::isatty)},
        {"link", WithErrnoUpdate(IOREWRITE2(::link))},
        {"symlink", WithErrnoUpdate(IOREWRITE2(::symlink))},
        {"readlink", WithErrnoUpdate(::readlink)},
        {"unlink", WithErrnoUpdate(IOREWRITE1(::unlink))},
        {"unlinkat", unlinkat},
        {"rmdir", WithErrnoUpdate(IOREWRITE1(::rmdir))},
        {"gethostname", WithErrnoUpdate(::gethostname)},
        {"fsync", WithErrnoUpdate(::fsync)},
        {"sync", WithErrnoUpdate(::sync)},
        {"getpagesize", ::getpagesize},
        {"getdtablesize", ::getdtablesize},
#if LIBC_SHIM_DEFINE_VARIADIC
        {"syscall", fakesyscall},
#endif
        {"getrandom", getrandom},
        {"gettid", gettid},
        {"sched_get_priority_min", sched_get_priority_min},
        {"sched_get_priority_max", sched_get_priority_max},
        {"lockf", WithErrnoUpdate(::lockf)},
        {"swab", ::swab},
        {"pathconf", ::pathconf},
        {"truncate", ::truncate},
        {"fdatasync", WithErrnoUpdate(fdatasync)},

        /* Use our impl or fallback to system */
        {"ftruncate", WithErrnoUpdate(ftruncate)},
        {"pread", WithErrnoUpdate(pread)},
        {"pwrite", WithErrnoUpdate(pwrite)},
        {"__system_property_foreach", (void*)stub},
        {"fputwc", (void*)stub},
        {"__system_property_read", (void*)stub},
        {"abs", (void*)stub},
        {"utimes", (void*)stub},
        {"__strrchr_chk", (void*)stub},
    });
}

void shim::add_signal_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        // Stub this binding until https://github.com/minecraft-linux/libc-shim/issues/9 is resolved, the current implementation currupts the stack
        // {"signal", ::signal},
        // {"bsd_signal", ::signal},
        // {"kill", ::kill},
        // {"killpg", ::killpg},
        {"signal", +[](int __sig, void* __handler) -> void* {
            return nullptr;
        }},
        {"bsd_signal", +[](int __sig, void* __handler) -> void* {
            return nullptr;
        }},
        {"kill", +[](int __pid, int __sig) -> int {
            return 0;
        }},
        {"killpg", +[](int __pid, int __sig) -> int {
            return 0;
        }},
        {"raise", ::raise},
        // {"sigaction", ::sigaction},
        // {"sigprocmask", ::sigprocmask},
        // {"sigemptyset", ::sigemptyset},
        // {"sigaddset", ::sigaddset}
        {"sigaction", +[](int __sig, const void * __act, void * __oact) -> int {
            return 0;
        }},
        {"sigprocmask", +[](int __how, const void * __set, void * __oset) -> int {
            return 0;
        }},
        {"sigemptyset", +[](void*) -> int {
            return 0;
        }},
        {"sigfillset", +[](void*) -> int {
            return 0;
        }},
        {"sigaddset", +[](void *__set, int __signo) -> int {
            return 0;
        }},
        {"sigdelset", +[](void *__set, int __signo) -> int {
            return 0;
        }}
    });
}

void shim::add_string_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        /* string.h */
        {"memccpy", ::memccpy},
        {"memchr", (void *(*)(void *, int, size_t)) ::memchr},
        {"memcmp", (int (*)(const void *, const void *, size_t)) ::memcmp},
        {"memcpy", ::memcpy},
        {"__memcpy_chk", __memcpy_chk},
        {"memmove", ::memmove},
        {"__memmove_chk", __memmove_chk},
        {"memset", ::memset},
        {"__memset_chk", ::__memset_chk},
        {"memmem", ::memmem},
#if LIBC_SHIM_DEFINE_VARIADIC
        {"__vsprintf_chk", __vsprintf_chk},
#endif
        {"strchr", (char *(*)(char *, int)) ::strchr},
        {"strrchr", (char *(*)(char *, int)) ::strrchr},
        {"strlen", ::strlen},
        {"__strlen_chk", strlen_chk},
        {"__strchr_chk", strchr_chk},
        {"__strrchr_chk", strrchr_chk},
        {"strcmp", ::strcmp},
        {"strcpy", ::strcpy},
        {"__strcpy_chk", __strcpy_chk},
        {"strcat", ::strcat},
        {"strdup", ::strdup},
        {"strstr", (char *(*)(char *, const char *)) ::strstr},
        {"strtok", ::strtok},
        {"strtok_r", ::strtok_r},
        {"strerror", strerror},
        {"strerror_r", strerror_r},
        {"strnlen", ::strnlen},
        {"strncat", ::strncat},
        {"strndup", ::strndup},
        {"strncmp", ::strncmp},
        {"strncpy", ::strncpy},
        {"__strcat_chk", __strcat_chk},
        {"__strncat_chk", __strncat_chk},
        {"__strncpy_chk", __strncpy_chk},
        {"__strncpy_chk2", __strncpy_chk2},
        {"strlcpy", bionic::strlcpy},
        {"strcspn", ::strcspn},
        {"strpbrk", (char *(*)(char *, const char *)) ::strpbrk},
        {"strsep", ::strsep},
        {"strspn", ::strspn},
        {"strsignal", ::strsignal},
        {"strcoll", ::strcoll},
        {"strxfrm", ::strxfrm},
        {"strcoll_l", ::strcoll_l},
        {"strxfrm_l", ::strxfrm_l},
        {"islower_l", ::islower_l},
        {"isupper_l", ::isupper_l},
        {"tolower_l", ::tolower_l},
        {"toupper_l", ::toupper_l},

        /* strings.h */
        {"bcmp", ::bcmp},
        {"bcopy", ::bcopy},
        {"bzero", ::bzero},
        {"ffs", ::ffs},
        {"index", ::index},
        {"rindex", ::rindex},
        {"strcasecmp", ::strcasecmp},
        {"strncasecmp", ::strncasecmp},
    });
}

void shim::add_socket_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        /* socket.h */
        {"sendfile", sendfile},
        {"__cmsg_nxthdr", __cmsg_nxthdr},
    });
}

void shim::add_wchar_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        /* wchar.h */
        {"wcscat", ::wcscat},
        {"wcscpy", ::wcscpy},
        {"wcsncpy", ::wcsncpy},
        {"wcscmp", ::wcscmp},
        {"wcslen", ::wcslen},
        {"wctob", ::wctob},
        {"btowc", ::btowc},
        {"wmemchr", (wchar_t *(*)(wchar_t *, wchar_t, size_t)) ::wmemchr},
        {"wmemcmp", ::wmemcmp},
        {"wmemcpy", ::wmemcpy},
        {"wmemset", ::wmemset},
        {"wmemmove", ::wmemmove},
        {"wcrtomb", ::wcrtomb},
        {"mbrtowc", ::mbrtowc},
        {"wcscoll", ::wcscoll},
        {"wcsxfrm", ::wcsxfrm},
        {"wcsftime", ::wcsftime},
        {"mbsrtowcs", ::mbsrtowcs},
        {"mbsnrtowcs", ::mbsnrtowcs},
        {"wcsnrtombs", ::wcsnrtombs},
        {"mbrlen", mbrlen},
        {"wcstol", wcstol},
        {"wcstoul", wcstoul},
        {"wcstoll", wcstoll},
        {"wcstoull", wcstoull},
        {"wcstof", wcstof},
        {"wcstod", wcstod},
        {"wcstold", wcstold},
#if LIBC_SHIM_DEFINE_VARIADIC
        {"swprintf", swprintf},
#endif
        {"wcscoll_l", ::wcscoll_l},
        {"wcsxfrm_l", ::wcsxfrm_l},

        /* wctype.h */
        {"wctype", ::wctype},
        {"iswspace", ::iswspace},
        {"iswctype", ::iswctype},
        {"towlower", ::towlower},
        {"towlower_l", ::towlower_l},
        {"towupper", ::towupper},
        {"towupper_l", ::towupper_l},

        {"iswlower",  iswlower},
        {"iswlower_l",  ::iswlower_l},
        {"iswprint",  iswprint},
        {"iswprint_l",  ::iswprint_l},
        {"iswblank",  iswblank},
        {"iswblank_l",  ::iswblank_l},
        {"iswcntrl",  iswcntrl},
        {"iswcntrl_l",  ::iswcntrl_l},
        {"iswupper",  iswupper},
        {"iswupper_l",  ::iswupper_l},
        {"iswalpha",  iswalpha},
        {"iswalpha_l",  ::iswalpha_l},
        {"iswdigit",  iswdigit},
        {"iswdigit_l",  ::iswdigit_l},
        {"iswpunct",  iswpunct},
        {"iswpunct_l",  ::iswpunct_l},
        {"iswxdigit", iswxdigit},
        {"iswxdigit_l", ::iswxdigit_l},
        {"iswspace_l", ::iswspace_l},
    });
}

void shim::add_mman_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        /* sys/mman.h */
        {"mmap", WithErrnoUpdate(mmap)},
        {"munmap", WithErrnoUpdate(::munmap)},
        {"mprotect", WithErrnoUpdate(::mprotect)},
        {"madvise", WithErrnoUpdate(::madvise)},
        {"msync", WithErrnoUpdate(::msync)},
        {"mlock", WithErrnoUpdate(::mlock)},
        {"munlock", WithErrnoUpdate(::munlock)},
#if LIBC_SHIM_DEFINE_VARIADIC
        {"mremap", mremap},
#endif
    });
}

void shim::add_resource_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        /* sys/resource.h */
        {"getrusage", WithErrnoUpdate(getrusage)},
        {"getpriority", WithErrnoUpdate(getpriority)},
        {"getrlimit", WithErrnoUpdate(getrlimit)}
    });
}

void shim::add_prctl_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.push_back({"prctl", WithErrnoUpdate(prctl)});
}

void shim::add_locale_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"newlocale", newlocale},
        {"uselocale", uselocale},
        {"freelocale", freelocale},
        {"setlocale", setlocale},
        {"localeconv", localeconv}
    });
}

void shim::add_setjmp_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
#ifdef USE_BIONIC_SETJMP
        {"setjmp", bionic_setjmp},
        {"longjmp", bionic_longjmp},
#else
        {"setjmp", _setjmp},
        {"longjmp", longjmp},
#endif
#ifdef __arm__
// Needed for Minecraft armv7
        {"sigsetjmp", __sigsetjmp},
        {"siglongjmp", siglongjmp},
#endif
    });
}

void shim::add_misc_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"uname", uname}, // TODO: This may be wrong?

        {"utime", utime},

        {"writev", writev},

        {"openlog", openlog},
        {"closelog", closelog},
        {"syslog", syslog},
#ifndef HAS_ARC4RANDOM_BUF
        {"arc4random", shim::arc4random},
#else
        {"arc4random", ::arc4random},
#endif
    });
}

std::vector<shimmed_symbol> shim::get_shimmed_symbols() {
    std::vector<shimmed_symbol> ret;
    add_common_shimmed_symbols(ret);
    add_stdlib_shimmed_symbols(ret);
    add_malloc_shimmed_symbols(ret);
    add_ctype_shimmed_symbols(ret);
    add_math_shimmed_symbols(ret);
    add_time_shimmed_symbols(ret);
    add_wait_shimmed_symbols(ret);
    add_sched_shimmed_symbols(ret);
    add_unistd_shimmed_symbols(ret);
    add_signal_shimmed_symbols(ret);
    add_string_shimmed_symbols(ret);
    add_wchar_shimmed_symbols(ret);
    add_pthread_shimmed_symbols(ret);
    add_sem_shimmed_symbols(ret);
    add_network_shimmed_symbols(ret);
    add_dirent_shimmed_symbols(ret);
    add_stat_shimmed_symbols(ret);
    add_cstdio_shimmed_symbols(ret);
    add_mman_shimmed_symbols(ret);
    add_resource_shimmed_symbols(ret);
    add_prctl_shimmed_symbols(ret);
    add_locale_shimmed_symbols(ret);
    add_setjmp_shimmed_symbols(ret);
    add_ioctl_shimmed_symbols(ret);
    add_fcntl_shimmed_symbols(ret);
    add_poll_select_shimmed_symbols(ret);
    add_epoll_shimmed_symbols(ret);
    add_misc_shimmed_symbols(ret);
    add_sysconf_shimmed_symbols(ret);
    add_system_properties_shimmed_symbols(ret);
    add_eventfd_shimmed_symbols(ret);
    add_fnmatch_shimmed_symbols(ret);
    add_socket_shimmed_symbols(ret);
    add_statvfs_shimmed_symbols(ret);
    return ret;
}
