.section .data
NITER:      .quad 10
newline:    .asciz "\n"
buf:        .space 32

.section .text
.global _start

_start:
    mov $0, %r12                # i = 0
    mov NITER(%rip), %r13       # r13 = NITER

loop_start:
    cmp %r13, %r12
    jge done

    # Save all general-purpose registers
    push %rax
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
    push %r14
    push %r15
    push %r12   # Save loop counter
    # r13 is NITER, don't need to save

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
    pop %r12
    pop %r15
    pop %r14
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
    pop %rax

    # Compute cycles = end - start
    mov %r15, %rax
    sub %r14, %rax

    # Print cycles (convert to string)
    mov $buf, %rsi
    mov $32, %rcx
    call u64_to_str
    mov $1, %rdi            # stdout
    mov $buf, %rsi
    mov %rax, %rdx          # length
    mov $1, %rax            # sys_write
    syscall

    # Print newline
    mov $1, %rax
    mov $1, %rdi
    mov $newline, %rsi
    mov $1, %rdx
    syscall

    inc %r12
    jmp loop_start

done:
    # exit(0)
    mov $60, %rax
    xor %rdi, %rdi
    syscall

# rax = value, rsi = buf, rcx = maxlen
# returns: rax = strlen
# buf is filled with ASCII decimal digits, no trailing null
.type u64_to_str, @function
u64_to_str:
    mov %rsi, %r8      # save buf
    mov $0, %r9        # digit count
    mov %rax, %r10     # value
    cmp $0, %rax
    jne .u64_loop
    movb $'0', (%rsi)
    mov $1, %rax
    ret
.u64_loop:
    mov %r10, %rdx
    mov $10, %rbx
    xor %rax, %rax
    div %rbx           # quotient in rax, remainder in rdx
    add $'0', %dl
    mov %dl, (%rsi)
    inc %rsi
    inc %r9
    mov %rax, %r10
    test %rax, %rax
    jnz .u64_loop
    # reverse string in place
    mov %r8, %rsi      # buf
    mov %r9, %rcx      # len
    shr $1, %rcx
    jz .u64_done
    mov %r8, %rdi      # start
    lea -1(%r8, %r9), %rdx # end
.u64_rev:
    movb (%rdi), %al
    movb (%rdx), %bl
    movb %bl, (%rdi)
    movb %al, (%rdx)
    inc %rdi
    dec %rdx
    loop .u64_rev
.u64_done:
    mov %r9, %rax      # return len
    ret 