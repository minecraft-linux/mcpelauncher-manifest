#pragma once

#include <atomic>
#include <cstdint>
#include <pthread.h>
#include <libc_shim.h>
#include "meta.h"
#include "argrewrite.h"
#include "common.h"

namespace shim {

    namespace bionic {

        struct pthread_mutex_t {
#if defined(__LP64__)
            size_t init_value;
            ::pthread_mutex_t *wrapped;
            int64_t priv[2];
#else
            ::pthread_mutex_t *wrapped;
#endif
        };

        constexpr size_t mutex_init_value = 0;
        constexpr size_t recursive_mutex_init_value = 0x4000;
        constexpr size_t errorcheck_mutex_init_value = 0x8000;

        inline bool is_mutex_initialized(pthread_mutex_t const *m) {
#if defined(__LP64__)
            return (m->wrapped != nullptr);
#else
            return ((size_t) m->wrapped != mutex_init_value &&
                    (size_t) m->wrapped != recursive_mutex_init_value &&
                    (size_t) m->wrapped != errorcheck_mutex_init_value);
#endif
        }

        void mutex_static_initializer(pthread_mutex_t *mutex, pthread_mutex_t &old);

        struct pthread_cond_t {
            ::pthread_cond_t *wrapped;
#if defined(__LP64__)
            int64_t priv[5];
#endif
        };

        inline bool is_cond_initialized(pthread_cond_t const *m) {
            auto wrapped_ptr = reinterpret_cast<const std::atomic<pthread_cond_t> *>(m);
            auto val = wrapped_ptr->load(std::memory_order_relaxed);
            return val.wrapped != nullptr;
        }

        void cond_static_initializer(pthread_cond_t *cond, pthread_cond_t &old);

        struct pthread_rwlock_t {
            ::pthread_rwlock_t *wrapped;
#if defined(__LP64__)
            int64_t priv[6];
#endif
        };

        inline bool is_rwlock_initialized(pthread_rwlock_t const *m) {
            return m->wrapped != nullptr;
        }

        void rwlock_static_initializer(pthread_rwlock_t *rwlock, pthread_rwlock_t &old);

        template <>
        inline auto to_host<pthread_mutex_t>(pthread_mutex_t const *o) {
            auto m = detail::load_wrapper(o);
            mutex_static_initializer(const_cast<pthread_mutex_t *>(o), m.value);
            return m.value.wrapped;
        }
        template <>
        inline auto to_host<pthread_cond_t>(pthread_cond_t const *o) {
            auto m = detail::load_wrapper(o);
            cond_static_initializer(const_cast<pthread_cond_t *>(o), m.value);
            return m.value.wrapped;
        }
        template <>
        inline auto to_host<pthread_rwlock_t>(pthread_rwlock_t const *o) {
            auto m = detail::load_wrapper(o);
            rwlock_static_initializer(const_cast<pthread_rwlock_t *>(o), m.value);
            return m.value.wrapped;
        }

        enum class mutex_type : uint32_t {
            NORMAL = 0,
            RECURSIVE = 1,
            ERRORCHECK = 2,
            END = 2
        };

        static int to_host_mutex_type(mutex_type type);

        struct pthread_mutexattr_t {
            mutex_type type : 4;
        };

        struct pthread_condattr_t {
            bool shared : 1;
            clock_type clock : 2;
        };

        enum class sched_policy {
            OTHER = 0
        };

        static int to_host_sched_policy(sched_policy type);
        static sched_policy from_host_sched_policy(int type);

        struct pthread_attr_t {
            bool detached : 1;
            bool user_stack : 1;
            uint32_t filler : 32-2;
            void* stack_base;
            size_t stack_size;
            size_t guard_size;
            sched_policy sched_policy;
            int sched_priority;
#if defined(__LP64__)
            int64_t priv[2];
#endif
        };

        struct sched_param {
            int sched_priority;
        };

        struct pthread_cleanup_t {
            pthread_cleanup_t *prev;
            void (*routine)(void *);
            void *arg;
        };

        using pthread_key_t = uint32_t;
        using pthread_once_t = int;

        struct pthread_cleanup_holder {
            pthread_cleanup_t *current = nullptr;

            ~pthread_cleanup_holder();
        };

