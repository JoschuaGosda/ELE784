#include "MyModule.h"

#define PORTNUMBER 2
#define BUFFER_CAPACITY 128
#define BUFFER_ELEMENTSIZE 8

MODULE_AUTHOR("Bruno");
MODULE_LICENSE("Dual BSD/GPL");

int MyModule_X = 5;

module_param(MyModule_X, int, S_IRUGO);

EXPORT_SYMBOL_GPL(MyModule_X);

dev_t My_dev;
struct class *MyClass;

struct file_operations MyModule_fops = {
    .owner = THIS_MODULE,
    .read = MyModule_read,
    .write = MyModule_write,
    .open = MyModule_open,
    .release = MyModule_release};

struct pData
{
  int8_t owner; // takes the owners id
  circular_buffer buf_rdwr;
  uint8_t numPort;
  bool fREAD;
  bool fWRITE;
  struct semaphore sem;
  struct spinlock splock;
  struct mutex mutex;
  wait_queue_head_t inQ, outQ;
} pData;

struct pData pdata[PORTNUMBER];

bool portOpen[PORTNUMBER];

struct cdev My_cdev;

static int __init mod_init(void)
{

  int n;

  // int alloc_chrdev_region(dev_t* dev, unsigned int firstminor, unsigned int count, char *name)
  alloc_chrdev_region(&My_dev, 0, PORTNUMBER, "MonPremierPilote");

  MyClass = class_create(THIS_MODULE, "MyModule");
  for (n = 0; n < PORTNUMBER; n++)
  {
    // create un noeud dans /dev
    device_create(MyClass, NULL, (My_dev + n), NULL, "MyModuleNode%d", n);
  }

  cdev_init(&My_cdev, &MyModule_fops); // add pointer in struct cdev to struct file operations
  cdev_add(&My_cdev, My_dev, PORTNUMBER); // add pilote to the kernel - struct cdev, dev number (/dev/My_dev1), int count

  for (n = 0; n < PORTNUMBER; n++)
  {
    cb_init(&pdata[n].buf_rdwr, BUFFER_CAPACITY, BUFFER_ELEMENTSIZE);
    pdata[n].fREAD = false;
    pdata[n].fWRITE = false;
    pdata[n].owner = -1;
    sema_init(&pdata[n].sem, 1);
    spin_lock_init(&pdata[n].splock);
    mutex_init(&pdata[n].mutex);
    init_waitqueue_head(&pdata[n].outQ);
    init_waitqueue_head(&pdata[n].outQ);
  }

  printk(KERN_WARNING "MyMod : Kernel Module initilized with number  %u\n", MyModule_X);

  return 0;
}

static int MyModule_open(struct inode *inode, struct file *filp)
{

  uint32_t numPort;
  struct pData *pdata_p;

  printk(KERN_WARNING "MyMod: OPEN");

  // get the number of the device driver and connect to the right private data structure
  numPort = MINOR(inode->i_rdev);
  pdata_p = &pdata[numPort];
  filp->private_data = pdata_p;

  spin_lock(&pdata_p->splock);
  if (pdata_p->owner < 0)
  {
    pdata_p->owner = current_uid().val;
    printk(KERN_WARNING "MyMod: owner is set to %u", pdata_p->owner);
  }
  else if (pdata_p->owner != current_uid().val)
  {
    printk(KERN_WARNING "MyMod: current user is not the owner");
    spin_unlock(&pdata_p->splock);
    return -EACCES;
  } else 
  {
    printk(KERN_WARNING "MyMod: current user is the owner");
  }

  switch (filp->f_flags & O_ACCMODE)
  {
  case O_RDONLY:
    printk(KERN_WARNING "MyMod: O_RDONLY access");
    // statements
    if (pdata_p->fREAD)
    {
      spin_unlock(&pdata_p->splock);
      return -EACCES;
    }
    else
    {
      pdata_p->fREAD = true;
    }
    break;

  case O_WRONLY:
    printk(KERN_WARNING "MyMod: O_WRONLY access");
    // statements
    if (pdata_p->fWRITE)
    {
      spin_unlock(&pdata_p->splock);
      return -EACCES;
    }
    else
    {
      pdata_p->fWRITE = true;
    }
    break;

  case O_RDWR:
    printk(KERN_WARNING "MyMod: O_RDWR access");
    if (pdata_p->fWRITE | pdata_p->fREAD)
    {
      spin_unlock(&pdata_p->splock);
      return -EACCES;
    }
    else
    {
      pdata_p->fWRITE = true;
      pdata_p->fREAD = true;
    }
    break;

  default:
    printk(KERN_WARNING "MyMod: file is tried to be opened differently than implemented");
    // default statements
  }
  spin_unlock(&pdata_p->splock);

    // TODO: Port Série doit être placé en mode Réception
  printk(KERN_WARNING "MyMod: OPEN end\n");
  return 0;
}

