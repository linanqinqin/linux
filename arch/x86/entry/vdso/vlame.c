/* linanqinqin */

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Nanqinqin Li <linanqinqin@princeton.edu>. All Rights Reserved.
 */
#include <vdso/lame.h>
#include <asm/vdso/lame_data.h>

__attribute__((naked)) void __vdso_lame_entry(void) {
    asm volatile(
        /* Save registers we'll use */
        "pushq %rax\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        
        /* Get CPU ID using rdtscp */
        "rdtscp\n"                    /* Returns CPU ID in %ecx */
        "andl $0xFF, %ecx\n"          /* Mask to get CPU ID (assuming < 256 cores) */
        
        "leaq lame_handle_array(%rip), %rdx\n"

        /* Put CPU ID in r13 */
        "movq %rcx, %r13\n"
        
        /* Restore registers */
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