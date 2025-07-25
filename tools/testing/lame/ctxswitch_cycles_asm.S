.text
.globl ctxswitch_cycles_asm
.type ctxswitch_cycles_asm, @function
ctxswitch_cycles_asm:
    # Save all general-purpose registers
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    # rdtscp start
    xor %eax, %eax
    xor %edx, %edx
    xor %ecx, %ecx
    rdtscp
    shl $32, %rdx
    or %rdx, %rax
    mov %rax, %r14      # r14 = start

    # int 0x1f
    int $0x1f

    # rdtscp end
    xor %eax, %eax
    xor %edx, %edx
    xor %ecx, %ecx
    rdtscp
    shl $32, %rdx
    or %rdx, %rax
    mov %rax, %r15      # r15 = end

    # Restore all general-purpose registers
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx

    # Compute cycles = end - start
    mov %r15, %rax
    sub %r14, %rax
    ret 