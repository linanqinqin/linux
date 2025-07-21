#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <asm/vdso/lame_data.h>

// External declaration of the VDSO symbol
extern struct lame_handle lame_handle_array[];

int main() {
    char input[10];
    
    printf("Program B: VDSO Read Test\n");
    printf("lame_handle_array address: %p\n", (void*)lame_handle_array);
    printf("Initial lame_handle_array[0].active: %lu\n", lame_handle_array[0].active);
    
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
        
        printf("Current lame_handle_array[0].active: %lu\n", lame_handle_array[0].active);
    }
    
    return 0;
}