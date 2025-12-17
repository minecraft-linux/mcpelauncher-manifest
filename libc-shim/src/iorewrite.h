#pragma once
#include <libc_shim.h>
#include <cstring>

static std::string iorewrite0(const char * path) {
    auto len = path ? strlen(path) : 0;
    if(len > 0) {
        for(auto&& kv : shim::rewrite_filesystem_access) {
            if (path[0] != '/' && kv.first == ".") {
                return kv.second + path;
            }
            if (len >= kv.first.size() && !memcmp(path, kv.first.data(), kv.first.size()) && kv.second.rfind(kv.first.data(), 0)) {
                return kv.second + std::string(path + kv.first.length());
            }
        }
        return path;
    }
    return {};
}

template<class T> struct iorewrite1;
template<class R, class ... arg > struct iorewrite1<R (*) (const char * ,arg...)> {
    template<R(*org)(const char *, arg...)> static R rewrite(const char *path1, arg...a) {
        return org(iorewrite0(path1).data(), a...);
    }
};

template<class R, class ... arg > struct iorewrite1<R (*)(const char *,arg...) noexcept> : iorewrite1<R (*)(const char *,arg...)> {};
template<class R, class ... arg > struct iorewrite1<R(const char *,arg...)> : iorewrite1<R (*)(const char *,arg...)> {};
template<class R, class ... arg > struct iorewrite1<R(const char *,arg...) noexcept> : iorewrite1<R (*)(const char *,arg...)> {};

template<class T> struct iorewrite2;
template<class R, class ... arg > struct iorewrite2<R (*)(const char *,const char *,arg...)> {
    template<R(*org)(const char *,const char *,arg...)> static R rewrite(const char *path1, const char *path2, arg...a) {
        return org(iorewrite0(path1).data(), iorewrite0(path2).data(), a...);
    }
};

template<class R, class ... arg > struct iorewrite2<R (*)(const char *,const char *,arg...) noexcept> : iorewrite2<R (*)(const char *,const char *,arg...)> {};
template<class R, class ... arg > struct iorewrite2<R(const char *,const char *,arg...)> : iorewrite2<R (*)(const char *,const char *,arg...)> {};
template<class R, class ... arg > struct iorewrite2<R(const char *,const char *,arg...) noexcept> : iorewrite2<R (*)(const char *,const char *,arg...)> {};

#define IOREWRITE1(func) (iorewrite1<decltype(func)>::rewrite<func>)
#define IOREWRITE2(func) (iorewrite2<decltype(func)>::rewrite<func>)
