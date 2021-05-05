#include "Schedule.h"



Schedule::Schedule() {
    //For Testing purposes
    //-----------------------------------------------------
    addSchedule(0, 0, 0, 30, 1, 6, 30, 1);
    addSchedule(0, 1, 7, 30, 1, 12, 30, 1);
    addSchedule(0, 2, 5, 30, 0, 7, 15, 0);

    addSchedule(1, 0, 1, 30, 1, 6, 30, 1);
    addSchedule(1, 1, 5, 30, 0, 7, 15, 0);

    addSchedule(2, 0, 2, 30, 1, 6, 30, 1);
    addSchedule(2, 1, 5, 30, 0, 7, 15, 0);

    addSchedule(3, 0, 3, 30, 1, 6, 30, 1);
    addSchedule(3, 1, 7, 30, 1, 12, 30, 1);
    addSchedule(3, 2, 5, 30, 0, 9, 15, 0);

    addSchedule(4, 0, 4, 30, 1, 6, 30, 1);
    addSchedule(4, 1, 7, 30, 1, 12, 30, 1);
    addSchedule(4, 2, 5, 30, 0, 12, 15, 0);

    addSchedule(5, 0, 5, 30, 1, 8, 45, 1);
    addSchedule(5, 1, 3, 30, 1, 8, 45, 1);

    addSchedule(6, 0, 6, 30, 1, 6, 30, 1);
    addSchedule(6, 1, 5, 30, 0, 7, 15, 0);

    //-----------------------------------------------------

}

void Schedule::addSchedule(uint8_t day, uint8_t num, uint8_t _startHr, uint8_t _startMin, uint8_t _startPM, uint8_t _endHr, uint8_t _endMin, uint8_t _endPM)
{
    // uint8_t toAddStart[2];
    // uint8_t toAddEnd[2];
    // uint8_t difference;

    // to24Hour(_startHr, _startMin, _startPM, toAddStart);
    // to24Hour(_endHr, _endMin, _endPM, toAddEnd);

    // difference = (toAddStart[HOUR] > toAddEnd[HOUR]) ? (24 - toAddStart[HOUR] + toAddEnd[HOUR]) : (toAddStart[HOUR] - toAddEnd[HOUR]);

    // uint8_t counter = toAddStart[HOUR];
    // for (int i = 0; i < difference; i++)
    // {
    //     if (timeline[day][counter] == true)
    //     {
    //     }
    //     counter = (counter + 1) % 24;
    // }

    scheduleInfo[day][num].startHr = _startHr;
    scheduleInfo[day][num].startMin = _startMin;
    scheduleInfo[day][num].startPM = _startPM;
    scheduleInfo[day][num].endHr = _endHr;
    scheduleInfo[day][num].endMin = _endMin;
    scheduleInfo[day][num].endPM = _endPM;
    scheduleInfo[day][num].active = true;
}

uint8_t* Schedule::getOneScheduleInfo(uint8_t day, uint8_t num, uint8_t buffer[7]) {

    buffer[0] = scheduleInfo[day][num].startHr;
    buffer[1] = scheduleInfo[day][num].startMin;
    buffer[2] = scheduleInfo[day][num].startPM;
    buffer[3] = scheduleInfo[day][num].endHr;
    buffer[4] = scheduleInfo[day][num].endMin;
    buffer[5] = scheduleInfo[day][num].endPM;
    buffer[6] = scheduleInfo[day][num].active;
    
    return buffer;
}

uint8_t Schedule::numOfSchedule(uint8_t day) {
    return scheduleSize[day];
}

uint8_t Schedule::numOfTotalSchedule(void) {
    uint8_t temp = 0;
    for(int i=0;i<7;i++){
        temp+=scheduleSize[i];
    }
    return temp;
}

void Schedule::orderSchedule(uint8_t day){
    
}

void Schedule::clearOneSchedule(uint8_t day, uint8_t num) {
    scheduleInfo[day][num].active = 0;
}

uint8_t* Schedule::to24Hour(uint8_t hour, uint8_t minute, uint8_t pm, uint8_t buffer[]){
    buffer[0] = hour*12*pm;
    buffer[1] = minute;
    return buffer;
}





