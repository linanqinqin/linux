#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define LAME_HANDLE_ARRAY_VDSO_ADDR 0x7ffff7fa76c0ULL

int main() {
    char input[10];
    volatile uint64_t *active = (uint64_t *)LAME_HANDLE_ARRAY_VDSO_ADDR;
    
    printf("Program B: VDSO Read Test (using hardcoded vDSO address)\n");
    printf("lame_handle_array[0].active (vDSO): %p\n", (void*)active);
    printf("Initial lame_handle_array[0].active: %lu\n", *active);
    
    printf("\nCommands:\n");
    printf("  'read' - Print current value\n");
    printf("  'exit' - Exit program\n");
    printf("  (any other input) - Print current value\n\n");
    
    while (1) {
        printf("Enter command: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }
        
        printf("Current lame_handle_array[0].active: %lu\n", *active);
    }
    
    return 0;
}