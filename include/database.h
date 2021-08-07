#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>

//Valve Closed/Open
constexpr bool CLOSE    = 0;
constexpr bool OPEN     = 1;


enum ListOfCrop{CORN = 0, BEAN, COTTON, TOMATO, POTATO, TOBACCO, PAPAYA, MAX_CROP};


class Database{
public:
    Database();    
    //true = open, false = close
    bool valveStatus(uint8_t _valveNum);
    uint8_t crop(uint8_t _cropNum);
    uint8_t soilSensor(uint8_t _soilNum);
    uint8_t temperature();
    uint8_t cropThreshold(uint8_t crop);
    //Index = 0 for day, index = 1 for num
    uint8_t getSelectedDate(bool index);
    uint8_t getSelectedCropNum(void);
    uint8_t getSelectedValveNum(void);
    uint8_t getDayOfWeek(void);
    uint8_t getHour(void);
    uint8_t getMinute(void);
    bool getPM(void);

    void setValveStatus(uint8_t _num, bool _status);
    void setSoilSensor(uint8_t _num, uint8_t _soil);
    void setCrop(uint8_t valveNum, uint8_t crop);
    void setTemperature(uint8_t _temp);
    void setCropThreshold(uint8_t _crop, uint8_t _threshold);
    void setSelectedCropNum(uint8_t _currentCrop);
    void setSelectedValveNum(uint8_t _currentValve);
    void setSelectedDate(uint8_t value, bool index);//Index = 0 if value is date, index = 1 if value is num of date
    bool setDayOfWeek(uint8_t _day); //False if same day
    void setTime(uint8_t _hour, uint8_t _minute, bool _isPM);
    void setHour(uint8_t _hour);
    void setMinute(uint8_t _minute);
    void setPM(bool _isPM);

    void save(uint8_t data[MAX_CROP]);
    void load(uint8_t data[MAX_CROP]);

private:
    uint8_t valveArr[3] = {0};
    uint8_t cropArr[3] = {0};
    uint8_t soilArr[3] = {0};
    
    uint8_t cropThresholdArr[MAX_CROP];
    uint8_t selectedValveNum = 0;
    uint8_t selectedCropNum = 0;
    uint8_t selectedDate[2] = {0};
    uint8_t dayOfWeek = 9;
    uint8_t temp = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    bool PM = 0;
};









#endif