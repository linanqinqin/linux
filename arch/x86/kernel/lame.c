/* linanqinqin */
// SPDX-License-Identifier: GPL-2.0-only
/*
 * LAME (Latency-Aware Memory Exception) Runtime Configuration Module
 *
 * This module provides a character device interface for runtime configuration
 * of the LAME exception handler through ioctl commands.
 *
 * Copyright (C) 2025 Nanqinqin Li <linanqinqin@princeton.edu>
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <asm/desc.h>
#include <asm/trapnr.h>
#include <asm/set_memory.h>
#include <uapi/linux/lame.h>

/* char device name */
#define LAME_DEVICE_NAME "lame"

/* Module parameters */
static int major = 0;  /* 0 means auto-assign */
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major device number for LAME device");

/* Global variables */
static dev_t lame_dev;
static struct cdev *lame_cdev;
static struct class *lame_class;
static struct device *lame_device;

/* Forward declarations */
static int lame_open(struct inode *inode, struct file *file);
static int lame_release(struct inode *inode, struct file *file);
static long lame_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/* File operations structure */
static const struct file_operations lame_fops = {
    .owner = THIS_MODULE,
    .open = lame_open,
    .release = lame_release,
    .unlocked_ioctl = lame_ioctl,
    .compat_ioctl = lame_ioctl,  /* For 32-bit compatibility */
};

/* File operation implementations */
static int lame_open(struct inode *inode, struct file *file)
{
    pr_debug("LAME device opened\n");
    return 0;
}

static int lame_release(struct inode *inode, struct file *file)
{
    pr_debug("LAME device closed\n");
    return 0;
}

/* IOCTL handler - LAME logic will be implemented here later */
static int lame_register_ioctl(struct file *file, unsigned long arg)
{
    struct lame_arg user_arg;
    int ret = 0;
    
    pr_debug("LAME_REGISTER ioctl called\n");
    
    /* Copy argument from user space */
    if (copy_from_user(&user_arg, (void __user *)arg, sizeof(user_arg))) {
        pr_err("Failed to copy argument from user space\n");
        return -EFAULT;
    }
    
    pr_debug("LAME_REGISTER: is_present=%d, handler_addr=0x%llx\n", 
             user_arg.is_present, user_arg.handler_stub_addr);
    
    /* TODO: Implement actual LAME logic here */
    /* For now, just acknowledge the request */
    
    if (user_arg.is_present) {
        pr_info("LAME_REGISTER: enabling LAME handler at 0x%llx\n", 
                user_arg.handler_stub_addr);
        /* TODO: Enable LAME handler */
    } else {
        pr_info("LAME_REGISTER: disabling LAME\n");
        /* TODO: Disable LAME handler */
    }
    
    return ret;
}

/* Main IOCTL dispatcher */
static long lame_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    
    /* Check if the command is for us */
    if (_IOC_TYPE(cmd) != LAME_IOC_MAGIC) {
        pr_err("Invalid ioctl magic number\n");
        return -ENOTTY;
    }
    
    /* Check if the command number is valid */
    if (_IOC_NR(cmd) > 1) {
        pr_err("Invalid ioctl command number\n");
        return -ENOTTY;
    }
    
    /* Dispatch to appropriate handler */
    switch (cmd) {
    case LAME_REGISTER:
        ret = lame_register_ioctl(file, arg);
        break;
    default:
        pr_err("Unknown ioctl command: 0x%x\n", cmd);
        ret = -ENOTTY;
        break;
    }
    
    return ret;
}

/* Module initialization */
static int __init lame_init(void)
{
    int ret;
    
    pr_info("LAME module initializing...\n");
    
    /* Allocate device number */
    if (major) {
        lame_dev = MKDEV(major, 0);
        ret = register_chrdev_region(lame_dev, 1, LAME_DEVICE_NAME);
    } else {
        ret = alloc_chrdev_region(&lame_dev, 0, 1, LAME_DEVICE_NAME);
    }
    
    if (ret < 0) {
        pr_err("Failed to allocate device number: %d\n", ret);
        return ret;
    }
    
    /* Create character device */
    lame_cdev = cdev_alloc();
    if (!lame_cdev) {
        pr_err("Failed to allocate cdev\n");
        ret = -ENOMEM;
        goto error_unregister;
    }
    
    cdev_init(lame_cdev, &lame_fops);
    lame_cdev->owner = THIS_MODULE;
    
    ret = cdev_add(lame_cdev, lame_dev, 1);
    if (ret < 0) {
        pr_err("Failed to add cdev: %d\n", ret);
        goto error_cdev_del;
    }
    
    /* Create device class */
    lame_class = class_create(THIS_MODULE, LAME_DEVICE_NAME);
    if (IS_ERR(lame_class)) {
        pr_err("Failed to create class: %ld\n", PTR_ERR(lame_class));
        ret = PTR_ERR(lame_class);
        goto error_cdev_del;
    }
    
    /* Create device file */
    lame_device = device_create(lame_class, NULL, lame_dev, NULL, LAME_DEVICE_NAME);
    if (IS_ERR(lame_device)) {
        pr_err("Failed to create device: %ld\n", PTR_ERR(lame_device));
        ret = PTR_ERR(lame_device);
        goto error_class_destroy;
    }
    
    pr_info("LAME device created successfully (major=%d, minor=%d)\n", 
            MAJOR(lame_dev), MINOR(lame_dev));
    pr_info("Device file: /dev/%s\n", LAME_DEVICE_NAME);
    
    return 0;
    
error_class_destroy:
    class_destroy(lame_class);
error_cdev_del:
    cdev_del(lame_cdev);
error_unregister:
    unregister_chrdev_region(lame_dev, 1);
    return ret;
}

/* Module cleanup */
static void __exit lame_exit(void)
{
    pr_info("LAME module unloading...\n");
    
    /* Remove device file */
    if (lame_device) {
        device_destroy(lame_class, lame_dev);
    }
    
    /* Remove device class */
    if (lame_class) {
        class_destroy(lame_class);
    }
    
    /* Remove character device */
    if (lame_cdev) {
        cdev_del(lame_cdev);
    }
    
    /* Unregister device number */
    unregister_chrdev_region(lame_dev, 1);
    
    pr_info("LAME module unloaded successfully\n");
}

/* Module information */
module_init(lame_init);
module_exit(lame_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nanqinqin Li");
MODULE_DESCRIPTION("LAME (Latency-Aware Memory Exception) Runtime Configuration Module");
MODULE_VERSION("1.0"); 
/* end */
