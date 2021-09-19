#ifndef AT24C32_H
#define AT24C32_H

#include <Arduino.h>



class AT24C32{
    
public:

    AT24C32();

    void write(unsigned int  addr, byte data);
    void write(unsigned int  addr, byte dataBuffer[], int len);

    byte read(unsigned int  addr);
    void read(unsigned int address, byte *data, int n);

private:


    //128 pages of 32 bytes each
    byte   EEPROM_ADDRESS    = 0x57;
    byte   EEPROM_PAGE_SIZE  = 32;
    byte   EEPROM_PAGES      = 128; 

    void read(unsigned int address, byte *data, int offset, int n);
};





#endif
