#include "MyModule.h"

DEFINE_SPINLOCK(lock_access);
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
	.owner		=	THIS_MODULE,
	.read		  =	MyModule_read,
	.write		=	MyModule_write,
	.open		  =	MyModule_open,
	.release	=	MyModule_release
};

struct pData {
  int8_t owner; // takes the owners id
  circular_buffer buf_rdwr;
  uint8_t numPort;
  bool READ;
  bool WRITE;
  enum status
  {
    NONE = 0,
    RD = O_RDONLY,
    WR = O_WRONLY,
    RDWR = O_RDWR
  } mode;
} pData;

struct pData pdata[PORTNUMBER];

bool portOpen[PORTNUMBER];

struct cdev My_cdev;


static int __init mod_init (void) {

  int n;

  alloc_chrdev_region (&My_dev,  0,  PORTNUMBER, "MonPremierPilote");

  MyClass = class_create(THIS_MODULE, "MyModule");
  for (n = 0; n < PORTNUMBER; n++) {
  	device_create(MyClass, NULL, (My_dev + n), NULL, "MyModuleNode%d", n);
  }

  cdev_init(&My_cdev, &MyModule_fops);
  cdev_add(&My_cdev, My_dev, PORTNUMBER); 

  for (n = 0; n < PORTNUMBER; n++){
    cb_init(&pdata[n].buf_rdwr, BUFFER_CAPACITY, BUFFER_ELEMENTSIZE);
    pdata[n].mode = NONE;
    pdata[n].READ = false;
    pdata[n].WRITE = false;
    pdata[n].owner = -1;
  }

  printk(KERN_WARNING"MyMod : Hello World ! MyModule_X = %u\n", MyModule_X);

  return 0;
}

static ssize_t MyModule_read(struct file *filp, char *buff , size_t len, loff_t *off) {
  
  // send from kernelSpace to userSpace
  uint8_t BufR[8];
  int i;
  struct pData *pdata_p = filp->private_data;

  // TODO: protection d'access
  if (!((filp->f_flags & O_ACCMODE) == O_RDONLY || (filp->f_flags & O_ACCMODE) == O_RDWR)) {
    return -EACCES;
  }

    printk(KERN_WARNING "MyMod: READ\n");

  // retire content of ring buffer to local buffer une a la fois
  for(i = 0; i < len; i++){
    cb_pop(&pdata_p->buf_rdwr, &BufR[i]);
  }

  printk(KERN_WARNING"MyMod: copied from ring buffer to local buffer \n");

  if(copy_to_user(buff, &BufR, len) == 0){
    printk(KERN_WARNING "MyMod: All bytes have been sent to user");
  } else {
    printk(KERN_WARNING "MyMod: Some bytes have not been sent to user");
    // TODO: implement the mechanism depending on blocking / non-blocking
  }
  return 0;
}

static ssize_t MyModule_write(struct file *filp, const char *buff , size_t len, loff_t *off) {

  // copy from userSpace to kernelSpace
  uint8_t BufW[8];
  int i;



  // TODO: protection d'access
  if (!((filp->f_flags & O_ACCMODE) == O_WRONLY || (filp->f_flags & O_ACCMODE) == O_RDWR))
  {
    return -EACCES;
  }

  struct pData *pdata_p= filp->private_data;

  printk(KERN_WARNING"MyMod: WRITE\n");
  copy_from_user(&BufW, buff, len);

  for (i=0; i<8; i++){
    printk(KERN_WARNING"MyMod: Local Buffer %d\n", BufW[i]);
  }

  // copy content of local buffer to ring buffer une a la fois
  for(i = 0; i < len; i++){
    cb_push(&pdata_p->buf_rdwr, &BufW[i]);
  }
  return 0;
}

static int MyModule_open(struct inode *inode, struct file *filp) {

  uint32_t numPort;
  struct pData *pdata_p;

  printk(KERN_WARNING "MyMod: OPEN");

  // get the port the owner want to joint	
  numPort = MINOR(inode->i_rdev);
  pdata_p = &pdata[numPort];

  //printk(KERN_WARNING "MyMod: pdata_p->owner %d", pdata_p->owner);
  //printk(KERN_WARNING "MyMod: currentuid  %u", current_uid().val);

  if (pdata_p->owner < 0) {
    pdata_p->owner = current_uid().val;
    printk(KERN_WARNING "MyMod: user value changed to %u", pdata_p->owner);
  }
  else if (pdata_p->owner != current_uid().val)
  {
    printk(KERN_WARNING "MyMod: users are not the same");
    return -EACCES;
  }

  // avoid open file 2 time in the same mode
  if ((pdata_p->mode == (filp->f_flags & O_ACCMODE)) || pdata_p->mode == O_RDWR)
  {
    printk(KERN_WARNING "MyMod: E MODE");
    return -EACCES;
  }
  // already a reader or a writer, so you become readwriter
  else if (!(pdata_p->mode == NONE))
  {
    pdata_p->mode = RDWR;
  }
  // file was not opened before
  else if ((filp->f_flags & O_ACCMODE) == O_RDONLY) {
    pdata_p->mode = RD;
  } else {
    pdata_p->mode = WR;
  }
  

  filp->private_data = pdata_p;
  // TODO: Port Série doit être placé en mode Réception
  printk(KERN_WARNING "MyMod: OPEN end\n");
  return 0;
}

static int MyModule_release(struct inode *inode, struct file *filp) {


  printk(KERN_WARNING"MyMod: RELEASE\n");

  // TODO: Et si le mode d’ouverture était O_RDONL Y ou O_RDWR, la Réception du Port Série doit être interrompue afin d’arrêter de recevoir des données

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

  //cb_free(&perso.buf_rdwr);

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





