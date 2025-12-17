#include "errno.h"

#include <cerrno>
#include <cstring>

char *shim::strerror(int err) {
    return ::strerror(bionic::translate_errno_to_host(err));
}

int shim::strerror_r(int err, char* buf, size_t len) {
#ifdef __USE_GNU
    if (::strerror_r(bionic::translate_errno_to_host(err), buf, len) != nullptr)
        return 0;
    return -1;
#else
    return ::strerror_r(bionic::translate_errno_to_host(err), buf, len);
#endif
}

#ifdef ERRNO_TRANSLATION

thread_local int shim::bionic::errno_value;

void shim::bionic::update_errno() {
    errno_value = translate_errno_from_host(errno);
}

void shim::bionic::sync_errno() {
    errno = translate_errno_to_host(errno_value);
}

int *shim::bionic::get_errno() {
    return &errno_value;
}

void shim::bionic::set_errno(int err) {
    errno_value = err;
    errno = translate_errno_to_host(err);
}

#else

int *shim::bionic::get_errno() {
    return &errno;
}

void shim::bionic::set_errno(int err) {
    errno = err;
}

#endif

int shim::bionic::translate_errno_from_host(int err) {
    switch (err) {
        case EPERM: return 1;
        case ENOENT: return 2;
        case ESRCH: return 3;
        case EINTR: return 4;
        case EIO: return 5;
        case ENXIO: return 6;
        case E2BIG: return 7;
        case ENOEXEC: return 8;
        case EBADF: return 9;
        case ECHILD: return 10;
        case EAGAIN: return 11;
        case ENOMEM: return 12;
        case EACCES: return 13;
        case EFAULT: return 14;
        case ENOTBLK: return 15;
        case EBUSY: return 16;
        case EEXIST: return 17;
        case EXDEV: return 18;
        case ENODEV: return 19;
        case ENOTDIR: return 20;
        case EISDIR: return 21;
        case EINVAL: return 22;
        case ENFILE: return 23;
        case EMFILE: return 24;
        case ENOTTY: return 25;
        case ETXTBSY: return 26;
        case EFBIG: return 27;
        case ENOSPC: return 28;
        case ESPIPE: return 29;
        case EROFS: return 30;
        case EMLINK: return 31;
        case EPIPE: return 32;
        case EDOM: return 33;
        case ERANGE: return 34;
        case EDEADLK: return 35;
        case ENAMETOOLONG: return 36;
        case ENOLCK: return 37;
        case ENOSYS: return 38;
        case ENOTEMPTY: return 39;
        case ELOOP: return 40;
        case ENOMSG: return 42;
        case EIDRM: return 43;
#ifdef __linux__
        case ECHRNG: return 44;
        case EL2NSYNC: return 45;
        case EL3HLT: return 46;
        case EL3RST: return 47;
        case ELNRNG: return 48;
        case EUNATCH: return 49;
        case ENOCSI: return 50;
        case EL2HLT: return 51;
        case EBADE: return 52;
        case EBADR: return 53;
        case EXFULL: return 54;
        case ENOANO: return 55;
        case EBADRQC: return 56;
        case EBADSLT: return 57;
        case EBFONT: return 59;
#endif
        case ENOSTR: return 60;
        case ENODATA: return 61;
        case ETIME: return 62;
        case ENOSR: return 63;
#ifdef __linux__
        case ENONET: return 64;
        case ENOPKG: return 65;
#endif
        case EREMOTE: return 66;
        case ENOLINK: return 67;
#ifdef __linux__
        case EADV: return 68;
        case ESRMNT: return 69;
        case ECOMM: return 70;
#endif
        case EPROTO: return 71;
        case EMULTIHOP: return 72;
#ifdef __linux__
        case EDOTDOT: return 73;
#endif
        case EBADMSG: return 74;
        case EOVERFLOW: return 75;
#ifdef __linux__
        case ENOTUNIQ: return 76;
        case EBADFD: return 77;
        case EREMCHG: return 78;
        case ELIBACC: return 79;
        case ELIBBAD: return 80;
        case ELIBSCN: return 81;
        case ELIBMAX: return 82;
        case ELIBEXEC: return 83;
#endif
        case EILSEQ: return 84;
#ifdef __linux__
        case ERESTART: return 85;
        case ESTRPIPE: return 86;
#endif
        case EUSERS: return 87;
        case ENOTSOCK: return 88;
        case EDESTADDRREQ: return 89;
        case EMSGSIZE: return 90;
        case EPROTOTYPE: return 91;
        case ENOPROTOOPT: return 92;
        case EPROTONOSUPPORT: return 93;
        case ESOCKTNOSUPPORT: return 94;
        case EOPNOTSUPP: return 95;
        case EPFNOSUPPORT: return 96;
        case EAFNOSUPPORT: return 97;
        case EADDRINUSE: return 98;
        case EADDRNOTAVAIL: return 99;
        case ENETDOWN: return 100;
        case ENETUNREACH: return 101;
        case ENETRESET: return 102;
        case ECONNABORTED: return 103;
        case ECONNRESET: return 104;
        case ENOBUFS: return 105;
        case EISCONN: return 106;
        case ENOTCONN: return 107;
        case ESHUTDOWN: return 108;
        case ETOOMANYREFS: return 109;
        case ETIMEDOUT: return 110;
        case ECONNREFUSED: return 111;
        case EHOSTDOWN: return 112;
        case EHOSTUNREACH: return 113;
        case EALREADY: return 114;
        case EINPROGRESS: return 115;
        case ESTALE: return 116;
#ifdef __linux__
        case EUCLEAN: return 117;
        case ENOTNAM: return 118;
        case ENAVAIL: return 119;
        case EISNAM: return 120;
        case EREMOTEIO: return 121;
#endif
        case EDQUOT: return 122;
#ifdef __linux__
        case ENOMEDIUM: return 123;
        case EMEDIUMTYPE: return 124;
#endif
        case ECANCELED: return 125;
#ifdef __linux__
        case ENOKEY: return 126;
        case EKEYEXPIRED: return 127;
        case EKEYREVOKED: return 128;
        case EKEYREJECTED: return 129;
#endif
        case EOWNERDEAD: return 130;
        case ENOTRECOVERABLE: return 131;
        default: return err;
    }
}

