#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/device.h>

#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

// used for open & release
#include <stdbool.h>
#include <linux/cred.h> 
#include <linux/spinlock.h>

static ssize_t MyModule_read(struct file *file, char *buff , size_t len, loff_t *off);
static ssize_t MyModule_write(struct file *file, const char *buff , size_t len, loff_t *off);
static int MyModule_open(struct inode *inode, struct file *filp);
static int MyModule_release(struct inode *inode, struct file *filp);
