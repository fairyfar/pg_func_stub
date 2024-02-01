/*
 * MIT License
 * 
 * Copyright (c) 2019 coolxv
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Based on cpp-stub: https://github.com/coolxv/cpp-stub
 * Modified by FairyFar@msn.com
 */

#ifndef __PG_FUNC_STUB_H__
#define __PG_FUNC_STUB_H__

#ifdef _WIN32 
//windows
#include <windows.h>
#include <processthreadsapi.h>
#else
//linux or macos
#include <unistd.h>
#include <sys/mman.h>
#endif
//valgrind
#ifdef __VALGRIND__
#include <valgrind/valgrind.h>
#endif

/**********************************************************
                  replace function
**********************************************************/
#ifdef __VALGRIND__
#define VALGRIND_CACHE_FLUSH(addr, size) VALGRIND_DISCARD_TRANSLATIONS(addr, size)
#else
#define VALGRIND_CACHE_FLUSH(addr, size)
#endif

#ifdef _WIN32 
#define CACHEFLUSH(addr, size) FlushInstructionCache(GetCurrentProcess(), addr, size);VALGRIND_CACHE_FLUSH(addr, size)
#else
#define CACHEFLUSH(addr, size) __builtin___clear_cache(addr, addr + size);VALGRIND_CACHE_FLUSH(addr, size)
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
    #define CODESIZE 16U
    #define CODESIZE_MIN 16U
    #define CODESIZE_MAX CODESIZE
    // ldr x9, +8 
    // br x9 
    // addr 
    #define REPLACE_FAR(fn, fn_stub)\
        ((uint32_t*)fn)[0] = 0x58000040 | 9;\
        ((uint32_t*)fn)[1] = 0xd61f0120 | (9 << 5);\
        *(long long *)(fn + 8) = (long long )fn_stub;\
        CACHEFLUSH((char *)fn, CODESIZE);
    #define REPLACE_NEAR(fn, fn_stub) REPLACE_FAR(fn, fn_stub)
#elif defined(__arm__) || defined(_M_ARM)
    #define CODESIZE 8U
    #define CODESIZE_MIN 8U
    #define CODESIZE_MAX CODESIZE
    // ldr pc, [pc, #-4]
    #define REPLACE_FAR(fn, fn_stub)\
        if ((uintptr_t)fn & 0x00000001) { \
          *(uint16_t *)&f[0] = 0xf8df;\
          *(uint16_t *)&f[2] = 0xf000;\
          *(uint16_t *)&f[4] = (uint16_t)(fn_stub & 0xffff);\
          *(uint16_t *)&f[6] = (uint16_t)(fn_stub >> 16);\
        } else { \
          ((uint32_t*)fn)[0] = 0xe51ff004;\
          ((uint32_t*)fn)[1] = (uint32_t)fn_stub;\
        }\
        CACHEFLUSH((char *)((uintptr_t)fn & 0xfffffffe), CODESIZE);
    #define REPLACE_NEAR(fn, fn_stub) REPLACE_FAR(fn, fn_stub)
#elif defined(__thumb__) || defined(_M_THUMB)
    #define CODESIZE 12
    #define CODESIZE_MIN 12
    #define CODESIZE_MAX CODESIZE
    // NOP
    // LDR.W PC, [PC]
    #define REPLACE_FAR(fn, fn_stub)\
        uint32_t clearBit0 = fn & 0xfffffffe;\
        char *f = (char *)clearBit0;\
        if (clearBit0 % 4 != 0) {\
            *(uint16_t *)&f[0] = 0xbe00;\
        }\
        *(uint16_t *)&f[2] = 0xf8df;\
        *(uint16_t *)&f[4] = 0xf000;\
        *(uint16_t *)&f[6] = (uint16_t)(fn_stub & 0xffff);\
        *(uint16_t *)&f[8] = (uint16_t)(fn_stub >> 16);\
        CACHEFLUSH((char *)f, CODESIZE);
    #define REPLACE_NEAR(fn, fn_stub) REPLACE_FAR(fn, fn_stub)
