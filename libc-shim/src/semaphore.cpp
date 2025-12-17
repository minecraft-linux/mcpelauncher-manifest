#include "no-fortify.h"
#include "semaphore.h"

#include <stdexcept>
#include "meta.h"
#include "errno.h"

#ifdef __APPLE__
#include <errno.h>
#include <mach/mach.h>
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101200
// for shim::clock_gettime 10.10 - 10.12
#include "common.h"
#endif
#endif

using namespace shim;

#ifndef __APPLE__

int shim::sem_init(sem_t *sem, int pshared, unsigned int value) {
    return detail::make_c_wrapped_via_resolver<sem_t_resolver, int, unsigned int>(sem, ::sem_init, pshared, value);
}

int shim::sem_destroy(sem_t *sem) {
    return detail::destroy_c_wrapped_via_resolver<sem_t_resolver>(sem, ::sem_destroy);
}

#else

int shim::sem_init(sem_t *sem, int pshared, unsigned int value) {
    if (pshared)
        handle_runtime_error("sem_init: pshared not supported");

    auto sem_create_wrapper = +[](semaphore_t *sem, int policy, int value) {
        return semaphore_create(mach_task_self(), sem, policy, value);
    };
    int ret = detail::make_c_wrapped_via_resolver<sem_t_resolver, int, int>(sem, sem_create_wrapper, pshared, (int) value);
    if (ret) {
        if (ret == KERN_INVALID_ARGUMENT)
            errno = EINVAL;
        if (ret == KERN_RESOURCE_SHORTAGE)
            errno = ENOMEM;
        bionic::update_errno();
        return -1;
    }
    bionic::update_errno();
    return 0;
}

int shim::sem_destroy(host_sem_t *sem) {
    auto sem_destroy_wrapper = +[](semaphore_t *sem) {
        return semaphore_destroy(mach_task_self(), *sem);
    };
    return detail::destroy_c_wrapped_via_resolver<sem_t_resolver>(sem, sem_destroy_wrapper);
}

int shim::sem_wait(host_sem_t *sem) {
    return semaphore_wait(*sem);
}

int shim::sem_post(host_sem_t *sem) {
    return semaphore_signal(*sem);
}

int shim::sem_timedwait(host_sem_t *sem, const struct timespec *abs_timeout) {
    struct timespec now;
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101200
    shim::clock_gettime(shim::bionic::clock_type::REALTIME, &now);
#else
    clock_gettime(CLOCK_REALTIME, &now);
#endif
    mach_timespec_t ts;
    if  (abs_timeout->tv_sec > now.tv_sec ||
         (abs_timeout->tv_sec == now.tv_sec && abs_timeout->tv_nsec > now.tv_nsec)) {
        ts.tv_sec = (unsigned int) (abs_timeout->tv_sec - now.tv_sec);
        ts.tv_nsec = abs_timeout->tv_nsec - now.tv_nsec;
    } else {
        ts.tv_sec = 0;
        ts.tv_nsec = 0;
    }
    auto ret = semaphore_timedwait(*sem, ts);
    if (ret) {
        if (ret == KERN_OPERATION_TIMED_OUT)
            errno = ETIMEDOUT;
        if (ret == KERN_ABORTED)
            errno = EINTR;
        bionic::update_errno();
        return -1;
    }
    bionic::update_errno();
    return 0;
}

#endif

void shim::add_sem_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"sem_init", sem_init},
        {"sem_destroy", sem_destroy},
        {"sem_wait", &detail::arg_rewrite_helper<int (::sem_t *)>::rewrite<sem_wait>},
        {"sem_timedwait", &detail::arg_rewrite_helper<int (::sem_t *, const struct timespec *)>::rewrite<sem_timedwait>},
        {"sem_post", &detail::arg_rewrite_helper<int (::sem_t *)>::rewrite<sem_post>},
    });
}