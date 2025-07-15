#include <linux/linkage.h>
.code64
.section .text

/* Offsets and sizes from struct lame_handle and struct lame_ctx */
/* These should match the C header! */
.equ LAME_HANDLE_SIZE, 352
.equ LAME_HANDLE_ACTIVE, 0
.equ LAME_HANDLE_CTX, 8
.equ LAME_HANDLE_SWITCH_COUNT, 344
.equ LAME_CTX_SIZE, 160
.equ LAME_CTX_R13, 104
.equ LAME_CTX_VALID, 144

    .align 16
SYM_CODE_START(__vdso_lame_entry)
    /* Prologue: save clobbered registers */
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi

    /* Get CPU ID using rdtscp */
    rdtscp                  /* TSC in rax:rdx, CPU ID in rcx */
    andl $0xFF, %ecx        /* Mask to get CPU ID (assume <256 cores) */

    /* Calculate array index: cpu_id * sizeof(lame_handle) */
    movq $LAME_HANDLE_SIZE, %rax
    mulq %rcx               /* rax = cpu_id * sizeof(lame_handle) */

    /* Get pointer to handle */
    leaq lame_handle_array(%rip), %rdx
    addq %rax, %rdx         /* rdx = &lame_handle_array[cpu_id] */

    /* Get current active index */
    movzbl LAME_HANDLE_ACTIVE(%rdx), %eax   /* active index -> eax */

    /* Save r13 to current active context's r13 */
    movq %r13, LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE)
    addq $LAME_CTX_R13, LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE)
    movq %r13, (%rdx)
    subq $LAME_CTX_R13, LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE)

    /* Mark current context as valid */
    movl $1, LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE)
    addq $LAME_CTX_VALID, LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE)
    movl $1, (%rdx)
    subq $LAME_CTX_VALID, LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE)

    /* Switch to next coroutine */
    xorl $1, %eax           /* flip active index (0->1, 1->0) */
    movb %al, LAME_HANDLE_ACTIVE(%rdx)      /* update active index */

    /* Get next context */
    movzbl %al, %eax
    movq LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE), %r13
    addq $LAME_CTX_R13, LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE)
    movq (%rdx), %r13
    subq $LAME_CTX_R13, LAME_HANDLE_CTX(%rdx,%rax,LAME_CTX_SIZE)

    /* Increment switch count */
    incl LAME_HANDLE_SWITCH_COUNT(%rdx)

    /* Epilogue: restore registers */
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rax

    /* Return from interrupt */
    iretq
SYM_CODE_END(__vdso_lame_entry) 