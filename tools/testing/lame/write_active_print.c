#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <asm/vdso/lame_data.h>

int main() {
    printf("Program A: VDSO Write Test\n");
    printf("lame_handle_array address: %p\n", (void*)lame_handle_array);
    printf("Initial lame_handle_array[0].active: %lu\n", lame_handle_array[0].active);
    
    printf("\nPress Enter to trigger int 0x1f (writes 99 to lame_handle_array[0].active)...\n");
    getchar();
    
    // Trigger the interrupt
    __asm__ volatile("int $0x1f");
    
    printf("After int 0x1f, lame_handle_array[0].active: %lu\n", lame_handle_array[0].active);
    
    if (lame_handle_array[0].active == 99) {
        printf("SUCCESS: Value was written correctly!\n");
    } else {
        printf("FAILURE: Expected 99, got %lu\n", lame_handle_array[0].active);
    }
    
    return 0;
}