#include <linux/linkage.h>
#include "lame_data_asm.h"

.code64
.section .text

    .align 16
SYM_CODE_START(__vdso_lame_entry)
    /* Save registers that will be used */
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi

    /* 1. Get cpuid using rdtscp */
    rdtscp
    andl $0xFF, %ecx        /* Mask to get CPU ID (assuming < 256 cores) */

    /* 2. Use cpuid as index into lame_handle_array */
    movq $lame_handle_array, %rdx
    movl %ecx, %eax
    imulq $LAME_HANDLE_SIZE, %rax
    addq %rax, %rdx         /* rdx = &lame_handle_array[cpuid] */

    /* 3. Get .active field (index into ctx array) */
    movl LAME_HANDLE_ACTIVE(%rdx), %r10d
    imulq $LAME_CTX_SIZE, %r10
    
    /* 4. Save r13 to lame_handle_array[cpuid].ctx[active].r13 */
    movq %rdx, %r11
    addq $LAME_HANDLE_CTX, %r11      /* r11 = &ctx[0] */
    addq %r10, %r11                 /* r11 = &ctx[active] */
    addq $LAME_CTX_R13, %r11        /* r11 = &ctx[active].r13 */
    movq %r13, (%r11)

    /* Restore registers */
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rax

    /* Return from interrupt */
    iretq
SYM_CODE_END(__vdso_lame_entry) 