/*
 * Test program for LAME (Latency-Aware Memory Exception) ioctl interface
 *
 * This program tests the basic ioctl functionality of the LAME kernel module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* Include the LAME header */
#include <linux/lame.h>

void __attribute__((naked)) my_lame_handler(void)
{
    asm volatile ("iretq");
}

int main(void)
{
    int fd;
    struct lame_arg arg;
    int ret;
    
    printf("LAME Test Program\n");
    printf("=================\n");
    
    /* Open the LAME device */
    fd = open("/dev/lame", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[errno %d] Failed to open /dev/lame\n", errno);
        return 1;
    }
    
    printf("Successfully opened /dev/lame\n");
    
    /* Test 1: Enable LAME */
    printf("\nTest 1: Enabling LAME...\n");
    arg.is_present = 1;
    arg.handler_stub_addr = (uint64_t)my_lame_handler;
    
    ret = ioctl(fd, LAME_REGISTER, &arg);
    if (ret < 0) {
        fprintf(stderr, "[errno %d] ioctl LAME_REGISTER (enable) failed\n", errno);
    } else {
        printf("LAME_REGISTER (enable) succeeded\n");
    }
    
    /* Test 2: Disable LAME */
    printf("\nTest 2: Disabling LAME...\n");
    arg.is_present = 0;
    
    ret = ioctl(fd, LAME_REGISTER, &arg);
    if (ret < 0) {
        fprintf(stderr, "[errno %d] ioctl LAME_REGISTER (disable) failed\n", errno);
    } else {
        printf("LAME_REGISTER (disable) succeeded\n");
    }
    
    /* Test 3: Invalid command */
    printf("\nTest 3: Testing invalid ioctl command...\n");
    ret = ioctl(fd, 0x12345678, &arg);
    if (ret < 0) {
        printf("Invalid ioctl correctly rejected (expected)\n");
    } else {
        printf("WARNING: Invalid ioctl was accepted (unexpected)\n");
    }
    
    /* Close the device */
    close(fd);
    printf("\nTest completed successfully\n");
    
    return 0;
} 