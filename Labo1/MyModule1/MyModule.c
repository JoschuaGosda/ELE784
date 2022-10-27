#include "MyModule.h"

DEFINE_SPINLOCK(lock_access);
#define PORTNUMBER 2
#define BUFFERCAPACITY 128


MODULE_AUTHOR("Bruno");
MODULE_LICENSE("Dual BSD/GPL");

int MyModule_X = 5;

module_param(MyModule_X, int, S_IRUGO);

EXPORT_SYMBOL_GPL(MyModule_X);

dev_t My_dev;
struct class *MyClass;

struct file_operations MyModule_fops = {
	.owner		=	THIS_MODULE,
	.read		=	MyModule_read,
	.write		=	MyModule_write,
	.open		=	MyModule_open,
	.release	=	MyModule_release
};

typedef struct circular_buffer
{
  void *buffer;     // data buffer
  void *buffer_end; // end of data buffer
  size_t capacity;  // maximum number of items in the buffer
  size_t count;     // number of items in the buffer
  size_t sz;        // size of each item in the buffer
  void *head;       // pointer to head
  void *tail;       // pointer to tail
} circular_buffer;

void cb_init(circular_buffer *cb, size_t capacity, size_t sz)
{
  cb->buffer = kmalloc(capacity * sz, GFP_KERNEL); // GFP_KERNEL - Allocate normal kernel ram. May sleep.
  if (cb->buffer == NULL)
    // handle error
    cb->buffer_end = (char *)cb->buffer + capacity * sz;
  cb->capacity = capacity;
  cb->count = 0;
  cb->sz = sz;
  cb->head = cb->buffer;
  cb->tail = cb->buffer;
}

void cb_free(circular_buffer *cb)
{
  kfree(cb->buffer);
  // clear out other fields too, just to be safe
}

void cb_push_back(circular_buffer *cb, const void *item)
{
  if (cb->count == cb->capacity)
  {
    // handle error
    printk(KERN_WARNING"MyMod: Buffer is full \n");
  }
  memcpy(cb->head, item, cb->sz);
  cb->head = (char *)cb->head + cb->sz;
  if (cb->head == cb->buffer_end)
    cb->head = cb->buffer;
  cb->count++;
}

void cb_pop_front(circular_buffer *cb, void *item)
{
  if (cb->count == 0)
  {
    // handle error
    printk(KERN_WARNING"MyMod: Buffer is empty \n");
  }
  memcpy(item, cb->tail, cb->sz);
  cb->tail = (char *)cb->tail + cb->sz;
  if (cb->tail == cb->buffer_end)
    cb->tail = cb->buffer;
  cb->count--;
}

typedef struct Perso {
  uint16_t owner[PORTNUMBER]; // takes the owners id
  bool file_read[PORTNUMBER], file_write[PORTNUMBER];
  bool blocking[PORTNUMBER];
  circular_buffer buf_rdwr;
} Perso;

Perso perso;

struct cdev My_cdev;

static ssize_t MyModule_read(struct file *filp, char *buff , size_t len, loff_t *off) {
   // send from kernelSpace to userSpace
  uint8_t BufR[8];
  int i;

  printk(KERN_WARNING"MyMod: READ\n");

  // retire content of ring buffer to local buffer une a la fois
  for(i = 0; i < len; i++){
    cb_pop_front(&perso.buf_rdwr, &BufR[i]);
  }

  printk(KERN_WARNING"MyMod: copied from ring buffer to local buffer \n");

  copy_to_user(buff, &BufR, len);

  printk(KERN_WARNING"MyMod: copied local buffer to user \n");


  return 0;
 
}

static ssize_t MyModule_write(struct file *filp, const char *buff , size_t len, loff_t *off) {

  // copy from userSpace to kernelSpace
  uint8_t BufW[8];
  int i;

  printk(KERN_WARNING"MyMod: WRITE\n");
  copy_from_user(&BufW, buff, len);

  for (i=0; i<8; i++){
    printk(KERN_WARNING"MyMod: Local Buffer %d\n", BufW[i]);
  }

  // copy content of local buffer to ring buffer une a la fois
  for(i = 0; i < len; i++){
    cb_push_back(&perso.buf_rdwr, &BufW[i]);
  }

  

  return 0;

}

