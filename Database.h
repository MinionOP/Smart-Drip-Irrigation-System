/*
 * ----------------------------------------------------------------
 * 
 * 
 * 
 * 
 *
 * 
 * 
 * 
 *
 * 
 * 
 * 
 * 
 *
 * 
 * 
 *
 * 
 * ----------------------------------------------------------------
 */
#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>

//Valve Closed/Open
#define CLOSE 0
#define OPEN 1

enum ListOfCrop{CORN = 0, BEAN, COTTON, TOMATO, POTATO, TOBACCO, PAPAYA, MAX_CROP};


class Database{
public:
    Database();    
    bool getValveStatus(uint8_t num);
    uint8_t getCrop(uint8_t num);
    uint8_t getSoilSensor(uint8_t num);
    uint8_t getTemperature();
    uint8_t getCropThreshold(uint8_t num);
    uint8_t getCurrentCrop(void);
    uint8_t getCurrentValve(void);
    uint8_t getCurrentSoil(void);
    uint8_t getHour(void);
    uint8_t getMinute(void);
    bool getPM(void);

    void setValveStatus(uint8_t _num, bool _status);
    void setSoilSensor(uint8_t _num, uint8_t _soil);
    void setCrop(uint8_t valveNum, uint8_t crop);
    void setTemperature(uint8_t _temp);
    void setCropThreshold(uint8_t _crop, uint8_t _threshold);
    void setCurrentCrop(uint8_t _currentCrop);
    void setCurrentValve(uint8_t _currentValve);
    void setTime(uint8_t _hour, uint8_t _minute, bool _isPM);
    void setHour(uint8_t _hour);
    void setMinute(uint8_t _minute);
    void setPM(bool _isPM);

private:
    uint8_t valve[3] = {0};
    uint8_t crop[3] = {0};
    bool soil[3] = {0};
    uint8_t cropThreshold[MAX_CROP];
    uint8_t currentValve = 0;
    uint8_t currentSoil = 0;
    uint8_t currentCrop = 0;
    uint8_t temperature = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    bool PM = 0;
};









#endif
