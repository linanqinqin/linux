// lame-int.c
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

extern volatile int lame_handler_count;
extern int __vdso_lame_add(int x, int y);  // declare the vDSO function

static inline void trigger_lame(void)
{
    asm volatile("int $0x1f");
}

int main(int argc, char *argv[]) {
    int result = __vdso_lame_add(0, 0);
    printf("lame_add = %x\n", result);

    uint64_t counter;
    
    printf("=== LAME Handler Test (Register-based) ===\n");
    
    /* Check command line arguments */
    if (argc != 2) {
        printf("Usage: %s <initial_value>\n", argv[0]);
        printf("Example: %s 99\n", argv[0]);
        return 1;
    }

    /* Parse the initial value from command line */
    counter = strtoull(argv[1], NULL, 10);

    /* Set initial value in r13 */
    asm volatile("movq %0, %%r13" : : "r"(counter) : "r13");
    printf("Set counter in r13 to: %lu\n", counter);
    
    /* Trigger the LAME interrupt */
    printf("Triggering LAME interrupt (INT $0x1F)...\n");
    trigger_lame();
    
    /* Read the value back from r13 */
    asm volatile("movq %%r13, %0" : "=r"(counter) : : "r13");
    printf("After interrupt, counter in r13: %lu\n", counter);
    
    /* Verify the result */
    uint64_t expected = strtoull(argv[1], NULL, 10) + 1;
    if (counter == expected) {
        printf("✓ SUCCESS: Counter was incremented by 1!\n");
        printf("✓ LAME handler is working correctly!\n");
        return 0;
    } else {
        printf("✗ FAILURE: Expected %lu, got %lu\n", expected, counter);
        printf("✗ LAME handler may not be working\n");
        return 1;
    }

    return 0;
}
