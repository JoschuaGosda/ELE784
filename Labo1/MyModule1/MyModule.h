#ifndef MYMODULE_H
#define MYMODULE_H

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
#include "newbuffer.c"
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>



// used for open & release
#include <stdbool.h>
#include <linux/cred.h> 
#include <linux/spinlock.h>
#include <asm/ioctl.h>
#include <asm/io.h>
#include <linux/ioctl.h>
#include "ioctl_config.h"
#include "port_config.c"


 struct pData
{
  int8_t owner; // takes the owners id
 // circular_buffer b_wr, b_rd;
  circular_buffer* buf_wr;
  circular_buffer* buf_rd;

  uint8_t numPort;
  bool fREAD;
  bool fWRITE;
  struct semaphore sem;
  struct spinlock splock;
  struct mutex mutex;
  wait_queue_head_t RdQ, WrQ;
  uint16_t base_addr;
  uint8_t num_interupt;
} pData;


static ssize_t MyModule_read(struct file *file, char *buff , size_t len, loff_t *off);
static ssize_t MyModule_write(struct file *file, const char *buff , size_t len, loff_t *off);
static int MyModule_open(struct inode *inode, struct file *filp);
static int MyModule_release(struct inode *inode, struct file *filp);
static long MyModule_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);



#endif
