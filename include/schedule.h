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
    
    /**
     * @brief Add a new schedule to scheduler
     * 
     * @return true if success
     * @return false if failed
     */
    bool update(uint8_t day, uint8_t num, uint8_t _startHr, uint8_t _startMin, bool _startPM, uint8_t _endHr, uint8_t _endMin, bool _endPM);

    /**
     * @brief Disable selected schedule. It will be skip when schedule is running 
     */
    void disableDay(uint8_t day, uint8_t num);

    /**
     * @brief Enable selected schedule.
     * 
     */
    void enableDay(uint8_t day, uint8_t num);

    /**
     * @brief Permanently delete selected schedule from scheduler
     */
    bool clear(uint8_t day, uint8_t num);

    /**
     * @brief Get schedule from scheduler
     * 
     * @param day - value from enum 'Day'
     * @param num - value between 0-2
     * @param buffer - empty array
     * @return uint8_t* schedule information
     */
    uint8_t *getInfo(uint8_t day, uint8_t num, uint8_t buffer[8]);


    bool isDayActive(uint8_t day, uint8_t num);

    /**
     * @param data - Copy entire scheduler into array.  
     */
    void save(uint8_t data[S_SAVE_SIZE]); 

    /**
     * @param data - Overwrite current scheduler with data array
     */
    void load(uint8_t data[S_SAVE_SIZE]);

    /**
     * @brief Find the closest available schedule based on the input
     * 
     * @param day - value from enum 'Day'
     * @param hour - 24 hour format
     * @return uint8_t* - Return the hour (24 hour) in index 0 and minute in index 1. 
     *                    Return 25 in index 0 if no valid schedule is found
     */
    uint8_t *locateClosest(uint8_t day, uint8_t hour, uint8_t min, uint8_t buffer[2]);

    /**
     * @brief locateClosest must have been previously called to use this method correctly.
     *        Find the next available schedule from planner 
     * 
     * @param day - value from enum 'Day'
     * @return uint8_t* - Return the hour (24 hour) in index 0 and minute in index 1. 
     *                    Return 25 in index 0 if no valid schedule is found
     */
    uint8_t* next(uint8_t day, uint8_t buffer[2]);

    
    /**
     * @brief Enable scheduler
     */
    void enable(void);

    /**
     * @brief Disable scheduler and clear all flags
     * 
     */
    void disable(void);
    void toggleSchedule(void);
    bool isEnable(void);

    /**
     * @brief Device is currently "running". It is in the timeframe set by user.
     * Example: Device is schedule to time on from Monday 10:30am - 11:30am
     * 
     * @return true - it in the time slot, and scheduler is enabled
     * @return false - not in the time slot
     */
    bool isRunning(void);

    /**
     * @brief A flag that is set if a schedule (struct DaySchedule) was added/remove or time/date was changed 
     *        while scheduler is enabled or running
     * 
     * @return true - scheduler was recently changed
     * @return false - no change have been made to scheduler
     */
    bool isReschedule(void);
    void clearRescheduleFlag(void);


    void setDeadline(uint8_t buffer[2]);
    uint8_t *getDeadline(uint8_t buffer[2], bool hour24 = false);
    bool isDeadline(void);
    void clearDeadlineFlag(void);

    /**
     * @brief Get the day that is currently being edited by user
     *    (day)     Friday:
     *    (number 1)   10:30am - 11:30am
     *    (number 2)    2:00pm - 2:45pm
     *    (number 3)    3:00pm - 9:15pm
     * @return uint8_t value from enum "Day"
     */
    uint8_t getActiveDay(void);

    /**
     * @brief Get the number being edited by user
     *    (day)    Friday:
     *    (number 1)   10:30am - 11:30am
     *    (number 2)    2:00pm - 2:45pm
     *    (number 3)    3:00pm - 9:15pm
     * @return uint8_t 
     */
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
    uint8_t deadline[2] = {0};
    uint8_t activeNum = 0;
    uint8_t activeDay = 0;
    bool scheduleFlag = 0;
    bool deadlineFlag = false;
    bool activeFlag = 0;
    bool isInitial = true;
    bool reschedule = false;
    bool runningFlag = false;

    //Return 0 if first set is earlier
    bool compare(uint8_t hour1, uint8_t minute1, bool pm1, uint8_t hour2, uint8_t minute2, bool pm2);
    //bool erase(uint8_t day, uint8_t num);
    void clearOne(uint8_t day, uint8_t num);
    void reorder(uint8_t day, uint8_t num, uint8_t hour, uint8_t min, bool pm);

    uint8_t to24Hour(uint8_t hour, bool pm);
    uint8_t* to12Hour(uint8_t hour, uint8_t buffer[2]);
    uint16_t timelineSize(uint8_t day);
};

#endif