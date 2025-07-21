#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

// External declaration of the VDSO symbol
extern struct lame_handle lame_handle_array[];

// Structure definitions (matching the C header)
struct lame_ctx {
    uint64_t rsp;
    uint64_t rip;
    uint64_t rflags;
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, r8;
    uint64_t r9, r10, r11, r12;
    uint64_t r13, r14, r15;
    uint32_t valid;
    uint32_t in_use;
    uint32_t padding[2];
} __attribute__((packed, aligned(64)));

struct lame_handle {
    uint64_t active;
    uint64_t bundle_id;
    uint64_t last_switch_tsc;
    uint32_t switch_count;
    uint32_t cpu_id;
    struct lame_ctx ctx[2];  // LAME_COROUTINE_DEPTH = 2
} __attribute__((packed, aligned(64)));

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