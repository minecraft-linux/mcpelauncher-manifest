
/*
 * Copyright (C) 2015 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdio.h>


static long setjmp_cookie;

void __bionic_init_setjmp_cookie() {
    long value = (long) arc4random();
    // Mask off the last bit to store the signal flag.
    setjmp_cookie = value & ~1;
}
long __bionic_setjmp_cookie_get(long sigflag) {
    if (!setjmp_cookie)
        __bionic_init_setjmp_cookie();
    if (sigflag & ~1) {
        printf("unexpected sigflag value: %ld\n", sigflag);
        abort();
    }
    return setjmp_cookie | sigflag;
}
// Aborts if cookie doesn't match, returns the signal flag otherwise.
long __bionic_setjmp_cookie_check(long cookie) {
    if (setjmp_cookie != (cookie & ~1)) {
        printf("setjmp cookie mismatch");
        abort();
    }
    return cookie & 1;
}
long __bionic_setjmp_checksum_mismatch() {
    printf("setjmp checksum mismatch\n");
    abort();
}
