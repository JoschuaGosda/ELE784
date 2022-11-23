#define PORTNUMBER 2
#define BUFFER_CAPACITY 10
#define BUFFER_ELEMENTSIZE 1 //element size is 1 byte

#include "MyModule.h"


MODULE_AUTHOR("TheBestTeam");
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
    .release = MyModule_release,
    .unlocked_ioctl = MyModule_ioctl
};

uint8_t IER;
uint8_t LCR;

irqreturn_t isrSerialPort(int num_irq, void *pdata){

        
    //check wich port : todo ?? does the pdata is already the right pdata
   // uint8_t portID =  pdata->numPort

    //get inb register
    //LSR
    uint8_t tpLSR;
    uint8_t tpdata;
    struct pData *pdata_p =  pdata;


    tpLSR = inb(pdata_p->base_addr + LSR_REG); // line status register
     
    printk(KERN_ALERT"MyMod : interrupt check");

    
    //RX  RBR - read
    if(tpLSR & LSR_DR) {
        printk(KERN_ALERT"MyMod : int. RX ");
        LCR &= ~DLAB_REG;
        outb(LCR, pdata_p->base_addr + LCR_REG);     
        tpdata = inb(pdata_p->base_addr + RBR_REG); 
        
        cb_push(pdata_p->buf_rd,&tpdata);
        
        wake_up_interruptible(&pdata_p->WrQ);
        printk(KERN_ALERT"MyMod : buffer count is %lu", pdata_p->buf_wr->count);
    } 


    // TX THRE data to transmit
    if(tpLSR & LSR_THRE) {
        printk(KERN_ALERT"MyMod : int. TX ");
       
        cb_pop(pdata_p->buf_wr,&tpdata);
        
        LCR &= ~DLAB_REG;
        outb(LCR, pdata_p->base_addr + LCR_REG); 
        outb(tpdata,pdata_p->base_addr + THR_REG);
        
        printk(KERN_ALERT"MyMod : buffer count is %lu", pdata_p->buf_wr->count);
        
    }

    // TX TEMT  tpLSR & LSR_TEMT &&
    if( pdata_p->buf_wr->count == 0) {
        //disable TX
        IER &= ~ETBEI;
        outb(IER, pdata_p->base_addr + IER_REG);
        printk(KERN_ALERT"MyMod : buffer_wr emtpy");
        wake_up_interruptible(&pdata_p->RdQ);
    }

    return IRQ_HANDLED;
}

struct pData pdata[PORTNUMBER];

bool portOpen[PORTNUMBER];

struct cdev My_cdev;

