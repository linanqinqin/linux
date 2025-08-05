/* linanqinqin */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * LAME (Latency-Aware Memory Exception) user-space interface
 *
 * This header defines the ioctl interface for the LAME kernel module.
 */

#ifndef _UAPI_LINUX_LAME_H
#define _UAPI_LINUX_LAME_H

#include <linux/ioctl.h>
#include <stdint.h>

/* IOCTL command codes */
#define LAME_IOC_MAGIC 'L'
#define LAME_REGISTER _IOW(LAME_IOC_MAGIC, 1, struct lame_arg)

/* Argument structure */
struct lame_arg {
    uint8_t is_present;        /* Use uint8_t instead of bool for ABI stability */
    uint64_t handler_stub_addr; /* Use uint64_t instead of void* for ABI stability */
};

#endif /* _UAPI_LINUX_LAME_H */ 
/* end */