        extern thread_local pthread_cleanup_holder cleanup;

    }

    using pthread_attr_t = bionic::pthread_attr_t;

    using pthread_mutex_t = bionic::pthread_mutex_t;
    using pthread_mutexattr_t = bionic::pthread_mutexattr_t;

    using pthread_cond_t = bionic::pthread_cond_t;
    using pthread_condattr_t = bionic::pthread_condattr_t;

    using pthread_rwlock_t = bionic::pthread_rwlock_t;

    using pthread_key_t = bionic::pthread_key_t;
    using pthread_once_t = bionic::pthread_once_t;

    struct host_pthread_attr {

        ::pthread_attr_t attr;

        host_pthread_attr(bionic::pthread_attr_t const *bionic_attr);

        ~host_pthread_attr();

    };

    struct host_mutexattr {

        ::pthread_mutexattr_t attr;

        host_mutexattr(bionic::pthread_mutexattr_t const *bionic_attr);

        ~host_mutexattr();

    };

    struct host_condattr {

        ::pthread_condattr_t attr;

        host_condattr(bionic::pthread_condattr_t const *bionic_attr);

        ~host_condattr();

    };

    int pthread_create(pthread_t *thread, pthread_attr_t const *attr, void* (*fn)(void *), void *arg);
    int pthread_setschedparam(pthread_t thread, bionic::sched_policy policy, const bionic::sched_param *param);
    int pthread_getschedparam(pthread_t thread, bionic::sched_policy *policy, bionic::sched_param *param);
    int pthread_getattr_np(pthread_t th, pthread_attr_t* attr);

    int pthread_attr_init(pthread_attr_t *attr);
    int pthread_attr_destroy(pthread_attr_t *attr);
    int pthread_attr_setdetachstate(pthread_attr_t *attr, int value);
    int pthread_attr_getdetachstate(pthread_attr_t *attr, int *value);
    int pthread_attr_setschedparam(pthread_attr_t *attr, const bionic::sched_param *value);
    int pthread_attr_getschedparam(pthread_attr_t *attr, bionic::sched_param *value);
    int pthread_attr_setstacksize(pthread_attr_t *attr, size_t value);
    int pthread_attr_getstack(pthread_attr_t *attr, void **stackaddr, size_t *stacksize);
    int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *value);

    int pthread_setname_np(pthread_t thread, const char* name);

    int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
    int pthread_mutex_destroy(pthread_mutex_t *mutex);
    int pthread_mutex_lock(pthread_mutex_t *mutex);
    int pthread_mutex_unlock(pthread_mutex_t *mutex);
    int pthread_mutex_trylock(pthread_mutex_t *mutex);

    int pthread_mutexattr_init(pthread_mutexattr_t *attr);
    int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
    int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
    int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);

    int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
    int pthread_cond_destroy(pthread_cond_t* cond);
    int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
    int pthread_cond_broadcast(pthread_cond_t *cond);
    int pthread_cond_signal(pthread_cond_t *cond);
    int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *ts);

    int pthread_condattr_init(pthread_condattr_t *attr);
    int pthread_condattr_destroy(pthread_condattr_t *attr);
    int pthread_condattr_setclock(pthread_condattr_t *attr, int clock);
    int pthread_condattr_getclock(const pthread_condattr_t *attr, int *clock);

    int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
    int pthread_rwlock_destroy(pthread_rwlock_t* rwlock);

    int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
    int pthread_key_delete(pthread_key_t key);
    int pthread_setspecific(pthread_key_t key, const void *value);
    void *pthread_getspecific(pthread_key_t key);

    void pthread_cleanup_push_impl(bionic::pthread_cleanup_t *c, void (*cb)(void *), void *arg);
    void pthread_cleanup_pop_impl(bionic::pthread_cleanup_t *c, int execute);

    int pthread_once(pthread_once_t *control, void (*routine)());

    pid_t pthread_gettid_np(pthread_t thread);

    void add_pthread_shimmed_symbols(std::vector<shimmed_symbol> &list);

    namespace detail {

        template <>
        struct arg_rewrite<::pthread_rwlock_t *> : bionic_ptr_rewriter<typename ::pthread_rwlock_t *, pthread_rwlock_t *> {};

    }

}
