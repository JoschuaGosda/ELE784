#include "MyModule.h"

#define PORTNUMBER 2
#define BUFFER_CAPACITY 3
#define BUFFER_ELEMENTSIZE 1 //element size is 1 byte

MODULE_AUTHOR("TheBestTeam");
MODULE_LICENSE("Dual BSD/GPL");

int MyModule_X = 5;

module_param(MyModule_X, int, S_IRUGO);

EXPORT_SYMBOL_GPL(MyModule_X);

dev_t My_dev;
struct class *MyClass;

struct file_operations MyModule_fops = {.owner = THIS_MODULE,
                                        .read = MyModule_read,
                                        .write = MyModule_write,
                                        .open = MyModule_open,
                                        .release = MyModule_release};

struct pData {
    int8_t owner;  // takes the owners id
    circular_buffer buf_rdwr;
    uint8_t numPort;
    bool fREAD;
    bool fWRITE;
    struct semaphore sem;
    struct spinlock splock;
    struct mutex mutex;
    wait_queue_head_t RdQ, WrQ;
} pData;

struct pData pdata[PORTNUMBER];

bool portOpen[PORTNUMBER];

struct cdev My_cdev;

static int __init mod_init(void) {
    int n;

    // int alloc_chrdev_region(dev_t* dev, unsigned int firstminor, unsigned int
    // count, char *name)
    alloc_chrdev_region(&My_dev, 0, PORTNUMBER, "MonPremierPilote");

    MyClass = class_create(THIS_MODULE, "MyModule");
    for (n = 0; n < PORTNUMBER; n++) {
        // create un noeud dans /dev
        device_create(MyClass, NULL, (My_dev + n), NULL, "MyModuleNode%d", n);
    }

    cdev_init(&My_cdev, &MyModule_fops);  // add pointer in struct cdev to
                                          // struct file operations
    cdev_add(&My_cdev, My_dev,
             PORTNUMBER);  // add pilote to the kernel - struct cdev, dev number
                           // (/dev/My_dev1), int count

    for (n = 0; n < PORTNUMBER; n++) {
        cb_init(&pdata[n].buf_rdwr, BUFFER_CAPACITY, BUFFER_ELEMENTSIZE);
        pdata[n].fREAD = false;
        pdata[n].fWRITE = false;
        pdata[n].owner = -1;
        sema_init(&pdata[n].sem, 1);
        spin_lock_init(&pdata[n].splock);
        mutex_init(&pdata[n].mutex);
        init_waitqueue_head(&pdata[n].RdQ);
        init_waitqueue_head(&pdata[n].WrQ);
    }

    printk(KERN_WARNING "MyMod : Kernel Module initilized with number  %u\n",
           MyModule_X);

    return 0;
}

static int MyModule_open(struct inode *inode, struct file *filp) {
    uint32_t numPort;
    struct pData *pdata_p;

    printk(KERN_WARNING "MyMod: OPEN");

    // get the number of the device driver and connect to the right private data
    // structure
    numPort = MINOR(inode->i_rdev);
    pdata_p = &pdata[numPort];
    filp->private_data = pdata_p;

    spin_lock(&pdata_p->splock);
    if (pdata_p->owner < 0) {
        pdata_p->owner = current_uid().val;
        printk(KERN_WARNING "MyMod: New owner is set to %u", pdata_p->owner);
    } else if (0/*pdata_p->owner != current_uid().val*/)  // TODO 2e time the
                                                            // same owner open
                                                            // its the #3026 ??
    {
        printk(KERN_WARNING "MyMod: ERR current user is :%u and new is : %u",
               pdata_p->owner, current_uid().val);
        spin_unlock(&pdata_p->splock);
        return -EACCES;
    } else {
        printk(KERN_WARNING "MyMod: current user is the owner");
    }

    switch (filp->f_flags & O_ACCMODE) {
        case O_RDONLY:
            printk(KERN_WARNING "MyMod: O_RDONLY access");
            // statements
            if (pdata_p->fREAD) {
                spin_unlock(&pdata_p->splock);
                return -EACCES;
            } else {
                pdata_p->fREAD = true;
            }
            break;

        case O_WRONLY:
            printk(KERN_WARNING "MyMod: O_WRONLY access");
            // statements
            if (pdata_p->fWRITE) {
                spin_unlock(&pdata_p->splock);
                return -EACCES;
            } else {
                pdata_p->fWRITE = true;
            }
            break;

        case O_RDWR:
            printk(KERN_WARNING "MyMod: O_RDWR access");
            if (pdata_p->fWRITE || pdata_p->fREAD) {
                spin_unlock(&pdata_p->splock);
                return -EACCES;
            } else {
                pdata_p->fWRITE = true;
                pdata_p->fREAD = true;
            }
            break;

        default:
            printk(KERN_WARNING
                   "MyMod: file is tried to be opened differently than "
                   "implemented");
            // default statements
    }
    spin_unlock(&pdata_p->splock);

    // TODO: Port Série doit être placé en mode Réception
    printk(KERN_WARNING "MyMod: OPEN end\n");
    return 0;
}

