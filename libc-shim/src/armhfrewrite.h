#pragma once
#ifdef __arm__
template<class T> struct armhfrewrite;
template<class R, class ... arg > struct armhfrewrite<R (*)(arg...)> {
    template<R(*org)(arg...)> static __attribute__((pcs("aapcs"))) R rewrite(arg...a) {
        return org(a...);
    }
};

template<class R, class ... arg > struct armhfrewrite<R (*)(arg...) noexcept> : armhfrewrite<R (*)(arg...)> {};

#define ARMHFREWRITE(func) (void*)&armhfrewrite<decltype(&func)>::rewrite<&func>
#else
#define ARMHFREWRITE(func) func
#endif