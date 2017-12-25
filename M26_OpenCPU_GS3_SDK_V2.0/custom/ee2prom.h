

#ifndef  __EEPROM__
#define __EEPROM__

extern void eeprom_Init(void);
extern unsigned char I2CWriteByte(unsigned int mem_addr,unsigned char*DDATAp,unsigned int count);

#endif