static int __init mod_init(void) {
    int n;
    unsigned long number = 8; 
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


    for (n = 0; n < PORTNUMBER; n++) {

	pdata[n].buf_rd = (circular_buffer *) kmalloc(sizeof(circular_buffer), GFP_KERNEL);
	pdata[n].buf_wr = (circular_buffer *) kmalloc(sizeof(circular_buffer), GFP_KERNEL);

        cb_init(pdata[n].buf_rd, BUFFER_CAPACITY, BUFFER_ELEMENTSIZE);
	    cb_init(pdata[n].buf_wr, BUFFER_CAPACITY, BUFFER_ELEMENTSIZE);

        pdata[n].fREAD = false;
        pdata[n].fWRITE = false;
        pdata[n].owner = -1;

        sema_init(&pdata[n].sem, 1);
        spin_lock_init(&pdata[n].splock);
        mutex_init(&pdata[n].mutex);
        init_waitqueue_head(&pdata[n].RdQ);
        init_waitqueue_head(&pdata[n].WrQ);


    }

	    pdata[0].base_addr = 0xc030; //todo verif
	    pdata[0].num_interupt = 20;
        SetDefaultConfig(pdata[0].base_addr);

	    pdata[1].base_addr = 0xc020; //todo verif
	    pdata[1].num_interupt = 21;
        SetDefaultConfig(pdata[1].base_addr);
   
    if (request_region(pdata[0].base_addr, number, "hellobonjour") == NULL) {
        printk(KERN_WARNING "MyMod : request_region() unsuccesfull--------0 \n");
        return - EPERM;
    }
    if (request_region(pdata[1].base_addr, number, "hellobonjour") == NULL) {
        printk(KERN_WARNING "MyMod : request_region() unsuccesfull---------1 \n");
        return - EPERM;
    }

    if(request_irq(pdata[0].num_interupt,&isrSerialPort,IRQF_SHARED,"MonPortSerie_0_IRQ",&(pdata[0]))) {
        printk(KERN_WARNING "MyMod : Request_irq unsuccesfull  \n");
        release_region(pdata[0].base_addr, number);
    }

    if(request_irq(pdata[1].num_interupt,&isrSerialPort,IRQF_SHARED,"MonPortSerie_1_IRQ",&(pdata[1]))) {
        printk(KERN_WARNING "MyMod : Request_irq unsuccesfull  \n");
        release_region(pdata[1].base_addr, number);
    }

    cdev_add(&My_cdev, My_dev,
             PORTNUMBER);  // add pilote to the kernel - struct cdev, dev number
                           // (/dev/My_dev1), int count
    printk(KERN_WARNING "MyMod : Kernel Module initilized with number  %u\n",
           MyModule_X);

    IER &= ~(ETBEI);
    outb(IER, pdata[0].base_addr + IER_REG);
    outb(IER, pdata[1].base_addr + IER_REG);
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

    spin_lock_irq(&pdata_p->splock);
    if (pdata_p->owner < 0) {
        pdata_p->owner = current_uid().val;
        printk(KERN_WARNING "MyMod: New owner is set to %u", pdata_p->owner);
    } else if (0/*pdata_p->owner != current_uid().val*/)  // TODO 2e time the
                                                            // same owner open
                                                            // its the #3026 ??
    {
        printk(KERN_WARNING "MyMod: ERR current user is :%u and new is : %u",
               pdata_p->owner, current_uid().val);
        spin_unlock_irq(&pdata_p->splock);
        return -EACCES;
    } else {
        printk(KERN_WARNING "MyMod: current user is the owner");
    }

    switch (filp->f_flags & O_ACCMODE) {
        case O_RDONLY:
            printk(KERN_WARNING "MyMod: O_RDONLY access");
            // statements
            if (pdata_p->fREAD) {
                spin_unlock_irq(&pdata_p->splock);
                return -EACCES;
            } else {
                pdata_p->fREAD = true;
                
            }
            break;

        case O_WRONLY:
            printk(KERN_WARNING "MyMod: O_WRONLY access");
            // statements
            if (pdata_p->fWRITE) {
                spin_unlock_irq(&pdata_p->splock);
                return -EACCES;
            } else {
                pdata_p->fWRITE = true;
                IER &= ~(ETBEI);
                outb(IER, pdata_p->base_addr + IER_REG);
            }
            break;

        case O_RDWR:
            printk(KERN_WARNING "MyMod: O_RDWR access");
            if (pdata_p->fWRITE || pdata_p->fREAD) {
                spin_unlock_irq(&pdata_p->splock);
                return -EACCES;
            } else {
                pdata_p->fWRITE = true;
                pdata_p->fREAD = true;
                IER &= ~(ETBEI);
                IER |= ERBFI;
                outb(IER, pdata_p->base_addr + IER_REG);
            }
            break;

        default:
            printk(KERN_WARNING
                   "MyMod: file is tried to be opened differently than "
                   "implemented");
            // default statements
    }
    spin_unlock_irq(&pdata_p->splock);

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

    spin_lock_irq(&pdata_p->splock);
    // TODO: protection d'access
    if (!((filp->f_flags & O_ACCMODE) == O_RDONLY ||
          (filp->f_flags & O_ACCMODE) == O_RDWR)) {
        spin_unlock_irq(&pdata_p->splock);
        return -EACCES;
    }
    spin_unlock_irq(&pdata_p->splock);
   
   
    // mutex_lock(&pdata_p->mutex);// todo ??should be semaphore
    if (down_interruptible(&pdata_p->sem)) {
        return -ERESTARTSYS;
    }
    if (pdata_p->buf_rd->count == 0) {  // no data in the cbuffer
        up(&pdata_p->sem);

        if (filp->f_flags & O_NONBLOCK) {
            printk(KERN_WARNING
                   "MyMod: READ : NO DATA AVAILABLE AND NON BLOCK\n");
            return -EAGAIN;
        } else {
            printk(KERN_WARNING "MyMod: READ : NO DATA BUT WILL WAIT\n");
            if (wait_event_interruptible(
                pdata_p->RdQ, pdata_p->buf_rd->count > 0)) { 
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
    count = (len <= cb_count(pdata_p->buf_rd))
                ? len
                : cb_count(pdata_p->buf_rd);

    // retire content of ring buffer to local buffer une a la fois

    printk(KERN_WARNING "MyMod: count value is %u", (int) count);


    for (i = 0; i < count; i++) {
        cb_pop(pdata_p->buf_rd, &(BufR[i]));
    }
    printk(KERN_WARNING "MyMod: Read after pop buff_count : %lu",cb_count(pdata_p->buf_rd));
    // mutex_unlock(&pdata_p->mutex);
    
   // wake_up_interruptible(&pdata_p->WrQ);
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
    size_t buffer_capacity;

    spin_lock_irq(&pdata_p->splock);
    // protection d'access
    if (!((filp->f_flags & O_ACCMODE) == O_WRONLY ||
          (filp->f_flags & O_ACCMODE) == O_RDWR)) {
        spin_unlock_irq(&pdata_p->splock);
        return -EACCES;
    }
    spin_unlock_irq(&pdata_p->splock);

    printk(KERN_WARNING "MyMod: WRITE\n");
    if (down_interruptible(&pdata_p->sem)) { // returns 0 if sucess
        return -ERESTARTSYS;
    }


    buffer_capacity = cb_getBufferSize(pdata_p->buf_wr);

    if (pdata_p->buf_wr->count == buffer_capacity) {  // buffer is full(if buffsize = 10 then 0 to 9
        up(&pdata_p->sem);

        if (filp->f_flags & O_NONBLOCK) {
            printk(KERN_WARNING "MyMod: WRITE : BUFFER FULL AND NON BLOCK\n");
            return -EAGAIN;
        } else {
            printk(KERN_WARNING "MyMod: WRITE : NO SPACE BUT WILL WAIT\n");
            if (wait_event_interruptible(
                pdata_p->WrQ,
                pdata_p->buf_wr->count < buffer_capacity)) {
                    return -ERESTARTSYS;
            }
        }
		printk(KERN_WARNING "MyMod: WRITE : WAIT IS OVER\n");
        
        if (down_interruptible(&pdata_p->sem)) {
            return -ERESTARTSYS;
        }
    }

    // place is buffer, ajust count. wich one is minimal ASK or AVAILABLE ?
    count = (len <= buffer_capacity - pdata_p->buf_wr->count)
                ? len
                : (buffer_capacity - pdata_p->buf_wr->count);

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
                cb_push(pdata_p->buf_wr, &(BufW[i]));
            }
            up(&pdata_p->sem);
           // wake_up_interruptible(&pdata->RdQ);
            IER |= ETBEI;
            outb(IER, pdata_p->base_addr + IER_REG);

            printk(KERN_WARNING "MyMod: WRITE buff count : %lu",
                cb_count(pdata_p->buf_wr));
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

    spin_lock_irq(&pdata_p->splock);
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
    spin_unlock_irq(&pdata_p->splock);

    // desattach the private data
    filp->private_data = NULL;

    printk(KERN_WARNING "MyMod: RELEASE end\n");

    // TODO: Et si le mode d’ouverture était O_RDONL Y ou O_RDWR, la Réception
    // du Port Série doit être interrompue afin d’arrêter de recevoir des
    // données

    return 0;
}


static long MyModule_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int retval = 0;
    circular_buffer *temp_buff_Rd, *temp_buff_Wr;
   
    struct pData *pdata_p = filp->private_data;
    printk(KERN_WARNING "MyMod: IOCTL\n");

    if (_IOC_TYPE(cmd) != IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > IOC_MAXNR) return -ENOTTY;

    //spin_lock(&pdata_p->splock); 
    //spin_unlock(&pdata_p->splock); 

    switch(cmd) {
       case SETBAUDRATE : // Execute SetBaudRate function
            if (SetBaudRate(arg,pdata_p->base_addr)) {
		          retval = 1;
	          } else return -ENOTTY;
            break;

       case SETDATASIZE : // Execute SetDataSize function
            if (SetDataSize(arg,pdata_p->base_addr)) {
		          retval = 1;
	          } else return -ENOTTY;
            break;

       case SETPARITY : // Execute SetParity function
            if (SetParity(arg,pdata_p->base_addr)) {
		          retval = 1;
	          } else return -ENOTTY;
            break;

       case GETBUFFERSIZE : // Execute GetBufSize function
		    printk(KERN_WARNING "MyMod: case GETBUFFSERSIZE\n");
		    retval = (int) cb_getBufferSize(pdata_p->buf_rd);
            break;

       case SETBUFFERSIZE : // Execute SetBufSize function
        //if (!capable(CAP_SYS_ADMIN)) return -EPERM;
	//check if possible 
	if(pdata_p->buf_wr->count > (size_t)arg || pdata_p->buf_wr->count > (size_t)arg  ){
		printk(KERN_WARNING "MyMod: buffer size request is too large\n");
		return 0;
	  }
        printk(KERN_WARNING "MyMod: case SETBUFFSERSIZE\n");

	temp_buff_Rd = cb_setBufferSize(pdata_p->buf_rd,(size_t)arg);
	temp_buff_Wr = cb_setBufferSize(pdata_p->buf_wr,(size_t)arg);

	

	pdata_p->buf_rd = temp_buff_Rd;
	pdata_p->buf_wr = temp_buff_Wr;

           // printk(KERN_WARNING "MyMod: IOCTL new buffer capacity %ld\n", pdata_p->buf_rd->capacity);
         //if(&pdata_p->buf_rd == NULL) {
	 //  return -ENOTTY;
	//}
 	retval = 1;
        break;
       default : return -ENOTTY;
     
    }
    return retval;
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
        cb_free(pdata[n].buf_rd);
        cb_free(pdata[n].buf_wr);
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

    release_region(pdata[0].base_addr, 8);
    release_region(pdata[1].base_addr, 8);

    free_irq(pdata[0].num_interupt,&(pdata[0]));
    free_irq(pdata[1].num_interupt,&(pdata[1]));

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
