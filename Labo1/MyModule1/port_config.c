
#include "port_config.h"


static int SetDefaultConfig(void){
  SetBaudRate(115200);
  SetDataSize(8);
  SetParity(0);
  SetStopBit(1);
  
  //fifo enable
  FCR |= 0x01;
  FCR &= ~(0xC0);
	
}

static int SetBaudRate(unsigned long arg) {
    int retval = 0;
    if (arg<50 || arg>115200) {
     	printk(KERN_WARNING "Mymod: The speed must be between 50 and 115200 Baud.\n");
	return -ENOTTY;
    } 

    LCR |= DLAB;
    uint16_t PORT_DIVISOR = ((FREQ_BASE)/(16*arg));
    DLL = PORT_DIVISOR & 0xFF;
    DLM = PORT_DIVISOR & 0xFF00;
    LCR &= ~DLAB;
	retval = 1;

    return retval;
}

static int SetDataSize(unsigned long arg) {
    int retval = 0;
    if (arg<5 && arg>8) {
     	printk(KERN_WARNING "Mymod: The size of the communication data must be betwween 5 and 8 bits.\n");
	return -ENOTTY;
    }
    switch(arg) {
        case 5:
          ## LCR |= 0x00;
           break;
        case 6:
           LCR |= 0x01;
           break;
        case 7:
           LCR |= 0x02;
           break;
        case 8:
           LCR |= 0x03;
           break;         
    }
        
    
	    // mettre instruction pour taille des données
	    LCR |= 0
        printk(KERN_WARNING "Mymod: Setting Data Size.\n");
	retval = 1;
    
    return retval;
}

static int SetParity(unsigned long arg) {
    int retval = 0;
    if (arg>3) {
	printk(KERN_WARNING "Mymod: The allowed parity types are : no parity (0), odd parity (1) and even parity (2).\n");
	return -ENOTTY;
    } 	
	switch(air) {
        case 0:
            LCR &= ~(0x08);
            break;
        case 1:
            LCR |= (0x08);
            LCR &= ~(0x10);
            break;
        case 2:
            LCR |= (0x08);
            LCR |= (0x10);
            break;            
        
        // mettre instruction pour changer la parité
        printk(KERN_WARNING "Mymod: Setting Parity.\n");
	retval = 1;
    }
    return retval;
}


static int SetStopBit(unsigned long arg) {
    int retval = 0;
    if(!(arg == 1 || arg == 2)) {
     	printk(KERN_WARNING "Mymod: The speed must be between 50 and 115200 Baud.\n");
	return -ENOTTY;
    } 
    if(arg == 1) {
        LCR &= ~(0x04);

    } else (arg == 2) {
        LCR |= (0x04);
    }

	    // mettre instruction pour changer cette vitesse
        printk(KERN_WARNING "Mymod: Setting Baud Rate.\n");
	retval = 1;
    
    return retval;
}