static ssize_t MyModule_read(struct file *filp, char *buff, size_t len, loff_t *off)
{

  // send from kernelSpace to userSpace
  uint8_t BufR[8];
  int i;
  size_t count;
  struct pData *pdata_p = filp->private_data;

  printk(KERN_WARNING "MyMod: READ\n");

  spin_lock(&pdata_p->splock);
  // TODO: protection d'access
  if (!((filp->f_flags & O_ACCMODE) == O_RDONLY || (filp->f_flags & O_ACCMODE) == O_RDWR))
  {
    spin_unlock(&pdata_p->splock);
    return -EACCES;
  }
  spin_unlock(&pdata_p->splock);

  mutex_lock(&pdata_p->mutex);// todo ??should be semaphore 

  while(cb_count(&pdata_p->buf_rdwr) == 0 ){// no data in the cbuffer
    if(filp->f_flags & O_NONBLOCK) {
      printk(KERN_WARNING "MyMod: READ : NO DATA AVAILABLE AND NON BLOCK\n");
      return -EAGAIN;
    } else {
        printk(KERN_WARNING "MyMod: READ : NO DATA BUT WILL WAIT\n");
        wait_event_interruptible(pdata_p->inQ, cb_count(&pdata_p->buf_rdwr) > 0 ); // waiting for someting in the buffer
      }
  }
  //data in the buffer , on ajuste le compte , plus petit entre ask et available
  count = (len <= cb_count(&pdata_p->buf_rdwr)) ? len : cb_count(&pdata_p->buf_rdwr);

  // retire content of ring buffer to local buffer une a la fois
  
  for (i = 0; i < count ; i++) {
    cb_pop(&pdata_p->buf_rdwr, &BufR[i]);
  }
  mutex_unlock(&pdata_p->mutex);

  printk(KERN_WARNING "MyMod: copied from ring buffer to local buffer \n");

  if (copy_to_user(buff, &BufR, count) == 0) {
    printk(KERN_WARNING "MyMod: All bytes have been sent to user");
    return 0;
  } else {
      printk(KERN_WARNING "MyMod: Some bytes have been sent but sill available : %lu", count);
      return count;
    // TODO: implement the mechanism depending on blocking / non-blocking
  }
  //something went wrong 
  return -EFAULT; 
  
}

static ssize_t MyModule_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
  // copy from userSpace to kernelSpace
  uint8_t BufW[8];
  int i;
  struct pData *pdata_p = filp->private_data;

  spin_lock(&pdata_p->splock);
  // protection d'access
  if (!((filp->f_flags & O_ACCMODE) == O_WRONLY || (filp->f_flags & O_ACCMODE) == O_RDWR))
  {
    spin_unlock(&pdata_p->splock);
    return -EACCES;
  }
  spin_unlock(&pdata_p->splock);

  printk(KERN_WARNING "MyMod: WRITE\n");
  copy_from_user(&BufW, buff, len);

  for (i = 0; i < 8; i++)
  {
    printk(KERN_WARNING "MyMod: Local Buffer %d\n", BufW[i]);
  }

  mutex_lock(&pdata_p->mutex);
  // copy content of local buffer to ring buffer une a la fois
  for (i = 0; i < len; i++)
  {
    cb_push(&pdata_p->buf_rdwr, &BufW[i]);
  }
  mutex_unlock(&pdata_p->mutex);

  return 0;
}


static int MyModule_release(struct inode *inode, struct file *filp)
{

  //uint32_t numPort;
  struct pData *pdata_p = filp->private_data;

  printk(KERN_WARNING "MyMod: RELEASE");

  spin_lock(&pdata_p->splock);
  switch (filp->f_flags & O_ACCMODE)
  {
  case O_RDONLY:
    printk(KERN_WARNING "MyMod: O_RDONLY acces reset");
    // statements
    pdata_p->fREAD = false;
    break;

  case O_WRONLY:
    printk(KERN_WARNING "MyMod: O_WRONLY access reset");
    // statements
    pdata_p->fWRITE = false;
    break;

  case O_RDWR:
    printk(KERN_WARNING "MyMod: O_RDWR access reset");
    pdata_p->fWRITE = false;
    pdata_p->fREAD = false;
    break;

  default:
    // default statements
    printk(KERN_WARNING "MyMod: file is tried to be opened differently than implemented");
  }

  printk(KERN_WARNING "MyMod: fWRITE flag: %u\n", pdata_p->fWRITE);
  printk(KERN_WARNING "MyMod: fREAD flag: %u\n", pdata_p->fREAD);

  // reset owner if no current read and write
  if (!(pdata_p->fREAD | pdata_p->fWRITE))
  {
    pdata_p->owner = -1;
    printk(KERN_WARNING "MyMod: owner is set to %d", pdata_p->owner);
  }
  spin_unlock(&pdata_p->splock);

  // desattach the private data 
  filp->private_data = NULL;

  printk(KERN_WARNING "MyMod: RELEASE end\n");

  // TODO: Et si le mode d’ouverture était O_RDONL Y ou O_RDWR, la Réception du Port Série doit être interrompue afin d’arrêter de recevoir des données

  return 0;
}

static void __exit mod_exit(void)
{
  int n;

  cdev_del(&My_cdev);

  for (n = 0; n < PORTNUMBER; n++)
  {
    device_destroy(MyClass, (My_dev + n));
  }
  class_destroy(MyClass);

  unregister_chrdev_region(My_dev, PORTNUMBER);

  // cb_free(&perso.buf_rdwr);

  printk(KERN_WARNING "MyMod : GOODBYE CRUEL WORLD !\n");
}

module_init(mod_init);
module_exit(mod_exit);

// make -C /usr/src/linux-source-4.15.18 M=`pwd` modules
// sudo insmod ./MyModule.ko
// sudo rmmod MyModule
// lsmod | grep MyModule
/// var/log/
// dmesg -w | grep MyMod
