#include "AT24C32_eeprom.h"

#include <Wire.h>


AT24C32::AT24C32()
{
    Wire.begin();
}

void AT24C32::write(unsigned int address, byte data) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    if(Wire.endTransmission()==0) {
    	Wire.beginTransmission(EEPROM_ADDRESS);
    	Wire.write(address >> 8);
    	Wire.write(address & 0xFF);
      	Wire.write(data);
    	Wire.endTransmission();
    	delay(20);
    }
}

void AT24C32::write(unsigned int  addr, byte dataBuffer[], int len)
{
    int remaining = len;
    int offD = 0;   //data offset
    int offP;       //page offset
    int i = 0;   

    while(remaining > 0){
        offP = addr % EEPROM_PAGE_SIZE;
        i = min(min(remaining,30),EEPROM_PAGE_SIZE - offP);
        Wire.beginTransmission(EEPROM_ADDRESS);
        if(Wire.endTransmission() == 0){
            Wire.beginTransmission(EEPROM_ADDRESS);
            Wire.write(addr >> 8);
            Wire.write(addr && 0xFF);
            byte *addr2 = dataBuffer + offD;
            Wire.write(addr2, i);
            Wire.endTransmission();
            delay(20);
        }
        remaining -= i;
        offD += i;
        addr +=i;
    }

}

byte AT24C32::read(unsigned int  addr)
{
    byte b = 0;
	int r = 0;
	Wire.beginTransmission(EEPROM_ADDRESS);
    if (Wire.endTransmission()==0) {
     	Wire.beginTransmission(EEPROM_ADDRESS);
    	Wire.write(addr >> 8);
    	Wire.write(addr & 0xFF);
    	if (Wire.endTransmission()==0) {
			Wire.requestFrom(EEPROM_ADDRESS, 1);
			while (Wire.available() > 0 && r<1) {
				b = (byte)Wire.read();
				r++;
			}
    	}
    }
    return b;
    // byte data =0;
    // int s = 0;
    // Wire.beginTransmission(EEPROM_ADDRESS);

    // if(Wire.endTransmission() == 0){
    //     Wire.beginTransmission(EEPROM_ADDRESS);
    // 	Wire.write(addr >> 8);
    // 	Wire.write(addr & 0xFF);
    //     if(Wire.endTransmission() == 0){
    //         Wire.requestFrom(EEPROM_ADDRESS,1);
    //         while(Wire.available() > 0 && s < 1){
    //             data = (byte)Wire.read();
    //             s++;
    //         }
    //     }
    // }

    // return data;

}
// void AT24C32::read(unsigned int  addr, byte dataBuffer[], int len)
// {
//     int remaining = len;
//     int offD = 0;

//     while(remaining > 0){
//         int _len = remaining;
//         if(_len > 32){
//             _len = 32;
//         }

//         Wire.beginTransmission(EEPROM_ADDRESS);
//         if(Wire.endTransmission() == 0){
//             Wire.beginTransmission(EEPROM_ADDRESS);
//             Wire.write(addr >> 8);
//             Wire.write(addr && 0xFF);
//             if(Wire.endTransmission() == 0){
//                 int s = 0;
//                 Wire.requestFrom(int(EEPROM_ADDRESS),_len);
//                 while(Wire.available() > 0 && s < _len){
//                     dataBuffer[offD+s] = (byte)Wire.read();
//                     s++;
//                 }
//             }
//         }
//         remaining -=_len;
//         addr += _len;
//         offD += _len;

//     }
// }


void AT24C32::read(unsigned int address, byte *data, int n) {
	int c = n;
	int offD = 0;
	// read until are n bytes read
	while (c > 0) {
		// read maximal 32 bytes
		int nc = c;
		if (nc > 32)
			nc = 32;
		read(address, data, offD, nc);
		address+=nc;
		offD+=nc;
		c-=nc;
	}
}


/**
 * Read sequence of n bytes to offset
 */
void AT24C32::read(unsigned int address, byte *data, int offset, int n) {
	Wire.beginTransmission(EEPROM_ADDRESS);
    if (Wire.endTransmission()==0) {
     	Wire.beginTransmission(EEPROM_ADDRESS);
    	Wire.write(address >> 8);
    	Wire.write(address & 0xFF);
    	if (Wire.endTransmission()==0) {
			int r = 0;
    		Wire.requestFrom(EEPROM_ADDRESS, n);
			while (Wire.available() > 0 && r<n) {
				data[offset+r] = (byte)Wire.read();
				r++;
			}
    	}
    }
}
