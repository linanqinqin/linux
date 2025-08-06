/*
 * Test program for LAME (Latency-Aware Memory Exception) ioctl interface
 *
 * This program tests the basic ioctl functionality of the LAME kernel module.
 * Now uses lameutils.h helper functions for cleaner code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "lameutils.h"

void __attribute__((naked)) my_lame_handler(void)
{
    asm volatile ("iretq");
}

int main(void)
{
    int ret;
    
    printf("LAME Test Program (using lameutils.h)\n");
    printf("=====================================\n");
    
    /* Test 1: Enable LAME */
    printf("\nTest 1: Enabling LAME...\n");
    ret = lame_handler_register(my_lame_handler, 1);
    if (ret < 0) {
        fprintf(stderr, "[errno %d] Failed to register LAME handler\n", errno);
        return 1;
    } else {
        printf("LAME handler registered successfully\n");
    }
    
    /* Test 2: Invoke the interrupt */
    printf("\nTest 2: Invoking LAME interrupt...\n");
    printf("Before lame_int()\n");
    lame_int();
    printf("After lame_int()\n");
    
    /* Test 3: Disable LAME */
    printf("\nTest 3: Disabling LAME...\n");
    ret = lame_handler_register(NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "[errno %d] Failed to unregister LAME handler\n", errno);
        return 1;
    } else {
        printf("LAME handler unregistered successfully\n");
    }
    
    printf("\nTest completed successfully\n");
    
    return 0;
} 