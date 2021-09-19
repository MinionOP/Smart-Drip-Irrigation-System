#include "database.h"

Database::Database()
{
    for (int i = 0; i < MAX_CROP; i++)
    {
        cropThresholdArr[i] = 80;
    }
}

void Database::save(uint8_t data[C_SAVE_SIZE])
{
    for (int i = 0; i < MAX_CROP; i++)
    {
        data[i] = cropThresholdArr[i];
    }
    for (int i = MAX_CROP, j = 0; i < MAX_CROP + 3; i++)
    {
        data[i] = cropArr[j];
        j++;
    }
    for (int i = MAX_CROP + 3, j = 0; i < C_SAVE_SIZE; i++)
    {
        data[i] = valveAvailArr[j];
        j++;
    }
}

void Database::load(uint8_t data[C_SAVE_SIZE])
{
    
    for (int i = 0; i < MAX_CROP; i++)
    {
        cropThresholdArr[i] = data[i];
    }
    for (int i = MAX_CROP, j = 0; i < MAX_CROP + 3; i++)
    {
        cropArr[j] = data[i];
        j++;
    }
    for (int i = MAX_CROP + 3, j = 0; i < C_SAVE_SIZE; i++)
    {
        valveAvailArr[j] = data[i];
        j++;
    }

}

bool Database::valveStatus(uint8_t _valveNum)
{
    return valveArr[_valveNum];
}

uint8_t Database::crop(uint8_t _cropNum)
{
    return cropArr[_cropNum];
}

uint8_t Database::soilSensor(uint8_t _soilNum)
{
    return soilArr[_soilNum];
}
uint8_t Database::temperature()
{
    return temp;
}
uint8_t Database::cropThreshold(uint8_t crop)
{
    return cropThresholdArr[crop];
}
uint8_t Database::getSelectedCropNum(void)
{
    return selectedCropNum;
}
uint8_t Database::getSelectedValveNum(void)
{
    return selectedValveNum;
}

uint8_t Database::getSelectedDate(bool index)
{
    return selectedDate[index];
}

uint8_t Database::getDayOfWeek(void)
{
    return dayOfWeek;
}

uint8_t Database::getHour(void)
{
    return hour;
}

uint8_t Database::get24Hour(void){
    if (hour == 12)
    {
        if (pm == true)
        {
            return 12;
        }
        else
        {
            return 0;
        }
    }
    return (pm == false) ? (hour) : (hour + 12);
}

uint8_t Database::getMinute(void)
{
    return minute;
}
bool Database::getPM(void)
{
    return pm;
}

bool Database::isValveAvailable(uint8_t _valveNum)
{
    return valveAvailArr[_valveNum];
}

void Database::setValveAvailability(uint8_t _valveNum, bool status)
{
    if (valveAvailArr[_valveNum] == 1 && status == 0)
    {
        valveFlagArr[_valveNum] = 1;
    }
    valveAvailArr[_valveNum] = status;
}

bool Database::getValveFlag(uint8_t _valveNum)
{
    return valveFlagArr[_valveNum];
}

void Database::clearValveFlag(uint8_t _valveNum)
{
    valveFlagArr[_valveNum] = 0;
}

void Database::setValveStatus(uint8_t _valveNum, bool status)
{
    valveArr[_valveNum] = status;
}
void Database::setSoilSensor(uint8_t _soilNum, uint8_t _soil)
{
    if (_soil >= 100)
    {
        _soil = 100;
    }
    else if (_soil < 0)
    {
        _soil = 0;
    }
    if (soilArr[_soilNum] != _soil)
        soilArr[_soilNum] = _soil;
}
void Database::setCrop(uint8_t _valveNum, uint8_t _crop)
{
    cropArr[_valveNum] = _crop;
}

void Database::setTemperature(uint8_t _temp)
{
    temp = _temp;
}
void Database::setCropThreshold(uint8_t _crop, uint8_t _threshold)
{
    cropThresholdArr[_crop] = _threshold;
}
void Database::setSelectedCropNum(uint8_t _selectedCropNum)
{
    selectedCropNum = _selectedCropNum;
}
void Database::setSelectedValveNum(uint8_t _selectedValveNum)
{
    selectedValveNum = _selectedValveNum;
}
void Database::setSelectedDate(uint8_t value, bool index)
{
    selectedDate[index] = value;
}

bool Database::setDayOfWeek(uint8_t _dayOfWeek, bool reformat)
{
    //Sunday = 0 and Sat = 6. convert to Sunday = 6 Sat = 5
    //_day = (_day == 0)?5:(_day == 1)?6:_day-2;
    if (reformat)
    {
        _dayOfWeek = (_dayOfWeek == 0) ? 6 : _dayOfWeek - 1;
    }
    else
    {
        dayOfWeek = _dayOfWeek;
        return true;
    }
    if (dayOfWeek != _dayOfWeek)
    {
        dayOfWeek = _dayOfWeek;
        return true;
    }
    return false;
}

void Database::setTime(uint8_t _hour, uint8_t _minute, bool _isPM)
{
    hour = _hour;
    minute = _minute;
    pm = _isPM;
}
void Database::setHour(uint8_t _hour)
{
    hour = _hour;
}
void Database::setMinute(uint8_t _minute)
{
    minute = _minute;
}
void Database::setPM(bool _isPM)
{
    pm = _isPM;
}

uint8_t* Database::getCalenderDate(uint8_t buffer[3])
{
    for(int i=0;i<3;i++){
        buffer[i] = calenderDate[i];
    }
    return buffer;
}
void Database::setCalenderDate(uint8_t _month, uint8_t _day, uint16_t _year)
{
    if(calenderDate[0] != _month)
        calenderDate[0] = _month;
    if(calenderDate[1] != _day)
        calenderDate[1] = _day;
    if(calenderDate[2] != _year){
        _year -=2000;
        calenderDate[2] = _year;
    }
}

bool Database::isRTCFlagEnable(void)
{
    return toUpdateRTC;
}

void Database::clearRTCFlag(void)
{
    toUpdateRTC = false;
}
void Database::enableRTCFlag(void)
{
    toUpdateRTC = true;
}