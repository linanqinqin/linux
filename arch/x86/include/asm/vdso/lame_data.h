/* linanqinqin */

/* arch/x86/include/asm/vdso/lame_data.h */
/* SPDX-License-Identifier: GPL-2.0 */  

#ifndef _ASM_X86_VDSO_LAME_DATA_H
#define _ASM_X86_VDSO_LAME_DATA_H

#include <stdint.h>

/* LAME context structure - stores complete execution context */
struct lame_ctx {
    /* Critical registers for context switching */
    uint64_t rsp;       /* User stack pointer */
    uint64_t rip;       /* Resume instruction pointer */
    uint64_t rflags;    /* Saved flags */
    
    /* General purpose registers */
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, r8;
    uint64_t r9, r10, r11, r12;
    uint64_t r13, r14, r15;
    
    /* Status flags */
    uint32_t valid;      /* Context is valid */
    uint32_t in_use;     /* Context is currently in use */
    
    /* Padding for cache alignment */
    uint32_t padding[2];
} __attribute__((packed, aligned(64)));

/* LAME bundle handle - manages coroutine switching */
#define LAME_COROUTINE_DEPTH 2      /* for performance, this should be a power of 2 */

struct lame_handle {
    /* Active coroutine index */
    uint64_t active;
    
    /* Bundle metadata */
    uint64_t bundle_id;           /* Unique bundle identifier */
    uint64_t last_switch_tsc;     /* TSC of last context switch */
    uint32_t switch_count;        /* Number of switches in this bundle */
    uint32_t cpu_id;              /* CPU this bundle is assigned to */

    /* Context storage for coroutines */
    struct lame_ctx ctx[LAME_COROUTINE_DEPTH];
} __attribute__((packed, aligned(64)));

#define MAX_CPU_CORES 256  /* Or whatever maximum */

/* Global array - one entry per CPU core */
struct lame_handle lame_handle_array[MAX_CPU_CORES];

#endif /* _ASM_X86_VDSO_LAME_DATA_H */

/* end */
