/* linanqinqin */

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Nanqinqin Li <linanqinqin@princeton.edu>. All Rights Reserved.
 */
#include <vdso/lame.h>
#include <asm/vdso/lame_data.h>

/* Global array - one entry per CPU core */
struct lame_handle lame_handle_array[MAX_CPU_CORES];

__attribute__((naked)) void __vdso_lame_entry(void) {
    asm volatile(
        /* Save registers we'll use */
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rsi\n"
        "pushq %rdi\n" 

        /* Get CPU ID using rdtscp */
        "rdtscp\n"                    /* Returns CPU ID in %ecx */
        "andl $0xFF, %ecx\n"          /* Mask to get CPU ID (assuming < 256 cores) */
        
        /* Array index: cpuid * sizeof(lame_handle) */
        "movq %[lame_handle_size], %rax\n"
        "mulq %rcx\n"               /* rax = cpuid * sizeof(lame_handle) */ 

        "leaq lame_handle_array(%rip), %rdx\n"
        "addq %rax, %rdx\n"         /* rdx -> lame_handle_array[cpuid] */

        /* get offset of the current active index */
        "movzbl %[lame_handle_active](%rdx), %r10\n" /* r10 = active index */
        "imulq %[lame_ctx_size], %r10\n"            /* r10 = active index * sizeof(lame_ctx) */
        "addq %[lame_ctx_r13], %r10\n"              /* r10 -> lame_handle_array[cpuid].ctx[active_index].r13 */

        /* save r13 to the current active ctx */
        "movq %r13, (%r10)\n"
        
        /* Restore registers */
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        
        /* Return from interrupt */
        "iretq\n"
        : 
        : [lame_handle_size] "i" (LAME_HANDLE_SIZE),
          [lame_ctx_size] "i" (LAME_CTX_SIZE),
          [lame_ctx_r13] "i" (LAME_CTX_R13),
          [lame_handle_active] "i" (LAME_HANDLE_ACTIVE)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r10", "r13", "memory"
    );
}

int __vdso_lame_add(int x, int y) {
    int last_commit = 0x95a9354; // the SHA for last commit so that userspace knows which version is invoked 
    return x+y+last_commit;
}

int lame_add(int, int)
    __attribute__((weak, alias("__vdso_lame_add")));

/* end */