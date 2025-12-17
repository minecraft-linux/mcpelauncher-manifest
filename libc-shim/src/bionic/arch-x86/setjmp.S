/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "../bionic_asm.h"

// The internal structure of a jmp_buf is totally private.
// Current layout (changes from release to release):
//
// word   name            description
// 0      edx             registers
// 1      ebx
// 2      esp
// 3      ebp
// 4      esi
// 5      edi
// 6      sigmask         64-bit signal mask (not used with _setjmp / _longjmp)
// 7      "               "
// 8      sigflag/cookie  setjmp cookie in top 31 bits, signal mask flag in low bit
// 9      checksum        checksum of the core registers, to give better error messages.

#define _JB_EDX 0
#define _JB_EBX 1
#define _JB_ESP 2
#define _JB_EBP 3
#define _JB_ESI 4
#define _JB_EDI 5
#define _JB_SIGMASK 6
#define _JB_SIGFLAG 8
#define _JB_CHECKSUM 9

.macro m_mangle_registers reg
  xorl \reg,%edx
  xorl \reg,%ebx
  xorl \reg,%esp
  xorl \reg,%ebp
  xorl \reg,%esi
  xorl \reg,%edi
.endm

.macro m_unmangle_registers reg
  m_mangle_registers \reg
.endm

.macro m_calculate_checksum dst, src
  movl $0, \dst
  .irp i,0,1,2,3,4,5
    xorl (\i*4)(\src), \dst
  .endr
.endm

ENTRY(bionic_setjmp)
__BIONIC_WEAK_ASM_FOR_NATIVE_BRIDGE(bionic_setjmp)
  movl 4(%esp),%ecx
  mov $1,%eax
  jmp .L_sigsetjmp
END(bionic_setjmp)

ENTRY(bionic__setjmp)
__BIONIC_WEAK_ASM_FOR_NATIVE_BRIDGE(bionic__setjmp)
  movl 4(%esp),%ecx
  movl $0,%eax
  jmp .L_sigsetjmp
END(bionic__setjmp)

ENTRY(bionic_sigsetjmp)
__BIONIC_WEAK_ASM_FOR_NATIVE_BRIDGE(bionic_sigsetjmp)
  movl 4(%esp),%ecx
  movl 8(%esp),%eax

.L_sigsetjmp:
  PIC_PROLOGUE
  pushl %ecx
  pushl %eax
  call PIC_PLT(CDECL(__bionic_setjmp_cookie_get))
  addl $4,%esp
  popl %ecx
  PIC_EPILOGUE

  // Record the setjmp cookie and whether or not we're saving the signal mask.
  movl %eax,(_JB_SIGFLAG * 4)(%ecx)

  // Do we need to save the signal mask?
  testl $1,%eax
  jz 1f

  // Save the current signal mask.
  pushl %ecx
  PIC_PROLOGUE
  leal (_JB_SIGMASK * 4)(%ecx),%eax
  subl $8,%esp
  pushl %eax
  pushl $0 // NULL
  pushl $2 // SIG_SETMASK
  call PIC_PLT(CDECL(sigprocmask))
  addl $12,%esp
  addl $8,%esp
  PIC_EPILOGUE
  popl %ecx

1:
  // Fetch the setjmp cookie and clear the signal flag bit.
  movl (_JB_SIGFLAG * 4)(%ecx),%eax
  andl $-2,%eax

  // Save the callee-save registers.
  movl 0(%esp),%edx
  m_mangle_registers %eax
  movl %edx,(_JB_EDX * 4)(%ecx)
  movl %ebx,(_JB_EBX * 4)(%ecx)
  movl %esp,(_JB_ESP * 4)(%ecx)
  movl %ebp,(_JB_EBP * 4)(%ecx)
  movl %esi,(_JB_ESI * 4)(%ecx)
  movl %edi,(_JB_EDI * 4)(%ecx)
  m_unmangle_registers %eax

  m_calculate_checksum %eax, %ecx
  movl %eax, (_JB_CHECKSUM * 4)(%ecx)

  xorl %eax,%eax
  ret
END(bionic_sigsetjmp)

ENTRY(bionic_siglongjmp)
__BIONIC_WEAK_ASM_FOR_NATIVE_BRIDGE(bionic_siglongjmp)
  movl 4(%esp),%edx

  // Check the checksum before doing anything.
  m_calculate_checksum %eax, %edx
  xorl (_JB_CHECKSUM * 4)(%edx), %eax
  jnz 3f

  // Do we have a signal mask to restore?
  movl (_JB_SIGFLAG * 4)(%edx), %eax
  testl $1,%eax
  jz 1f

  // Restore the signal mask.
  leal (_JB_SIGMASK * 4)(%edx),%eax
  PIC_PROLOGUE
  pushl $0 // NULL
  pushl %eax
  pushl $2 // SIG_SETMASK
  call PIC_PLT(CDECL(sigprocmask))
  addl $12,%esp
  PIC_EPILOGUE

1:
  // Restore the callee-save registers.
  movl 4(%esp),%edx
  movl 8(%esp),%eax

  movl (_JB_SIGFLAG * 4)(%edx),%ecx
  andl $-2,%ecx

  movl %ecx,%ebx
  movl %ecx,%esp
  movl %ecx,%ebp
  movl %ecx,%esi
  movl %ecx,%edi
  xorl (_JB_EDX * 4)(%edx),%ecx
  xorl (_JB_EBX * 4)(%edx),%ebx
  xorl (_JB_ESP * 4)(%edx),%esp
  xorl (_JB_EBP * 4)(%edx),%ebp
  xorl (_JB_ESI * 4)(%edx),%esi
  xorl (_JB_EDI * 4)(%edx),%edi

  PIC_PROLOGUE
  pushl %eax
  pushl %ecx
  pushl (_JB_SIGFLAG * 4)(%edx)
  call PIC_PLT(CDECL(__bionic_setjmp_cookie_check))
  addl $4,%esp
  popl %ecx
  popl %eax
  PIC_EPILOGUE

  testl %eax,%eax
  jnz 2f
  incl %eax
2:
  movl %ecx,0(%esp)
  ret

3:
  PIC_PROLOGUE
  pushl (_JB_SIGMASK * 4)(%edx)
  call PIC_PLT(CDECL(__bionic_setjmp_checksum_mismatch))
END(bionic_siglongjmp)

ALIAS_SYMBOL(bionic_longjmp, bionic_siglongjmp)
__BIONIC_WEAK_ASM_FOR_NATIVE_BRIDGE(bionic_longjmp)
ALIAS_SYMBOL(bionic__longjmp, bionic_siglongjmp)
__BIONIC_WEAK_ASM_FOR_NATIVE_BRIDGE(bionic__longjmp)
