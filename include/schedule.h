#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>

enum Day
{
    MONDAY = 0,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY,
    NUM_DAY
};
enum Time
{
    AM = 0,
    PM = 1
};

//Num of DaySchedule Variables * Days in a week * Schedule per day. 8 * 7 * 3
constexpr uint8_t S_SAVE_SIZE     = 168;


class Schedule
{
public:
    Schedule();
    bool update(uint8_t day, uint8_t num, uint8_t _startHr, uint8_t _startMin, bool _startPM, uint8_t _endHr, uint8_t _endMin, bool _endPM);
    void disableDay(uint8_t day, uint8_t num);
    void enableDay(uint8_t day, uint8_t num);
    bool clear(uint8_t day, uint8_t num);

    uint8_t *getInfo(uint8_t day, uint8_t num, uint8_t buffer[8]);
    //uint8_t *getTimeline(uint8_t day, uint8_t buffer[288]);
    bool isDayActive(uint8_t day, uint8_t num);
    void save(uint8_t data[S_SAVE_SIZE]); 
    void load(uint8_t data[S_SAVE_SIZE]);

    uint8_t *locateClosest(uint8_t day, uint8_t hour, uint8_t min, uint8_t buffer[2]);
    uint8_t* next(uint8_t day, uint8_t buffer[2]);
    void disable(void);
    void enable(void);
    void toggleSchedule(void);
    bool isEnable(void);
    bool isRunning(void);
    bool isReschedule(void);
    void clearRescheduleFlag(void);

    uint8_t getActiveDay(void);
    uint8_t getActiveNum(void);


private:
    struct DaySchedule
    {
        uint8_t startHr = 0;
        uint8_t startMin = 0;
        bool startPM = 0;
        uint8_t endHr = 0;
        uint8_t endMin = 0;
        bool endPM = 0;
        bool active = false;
        bool empty = true;
    };

    DaySchedule scheduleTable[NUM_DAY][3];
    uint8_t scheduleOrder[7][3] = {0};
    uint8_t activeNum = 0;
    uint8_t activeDay = 0;
    bool scheduleFlag = 0;
    bool activeFlag = 0;
    bool isInitial = true;
    bool reschedule = false;

    //Return 0 if first set is earlier
    bool compare(uint8_t hour1, uint8_t minute1, bool pm1, uint8_t hour2, uint8_t minute2, bool pm2);
    //bool erase(uint8_t day, uint8_t num);
    void clearOne(uint8_t day, uint8_t num);
    void reorder(uint8_t day, uint8_t num, uint8_t hour, uint8_t min, bool pm);

    uint8_t to24Hour(uint8_t hour, bool pm);
    uint16_t timelineSize(uint8_t day);
};

#endif