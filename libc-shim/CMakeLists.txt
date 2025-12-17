cmake_minimum_required (VERSION 2.6...4.0.0)

project(libc-shim LANGUAGES CXX)

add_library(libc-shim src/common.cpp src/pthreads.cpp src/pthreads.h src/meta.h src/common.h src/semaphore.cpp src/semaphore.h src/network.cpp src/network.h src/dirent.cpp src/dirent.h src/cstdio.cpp src/cstdio.h src/errno.cpp src/errno.h src/ctype_data.h src/ctype_data.cpp src/bionic/strlcpy.cpp src/stat.cpp src/stat.h src/file_misc.cpp src/file_misc.h src/sysconf.cpp src/sysconf.h src/system_properties.cpp src/system_properties.h src/iorewrite.cpp src/iorewrite.h src/statvfs.h src/statvfs.cpp src/sched.h src/sched.cpp)
target_include_directories(libc-shim PUBLIC include/)
target_link_libraries(libc-shim PRIVATE logger)

if(APPLE OR FREEBSD)
    target_link_libraries(libc-shim PRIVATE epoll-shim)
endif()

if(APPLE AND NOT IS_64BIT)
    target_sources(libc-shim PUBLIC src/bionic/arch-x86/setjmp.S src/bionic/setjmp_cookie.c)
    target_compile_definitions(libc-shim PRIVATE USE_BIONIC_SETJMP)
endif()

include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
    #include <atomic>
    struct A {
        long long a;
        void* wrapper;
        long long b;
    };
    int main() {
        std::atomic<A> a;
        auto a2 = a.load();
        auto a3 = a2;
        a3.wrapper = &a;
        return a.compare_exchange_strong(a2, a3, std::memory_order_relaxed, std::memory_order_relaxed) ? 0 : 1;
    }
" HAVE_ATOMICS_WITHOUT_LIB)

# Link atomic library if needed
if(NOT HAVE_ATOMICS_WITHOUT_LIB)
    target_link_libraries(libc-shim PRIVATE atomic)
endif()
