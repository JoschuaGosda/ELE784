#include "MyModule.h"

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

struct cdev My_cdev;

static ssize_t MyModule_read(struct file *file, char *buff , size_t len, loff_t *off) {

  printk(KERN_WARNING"Bonjour les amis, READ\n");

  return 0;

}

static ssize_t MyModule_write(struct file *file, const char *buff , size_t len, loff_t *off) {

  printk(KERN_WARNING"Bonjour les amis, WRITE\n");

  return 0;

}

static int MyModule_open(struct inode *inode, struct file *filp) {

  printk(KERN_WARNING"Bonjour les amis, OPEN\n");

  return 0;

}

static int MyModule_release(struct inode *inode, struct file *filp) {

  printk(KERN_WARNING"Bonjour les amis, RELEASE\n");

  return 0;

}

static int __init mod_init (void) {
int n;

  alloc_chrdev_region (&My_dev,  0,  4, "MonPremierPilote");

  MyClass = class_create(THIS_MODULE, "MyModule");
  for (n = 0; n < 4; n++) {
  	device_create(MyClass, NULL, (My_dev + n), NULL, "MyModuleNode%d", n);
  }

  cdev_init(&My_cdev, &MyModule_fops);
  cdev_add(&My_cdev, My_dev, 4); 

  printk(KERN_WARNING"MyMod : Hello World ! MyModule_X = %u\n", MyModule_X);

  return 0;
}

static void __exit mod_exit (void) {
int n;

  cdev_del(&My_cdev);

  for (n = 0; n < 4; n++) {
  	device_destroy(MyClass, (My_dev + n));
  }
  class_destroy(MyClass);

  unregister_chrdev_region(My_dev, 4);

  printk(KERN_WARNING"MyMod : Goodbye cruel World !\n");

}

module_init(mod_init);
module_exit(mod_exit);

// make -C /usr/src/linux-headers-4.15.18 M=`pwd` modules
// sudo insmod ./MyModule.ko
// sudo rmmod MyModule
// lsmod | grep MyModule






