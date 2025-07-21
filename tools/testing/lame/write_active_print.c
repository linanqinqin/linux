#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#define LAME_HANDLE_ARRAY_VDSO_ADDR 0x7ffff7fa76c0ULL

int main() {
    volatile uint64_t *active = (uint64_t *)LAME_HANDLE_ARRAY_VDSO_ADDR;
    printf("Program A: VDSO Write Test (using hardcoded vDSO address)\n");
    printf("lame_handle_array[0].active (vDSO): %p\n", (void*)active);
    printf("Initial lame_handle_array[0].active: %lu\n", *active);
    
    printf("\nPress Enter to trigger int 0x1f (writes 99 to lame_handle_array[0].active)...\n");
    getchar();
    
    // Trigger the interrupt
    __asm__ volatile("int $0x1f");
    
    printf("After int 0x1f, lame_handle_array[0].active: %lu\n", *active);
    
    if (*active == 99) {
        printf("SUCCESS: Value was written correctly!\n");
    } else {
        printf("FAILURE: Expected 99, got %lu\n", *active);
    }
    
    return 0;
}