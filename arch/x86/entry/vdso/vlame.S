#include <linux/linkage.h>
#include "lame_data_asm.h"

.code64
.section .text

    .align 16
SYM_CODE_START(__vdso_lame_entry)
    /* Save registers that will be used */
    // pushq %rax
    // pushq %rcx
    // pushq %rdx
    // pushq %rsi
    // pushq %rdi
    // pushq %r10
    // pushq %r11

    // /* 1. Get cpuid using rdtscp */
    // rdtscp
    // andl $0xFF, %ecx        /* Mask to get CPU ID (assuming < 256 cores) */

    // /* 2. Use cpuid as index into lame_handle_array */
    // leaq lame_handle_array(%rip), %rdx
    // movl %ecx, %eax
    // imulq $LAME_HANDLE_SIZE, %rax
    // addq %rax, %rdx         /* rdx = &lame_handle_array[cpuid] */

    // /* 3. Get .active field (index into ctx array) */
    // movq LAME_HANDLE_ACTIVE(%rdx), %r10      /* load 64-bit active */
    // imulq $LAME_CTX_SIZE, %r10
    
    // /* 4. Save r13 to lame_handle_array[cpuid].ctx[active].r13 */
    // movq %rdx, %r11
    // addq $LAME_HANDLE_CTX, %r11      /* r11 = &ctx[0] */
    // addq %r10, %r11                 /* r11 = &ctx[active] */
    // addq $LAME_CTX_R13, %r11        /* r11 = &ctx[active].r13 */
    // movq %r13, (%r11)

    // /* 5. Increment active index */
    // movq LAME_HANDLE_ACTIVE(%rdx), %r10      /* load 64-bit active */
    // addq $1, %r10
    // andq $LAME_COROUTINE_INC_MASK, %r10
    // movq %r10, LAME_HANDLE_ACTIVE(%rdx)      /* store 64-bit active */
    // imulq $LAME_CTX_SIZE, %r10               /* r10 = active * sizeof(lame_ctx) */

    // /* 6. Restore r13 from the new active ctx lame_handle_array[cpuid].ctx[active].r13 */
    // movq %rdx, %r11
    // addq $LAME_HANDLE_CTX, %r11      /* r11 = &ctx[0] */
    // addq %r10, %r11                 /* r11 = &ctx[active] */
    // addq $LAME_CTX_R13, %r11        /* r11 = &ctx[active].r13 */
    // movq (%r11), %r13   
    
    // /* Restore registers */
    // popq %r11
    // popq %r10
    // popq %rdi
    // popq %rsi
    // popq %rdx
    // popq %rcx
    // popq %rax

    /* Return from interrupt */
    iretq
SYM_CODE_END(__vdso_lame_entry) 