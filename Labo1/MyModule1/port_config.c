//#include "MyModule.h"
#include "port_config.h"
//#include "MyModule.h"
uint8_t FCR, DLAB, DLL, DLM;
extern uint8_t LCR;

int SetDefaultConfig(uint16_t base_addr){
  DLAB = 0x80;
  SetBaudRate(115200,base_addr);
  SetDataSize(8,base_addr);
  SetParity(0,base_addr);
  SetStopBit(1,base_addr);
  
  //fifo enable
  FCR |= 0x01;
  FCR &= ~(0xC0);
  outb(FCR, base_addr + FCR_REG );
 return 1;
}

int SetBaudRate(unsigned long arg,uint16_t base_addr) {
    int retval = 0;
    uint16_t PORT_DIVISOR;
    if (arg<50 || arg>115200) {
     	printk(KERN_WARNING "MyMod: The speed must be between 50 and 115200 Baud.\n");
	return -ENOTTY;
    } 
    
    LCR |= DLAB;
    outb(LCR, base_addr + LCR_REG);
    PORT_DIVISOR = ((FREQ_BASE)/(16*arg));
    DLL = PORT_DIVISOR & 0xFF;
    outb(DLL, base_addr + DLL_REG);
    DLM = (uint8_t)((PORT_DIVISOR & 0xFF00)>>8);
    outb(DLM, base_addr + DLM_REG);
    LCR &= ~DLAB;
    outb(LCR, base_addr + LCR_REG);

    retval = 1;

    return retval;
}

int SetDataSize(unsigned long arg,uint16_t base_addr) {
    int retval = 0;
    if (arg<5 && arg>8) {
     	printk(KERN_WARNING "MyMod: The size of the communication data must be betwween 5 and 8 bits.\n");
	    return -ENOTTY;
    }
    switch(arg) {
        case 5:
            LCR |= 0x00;
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
        default:
            break;
    }
    
	    // mettre instruction pour taille des données
	    LCR |= 0;
        printk(KERN_WARNING "MyMod: Setting Data Size.\n");
	outb(LCR, base_addr + LCR_REG);
    retval = 1;
    
    return retval;
}

int SetParity(unsigned long arg,uint16_t base_addr) {
    int retval = 0;
    if (arg>3) {
	printk(KERN_WARNING "MyMod: The allowed parity types are : no parity (0), odd parity (1) and even parity (2).\n");
	return -ENOTTY;
    } 	
	switch(arg) {
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
       
	retval = 1;
    }
     printk(KERN_WARNING "MyMod: Setting Parity.\n");
    outb(LCR, base_addr + LCR_REG);
    return retval;
}


int SetStopBit(unsigned long arg,uint16_t base_addr) {
    int retval = 0;
    if(!(arg == 1 || arg == 2)) {
     	printk(KERN_WARNING "MyMod: The speed must be between 50 and 115200 Baud.\n");
	return -ENOTTY;
    } 
    if(arg == 1) {
        LCR &= ~(0x04);
    } else if(arg == 2) {
        LCR |= (0x04);
    }

	    // mettre instruction pour changer cette vitesse
        printk(KERN_WARNING "MyMod: Setting Baud Rate.\n");
	outb(LCR,  base_addr + LCR_REG);
    retval = 1;
    
    return retval;
}