static ssize_t MyModule_read(struct file *filp, char *buff, size_t len,
                             loff_t *off) {
    // send from kernelSpace to userSpace
    char BufR[8];
    int i, retval;
    size_t count;
    unsigned long left;
    struct pData *pdata_p = filp->private_data;

    printk(KERN_WARNING "MyMod: READ ask:%lu\n",len);

    spin_lock(&pdata_p->splock);
    // TODO: protection d'access
    if (!((filp->f_flags & O_ACCMODE) == O_RDONLY ||
          (filp->f_flags & O_ACCMODE) == O_RDWR)) {
        spin_unlock(&pdata_p->splock);
        return -EACCES;
    }
    spin_unlock(&pdata_p->splock);
   
   
    // mutex_lock(&pdata_p->mutex);// todo ??should be semaphore
    if (down_interruptible(&pdata_p->sem)) {
        return -ERESTARTSYS;
    }
    if (pdata_p->buf_rdwr.count == 0) {  // no data in the cbuffer
        up(&pdata_p->sem);

        if (filp->f_flags & O_NONBLOCK) {
            printk(KERN_WARNING
                   "MyMod: READ : NO DATA AVAILABLE AND NON BLOCK\n");
            return -EAGAIN;
        } else {
            printk(KERN_WARNING "MyMod: READ : NO DATA BUT WILL WAIT\n");
            if (wait_event_interruptible(
                pdata_p->RdQ, pdata_p->buf_rdwr.count > 0)) { 
                      // waiting for someting in the buffer
                return -ERESTARTSYS;    
            }
        }

	    printk(KERN_WARNING "MyMod: READ : WAIT IS OVER\n");
	
        if (down_interruptible(&pdata_p->sem)) {
            return -ERESTARTSYS;
        }
    }

    // data in the buffer , on ajuste le compte , plus petit entre ask et
    // available
    count = (len <= cb_count(&pdata_p->buf_rdwr))
                ? len
                : cb_count(&pdata_p->buf_rdwr);

    // retire content of ring buffer to local buffer une a la fois

    printk(KERN_WARNING "MyMod: count value is %u", (int) count);


    for (i = 0; i < count; i++) {
        cb_pop(&pdata_p->buf_rdwr, &(BufR[i]));
    }
    printk(KERN_WARNING "MyMod: Read after pop buff_count : %lu",cb_count(&pdata_p->buf_rdwr));
    // mutex_unlock(&pdata_p->mutex);
    
    wake_up_interruptible(&pdata_p->WrQ);
    up(&pdata_p->sem);
    // printk(KERN_WARNING "MyMod: copied from ring buffer to local buffer \n");
    //printk(KERN_WARNING "MyMod: popped data elements from global buffer");

    // retval is number of elements that are taken from global
    retval = count;
    left = copy_to_user(buff, &BufR, count);

    if (left == 0) {
        printk(KERN_WARNING "MyMod: READ SUCCES");
    } else {
        printk(KERN_WARNING
               "MyMod:  %lu bytes have not been copied from kernel space to user space\n",
                left);
        printk("This data is lost\n");
        // returning how many data  was really send
        retval = count - left;
    }
   
    return retval;
}

