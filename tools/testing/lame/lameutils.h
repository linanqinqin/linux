/*
 * LAME (Latency-Aware Memory Exception) utility functions
 *
 * This header provides helper functions for working with the LAME kernel module.
 */

#ifndef _LAMEUTILS_H
#define _LAMEUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/lame.h>

/**
 * lame_handler_register - Register or unregister a LAME handler
 * @handler: Pointer to the handler function (NULL to unregister)
 * @enable: 1 to register, 0 to unregister
 *
 * Returns: 0 on success, -1 on failure (errno is set)
 */
static inline int lame_handler_register(void *handler, int enable)
{
    int fd;
    struct lame_arg arg;
    int ret;

    /* Open the LAME device */
    fd = open("/dev/lame", O_RDWR);
    if (fd < 0) {
        return -1;
    }

    /* Set up the argument structure */
    arg.is_present = enable ? 1 : 0;
    arg.handler_stub_addr = enable ? (__u64)handler : 0;

    /* Perform the ioctl */
    ret = ioctl(fd, LAME_REGISTER, &arg);
    
    /* Close the device */
    close(fd);

    return ret;
}

/**
 * lame_int - Invoke the LAME interrupt (INT 0x1F)
 *
 * This is a naked inline function that simply invokes int 0x1f.
 * The handler registered via lame_handler_register will be called.
 */
static inline void __attribute__((naked)) lame_int(void)
{
    asm volatile("int $0x1f");
}

#endif /* _LAMEUTILS_H */ 