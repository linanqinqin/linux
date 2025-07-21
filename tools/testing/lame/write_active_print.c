#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

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