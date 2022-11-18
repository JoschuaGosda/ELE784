#ifndef MYMODULE
#define MYMODULE 

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

#include <linux/interrupt.h>
#include "port_config.c"

#define FREQ_BASE 1843200
#define LCR 0x03
#define DLL 0x00
#define DLM 0x01
#define FCR 0x02
#define DLAB 0x80
#define IER 0x01
#define ERBFI 0x01
#define ETBEI 0x02

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
