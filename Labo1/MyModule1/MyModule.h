#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/device.h>

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/kdev_t.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/sched.h> 

#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include "newbuffer.c"


// used for open & release
#include <stdbool.h>
#include <linux/cred.h> 
#include <linux/spinlock.h>



#define IOC_MAGIC 'k'
#define SETBAUDRATE         _IOW(IOC_MAGIC,0,int)
#define SETDATASIZE         _IOW(IOC_MAGIC,1,int)
#define SETPARITY           _IOW(IOC_MAGIC,2,int)
#define GETBUFFERSIZE       _IOR(IOC_MAGIC,3,int)
#define SETBUFFERSIZE       _IOW(IOC_MAGIC,4,int)



static ssize_t MyModule_read(struct file *file, char *buff , size_t len, loff_t *off);
static ssize_t MyModule_write(struct file *file, const char *buff , size_t len, loff_t *off);
static int MyModule_open(struct inode *inode, struct file *filp);
static int MyModule_release(struct inode *inode, struct file *filp);
static int MyModule_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

