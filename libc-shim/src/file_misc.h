#pragma once

#include <libc_shim.h>
#include <poll.h>
#include "common.h"

namespace shim {

    namespace bionic {

        enum class file_status_flags : int {
            RDONLY = 00,
            WRONLY = 01,
            RDWR = 02,
            CREAT = 0100,
            EXCL = 0200,
            NOCTTY = 0400,
            TRUNC = 01000,
            APPEND = 02000,
            NONBLOCK = 04000,
            CLOEXEC = 02000000,

            KNOWN_FLAGS = RDONLY|WRONLY|RDWR|CREAT|EXCL|NOCTTY|TRUNC|APPEND|NONBLOCK|CLOEXEC
        };

        int to_host_file_status_flags(bionic::file_status_flags flags);
        bionic::file_status_flags from_host_file_status_flags(int flags);

        enum class ioctl_index : unsigned long {
            FILE_NBIO = 0x5421,

            SOCKET_CGIFCONF = 0x8912,
            SOCKET_CGIFNETMASK = 0x891b
        };

        enum class fcntl_index : int {
            GETFD = 1,
            SETFD = 2,
            GETFL = 3,
            SETFL = 4,
            SETLK = 6
        };

        struct flock {
            short l_type;
            short l_whence;
            off_t l_start;
            off_t l_len;
            int l_pid;
        };

    }

    int ioctl(int fd, bionic::ioctl_index cmd, void *arg);

    int open(const char *pathname, bionic::file_status_flags flags, ...);

    int openat(int dirfd, const char *pathname, bionic::file_status_flags flags, ...);

    int open_2(const char *pathname, bionic::file_status_flags flags);

    int open_3(const char *pathname, bionic::file_status_flags flags, int mode);

    int fcntl(int fd, bionic::fcntl_index cmd, void *arg);

    int poll_via_select(pollfd *fds, nfds_t nfds, int timeout);

    int __FD_ISSET_chk(int fd, fd_set *set);
    void __FD_CLR_chk(int fd, fd_set *set);
    void __FD_SET_chk(int fd, fd_set *set);

    void add_ioctl_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_fcntl_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_poll_select_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_epoll_shimmed_symbols(std::vector<shimmed_symbol> &list);

    void add_eventfd_shimmed_symbols(std::vector<shimmed_symbol> &list);

}