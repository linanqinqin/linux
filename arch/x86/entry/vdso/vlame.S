#include <linux/linkage.h>
#include "lame_data_asm.h"

.code64
.section .text

    .align 16
SYM_CODE_START(__vdso_lame_entry)
    /* Step 1: Save minimal set of registers needed for address calculation */
    pushq %rdx      /* Will hold base of lame_handle_array */
    pushq %r11      /* Will be used as context base pointer */
    pushq %r10      /* Will be used for index math */
    pushq %rax      /* Will be used for scratch/cpuid math */

    /* Step 2: Get cpuid using rdtscp */
    rdtscp
    andl $0xFF, %ecx        /* Mask to get CPU ID (assuming < 256 cores) */

    /* Step 3: Compute base address of lame_handle_array[cpuid].ctx[active] in %r11 */
    leaq lame_handle_array(%rip), %rdx
    movl %ecx, %eax
    imulq $LAME_HANDLE_SIZE, %rax
    addq %rax, %rdx         /* rdx = &lame_handle_array[cpuid] */
    movq LAME_HANDLE_ACTIVE(%rdx), %r10      /* r10 = active */
    imulq $LAME_CTX_SIZE, %r10
    movq %rdx, %r11
    addq $LAME_HANDLE_CTX, %r11      /* r11 = &ctx[0] */
    addq %r10, %r11                 /* r11 = &ctx[active] */

    /* Step 4: Save all general-purpose registers (except those pushed) into context */
    /* rax will be restored from stack */
    movq %rbx, LAME_CTX_RBX(%r11)
    movq %rcx, LAME_CTX_RCX(%r11)
    /* rdx will be restored from stack */
    movq %rsi, LAME_CTX_RSI(%r11)
    movq %rdi, LAME_CTX_RDI(%r11)
    movq %rbp, LAME_CTX_RBP(%r11)
    movq %r8,  LAME_CTX_R8(%r11)
    movq %r9,  LAME_CTX_R9(%r11)
    /* r10 will be restored from stack */
    /* r11 will be restored from stack */
    movq %r12, LAME_CTX_R12(%r11)
    movq %r13, LAME_CTX_R13(%r11)
    movq %r14, LAME_CTX_R14(%r11)
    movq %r15, LAME_CTX_R15(%r11)

    /* Step 5: Pop and save the minimal set of registers */
    /* at this point rax holds the cpuid, which is no longer needed, thus using it as the scratch register */
    popq %rax /* pop and save rax */
    movq %rax, LAME_CTX_RAX(%r11)
    popq %rax /* pop and save r10 */
    movq %rax, LAME_CTX_R10(%r11)
    popq %rax /* pop and save r11 */
    movq %rax, LAME_CTX_R11(%r11)
    popq %rax /* pop and save rdx */
    movq %rax, LAME_CTX_RDX(%r11)

    /* Step 6: Pop and save the interrupt frame (RIP, CS, RFLAGS, RSP, SS) */
    // popq %rax         /* error code (not processed) */
    
    // popq %rax        /* RIP */
    // movq %rax, LAME_CTX_RIP(%r11)

    // popq %r10        /* CS  (not needed due to same-process switch) */
    // popq %r10        /* RFLAGS */
    // movq %r10, LAME_CTX_RFLAGS(%r11)
    /* no ss:rsp popping due to same-ring handling */
    /* ss:rsp is set up and supplied by the kernel directly into the ctx */

    /* Optionally, set valid/in_use fields */
    /* not needed for now */
    // movl $1, LAME_CTX_VALID(%r13)
    // movl $1, LAME_CTX_IN_USE(%r13)

    /* Step (only for single-thread switch testing): fabricate a new interrupt frame (which is the same one just popped)*/
    // pushq %r10      /* RFLAGS */
    // pushq $0x33     /* CS */
    // pushq %rax      /* RIP */

    /* Step 7: Increment active index and compute new context base in %r13 */
    /* reg values at this point:        */
    /* rax = (scratch)                  */
    /* rdx = &lame_handle_array[cpuid]  */
    /* r10 = active                     */
    /* r11 = &ctx[active]               */
    addq $1, %r10                               /* increment active */
    andq $LAME_COROUTINE_INC_MASK, %r10         /* assuming power of 2 */
    movq %r10, LAME_HANDLE_ACTIVE(%rdx)         /* store 64-bit active */
    imulq $LAME_CTX_SIZE, %r10                   /* r10 = active * sizeof(lame_ctx) */
    movq %rdx, %r11                              /* r11 = &lame_handle_array[cpuid] */
    addq $LAME_HANDLE_CTX, %r11                  /* r11 = &ctx[0] */
    addq %r10, %r11                              /* r11 = &ctx[active] */

    /* Step 8: Fabricate a new interrupt frame on the new stack */
    // pushq LAME_CTX_RFLAGS(%r11)/* RFLAGS */
    // pushq $0x33                /* CS (user code segment) */
    // pushq LAME_CTX_RIP(%r11)   /* RIP */

    /* Step 9: Restore all general-purpose registers from new context */
    // movq LAME_CTX_RAX(%r11), %rax
    // movq LAME_CTX_RBX(%r11), %rbx
    // movq LAME_CTX_RCX(%r11), %rcx
    // movq LAME_CTX_RDX(%r11), %rdx
    // movq LAME_CTX_RSI(%r11), %rsi
    // movq LAME_CTX_RDI(%r11), %rdi
    // movq LAME_CTX_RBP(%r11), %rbp
    // movq LAME_CTX_R8(%r11),  %r8
    // movq LAME_CTX_R9(%r11),  %r9
    // movq LAME_CTX_R10(%r11), %r10
    // movq LAME_CTX_R12(%r11), %r12
    // movq LAME_CTX_R13(%r11), %r13
    // movq LAME_CTX_R14(%r11), %r14
    // movq LAME_CTX_R15(%r11), %r15
    // movq LAME_CTX_R11(%r11), %r11   /* r11 must be the last to restore */

    /* Optionally, clear in_use for this context */
    // movl $0, LAME_CTX_IN_USE(%r11)

    /* Step 10: Return from interrupt (iretq will use the fabricated frame) */
    iretq
SYM_CODE_END(__vdso_lame_entry) 

    .align 16
SYM_CODE_START(__vdso_nop_entry)
    iretq
SYM_CODE_END(__vdso_nop_entry) 
