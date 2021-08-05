#include "AT24C32_eeprom.h"

#include <Wire.h>


AT24C32::AT24C32()
{
    Wire.begin();
}

void AT24C32::write(unsigned int  addr, byte data)
{
    Wire.beginTransmission(EEPROM_ADDRESS);
    if(Wire.endTransmission() == 0){
        Wire.beginTransmission(EEPROM_ADDRESS);
        Wire.write(addr>>8);
        Wire.write(addr && 0xFF);
        Wire.write(data);
        Wire.endTransmission();
        delay(20);
    }
}
void AT24C32::write(unsigned int  addr, byte dataBuffer[], byte len)
{
    int remaining = len;
    int offD = 0;   //data offset
    int offP;       //page offset
    int i = 0;   

    while(remaining > 0){
        offP = addr % pageSize;
        i = min(min(remaining,30),pageSize - offP);
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
    byte data =0;
    int s = 0;
    Wire.beginTransmission(EEPROM_ADDRESS);
    if(Wire.endTransmission() == 0){
        Wire.beginTransmission(EEPROM_ADDRESS);
        Wire.write(addr >> 8);
        Wire.write(addr && 0xFF);
        if(Wire.endTransmission() == 0){
            Wire.requestFrom(int(EEPROM_ADDRESS),1);
            while(Wire.available() > 0 && s < 1){
                data = (byte)Wire.read();
                s++;
            }
        }
    }
    return data;

}
void AT24C32::read(unsigned int  addr, byte dataBuffer[], byte len)
{
    int remaining = len;
    int offD = 0;

    while(remaining > 0){
        int _len = remaining;
        if(_len > 32){
            _len = 32;
        }

        Wire.beginTransmission(EEPROM_ADDRESS);
        if(Wire.endTransmission() == 0){
            Wire.beginTransmission(EEPROM_ADDRESS);
            Wire.write(addr >> 8);
            Wire.write(addr && 0xFF);
            if(Wire.endTransmission() == 0){
                int s = 0;
                Wire.requestFrom(int(EEPROM_ADDRESS),_len);
                while(Wire.available() > 0 && s < _len){
                    dataBuffer[offD+s] = (byte)Wire.read();
                    s++;
                }
            }
        }
        remaining -=_len;
        addr += _len;
        offD += _len;

    }
}