static int MyModule_open(struct inode *inode, struct file *filp) {

  uint32_t numPort; 

  printk(KERN_WARNING"MyMod: OPEN\n");
  // lock to protect critical code section
  spin_lock(&lock_access);
  // get the port the owner want to joint	
  numPort = MINOR(inode->i_rdev);
  
  printk(KERN_WARNING"MyMod open: ir_dev : %u\n",numPort);
  // get the owner if nothing is accessed yet
  if (!perso.file_write[numPort] && !perso.file_read[numPort]){
    perso.owner[numPort] = current_uid().val;
  }

  // check if the user that tries to open is the owner
  if (current_uid().val == perso.owner[numPort])
  {
    // check the access mode, update file_read, file_write flags or return error if applicable
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY && !perso.file_write[numPort])
    {
      perso.file_write[numPort] = true;
      spin_unlock(&lock_access);
    }
    else if ((filp->f_flags & O_ACCMODE) == O_RDONLY && !perso.file_read[numPort])
    {
      perso.file_read[numPort] = true;
      spin_unlock(&lock_access);
    }
    else if ((filp->f_flags & O_ACCMODE) == O_RDWR && (!perso.file_write[numPort] && !perso.file_read[numPort]))
    {
      perso.file_write[numPort] = true;
      perso.file_read[numPort] = true;
      spin_unlock(&lock_access);
    }
    else
    {
      spin_unlock(&lock_access);
      return -ENOTTY;
    }
  }
  else
  { // current user it not the owner
    spin_unlock(&lock_access);
    return -ENOTTY;
  }

  // TODO: Port Série doit être placé en mode Réception

  //printk(KERN_WARNING "MyMod (end Open): file_read[%d]: %d file_write: %d owner: %d \n", numPort, perso.file_read[numPort], perso.file_write[numPort], perso.owner[numPort]);

  return 0;

}

static int MyModule_release(struct inode *inode, struct file *filp) {
  uint32_t numPort;

  printk(KERN_WARNING"MyMod: RELEASE\n");

  // get the port the owner want to joint	
  numPort = MINOR(inode->i_rdev);

  spin_lock(&lock_access);
  if ((filp->f_flags & O_ACCMODE) == O_WRONLY && perso.file_write[numPort])
  {
    perso.file_write[numPort] = false;
    spin_unlock(&lock_access);
  }
    else if ((filp->f_flags & O_ACCMODE) == O_RDONLY && perso.file_read[numPort])
    {
      perso.file_read[numPort] = false;
      spin_unlock(&lock_access);
    }
    else if ((filp->f_flags & O_ACCMODE) == O_RDWR && (perso.file_write[numPort] && perso.file_read[numPort]))
    {
      perso.file_write[numPort] = false;
      perso.file_read[numPort] = false;
      spin_unlock(&lock_access);
    }
    else
    {
      spin_unlock(&lock_access);
      return -ENOTTY;
    }

  // TODO: Et si le mode d’ouverture était O_RDONL Y ou O_RDWR, la Réception du Port Série doit être interrompue afin d’arrêter de recevoir des données

    //printk(KERN_WARNING "MyMod: (end Release) file_read[%d]: %d file_write: %d owner: %d \n", numPort, perso.file_read, perso.file_write, perso.owner);

    return 0;

}

static int __init mod_init (void) {
int n, i;

  alloc_chrdev_region (&My_dev,  0,  PORTNUMBER, "MonPremierPilote");

  MyClass = class_create(THIS_MODULE, "MyModule");
  for (n = 0; n < PORTNUMBER; n++) {
  	device_create(MyClass, NULL, (My_dev + n), NULL, "MyModuleNode%d", n);
  }
  /*printk(KERN_WARNING"MyMod init : My_dev0 : %u , My_dev1 : %u ",My_dev,My_dev+1);
  uint16_t a = MAJOR(My_dev);
  uint16_t b = MINOR(My_dev);
  uint16_t aa = MAJOR(My_dev+1);
  uint16_t bb = MINOR(My_dev+1);
  printk(KERN_WARNING"MyMod DEV0 major :%u, minor: %u DEV1 maj :%u minor:%u ", a,b,aa,bb);*/

  cdev_init(&My_cdev, &MyModule_fops);
  cdev_add(&My_cdev, My_dev, PORTNUMBER); 

  // set file_read and file_write flags to FALSE 
  //todo loop with PORTNUMBER

  for (i=0; i < PORTNUMBER; i++){
    perso.file_read[i] = false;
    perso.file_write[i] = false;
    perso.blocking[i] = false;
  }

  cb_init(&perso.buf_rdwr, 128, 8);

  printk(KERN_WARNING"MyMod : Hello World ! MyModule_X = %u\n", MyModule_X);

  return 0;
}

static void __exit mod_exit (void) {
int n;

  cdev_del(&My_cdev);

  for (n = 0; n < PORTNUMBER; n++) {
  	device_destroy(MyClass, (My_dev + n));
  }
  class_destroy(MyClass);

  unregister_chrdev_region(My_dev, PORTNUMBER);

  cb_free(&perso.buf_rdwr);

  printk(KERN_WARNING"MyMod : Goodbye cruel World !\n");

}

module_init(mod_init);
module_exit(mod_exit);

// make -C /usr/src/linux-source-4.15.18 M=`pwd` modules
// sudo insmod ./MyModule.ko
// sudo rmmod MyModule
// lsmod | grep MyModule
///var/log/ 
// dmesg -w | grep MyMod





