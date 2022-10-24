#include "MyModule.h"

DEFINE_SPINLOCK(lock_access);
#define PORTNUMBER 2
 
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

uint16_t owner[PORTNUMBER]; // takes the owners id
bool file_read[PORTNUMBER], file_write[PORTNUMBER];
//uint16_t owner;
//uint8_t idMinor
//bool file_read, file_write;

struct cdev My_cdev;

static ssize_t MyModule_read(struct file *filp, char *buff , size_t len, loff_t *off) {


  printk(KERN_WARNING"MyMod: READ\n");

  return 0;
 
}

static ssize_t MyModule_write(struct file *filp, const char *buff , size_t len, loff_t *off) {

  printk(KERN_WARNING"MyMod: WRITE\n");

  return 0;

}

static int MyModule_open(struct inode *inode, struct file *filp) {
		//numPort = inode->i_rdev;
    uint32_t numPort; 
  printk(KERN_WARNING"MyMod: OPEN\n");
  // lock to protect critical code section
  spin_lock(&lock_access);
  // get the port the owner want to joint	
  numPort = MINOR(inode->i_rdev);
  
printk(KERN_WARNING"MyMod open: ir_dev : %u\n",numPort);
  // get the owner if nothing is accessed yet
  if (!file_write[numPort] && !file_read[numPort]){
    owner[numPort] = current_uid().val;
  }

  // check if the user that tries to open is the owner
  if (current_uid().val == owner[numPort]){
    // check the access mode, update file_read, file_write flags or return error if applicable
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY && !file_write[numPort]) {
        file_write[numPort] = true;
        spin_unlock(&lock_access);
    } else if ((filp->f_flags & O_ACCMODE) == O_RDONLY && !file_read[numPort]) {
        file_read[numPort] = true;
        spin_unlock(&lock_access);
    } else if ((filp->f_flags & O_ACCMODE) == O_RDWR && (!file_write[numPort] && !file_read[numPort])) {
        file_write[numPort] = true;
        file_read[numPort] = true;
        spin_unlock(&lock_access);
    } else {
      spin_unlock(&lock_access);
      return -ENOTTY;
    }
  } else { // current user it not the owner
    spin_unlock(&lock_access);
    return -ENOTTY;
  }

  // TODO: Port Série doit être placé en mode Réception


  printk(KERN_WARNING"MyMod (end Open): file_read[%lu]: %d file_write: %d owner: %d \n",numPort, file_read[numPort], file_write[numPort], owner[numPort]);

  return 0;

}

static int MyModule_release(struct inode *inode, struct file *filp) {
  printk(KERN_WARNING"MyMod: RELEASE\n");

    uint32_t numPort; 

  // get the port the owner want to joint	
  numPort = MINOR(inode->i_rdev);

  spin_lock(&lock_access);
  if ((filp->f_flags & O_ACCMODE) == O_WRONLY && file_write[numPort]) {
        file_write[numPort] = false;
        spin_unlock(&lock_access);
    } else if ((filp->f_flags & O_ACCMODE) == O_RDONLY && file_read[numPort]) {
        file_read[numPort] = false;
        spin_unlock(&lock_access);
    } else if ((filp->f_flags & O_ACCMODE) == O_RDWR && (file_write[numPort] && file_read[numPort])) {
        file_write[numPort] = false;
        file_read[numPort] = false;
        spin_unlock(&lock_access);
    } else {
      spin_unlock(&lock_access);
      return -ENOTTY;
    }

  // TODO: Et si le mode d’ouverture était O_RDONL Y ou O_RDWR, la Réception du Port Série doit être interrompue afin d’arrêter de recevoir des données


  printk(KERN_WARNING"MyMod: (end Release) file_read[%lu]: %d file_write: %d owner: %d \n",numPort, file_read, file_write, owner);

  return 0;

}

static int __init mod_init (void) {
int n;

  alloc_chrdev_region (&My_dev,  0,  PORTNUMBER, "MonPremierPilote");

  MyClass = class_create(THIS_MODULE, "MyModule");
  for (n = 0; n < PORTNUMBER; n++) {
  	device_create(MyClass, NULL, (My_dev + n), NULL, "MyModuleNode%d", n);
  }
  printk(KERN_WARNING"MyMod init : My_dev0 : %u , My_dev1 : %u ",My_dev,My_dev+1);
  uint16_t a = MAJOR(My_dev);
  uint16_t b = MINOR(My_dev);
  uint16_t aa = MAJOR(My_dev+1);
  uint16_t bb = MINOR(My_dev+1);

  printk(KERN_WARNING"MyMod DEV0 major :%u, minor: %u DEV1 maj :%u minor:%u ", a,b,aa,bb);
  cdev_init(&My_cdev, &MyModule_fops);
  cdev_add(&My_cdev, My_dev, PORTNUMBER); 

  // set file_read and file_write flags to FALSE 
  //todo loop with PORTNUMBER

  file_read[0] = false;
  file_write[0] = false;
  file_read[1] = false;
  file_write[1] = false;

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

  printk(KERN_WARNING"MyMod : Goodbye cruel World !\n");

}

module_init(mod_init);
module_exit(mod_exit);

// make -C /usr/src/linux-source-4.15.18 M=`pwd` modules
// sudo insmod ./MyModule.ko
// sudo rmmod MyModule
// lsmod | grep MyModule
///var/log/ dmesg | grep MyMod





