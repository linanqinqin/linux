#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <stdint.h>  /* For uint64_t */

/* Function to trigger LAME interrupt */
static inline void trigger_lame(void) {
    asm volatile("int $0x1f");
}

/* Function to get the returned value from r13 */
static inline uint64_t get_returned_value(void) {
    uint64_t value;
    asm volatile("movq %%r13, %0" : "=r" (value) : : "r13");
    return value;
}

/* Function to get CPU ID using Linux syscall */
static inline int get_cpu_id_syscall(void) {
    return sched_getcpu();
}

int main(void) {
    printf("Testing LAME CPU ID detection...\n");
    
    /* Get CPU ID using Linux syscall */
    int cpu_id_syscall = get_cpu_id_syscall();
    printf("CPU ID from syscall (sched_getcpu): %d\n", cpu_id_syscall);
    
    /* Trigger LAME interrupt */
    printf("Calling INT 0x1f...\n");
    trigger_lame();
    
    /* Get CPU ID from LAME handler */
    uint64_t cpu_id_lame = get_returned_value();
    printf("CPU ID from LAME handler (r13): %lu\n", cpu_id_lame);
    
    /* Compare results */
    if (cpu_id_syscall == cpu_id_lame) {
        printf("SUCCESS: CPU ID detection working correctly!\n");
        printf("- Both methods returned the same CPU ID: %d\n", cpu_id_syscall);
        return 0;
    } else {
        printf("FAILURE: CPU ID mismatch!\n");
        printf("- Syscall returned: %d\n", cpu_id_syscall);
        printf("- LAME handler returned: %lu\n", cpu_id_lame);
        return 1;
    }
}