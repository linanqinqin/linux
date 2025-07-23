#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define NUM_ITER 10

int main() {
    srand(time(NULL) ^ getpid());
    int failed = 0;
    uint64_t prev_r13 = 0;
    for (int i = 0; i < NUM_ITER; ++i) {
        uint64_t rand_val = ((uint64_t)rand() << 32) | rand();
        uint64_t r13_before = rand_val;
        uint64_t r13_after = 0;
        // Assign r13, trigger INT 0x1f, then read r13
        asm volatile (
            "mov %[val], %%r13\n\t"
            "int $0x1f\n\t"
            "mov %%r13, %[out]\n\t"
            : [out] "=r" (r13_after)
            : [val] "r" (r13_before)
            : "r13", "memory"
        );
        if (i > 0) {
            printf("Iter %d: r13 after=0x%lx, prev r13 before=0x%lx\n", i, r13_after, prev_r13);
            if (r13_after != prev_r13) {
                printf("  [FAIL] r13 mismatch!\n");
                failed = 1;
            }
        } else {
            printf("Iter %d: (first iteration, no check)\n", i);
        }
        prev_r13 = r13_before;
    }
    if (!failed) {
        printf("All r13 values matched previous iteration after LAME handler!\n");
        return 0;
    } else {
        printf("Some r13 values mismatched after LAME handler!\n");
        return 1;
    }
} 