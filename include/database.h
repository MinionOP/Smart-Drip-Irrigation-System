#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>

//Valve Closed/Open
constexpr bool CLOSE    = 0;
constexpr bool OPEN     = 1;


enum ListOfCrop{CORN = 0, BEAN, COTTON, TOMATO, POTATO, TOBACCO, PAPAYA, MAX_CROP};

constexpr uint8_t C_SAVE_SIZE = MAX_CROP + 3 + 3;



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
    uint8_t* getCalenderDate(uint8_t buffer[3]);
    uint8_t getHour(void);
    uint8_t get24Hour(void);
    uint8_t getMinute(void);
    bool getPM(void);
    bool isValveAvailable(uint8_t _valveNum);
    bool getValveFlag(uint8_t _valveNum);
    bool isRTCFlagEnable(void);

    void clearRTCFlag(void);
    void enableRTCFlag(void);

    void clearValveFlag(uint8_t _valveNum);
    void setValveStatus(uint8_t _num, bool _status);
    void setSoilSensor(uint8_t _num, uint8_t _soil);
    void setCrop(uint8_t valveNum, uint8_t crop);
    void setTemperature(uint8_t _temp);
    void setCropThreshold(uint8_t _crop, uint8_t _threshold);
    void setSelectedCropNum(uint8_t _currentCrop);
    void setSelectedValveNum(uint8_t _currentValve);
    void setSelectedDate(uint8_t value, bool index);//Index = 0 if value is date, index = 1 if value is num of date
    bool setDayOfWeek(uint8_t _dayOfWeek, bool reformat = true); //False if same day
    void setCalenderDate(uint8_t _month, uint8_t _day, uint16_t year);
    void setTime(uint8_t _hour, uint8_t _minute, bool _isPM);
    void setHour(uint8_t _hour);
    void setMinute(uint8_t _minute);
    void setPM(bool _isPM);
    void setValveAvailability(uint8_t _valveNum, bool status);

    void save(uint8_t data[C_SAVE_SIZE]);
    void load(uint8_t data[C_SAVE_SIZE]);

private:
    uint8_t valveArr[3] = {0};
    uint8_t cropArr[3] = {0};
    uint8_t soilArr[3] = {0};
    uint8_t batteryArr[2] = {0};
    bool valveAvailArr[3] = {true,true,true};
    bool valveFlagArr[3] = {0};
    
    uint8_t cropThresholdArr[MAX_CROP];
    uint8_t selectedValveNum = 0;
    uint8_t selectedCropNum = 0;
    uint8_t selectedDate[2] = {0};
    uint8_t dayOfWeek = 9;
    uint8_t calenderDate[3] = {1,26,21};
    uint8_t temp = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    bool pm = 0;
    bool toUpdateRTC = false;
};









#endif