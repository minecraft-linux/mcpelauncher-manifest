#include <atomic>
#include <cerrno>
#include <stdexcept>
#include <limits.h>
#include <mutex>
#include <signal.h>
#include "pthreads.h"
#include "errno.h"
#ifdef __FreeBSD__
#include <pthread_np.h>
#endif

using namespace shim;

thread_local bionic::pthread_cleanup_holder bionic::cleanup;

bionic::pthread_cleanup_holder::~pthread_cleanup_holder() {
    auto p = current;
    while (p) {
        p->routine(p->arg);
        p = p->prev;
    }
}

void bionic::mutex_static_initializer(shim::pthread_mutex_t *mutex, shim::pthread_mutex_t &old) {
    if (is_mutex_initialized(mutex))
        return;

#ifdef __LP64__
    auto init_value = mutex->init_value;
#else
    auto init_value = (size_t) mutex->wrapped;
#endif
    shim::pthread_mutex_t n = old;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (init_value == recursive_mutex_init_value)
        pthread_mutexattr_settype(&attr, (int) mutex_type::RECURSIVE);
    else if (init_value == errorcheck_mutex_init_value)
        pthread_mutexattr_settype(&attr, (int) mutex_type::ERRORCHECK);
    if (pthread_mutex_init(&n, &attr) != 0)
        handle_runtime_error("Failed to init mutex");
    pthread_mutexattr_destroy(&attr);

    if(!detail::update_wrapper(mutex, old, n)) {
        // Concurrent creation, we need to destroy the new one
        detail::destroy_c_wrapped<pthread_mutex_t>(&n, &::pthread_mutex_destroy);
    } else {
        old = n;
    }
}

void bionic::cond_static_initializer(shim::pthread_cond_t *cond, shim::pthread_cond_t &old) {
    if (is_cond_initialized(&old))
        return;
    shim::pthread_cond_t n = old;
    if(pthread_cond_init(&n, nullptr) == 0) {
        if(!detail::update_wrapper(cond, old, n)) {
            // Concurrent creation, we need to destroy the new one
            detail::destroy_c_wrapped<pthread_cond_t>(&n, &::pthread_cond_destroy);
        } else {
            old = n;
        }
    } else {
        handle_runtime_error("Failed to init cond");
    }
}

void bionic::rwlock_static_initializer(shim::pthread_rwlock_t *rwlock, shim::pthread_rwlock_t &old) {
    if (is_rwlock_initialized(&old))
        return;
    shim::pthread_rwlock_t n = old;
    if(pthread_rwlock_init(&n, nullptr) == 0) {
        if(!detail::update_wrapper(rwlock, old, n)) {
            // Concurrent creation, we need to destroy the new one
            detail::destroy_c_wrapped<pthread_rwlock_t>(&n, &::pthread_rwlock_destroy);
        } else {
            old = n;
        }
    } else {
        handle_runtime_error("Failed to init rwlock");
    }
}

int bionic::to_host_mutex_type(bionic::mutex_type type) {
    switch (type) {
        case mutex_type::NORMAL: return PTHREAD_MUTEX_NORMAL;
        case mutex_type::RECURSIVE: return PTHREAD_MUTEX_RECURSIVE;
        case mutex_type::ERRORCHECK: return PTHREAD_MUTEX_ERRORCHECK;
        default: handle_runtime_error("Invalid mutex type");
    }
}

int bionic::to_host_sched_policy(shim::bionic::sched_policy type) {
    switch (type) {
        case sched_policy::OTHER: return SCHED_OTHER;
        default: handle_runtime_error("Invalid sched policy");
    }
}

bionic::sched_policy bionic::from_host_sched_policy(int type) {
    switch (type) {
        case SCHED_OTHER: return sched_policy::OTHER;
        default: return (sched_policy) -1;
    }
}

