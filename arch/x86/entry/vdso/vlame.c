/* linanqinqin */

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Nanqinqin Li <linanqinqin@princeton.edu>. All Rights Reserved.
 */
#include <vdso/lame.h>
#include <asm/vdso/lame_data.h>

/* Thread-local storage for LAME data */
__thread struct lame_thread_data lame_tls_data __attribute__((visibility("hidden")));

/* Helper function to get TLS data pointer */
static inline struct lame_thread_data *get_lame_tls_data(void) {
    return &lame_tls_data;
}

__attribute__((naked)) void __vdso_lame_entry(void) {
    asm volatile(
        /* Save registers we'll use */
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        
        /* Call helper function to get TLS data pointer */
        "call get_lame_tls_data\n"
        "movq %rax, %rsi\n"  /* Save TLS data pointer in rsi */
        
        /* Step 1: Take current lame_count and put it in r13 */
        "movq 144(%rsi), %r13\n"        /* lame_count is at offset 144 */
        
        /* Step 2: Put value from r12 into lame_count */
        "movq %r12, 144(%rsi)\n"
        
        /* Restore registers */
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rax\n"
        
        /* Return from interrupt */
        "iretq\n"
    );
}

int __vdso_lame_add(int x, int y) {
    int last_commit = 0x95a9354; // the SHA for last commit so that userspace knows which version is invoked 
    return x+y+last_commit;
}

int lame_add(int, int)
    __attribute__((weak, alias("__vdso_lame_add")));

/* end */