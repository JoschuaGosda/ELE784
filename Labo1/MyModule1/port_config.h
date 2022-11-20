
#ifndef PORT_CONFIG_H
#define PORT_CONFIG_H

//#include "MyModule.h"
#define FREQ_BASE 1843200
#define LCR_REG 0x03
#define DLL_REG 0x00
#define DLM_REG 0x01
#define FCR_REG 0x02
#define DLAB_REG 0x80
#define IER_REG 0x01
#define ERBFI 0x01
#define ETBEI 0x02

int SetDefaultConfig(uint16_t base_addr);
int SetBaudRate(unsigned long arg,uint16_t base_addr);
int SetDataSize(unsigned long arg,uint16_t base_addr);
int SetParity(unsigned long arg,uint16_t base_addr);
int SetStopBit(unsigned long arg,uint16_t base_addr);





#endif