host_pthread_attr::host_pthread_attr(shim::pthread_attr_t const *bionic_attr) {
    ::pthread_attr_init(&attr);
    if (bionic_attr) {
        ::pthread_attr_setdetachstate(&attr, bionic_attr->detached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);
        if (bionic_attr->stack_size)
            ::pthread_attr_setstacksize(&attr, bionic_attr->stack_size);
        if (bionic_attr->sched_priority) {
            sched_param param;
            ::pthread_attr_getschedparam(&attr, &param);
            param.sched_priority = bionic_attr->sched_priority;
            ::pthread_attr_setschedparam(&attr, &param);
        }
    }
}

host_pthread_attr::~host_pthread_attr() {
    ::pthread_attr_destroy(&attr);
}

host_mutexattr::host_mutexattr(pthread_mutexattr_t const *bionic_attr) {
    ::pthread_mutexattr_init(&attr);
    if (bionic_attr)
        ::pthread_mutexattr_settype(&attr, bionic::to_host_mutex_type(bionic_attr->type));
}

host_mutexattr::~host_mutexattr() {
    ::pthread_mutexattr_destroy(&attr);
}

host_condattr::host_condattr(pthread_condattr_t const *bionic_attr) {
    ::pthread_condattr_init(&attr);
#ifndef __APPLE__
    if (bionic_attr)
        ::pthread_condattr_setclock(&attr, bionic::to_host_clock_type(bionic_attr->clock));
#endif
}

host_condattr::~host_condattr() {
    ::pthread_condattr_destroy(&attr);
}

static_assert(sizeof(pthread_t) <= sizeof(long), "pthread_t larger than bionic's not supported");

