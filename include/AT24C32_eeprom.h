#ifndef AT24C32_H
#define AT24C32_H

#include <Arduino.h>



class AT24C32{
    
public:


    AT24C32();

    /**
     * @brief Write a value to memory. Remember to validify 
     *         if same data is already in memory before writing 
     *         to prolong write cycle 
     * 
     * @param addr - address
     * @param data - value being written to memory
     */
    void write(unsigned int  addr, byte data);

    /**
     * @brief Write an array of bytes to memory. Remember to validify 
     *         if same data is already in memory before writing 
     *         to prolong write cycle 
     * 
     * @param addr - address
     * @param dataBuffer - array of value being written to memory
     * @param len - length of array
     */
    void write(unsigned int  addr, byte dataBuffer[], int len);

    /**
     * @brief Read a single byte from memory
     * 
     * @param addr - address
     * @return byte 
     */
    byte read(unsigned int  addr);

    /**
     * @brief Read an array of bytes from memory
     */
    void read(unsigned int address, byte *data, int n);

private:


    //128 pages of 32 bytes each
    byte   EEPROM_ADDRESS    = 0x57;
    byte   EEPROM_PAGE_SIZE  = 32;
    byte   EEPROM_PAGES      = 128; 

    void read(unsigned int address, byte *data, int offset, int n);
};





#endif
