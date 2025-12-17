#pragma once

#ifndef LIBC_SHIM_DEFINE_VARIADIC
#if (defined(__APPLE__) && defined(__aarch64__)) || (defined(_WIN32) && defined(__x86_64__))
#define LIBC_SHIM_DEFINE_VARIADIC 0
#else
#define LIBC_SHIM_DEFINE_VARIADIC 1
#endif
#endif