#elif defined(__mips64)
    #define CODESIZE 80U
    #define CODESIZE_MIN 80U
    #define CODESIZE_MAX CODESIZE
    //MIPS has no PC pointer, so you need to manually enter and exit the stack
    //120000ce0:  67bdffe0    daddiu  sp, sp, -32  //enter the stack
    //120000ce4:  ffbf0018    sd  ra, 24(sp)
    //120000ce8:  ffbe0010    sd  s8, 16(sp)
    //120000cec:  ffbc0008    sd  gp, 8(sp)
    //120000cf0:  03a0f025    move    s8, sp

    //120000d2c:  03c0e825    move    sp, s8  //exit the stack
    //120000d30:  dfbf0018    ld  ra, 24(sp)
    //120000d34:  dfbe0010    ld  s8, 16(sp)
    //120000d38:  dfbc0008    ld  gp, 8(sp)
    //120000d3c:  67bd0020    daddiu  sp, sp, 32
    //120000d40:  03e00008    jr  ra
    #define REPLACE_FAR(fn, fn_stub)\
        ((uint32_t *)fn)[0] = 0x67bdffe0;\
        ((uint32_t *)fn)[1] = 0xffbf0018;\
        ((uint32_t *)fn)[2] = 0xffbe0010;\
        ((uint32_t *)fn)[3] = 0xffbc0008;\
        ((uint32_t *)fn)[4] = 0x03a0f025;\
        *(uint16_t *)(fn + 20) = (long long)fn_stub >> 32;\
        *(fn + 22) = 0x19;\
        *(fn + 23) = 0x24;\
        ((uint32_t *)fn)[6] = 0x0019cc38;\
        *(uint16_t *)(fn + 28) = (long long)fn_stub >> 16;\
        *(fn + 30) = 0x39;\
        *(fn + 31) = 0x37;\
        ((uint32_t *)fn)[8] = 0x0019cc38;\
        *(uint16_t *)(fn + 36) = (long long)fn_stub;\
        *(fn + 38) = 0x39;\
        *(fn + 39) = 0x37;\
        ((uint32_t *)fn)[10] = 0x0320f809;\
        ((uint32_t *)fn)[11] = 0x00000000;\
        ((uint32_t *)fn)[12] = 0x00000000;\
        ((uint32_t *)fn)[13] = 0x03c0e825;\
        ((uint32_t *)fn)[14] = 0xdfbf0018;\
        ((uint32_t *)fn)[15] = 0xdfbe0010;\
        ((uint32_t *)fn)[16] = 0xdfbc0008;\
        ((uint32_t *)fn)[17] = 0x67bd0020;\
        ((uint32_t *)fn)[18] = 0x03e00008;\
        ((uint32_t *)fn)[19] = 0x00000000;\
        CACHEFLUSH((char *)fn, CODESIZE);
    #define REPLACE_NEAR(fn, fn_stub) REPLACE_FAR(fn, fn_stub)
#elif defined(__riscv) && __riscv_xlen == 64
    #define CODESIZE 24U
    #define CODESIZE_MIN 24U
    #define CODESIZE_MAX CODESIZE
    // absolute offset(64)
    // auipc t1,0
    // addi t1, t1, 16
    // ld t1,0(t1)
    // jalr x0, t1, 0
    // addr
    #define REPLACE_FAR(fn, fn_stub)\
        unsigned int auipc = 0x317;\
        *(unsigned int *)(fn) = auipc;\
        unsigned int addi = 0x1030313;\
        *(unsigned int *)(fn + 4) = addi;\
        unsigned int ld = 0x33303;\
        *(unsigned int *)(fn + 8) = ld;\
        unsigned int jalr = 0x30067;\
        *(unsigned int *)(fn + 12) = jalr;\
        *(unsigned long long*)(fn + 16) = (unsigned long long)fn_stub;\
        CACHEFLUSH((char *)fn, CODESIZE);
    #define REPLACE_NEAR(fn, fn_stub) REPLACE_FAR(fn, fn_stub)
