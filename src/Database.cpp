#include "Database.h"



Database::Database(){
    for(int i=0;i<MAX_CROP;i++){
        cropThreshold[i] = 80;
    }
}
    

bool Database::getValveStatus(uint8_t num){
    return valve[num];
}

uint8_t Database::getCrop(uint8_t num){
    return crop[num];
}

uint8_t Database::getSoilSensor(uint8_t num){
    return soil[num];
}
uint8_t Database::getTemperature(){
    return temperature;
}
uint8_t Database::getCropThreshold(uint8_t num){
    return cropThreshold[num];
}
uint8_t Database::getCurrentCrop(void){
    return currentCrop;
}
uint8_t Database::getCurrentValve(void){
    return currentValve;
}
uint8_t Database::getCurrentSoil(void){
    return currentSoil;
}
uint8_t Database::getHour(void){
    return hour;
}
uint8_t Database::getMinute(void){
    return minute;
}
bool Database::getPM(void){
    return PM;
}

void Database::setValveStatus(uint8_t num, bool status){
    valve[num] = status;
}
void Database::setSoilSensor(uint8_t _num, uint8_t _soil){
   soil[_num] = _soil;
}
void Database::setCrop(uint8_t _valveNum, uint8_t _crop){
    crop[_valveNum] = _crop;
}

void Database::setTemperature(uint8_t _temp){
    temperature = _temp;
}
void Database::setCropThreshold(uint8_t _crop, uint8_t _threshold){
    cropThreshold[_crop] = _threshold;
}
void Database::setCurrentCrop(uint8_t _currentCrop){
    currentCrop = _currentCrop;
}
void Database::setCurrentValve(uint8_t _currentValve){
    currentValve = _currentValve;
}

void Database::setTime(uint8_t _hour, uint8_t _minute, bool _isPM){
    hour = _hour;
    minute = _minute;
    PM = _isPM;
}
void Database::setHour(uint8_t _hour){
    hour = _hour;

}
void Database::setMinute(uint8_t _minute){
    minute = _minute;
}
void Database::setPM(bool _isPM){
    PM = _isPM;
}
