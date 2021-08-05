#include "database.h"


Database::Database(){
    for(int i=0;i<MAX_CROP;i++){
        cropThresholdArr[i] = 80;
    }
}

void Database::save(uint8_t data[MAX_CROP]){
    for(int i=0;i<MAX_CROP;i++){
        data[i] = cropThresholdArr[i];      
    }
}
void Database::load(uint8_t data[MAX_CROP]){
    for(int i=0;i<MAX_CROP;i++){
        cropThresholdArr[i] = data[i];
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

uint8_t Database::getSelectedDate(bool index){
    return selectedDate[index];
}


uint8_t Database::getDayOfWeek(void){
    return dayOfWeek;
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
void Database::setSelectedDate(uint8_t value, bool index){
    selectedDate[index] = value;
}

bool Database::setDayOfWeek(uint8_t _day){  
    //Sunday = 0 and Sat = 6. convert to Sunday = 6 Sat = 5
    //_day = (_day == 0)?5:(_day == 1)?6:_day-2;
    _day = (_day == 0)?6:_day-1;
    if(dayOfWeek !=_day){
        dayOfWeek = _day;
        return true;
    }
    return false;
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