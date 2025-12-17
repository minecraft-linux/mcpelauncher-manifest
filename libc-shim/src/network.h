#pragma once

#include <libc_shim.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdexcept>
#include <netdb.h>
#include "argrewrite.h"

#ifdef __FreeBSD__
// kinda hacky:
// this header re#defines inet_ntop/inet_pton as __inet_ntop
// so while namespace will be incompatible generic way (and we don't need to),
// I don't need to put other crutches.
#include <arpa/inet.h>
#endif

namespace shim {

    namespace bionic {

        using socklen_t = int;

        enum class ai_flags : int {
            PASSIVE = 1,
            CANONNAME = 2,
            NUMERICHOST = 4
        };

        enum class af_family : int {
            UNSPEC = 0,
            UNIX = 1,
            INET = 2,
            INET6 = 10,
            NETLINK = 16
        };

        enum class socktype : int {
            STREAM = 1,
            DGRAM = 2,
            RAW = 3
        };

        enum class ipproto : int {
            IP = 0,
            TCP = 6,
            UDP = 17,
            IPV6 = 41
        };

        ai_flags from_host_ai_flags(int flags);

        int to_host_ai_flags(ai_flags flags);

        af_family from_host_af_family(int family);

        int to_host_af_family(af_family family);

        socktype from_host_socktype(int socktype);

        int to_host_socktype(socktype socktype);

        ipproto from_host_ipproto(int proto);

        int to_host_ipproto(ipproto proto);

        struct sockaddr {
            uint16_t family;
        };
        struct sockaddr_un : sockaddr {
            char sun_path[108];
        };
        struct sockaddr_in : sockaddr {
            uint16_t port;
            uint32_t addr;
            char filler[8];
        };
        struct sockaddr_in6 : sockaddr {
            uint16_t port;
            uint32_t flow_info;
            uint8_t addr[16];
            uint32_t scope;
        };

        void from_host(const ::sockaddr *in, sockaddr *out);

        void to_host(const sockaddr *in, ::sockaddr *out);

        size_t get_host_len(const sockaddr *in);

        size_t get_bionic_len(const ::sockaddr *in);

        struct addrinfo {
            ai_flags ai_flags;
            af_family ai_family;
            socktype ai_socktype;
            ipproto ai_protocol;
            uint32_t ai_addrlen;
            char *ai_canonname;
            struct sockaddr *ai_addr;
            struct addrinfo *ai_next;
        };

        struct ifaddrs {
            struct ifaddrs* ifa_next;
            char* ifa_name;
            unsigned int ifa_flags;
            struct sockaddr* ifa_addr;
            struct sockaddr* ifa_netmask;
        };

        addrinfo *from_host_alloc(const ::addrinfo *in);

        void free_bionic_list(addrinfo *list);

        ::addrinfo *to_host_alloc(const addrinfo *in);

        void free_host_list(::addrinfo *list);

        /* nameinfo */

        enum class nameinfo_flags : int {
            NOFQDN = 1,
            NUMERICHOST = 2,
            NAMEREQD = 4,
            NUMERICSERV = 8,
            DGRAM = 16
        };

        int to_host_nameinfo_flags(nameinfo_flags flags);

        /* sockopt */

        int to_host_sockopt_so(int opt);
        int to_host_sockopt_ip(int opt);
        int to_host_sockopt_ipv6(int opt);

        int to_host_sockopt_level(int level);
        int to_host_sockopt(int level, int opt);

        /* ioctl */

        struct ifreq {
            char name[16];
            sockaddr_in addr;
#ifdef __LP64__
            char padding[8];
#endif
        };
        struct ifconf {
            int len;
            ifreq *req;
        };

#ifdef __LP64__
        static_assert(sizeof(ifreq) == 40, "ifreq must be 40 bytes big");
#else
        static_assert(sizeof(ifreq) == 32, "ifreq must be 32 bytes big");
#endif

    }

    int socket(bionic::af_family domain, bionic::socktype type, bionic::ipproto proto);

    int getaddrinfo(const char *node, const char *service, const bionic::addrinfo *hints, bionic::addrinfo **res);

    void freeaddrinfo(bionic::addrinfo *ai);

    int getnameinfo(const bionic::sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen,
            char *serv, socklen_t servlen, bionic::nameinfo_flags flags);

    int bind(int sockfd, const bionic::sockaddr *addr, socklen_t addrlen);

    int connect(int sockfd, const bionic::sockaddr *addr, socklen_t addrlen);

    ssize_t send(int sockfd, const void *buf, size_t len, int flags);

    ssize_t sendmsg(int sockfd, const msghdr *data, int flags);

    ssize_t recv(int sockfd, void *buf, size_t len, int flags);

    ssize_t recvmsg(int sockfd, struct msghdr *data, int flags);

    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const bionic::sockaddr *addr, socklen_t addrlen);

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, bionic::sockaddr *addr, socklen_t *addrlen);

    int getsockname(int sockfd, bionic::sockaddr *addr, socklen_t *addrlen);

    int getpeername(int sockfd, bionic::sockaddr *addr, socklen_t *addrlen);

    int accept(int sockfd, bionic::sockaddr *addr, socklen_t *addrlen);
    int accept4(int sockfd, bionic::sockaddr *addr, socklen_t *addrlen, int flag);

    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);

    int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

    int shutdown(int sockfd, int how);

    void add_network_shimmed_symbols(std::vector<shimmed_symbol> &list);

    int getifaddrs(bionic::ifaddrs** result);

    void freeifaddrs(bionic::ifaddrs *__ifa);

    int inet_pton(bionic::af_family af, const char *src, void *dst);

    const char * inet_ntop(bionic::af_family af, const void *src, char *dst, socklen_t size);

    namespace detail {

        struct sockaddr_in {
            sockaddr_storage haddr = {};
            socklen_t len = sizeof(sockaddr_storage);

            sockaddr_in(const bionic::sockaddr *addr, socklen_t) {
                if (addr) {
                    bionic::to_host(addr, ptr());
                    len = bionic::get_host_len(addr);
                } else {
                    len = 0;
                }
            }

            ::sockaddr *ptr() { return len ? (::sockaddr *) &haddr : nullptr; }
        };

        struct sockaddr_out {
            sockaddr_storage haddr = {};
            socklen_t len = sizeof(sockaddr_storage);

            ::sockaddr *ptr() { return (::sockaddr *) &haddr; }

            void apply(bionic::sockaddr *addr, socklen_t *addrlen) {
                if (bionic::get_bionic_len((::sockaddr *) &haddr) > *addrlen)
                    handle_runtime_error("buffer for sockaddr not big enough");
                bionic::from_host((::sockaddr *) &haddr, addr);
                *addrlen = bionic::get_bionic_len((::sockaddr *) &haddr);
            }
        };

        struct sock_send_flags {
            int fd;
            int src_flags;
            int flags;
#ifdef __APPLE__
            int saved_nosigpipe = 0;
#endif

            sock_send_flags(int fd, int flags);

            ~sock_send_flags();
        };

    }

}