int shim::pthread_create(pthread_t *thread, const shim::pthread_attr_t *attr, void *(*fn)(void *), void *arg) {
    host_pthread_attr hattr (attr);
    int ret = pthread_create(thread, &hattr.attr, fn, arg);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_setschedparam(pthread_t thread, bionic::sched_policy policy, const shim::bionic::sched_param *param) {
    int hpolicy;
    sched_param hparam;
    int ret = ::pthread_getschedparam(thread, &hpolicy, &hparam);
    if (ret != 0)
        return bionic::translate_errno_from_host(ret);
    if (policy != (bionic::sched_policy) -1)
        hpolicy = bionic::to_host_sched_policy(policy);
    hparam.sched_priority = param->sched_priority;
    ret = ::pthread_setschedparam(thread, hpolicy, &hparam);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_getschedparam(pthread_t thread, shim::bionic::sched_policy *policy, shim::bionic::sched_param *param) {
    int hpolicy;
    sched_param hparam;
    int ret = ::pthread_getschedparam(thread, &hpolicy, &hparam);
    if (ret != 0)
        return bionic::translate_errno_from_host(ret);
    *policy = bionic::from_host_sched_policy(hpolicy);
    param->sched_priority = hparam.sched_priority;
    return 0;
}

int shim::pthread_getattr_np(pthread_t th, pthread_attr_t* attr) {
#ifdef __APPLE__
    pthread_attr_init(attr);
    int hpolicy;
    sched_param hparam;
    int ret = ::pthread_getschedparam(th, &hpolicy, &hparam);
    if (ret != 0)
        return bionic::translate_errno_from_host(ret);
    attr->sched_priority = hparam.sched_priority;
    void* stackaddr = pthread_get_stackaddr_np(th);
    size_t stacksize = pthread_get_stacksize_np(th);
    attr->stack_base = stackaddr;
    attr->stack_size = stacksize;
#else
    ::pthread_attr_t hostattr;
#ifdef __FreeBSD__
    int ret = ::pthread_attr_get_np(th, &hostattr);
#else
    int ret = ::pthread_getattr_np(th, &hostattr);
#endif
    if (ret != 0)
        return bionic::translate_errno_from_host(ret);
    pthread_attr_init(attr);
    int detached = 0;
    ::pthread_attr_getdetachstate(&hostattr, &detached);
    pthread_attr_setdetachstate(attr, detached);
    sched_param hparam;
    ::pthread_attr_getschedparam(&hostattr, &hparam);
    attr->sched_priority = hparam.sched_priority;
    void* stackaddr = nullptr;
    size_t stacksize = 0;
    ::pthread_attr_getstack(&hostattr, &stackaddr, &stacksize);
    attr->stack_base = stackaddr;
    attr->stack_size = stacksize;
#endif
    return 0;
}

int shim::pthread_attr_init(pthread_attr_t *attr) {
    *attr = pthread_attr_t{false, false, 0, nullptr, 0, 0, bionic::sched_policy::OTHER, 0};
    return 0;
}

int shim::pthread_attr_destroy(shim::pthread_attr_t *attr) {
    return 0;
}

int shim::pthread_attr_setdetachstate(shim::pthread_attr_t *attr, int value) {
    if (value != 0 && value != 1)
        return EINVAL;
    attr->detached = value;
    return 0;
}

int shim::pthread_attr_getdetachstate(shim::pthread_attr_t *attr, int *value) {
    *value = attr->detached;
    return 0;
}

int shim::pthread_attr_setschedparam(shim::pthread_attr_t *attr, const shim::bionic::sched_param *value) {
    attr->sched_priority = value->sched_priority;
    return 0;
}

int shim::pthread_attr_getschedparam(shim::pthread_attr_t *attr, shim::bionic::sched_param *value) {
    value->sched_priority = attr->sched_priority;
    return 0;
}

int shim::pthread_attr_setstacksize(shim::pthread_attr_t *attr, size_t value) {
    if (value < PTHREAD_STACK_MIN)
        return 0;
    attr->stack_size = value;
    return 0;
}

int shim::pthread_attr_getstack(shim::pthread_attr_t *attr, void **stackaddr, size_t *stacksize) {
    if (stackaddr) {
        *stackaddr = attr->stack_base;
    }
    if (stacksize) {
        *stacksize = attr->stack_size;
    }
    return 0;
}

int shim::pthread_attr_getstacksize(shim::pthread_attr_t *attr, size_t *value) {
    *value = attr->stack_size;
    return 0;
}

int shim::pthread_setname_np(pthread_t thread, const char* name) {
#ifdef __linux__
    return ::pthread_setname_np(thread,name);
#else
    return 0;
#endif
}

int shim::pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    host_mutexattr hattr (attr);
    int ret = detail::make_c_wrapped<pthread_mutex_t, const ::pthread_mutexattr_t *>(mutex, &::pthread_mutex_init, &hattr.attr);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_mutex_destroy(pthread_mutex_t *wrapper) {
    auto mutex = detail::load_wrapper(wrapper);
    if (!bionic::is_mutex_initialized(mutex))
        return 0;
    int ret = detail::destroy_c_wrapped<pthread_mutex_t>(&mutex.value, &::pthread_mutex_destroy);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_mutex_lock(pthread_mutex_t *mutex) {
    int ret = ::pthread_mutex_lock(bionic::to_host(mutex));
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_mutex_unlock(pthread_mutex_t *mutex) {
    int ret = ::pthread_mutex_unlock(bionic::to_host(mutex));
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_mutex_trylock(pthread_mutex_t *mutex) {
    int ret = ::pthread_mutex_trylock(bionic::to_host(mutex));
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_mutexattr_init(pthread_mutexattr_t *attr) {
    *attr = pthread_mutexattr_t{bionic::mutex_type::NORMAL};
    return 0;
}

int shim::pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
    return 0;
}

int shim::pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
    if (type < 0 || type > (int) bionic::mutex_type::END)
        return EINVAL;
    attr->type = (bionic::mutex_type) type;
    return 0;
}

int shim::pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type) {
    *type = (int) attr->type;
    return 0;
}

int shim::pthread_cond_init(pthread_cond_t *cond, const shim::pthread_condattr_t *attr) {
    host_condattr hattr (attr);
    int ret = detail::make_c_wrapped<pthread_cond_t, const ::pthread_condattr_t *>(cond, &::pthread_cond_init, &hattr.attr);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_cond_destroy(pthread_cond_t *cond) {
    if (!bionic::is_cond_initialized(cond))
        return 0;
    return bionic::translate_errno_from_host(detail::destroy_c_wrapped<pthread_cond_t>(cond, &::pthread_cond_destroy));
}

int shim::pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    int ret = ::pthread_cond_wait(bionic::to_host(cond), bionic::to_host(mutex));
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_cond_broadcast(pthread_cond_t *cond) {
    int ret = ::pthread_cond_broadcast(bionic::to_host(cond));
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_cond_signal(pthread_cond_t *cond) {
    int ret = ::pthread_cond_signal(bionic::to_host(cond));
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *ts) {
    int ret = ::pthread_cond_timedwait(bionic::to_host(cond), bionic::to_host(mutex), ts);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_condattr_init(pthread_condattr_t *attr) {
    *attr = {false, bionic::clock_type::MONOTONIC};
    return 0;
}

int shim::pthread_condattr_destroy(pthread_condattr_t *attr) {
    return 0;
}

int shim::pthread_condattr_setclock(pthread_condattr_t *attr, int clock) {
    if (clock != (int) bionic::clock_type::MONOTONIC &&
        clock != (int) bionic::clock_type::REALTIME)
        return bionic::translate_errno_from_host(EINVAL);
    attr->clock = (bionic::clock_type) clock;
    return 0;
}

int shim::pthread_condattr_getclock(const pthread_condattr_t *attr, int *clock) {
    *clock = (int) attr->clock;
    return 0;
}

int shim::pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr) {
    if (attr != nullptr)
        handle_runtime_error("non-NULL rwlock attr is currently not supported");
    int ret = detail::make_c_wrapped<pthread_rwlock_t, const ::pthread_rwlockattr_t *>(rwlock, &::pthread_rwlock_init, nullptr);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_rwlock_destroy(pthread_rwlock_t *rwlock) {
    if (!bionic::is_rwlock_initialized(rwlock))
        return 0;
    int ret = detail::destroy_c_wrapped<pthread_rwlock_t>(rwlock, &::pthread_rwlock_destroy);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) {
    ::pthread_key_t host_key;
    int ret = ::pthread_key_create(&host_key, destructor);
    if (host_key > INT_MAX)
        handle_runtime_error("Unsupported host pthread key implementation");
    *key = host_key;
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_key_delete(pthread_key_t key) {
    int ret = ::pthread_key_delete((::pthread_key_t) key);
    return bionic::translate_errno_from_host(ret);
}

int shim::pthread_setspecific(pthread_key_t key, const void *value) {
    int ret = ::pthread_setspecific((::pthread_key_t) key, value);
    return bionic::translate_errno_from_host(ret);
}

void* shim::pthread_getspecific(pthread_key_t key) {
    return ::pthread_getspecific((::pthread_key_t) key);
}

void shim::pthread_cleanup_push_impl(shim::bionic::pthread_cleanup_t *c, void (*cb)(void *), void *arg) {
    c->prev = bionic::cleanup.current;
    c->routine = cb;
    c->arg = arg;
    bionic::cleanup.current = c;
}

void shim::pthread_cleanup_pop_impl(shim::bionic::pthread_cleanup_t *c, int execute) {
    bionic::cleanup.current = c->prev;
    if (execute)
        c->routine(c->arg);
}

int shim::pthread_once(pthread_once_t *control, void (*routine)()) {
    static std::recursive_mutex mutex;
    std::unique_lock<std::recursive_mutex> lock (mutex);
    if (*control == 0) {
        *control = 1;
        routine();
    }
    return 0;
}

pid_t shim::pthread_gettid_np(pthread_t thread) {
#ifdef __linux__
    pid_t ret = thread;
#elif defined(__FreeBSD__)
    /* in lib/libthr/thread/thr_private.h tid field is first */
    pid_t ret = (long)thread;
#else
    pid_t ret = thread->__sig;
#endif
    return ret;
}

void shim::add_pthread_shimmed_symbols(std::vector<shimmed_symbol> &list) {
    list.insert(list.end(), {
        {"pthread_create", pthread_create},
        {"pthread_equal", ::pthread_equal},
        {"pthread_join", ::pthread_join},
        {"pthread_detach", ::pthread_detach},
        {"pthread_kill", ::pthread_kill},
        {"pthread_self", ::pthread_self},
        {"pthread_atfork", ::pthread_atfork},
        {"pthread_setschedparam", pthread_setschedparam},
        {"pthread_getschedparam", pthread_getschedparam},
        {"pthread_getattr_np", pthread_getattr_np},
        {"pthread_attr_init", pthread_attr_init},
        {"pthread_attr_destroy", pthread_attr_destroy},
        {"pthread_attr_setdetachstate", pthread_attr_setdetachstate},
        {"pthread_attr_getdetachstate", pthread_attr_getdetachstate},
        {"pthread_attr_setschedparam", pthread_attr_setschedparam},
        {"pthread_attr_getschedparam", pthread_attr_getschedparam},
        {"pthread_attr_setstacksize", pthread_attr_setstacksize},
        {"pthread_attr_getstack", pthread_attr_getstack},
        {"pthread_attr_getstacksize", pthread_attr_getstacksize},

        {"pthread_setname_np", pthread_setname_np},

        {"pthread_mutex_init", pthread_mutex_init},
        {"pthread_mutex_destroy", pthread_mutex_destroy},
        {"pthread_mutex_lock", pthread_mutex_lock},
        {"pthread_mutex_unlock", pthread_mutex_unlock},
        {"pthread_mutex_trylock", pthread_mutex_trylock},
        {"pthread_mutexattr_init", pthread_mutexattr_init},
        {"pthread_mutexattr_destroy", pthread_mutexattr_destroy},
        {"pthread_mutexattr_settype", pthread_mutexattr_settype},
        {"pthread_mutexattr_gettype", pthread_mutexattr_gettype},

        {"pthread_cond_init", pthread_cond_init},
        {"pthread_cond_destroy", pthread_cond_destroy},
        {"pthread_cond_wait", pthread_cond_wait},
        {"pthread_cond_broadcast", pthread_cond_broadcast},
        {"pthread_cond_signal", pthread_cond_signal},
        {"pthread_cond_timedwait", pthread_cond_timedwait},
        // TODO figure out how to implement this correctly, this will use the default clock of the cond variable
        {"pthread_cond_timedwait_monotonic_np", pthread_cond_timedwait},
        {"pthread_condattr_init", pthread_condattr_init},
        {"pthread_condattr_destroy", pthread_condattr_destroy},
        {"pthread_condattr_setclock", pthread_condattr_setclock},
        {"pthread_condattr_getclock", pthread_condattr_getclock},

        {"pthread_rwlock_init", pthread_rwlock_init},
        {"pthread_rwlock_destroy", pthread_rwlock_destroy},
        {"pthread_rwlock_rdlock", &detail::arg_rewrite_helper<int (::pthread_rwlock_t *)>::rewrite<pthread_rwlock_rdlock>},
        {"pthread_rwlock_tryrdlock", &detail::arg_rewrite_helper<int (::pthread_rwlock_t *)>::rewrite<pthread_rwlock_tryrdlock>},
        {"pthread_rwlock_wrlock", &detail::arg_rewrite_helper<int (::pthread_rwlock_t *)>::rewrite<pthread_rwlock_wrlock>},
        {"pthread_rwlock_trywrlock", &detail::arg_rewrite_helper<int (::pthread_rwlock_t *)>::rewrite<pthread_rwlock_trywrlock>},
        {"pthread_rwlock_unlock", &detail::arg_rewrite_helper<int (::pthread_rwlock_t *)>::rewrite<pthread_rwlock_unlock>},

        {"pthread_key_create", pthread_key_create},
        {"pthread_key_delete", pthread_key_delete},
        {"pthread_setspecific", pthread_setspecific},
        {"pthread_getspecific", pthread_getspecific},

        {"__pthread_cleanup_push", pthread_cleanup_push_impl},
        {"__pthread_cleanup_pop", pthread_cleanup_pop_impl},

        {"pthread_once", pthread_once},
        {"pthread_gettid_np", pthread_gettid_np}
    });
}
