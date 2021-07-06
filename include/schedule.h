#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>


enum Day{MONDAY = 0, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY, NUM_DAY};
enum Time{HOUR = 0, MINUTE = 1, PM = 2};

class Schedule{
public:
    
    Schedule();
    uint8_t numOfSchedule(uint8_t day);
    uint8_t numOfTotalSchedule(void);
    void addSchedule(uint8_t day, uint8_t num, uint8_t _startHr, uint8_t _startMin, uint8_t _startPM, uint8_t _endHr, uint8_t _endMin, uint8_t _endPM);
    uint8_t* getOneScheduleInfo(uint8_t day, uint8_t num, uint8_t buffer[7]);

private:
    struct DaySchedule
    {
        uint8_t startHr = 0;
        uint8_t startMin = 0;
        bool startPM = 0;
        uint8_t endHr = 0;
        uint8_t endMin = 0;
        bool endPM = 0;
        bool active = 0;
    };

    DaySchedule scheduleInfo[NUM_DAY][3];
    uint8_t scheduleSize[7] = {0};
    bool timeline[7][24] = {{0}};

    //uint8_t* updateTimeline(void);
    
    uint8_t* to24Hour(uint8_t hour, uint8_t minute, uint8_t pm, uint8_t buffer[]);

    void clearOneSchedule(uint8_t day, uint8_t num);
    void orderSchedule(uint8_t day);
};


#endif