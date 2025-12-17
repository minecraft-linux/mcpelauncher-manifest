#include "no-fortify.h"
#include "sysconf.h"

#include <unistd.h>
#include <stdexcept>

long shim::sysconf(int name) {
    return ::sysconf(bionic::to_host_sysconf_id(name));
}

void shim::add_sysconf_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.push_back({"sysconf", sysconf});
}

int shim::bionic::to_host_sysconf_id(int arg) {
    switch (arg) {
        case 0x0000: return _SC_ARG_MAX;
        case 0x0001: return _SC_BC_BASE_MAX;
        case 0x0002: return _SC_BC_DIM_MAX;
        case 0x0003: return _SC_BC_SCALE_MAX;
        case 0x0004: return _SC_BC_STRING_MAX;
        case 0x0005: return _SC_CHILD_MAX;
        case 0x0006: return _SC_CLK_TCK;
        case 0x0007: return _SC_COLL_WEIGHTS_MAX;
        case 0x0008: return _SC_EXPR_NEST_MAX;
        case 0x0009: return _SC_LINE_MAX;
        case 0x000a: return _SC_NGROUPS_MAX;
        case 0x000b: return _SC_OPEN_MAX;
#ifdef _SC_PASS_MAX
        case 0x000c: return _SC_PASS_MAX;
#endif
        case 0x000d: return _SC_2_C_BIND;
        case 0x000e: return _SC_2_C_DEV;
        case 0x0010: return _SC_2_CHAR_TERM;
        case 0x0011: return _SC_2_FORT_DEV;
        case 0x0012: return _SC_2_FORT_RUN;
        case 0x0013: return _SC_2_LOCALEDEF;
        case 0x0014: return _SC_2_SW_DEV;
        case 0x0015: return _SC_2_UPE;
        case 0x0016: return _SC_2_VERSION;
        case 0x0017: return _SC_JOB_CONTROL;
        case 0x0018: return _SC_SAVED_IDS;
        case 0x0019: return _SC_VERSION;
        case 0x001a: return _SC_RE_DUP_MAX;
        case 0x001b: return _SC_STREAM_MAX;
        case 0x001c: return _SC_TZNAME_MAX;
        case 0x001d: return _SC_XOPEN_CRYPT;
        case 0x001e: return _SC_XOPEN_ENH_I18N;
        case 0x001f: return _SC_XOPEN_SHM;
        case 0x0020: return _SC_XOPEN_VERSION;
        case 0x0022: return _SC_XOPEN_REALTIME;
        case 0x0023: return _SC_XOPEN_REALTIME_THREADS;
        case 0x0024: return _SC_XOPEN_LEGACY;
        case 0x0025: return _SC_ATEXIT_MAX;
        case 0x0026: return _SC_IOV_MAX;
        case 0x0027: return _SC_PAGESIZE;
        case 0x0028: return _SC_PAGE_SIZE;
        case 0x0029: return _SC_XOPEN_UNIX;
        case 0x002e: return _SC_AIO_LISTIO_MAX;
        case 0x002f: return _SC_AIO_MAX;
        case 0x0030: return _SC_AIO_PRIO_DELTA_MAX;
        case 0x0031: return _SC_DELAYTIMER_MAX;
        case 0x0032: return _SC_MQ_OPEN_MAX;
        case 0x0033: return _SC_MQ_PRIO_MAX;
        case 0x0034: return _SC_RTSIG_MAX;
        case 0x0035: return _SC_SEM_NSEMS_MAX;
        case 0x0036: return _SC_SEM_VALUE_MAX;
        case 0x0037: return _SC_SIGQUEUE_MAX;
        case 0x0038: return _SC_TIMER_MAX;
        case 0x0039: return _SC_ASYNCHRONOUS_IO;
        case 0x003a: return _SC_FSYNC;
        case 0x003b: return _SC_MAPPED_FILES;
        case 0x003c: return _SC_MEMLOCK;
        case 0x003d: return _SC_MEMLOCK_RANGE;
        case 0x003e: return _SC_MEMORY_PROTECTION;
        case 0x003f: return _SC_MESSAGE_PASSING;
        case 0x0040: return _SC_PRIORITIZED_IO;
        case 0x0041: return _SC_PRIORITY_SCHEDULING;
        case 0x0042: return _SC_REALTIME_SIGNALS;
        case 0x0043: return _SC_SEMAPHORES;
        case 0x0044: return _SC_SHARED_MEMORY_OBJECTS;
        case 0x0045: return _SC_SYNCHRONIZED_IO;
        case 0x0046: return _SC_TIMERS;
        case 0x0047: return _SC_GETGR_R_SIZE_MAX;
        case 0x0048: return _SC_GETPW_R_SIZE_MAX;
        case 0x0049: return _SC_LOGIN_NAME_MAX;
        case 0x004a: return _SC_THREAD_DESTRUCTOR_ITERATIONS;
        case 0x004b: return _SC_THREAD_KEYS_MAX;
        case 0x004c: return _SC_THREAD_STACK_MIN;
        case 0x004d: return _SC_THREAD_THREADS_MAX;
        case 0x004e: return _SC_TTY_NAME_MAX;
        case 0x004f: return _SC_THREADS;
        case 0x0050: return _SC_THREAD_ATTR_STACKADDR;
        case 0x0051: return _SC_THREAD_ATTR_STACKSIZE;
        case 0x0052: return _SC_THREAD_PRIORITY_SCHEDULING;
        case 0x0053: return _SC_THREAD_PRIO_INHERIT;
        case 0x0054: return _SC_THREAD_PRIO_PROTECT;
        case 0x0055: return _SC_THREAD_SAFE_FUNCTIONS;
        case 0x0060: return _SC_NPROCESSORS_CONF;
        case 0x0061: return _SC_NPROCESSORS_ONLN;
        case 0x0062: return _SC_PHYS_PAGES;
#ifdef _SC_AVPHYS_PAGES
        case 0x0063: return _SC_AVPHYS_PAGES;
#endif
        case 0x0064: return _SC_MONOTONIC_CLOCK;
        default: handle_runtime_error("Unknown sysconf id %d", arg);
    }
}