static ssize_t MyModule_write(struct file *filp, const char *buff, size_t len,
                              loff_t *off) {
    // copy from userSpace to kernelSpace
    char BufW[8];
    int retval, i;
    size_t count;
    unsigned long left;
    struct pData *pdata_p = filp->private_data;

    spin_lock(&pdata_p->splock);
    // protection d'access
    if (!((filp->f_flags & O_ACCMODE) == O_WRONLY ||
          (filp->f_flags & O_ACCMODE) == O_RDWR)) {
        spin_unlock(&pdata_p->splock);
        return -EACCES;
    }
    spin_unlock(&pdata_p->splock);

    printk(KERN_WARNING "MyMod: WRITE\n");
    if (down_interruptible(&pdata_p->sem)) { // returns 0 if sucess
        return -ERESTARTSYS;
    }

    if (pdata_p->buf_rdwr.count == BUFFER_CAPACITY) {  // buffer is full(if buffsize = 10 then 0 to 9
        up(&pdata_p->sem);

        if (filp->f_flags & O_NONBLOCK) {
            printk(KERN_WARNING "MyMod: WRITE : BUFFER FULL AND NON BLOCK\n");
            return -EAGAIN;
        } else {
            printk(KERN_WARNING "MyMod: WRITE : NO SPACE BUT WILL WAIT\n");
            if (wait_event_interruptible(
                pdata_p->WrQ,
                pdata_p->buf_rdwr.count < BUFFER_CAPACITY)) {
                    return -ERESTARTSYS;
            }
        }
		printk(KERN_WARNING "MyMod: WRITE : WAIT IS OVER\n");
        
        if (down_interruptible(&pdata_p->sem)) {
            return -ERESTARTSYS;
        }
    }

    // place is buffer, ajust count. wich one is minimal ASK or AVAILABLE ?
    count = (len <= BUFFER_CAPACITY - pdata_p->buf_rdwr.count)
                ? len
                : (BUFFER_CAPACITY - pdata_p->buf_rdwr.count);

    printk(KERN_WARNING "MyMod: WRITE count : %lu\n",count);

    retval = count;

    if (count > 0){
        left = copy_from_user(&BufW, buff, count);
    
        if (left == 0) {
            // all data have been send
            printk(KERN_WARNING "MyMod: WRITE SUCCES\n");
            //mutex_lock(&pdata_p->mutex);
            //  copy content of local buffer to ring buffer une a la fois

            for (i = 0; i < count; i++) {
                cb_push(&pdata_p->buf_rdwr, &(BufW[i]));
            }
            up(&pdata_p->sem);
            wake_up_interruptible(&pdata->RdQ);
            
            printk(KERN_WARNING "MyMod: WRITE buff count : %lu",
                cb_count(&pdata_p->buf_rdwr));
            //mutex_unlock(&pdata_p->mutex);
        } else {
            // some data was not copy from user space to kernel space, is not written to global buffer at all -> return 0
            printk(
                KERN_WARNING
                "MyMod: WRITE problem when copying from user space to kernel space");
            retval = count - left;
        }
    }
    return retval;
}

static int MyModule_release(struct inode *inode, struct file *filp) {
    // uint32_t numPort;
    struct pData *pdata_p = filp->private_data;

    //printk(KERN_WARNING "MyMod: RELEASE");

    spin_lock(&pdata_p->splock);
    switch (filp->f_flags & O_ACCMODE) {
        case O_RDONLY:
            //printk(KERN_WARNING "MyMod: O_RDONLY acces reset");
            // statements
            pdata_p->fREAD = false;
            break;

        case O_WRONLY:
            //printk(KERN_WARNING "MyMod: O_WRONLY access reset");
            // statements
            pdata_p->fWRITE = false;
            break;

        case O_RDWR:
            //printk(KERN_WARNING "MyMod: O_RDWR access reset");
            pdata_p->fWRITE = false;
            pdata_p->fREAD = false;
            break;

        default:
            // default statements
            printk(KERN_WARNING
                   "MyMod: file is tried to be opened differently than "
                   "implemented");
    }

    printk(KERN_WARNING "MyMod:WRITER:%u READER:%u\n ", pdata_p->fWRITE, pdata_p->fREAD);

    // reset owner if no current read and write
    if (!(pdata_p->fREAD | pdata_p->fWRITE)) {
        pdata_p->owner = -1;
        //printk(KERN_WARNING "MyMod: owner is set to %d", pdata_p->owner);
    }
    spin_unlock(&pdata_p->splock);

    // desattach the private data
    filp->private_data = NULL;

    printk(KERN_WARNING "MyMod: RELEASE end\n");

    // TODO: Et si le mode d’ouverture était O_RDONL Y ou O_RDWR, la Réception
    // du Port Série doit être interrompue afin d’arrêter de recevoir des
    // données

    return 0;
}

static void __exit mod_exit(void) {
    int n;

    cdev_del(&My_cdev);
    unregister_chrdev_region(My_dev, PORTNUMBER);

    for (n = 0; n < PORTNUMBER; n++) {
        device_destroy(MyClass, (My_dev + n));
    }
    class_destroy(MyClass);

     for (n = 0; n < PORTNUMBER; n++) {
        cb_free(&pdata[n].buf_rdwr);
        /*
        pdata[n].fREAD = false;
        pdata[n].fWRITE = false;
        pdata[n].owner = -1;
        sem_destroy(&pdata[n].sem);
        spin_lock_destroy(&pdata[n].splock);
        mutex_destroy(&pdata[n].mutex);
        destroy_waitqueue_head(&pdata[n].RdQ);
        destroy_waitqueue_head(&pdata[n].WrQ);
        */
    }

    printk(KERN_WARNING "MyMod :------CLOSING---- !\n");
}

module_init(mod_init);
module_exit(mod_exit);

// make -C /usr/src/linux-source-4.15.18 M=`pwd` modules
// sudo insmod ./MyModule.ko
// sudo rmmod MyModule
// lsmod | grep MyModule
/// var/log/
// dmesg -w | grep MyMod