int shim::bionic::translate_errno_to_host(int err) {
    switch (err) {
        case 1: return EPERM;
        case 2: return ENOENT;
        case 3: return ESRCH;
        case 4: return EINTR;
        case 5: return EIO;
        case 6: return ENXIO;
        case 7: return E2BIG;
        case 8: return ENOEXEC;
        case 9: return EBADF;
        case 10: return ECHILD;
        case 11: return EAGAIN;
        case 12: return ENOMEM;
        case 13: return EACCES;
        case 14: return EFAULT;
        case 15: return ENOTBLK;
        case 16: return EBUSY;
        case 17: return EEXIST;
        case 18: return EXDEV;
        case 19: return ENODEV;
        case 20: return ENOTDIR;
        case 21: return EISDIR;
        case 22: return EINVAL;
        case 23: return ENFILE;
        case 24: return EMFILE;
        case 25: return ENOTTY;
        case 26: return ETXTBSY;
        case 27: return EFBIG;
        case 28: return ENOSPC;
        case 29: return ESPIPE;
        case 30: return EROFS;
        case 31: return EMLINK;
        case 32: return EPIPE;
        case 33: return EDOM;
        case 34: return ERANGE;
        case 35: return EDEADLK;
        case 36: return ENAMETOOLONG;
        case 37: return ENOLCK;
        case 38: return ENOSYS;
        case 39: return ENOTEMPTY;
        case 40: return ELOOP;
        case 42: return ENOMSG;
        case 43: return EIDRM;
#ifdef __linux__
        case 44: return ECHRNG;
        case 45: return EL2NSYNC;
        case 46: return EL3HLT;
        case 47: return EL3RST;
        case 48: return ELNRNG;
        case 49: return EUNATCH;
        case 50: return ENOCSI;
        case 51: return EL2HLT;
        case 52: return EBADE;
        case 53: return EBADR;
        case 54: return EXFULL;
        case 55: return ENOANO;
        case 56: return EBADRQC;
        case 57: return EBADSLT;
        case 59: return EBFONT;
#endif
        case 60: return ENOSTR;
        case 61: return ENODATA;
        case 62: return ETIME;
        case 63: return ENOSR;
#ifdef __linux__
        case 64: return ENONET;
        case 65: return ENOPKG;
#endif
        case 66: return EREMOTE;
        case 67: return ENOLINK;
#ifdef __linux__
        case 68: return EADV;
        case 69: return ESRMNT;
        case 70: return ECOMM;
#endif
        case 71: return EPROTO;
        case 72: return EMULTIHOP;
#ifdef __linux__
        case 73: return EDOTDOT;
#endif
        case 74: return EBADMSG;
        case 75: return EOVERFLOW;
#ifdef __linux__
        case 76: return ENOTUNIQ;
        case 77: return EBADFD;
        case 78: return EREMCHG;
        case 79: return ELIBACC;
        case 80: return ELIBBAD;
        case 81: return ELIBSCN;
        case 82: return ELIBMAX;
        case 83: return ELIBEXEC;
#endif
        case 84: return EILSEQ;
#ifdef __linux__
        case 85: return ERESTART;
        case 86: return ESTRPIPE;
#endif
        case 87: return EUSERS;
        case 88: return ENOTSOCK;
        case 89: return EDESTADDRREQ;
        case 90: return EMSGSIZE;
        case 91: return EPROTOTYPE;
        case 92: return ENOPROTOOPT;
        case 93: return EPROTONOSUPPORT;
        case 94: return ESOCKTNOSUPPORT;
        case 95: return EOPNOTSUPP;
        case 96: return EPFNOSUPPORT;
        case 97: return EAFNOSUPPORT;
        case 98: return EADDRINUSE;
        case 99: return EADDRNOTAVAIL;
        case 100: return ENETDOWN;
        case 101: return ENETUNREACH;
        case 102: return ENETRESET;
        case 103: return ECONNABORTED;
        case 104: return ECONNRESET;
        case 105: return ENOBUFS;
        case 106: return EISCONN;
        case 107: return ENOTCONN;
        case 108: return ESHUTDOWN;
        case 109: return ETOOMANYREFS;
        case 110: return ETIMEDOUT;
        case 111: return ECONNREFUSED;
        case 112: return EHOSTDOWN;
        case 113: return EHOSTUNREACH;
        case 114: return EALREADY;
        case 115: return EINPROGRESS;
        case 116: return ESTALE;
#ifdef __linux__
        case 117: return EUCLEAN;
        case 118: return ENOTNAM;
        case 119: return ENAVAIL;
        case 120: return EISNAM;
        case 121: return EREMOTEIO;
#endif
        case 122: return EDQUOT;
#ifdef __linux__
        case 123: return ENOMEDIUM;
        case 124: return EMEDIUMTYPE;
#endif
        case 125: return ECANCELED;
#ifdef __linux__
        case 126: return ENOKEY;
        case 127: return EKEYEXPIRED;
        case 128: return EKEYREVOKED;
        case 129: return EKEYREJECTED;
#endif
        case 130: return EOWNERDEAD;
        case 131: return ENOTRECOVERABLE;
        default: return err;
    }
}