#elif defined(__riscv) && __riscv_xlen == 32
    #define CODESIZE 20U
    #define CODESIZE_MIN 20U
    #define CODESIZE_MAX CODESIZE
    // absolute offset(32)
    // auipc t1,0
    // addi t1, t1, 16
    // lw t1,0(t1)
    // jalr x0, t1, 0
    // addr
    #define REPLACE_FAR(fn, fn_stub)\
        unsigned int auipc = 0x317;\
        *(unsigned int *)(fn) = auipc;\
        unsigned int addi = 0x1030313;\
        *(unsigned int *)(fn + 4) = addi;\
        unsigned int lw = 0x32303;\
        *(unsigned int *)(fn + 8) = lw;\
        unsigned int jalr = 0x30067;\
        *(unsigned int *)(fn + 12) = jalr;\
        *(unsigned int*)(fn + 16) = (unsigned int)fn_stub;\
        CACHEFLUSH((char *)fn, CODESIZE);
    #define REPLACE_NEAR(fn, fn_stub) REPLACE_FAR(fn, fn_stub)
#elif defined(__loongarch64) 
    #define CODESIZE 20U
    #define CODESIZE_MIN 20U
    #define CODESIZE_MAX CODESIZE
    // absolute offset(64)
    // PCADDI rd, si20 | 0 0 0 1 1 0 0 si20 rd
    // LD.D rd, rj, si12 | 0 0 1 0 1 0 0 0 1 1 si12 rj rd
    // JIRL rd, rj, offs | 0 1 0 0 1 1 offs[15:0] rj rd
    // addr
    #define REPLACE_FAR(fn, fn_stub)\
        unsigned int rd = 17;\
        unsigned int off = 12 >> 2;\
        unsigned int pcaddi = 0x0c << (32 - 7) | off << 5 | rd ;\
        rd = 17;\
        int rj = 17;\
        off = 0;\
        unsigned int ld_d = 0xa3 << 22 | off << 10 | rj << 5 | rd ;\
        rd = 0;\
        rj = 17;\
        off = 0;\
        unsigned int jirl = 0x13 << 26 | off << 10 | rj << 5| rd;\
        *(unsigned int *)fn = pcaddi;\
        *(unsigned int *)(fn + 4) = ld_d;\
        *(unsigned int *)(fn + 8) = jirl;\
        *(unsigned long long*)(fn + 12) = (unsigned long long)fn_stub;\
        CACHEFLUSH((char *)fn, CODESIZE);
    #define REPLACE_NEAR(fn, fn_stub) REPLACE_FAR(fn, fn_stub)
#else //__i386__ _x86_64__  _M_IX86 _M_X64
    #define CODESIZE 13U
    #define CODESIZE_MIN 5U
    #define CODESIZE_MAX CODESIZE
    //13 byte(jmp m16:64)
    //movabs $0x102030405060708,%r11
    //jmpq   *%r11
    #define REPLACE_FAR(fn, fn_stub)\
        *fn = 0x49;\
        *(fn + 1) = 0xbb;\
        *(long long *)(fn + 2) = (long long)fn_stub;\
        *(fn + 10) = 0x41;\
        *(fn + 11) = 0xff;\
        *(fn + 12) = 0xe3;\
        CACHEFLUSH((char *)fn, CODESIZE);
    //5 byte(jmp rel32)
    #define REPLACE_NEAR(fn, fn_stub)\
        *fn = 0xE9;\
        *(int *)(fn + 1) = (int)(fn_stub - fn - CODESIZE_MIN);\
        CACHEFLUSH((char *)fn, CODESIZE);
#endif

typedef struct func_stub
{
    char *key;
    char *fn;
    char code_buf[CODESIZE];
    bool far_jmp;
} func_stub;

extern void _pg_func_stub_init(void);
extern bool _pg_func_stub_set(char *key, char *fn, char *fn_stub);
extern bool _pg_func_stub_reset(char *key);
extern void _pg_func_stub_clear(void);

#endif // __PG_FUNC_STUB_H__
