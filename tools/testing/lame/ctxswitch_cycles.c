#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define NITER 10

static inline uint64_t rdtscp(uint32_t *aux)
{
    uint32_t lo, hi;
    asm volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(*aux) ::);
    return ((uint64_t)hi << 32) | lo;
}

int main() {
    uint64_t cycles[NITER];
    uint32_t aux;
    for (int i = 0; i < NITER; ++i) {
        uint64_t start, end;
        /* Save all general-purpose registers */
        uint64_t regs[16];
        asm volatile (
            "movq %%rax, 0(%0)\n\t"
            "movq %%rbx, 8(%0)\n\t"
            "movq %%rcx, 16(%0)\n\t"
            "movq %%rdx, 24(%0)\n\t"
            "movq %%rsi, 32(%0)\n\t"
            "movq %%rdi, 40(%0)\n\t"
            "movq %%rbp, 48(%0)\n\t"
            "movq %%r8,  56(%0)\n\t"
            "movq %%r9,  64(%0)\n\t"
            "movq %%r10, 72(%0)\n\t"
            "movq %%r11, 80(%0)\n\t"
            "movq %%r12, 88(%0)\n\t"
            "movq %%r13, 96(%0)\n\t"
            "movq %%r14, 104(%0)\n\t"
            "movq %%r15, 112(%0)\n\t"
            : : "r"(regs) : "memory"
        );
        start = rdtscp(&aux);
        // asm volatile ("int $0x1f" ::: "memory");
        end = rdtscp(&aux);
        /* Restore all general-purpose registers */
        asm volatile (
            "movq 0(%0), %%rax\n\t"
            "movq 8(%0), %%rbx\n\t"
            "movq 16(%0), %%rcx\n\t"
            "movq 24(%0), %%rdx\n\t"
            "movq 32(%0), %%rsi\n\t"
            "movq 40(%0), %%rdi\n\t"
            "movq 48(%0), %%rbp\n\t"
            "movq 56(%0), %%r8\n\t"
            "movq 64(%0), %%r9\n\t"
            "movq 72(%0), %%r10\n\t"
            "movq 80(%0), %%r11\n\t"
            "movq 88(%0), %%r12\n\t"
            "movq 96(%0), %%r13\n\t"
            "movq 104(%0), %%r14\n\t"
            "movq 112(%0), %%r15\n\t"
            : : "r"(regs) : "memory"
        );
        cycles[i] = end - start;
    }
    for (int i = 0; i < NITER; ++i) {
        printf("%lu\n", cycles[i]);
    }
    return 0;
} 