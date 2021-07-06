#include "database.h"


Database::Database(){
    for(int i=0;i<MAX_CROP;i++){
        cropThresholdArr[i] = 80;
    }
}
    

bool Database::valveStatus(uint8_t _valveNum){
    return valveArr[_valveNum];
}

uint8_t Database::crop(uint8_t _cropNum){
    return cropArr[_cropNum];
}

uint8_t Database::soilSensor(uint8_t _soilNum){
    return soilArr[_soilNum];
}
uint8_t Database::temperature(){
    return temp;
}
uint8_t Database::cropThreshold(uint8_t crop){
    return cropThresholdArr[crop];
}
uint8_t Database::getSelectedCropNum(void){
    return selectedCropNum;
}
uint8_t Database::getSelectedValveNum(void){
    return selectedValveNum;
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

void Database::setValveStatus(uint8_t _valveNum, bool status){
    valveArr[_valveNum] = status;
}
void Database::setSoilSensor(uint8_t _soilNum, uint8_t _soil){
   soilArr[_soilNum] = _soil;
}
void Database::setCrop(uint8_t _valveNum, uint8_t _crop){
    cropArr[_valveNum] = _crop;
}

void Database::setTemperature(uint8_t _temp){
    temp = _temp;
}
void Database::setCropThreshold(uint8_t _crop, uint8_t _threshold){
    cropThresholdArr[_crop] = _threshold;
}
void Database::setSelectedCropNum(uint8_t _selectedCropNum){
    selectedCropNum = _selectedCropNum;
}
void Database::setSelectedValveNum(uint8_t _selectedValveNum){
    selectedValveNum = _selectedValveNum;
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