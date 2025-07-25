#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <x86intrin.h> 

#define NITER 10

// static inline uint64_t rdtscp(uint32_t *aux)
// {
//     uint32_t lo, hi;
//     asm volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(*aux) ::);
//     return ((uint64_t)hi << 32) | lo;
// }

extern uint64_t ctxswitch_cycles_asm(void);

int main() {
    unsigned long long cycles[NITER];
    unsigned int aux;
    for (int i = 0; i < NITER; ++i) {
        cycles[i] = ctxswitch_cycles_asm();
    }
    for (int i = 0; i < NITER; ++i) {
        printf("%llu\n", cycles[i]);
    }
    return 0;
} 