#pragma once
// Values from android ndk linux headers

#if defined(__i386__)
#define FAKE_SYS_gettid 224
#define FAKE_SYS_getrandom 355
#elif defined(__arm__)
#define FAKE_SYS_gettid 224
#define FAKE_SYS_getrandom 384
#elif defined(__x86_64__)
#define FAKE_SYS_gettid 186
#define FAKE_SYS_getrandom 318
#elif defined(__aarch64__)
#define FAKE_SYS_gettid 178
#define FAKE_SYS_getrandom 278
#endif
