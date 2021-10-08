#include "schedule.h"

Schedule::Schedule()
{
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            scheduleOrder[i][j] = 3;
        }
    }

}

bool Schedule::update(uint8_t day, uint8_t num, uint8_t _startHr, uint8_t _startMin, bool _startPM, uint8_t _endHr, uint8_t _endMin, bool _endPM)
{

    if (
        (_startMin > 55) ||
        (_endMin > 55) ||
        (_startMin != 0 && _startMin % 5 != 0) ||
        (_endMin != 0 && _endMin % 5 != 0) ||
        (_startHr == 0 || _endHr == 0) ||
        (_startHr == _endHr && _startMin == _endMin && _startPM == _endPM))
    {
        return false;
    }

    bool toClear = true;

    if (!scheduleTable[day][num].empty)
    {
        if (
            scheduleTable[day][num].startHr == _startHr &&
            scheduleTable[day][num].startMin == _startMin &&
            scheduleTable[day][num].startPM == _startPM &&
            scheduleTable[day][num].endHr == _endHr &&
            scheduleTable[day][num].endMin == _endMin &&
            scheduleTable[day][num].endPM == _endPM)
        {
            toClear = false;
        }
        else
        {
            clear(day, num);
        }
    }

    if (toClear)
    {
        scheduleTable[day][num].startHr = _startHr;
        scheduleTable[day][num].startMin = _startMin;
        scheduleTable[day][num].startPM = _startPM;
        scheduleTable[day][num].endHr = _endHr;
        scheduleTable[day][num].endMin = _endMin;
        scheduleTable[day][num].endPM = _endPM;
    }
    //scheduleTable[day][num].active = (setToInActive)?false:true;
    scheduleTable[day][num].active = false;
    scheduleTable[day][num].empty = false;

    reorder(day, num, _startHr, _startMin, _startPM);

    reschedule = true;

    return true;
}

void Schedule::disableDay(uint8_t day, uint8_t num)
{
    scheduleTable[day][num].active = false;
}

//Bug, do not update other timer if schedule was previous inactive
void Schedule::enableDay(uint8_t day, uint8_t num)
{
    if (scheduleTable[day][num].active || scheduleTable[day][num].empty)
        return;

    uint8_t type = 0,
            _startHr = scheduleTable[day][num].startHr,
            _startMin = scheduleTable[day][num].startMin,
            _endHr = scheduleTable[day][num].endHr,
            _endMin = scheduleTable[day][num].endMin;

    uint16_t startPos = to24Hour(_startHr, scheduleTable[day][num].startPM) * 12 + (_startMin / 5),
             endPos = to24Hour(_endHr, scheduleTable[day][num].endPM) * 12 + (_endMin / 5),
             s2 = 0,
             e2 = 0;

    for (int i = 0, x = 3; i < 3; i++)
    {
        if (scheduleTable[day][i].empty || i == num || !scheduleTable[day][i].active)
            continue;

        s2 = to24Hour(scheduleTable[day][i].startHr, scheduleTable[day][i].startPM) * 12 + scheduleTable[day][i].startMin / 5;
        e2 = to24Hour(scheduleTable[day][i].endHr, scheduleTable[day][i].endPM) * 12 + scheduleTable[day][i].endMin / 5;

        type = (startPos < endPos && s2 < e2) ? 0 : (startPos > endPos && s2 < e2) ? 1
                                                : (startPos < endPos && s2 > e2)   ? 2
                                                                                   : 3;

        switch (type)
        {
        case 0:
        {
            if ((startPos <= s2 && endPos >= e2) || (startPos >= s2 && endPos <= e2))
            {
                scheduleTable[day][i].active = false;
                continue;
            }
            if (startPos <= s2 && endPos >= s2 && endPos <= e2)
            {
                x = 1;
            }
            if (startPos >= s2 && startPos <= e2 && endPos >= e2)
            {
                x = 0;
            }
            break;
        }
        case 1:
        {
            if ((startPos >= e2 && endPos >= e2) || (startPos <=s2 && endPos <= s2))
            {
                scheduleTable[day][i].active = false;
                continue;
            }
            if (startPos <= s2 && endPos <= e2)
            {
                x = 1;
            }
            if (startPos >= e2 && endPos >= e2)
            {
                x = 0;
            }
            break;
        }
        case 2:
        {
            if (startPos >= s2 && endPos <= 288)
            {
                scheduleTable[day][i].active = false;
                continue;
            }
            if (startPos <= s2 && endPos >= s2)
            {
                x = 1;
            }
            if (startPos <= e2 && endPos >= e2)
            {
                x = 0;
            }
            break;
        }
        case 3:
        {
            if ((startPos <= s2 && endPos >= e2) || (startPos >= s2 && endPos <= e2))
            {
                scheduleTable[day][i].active = false;
                continue;
            }
            if (startPos <= s2 && endPos <= e2)
            {
                x = 1;
            }
            if (startPos >= s2 && endPos >= e2)
            {
                x = 0;
            }
            break;
        }
        }
        if (x == 0)
        {
            startPos = e2;
            _startHr = scheduleTable[day][i].endHr;
            _startMin = scheduleTable[day][i].endMin;
        }
        else if (x == 1)
        {
            endPos = s2;
            _endHr = scheduleTable[day][i].startHr;
            _endMin = scheduleTable[day][i].startMin;
        }
        x = 3;
    }

    scheduleTable[day][num].startHr = _startHr;
    scheduleTable[day][num].startMin = _startMin;
    scheduleTable[day][num].endHr = _endHr;
    scheduleTable[day][num].endMin = _endMin;
    scheduleTable[day][num].active = true;
    reschedule = true;
}

bool Schedule::clear(uint8_t day, uint8_t num)
{
    scheduleTable[day][num].startHr = 0;
    scheduleTable[day][num].startMin = 0;
    scheduleTable[day][num].startPM = 0;
    scheduleTable[day][num].endHr = 0;
    scheduleTable[day][num].endMin = 0;
    scheduleTable[day][num].endPM = 0;
    scheduleTable[day][num].active = false;
    scheduleTable[day][num].empty = true;

    return true;
}

uint8_t *Schedule::getInfo(uint8_t day, uint8_t num, uint8_t buffer[8])
{

    buffer[0] = scheduleTable[day][num].startHr;
    buffer[1] = scheduleTable[day][num].startMin;
    buffer[2] = scheduleTable[day][num].startPM;
    buffer[3] = scheduleTable[day][num].endHr;
    buffer[4] = scheduleTable[day][num].endMin;
    buffer[5] = scheduleTable[day][num].endPM;
    buffer[6] = scheduleTable[day][num].active;
    buffer[7] = scheduleTable[day][num].empty;

    return buffer;
}

bool Schedule::isDayActive(uint8_t day, uint8_t num)
{
    if (scheduleTable[day][num].active)
    {
        return true;
    }
    return false;
}

void Schedule::clearOne(uint8_t day, uint8_t num)
{
    scheduleTable[day][num].active = 0;
}

void Schedule::reorder(uint8_t day, uint8_t num, uint8_t hour, uint8_t min, bool pm)
{
    uint16_t a1 = to24Hour(hour, pm)*12 + min/5,
             buf[3] = {0};
    uint8_t num2 = 0,
            counter = 0,
            s;

    for (int i = 0; i < 3; i++) {
        if (!scheduleTable[day][i].empty)
            s++;
    }

    for (int i = 0; i < s; i++)
    {
        num2 = scheduleOrder[day][i];
        if (num2 == 3)
        {
            buf[i] = a1;
            scheduleOrder[day][i] = num;
            counter++;
            break;
        }
        if (scheduleTable[day][num2].empty)
            continue;
        buf[i] = to24Hour(scheduleTable[day][num2].startHr, scheduleTable[day][num2].startPM)*12 + scheduleTable[day][num2].startMin/5;

        counter++;
    }
    for (int i = 0, value = 0; i < counter-1; i++) {
        for (int j = 0; j < counter - i-1; j++) {
            if (buf[j] > buf[j + 1]) {
                value = buf[j];
                buf[j] = buf[j + 1];
                buf[j + 1] = value;

                value = scheduleOrder[day][j];
                scheduleOrder[day][j] = scheduleOrder[day][j + 1];
                scheduleOrder[day][j + 1] = value;

            }
        }
    }
}

uint8_t *Schedule::locateClosest(uint8_t day, uint8_t hour, uint8_t min, uint8_t buffer[2])
{
    uint16_t a1 = hour * 12 + min / 5,
             a2,
             x = 288;
    uint8_t pos = 0, counter = 0;
            //s = 0;

    int b1 = 3, b2 = 3;

    for (int i = 0; i < 3; i++)
    {
        if (!scheduleTable[day][i].active)
        {
            counter++;
        }
        // else
        // {
        //     s = i;
        // }
        if (counter == 3)
        {
            buffer[0] = 25;
            runningFlag = false;
            return buffer;
        }
    }

    // if (counter == 2)
    // {
    //     pos = s;
    // }
    // else
    // {
    for (int i = 0; i < 3; i++)
    {
        if (!scheduleTable[day][i].active)
            continue;
        a2 = to24Hour(scheduleTable[day][i].startHr, scheduleTable[day][i].startPM) * 12 + scheduleTable[day][i].startMin / 5;
        if (a1 >= a2)
        {
            a2 = to24Hour(scheduleTable[day][i].endHr, scheduleTable[day][i].endPM) * 12 + scheduleTable[day][i].endMin / 5;
            if (a1 > a2 || a2 == a1)
            {
                continue;
            }
            b1 = 0;
        }

        uint16_t diff = (a2 > a1) ? (a2 - a1) : 0;
        if (diff < x)
        {
            b2 = (!b1) ? 0 : 1;
            pos = i;
            x = diff;
        }
    }
    //}
    if (b1 == 3 && b2 == 3)
    {
        buffer[0] = 25;
        runningFlag = false;
        return buffer;
    }
    else if (b2)
    {
        buffer[0] = to24Hour(scheduleTable[day][pos].startHr, scheduleTable[day][pos].startPM);
        buffer[1] = scheduleTable[day][pos].startMin;
        isInitial = true;
    }
    else
    {
        buffer[0] = to24Hour(scheduleTable[day][pos].endHr, scheduleTable[day][pos].endPM);
        buffer[1] = scheduleTable[day][pos].endMin;
        isInitial = false;
    }

    activeNum = pos;
    activeDay = day;
    runningFlag = true;

    return buffer;
}

uint8_t *Schedule::next(uint8_t day, uint8_t buffer[2])
{
    uint8_t c = 4;
    for (int i = 0, v = 0; i < 3; i++) {
        if (activeNum == scheduleOrder[day][i] && c == 4) {
            c = i;
        }
        if (scheduleTable[day][i].active) {
            v = 1;
        }
        if ((i == 2 && v == 0)|| (c == 2 && !isInitial)) {
            buffer[0] = 25;
            runningFlag = false;
            return buffer;
        }
    }
    if (isInitial)
    {
        isInitial = false;
        buffer[0] = to24Hour(scheduleTable[day][activeNum].endHr, scheduleTable[day][activeNum].endPM);
        buffer[1] = scheduleTable[day][activeNum].endMin;
        return buffer;
    }
    int hr1 = scheduleTable[day][scheduleOrder[day][c]].endHr,
        min1 = scheduleTable[day][scheduleOrder[day][c]].endMin,
        pm1 = scheduleTable[day][scheduleOrder[day][c]].endPM;

    for (int i = (c+1); i < 3; i++)
    {
        if (scheduleTable[day][scheduleOrder[day][i]].active)
        {
            activeNum = scheduleOrder[day][i];
            if (scheduleTable[day][activeNum].startHr == hr1 &&
                scheduleTable[day][activeNum].startMin == min1 &&
                scheduleTable[day][activeNum].startPM == pm1) {

                buffer[0] = to24Hour(scheduleTable[day][activeNum].endHr, scheduleTable[day][activeNum].endPM);
                buffer[1] = scheduleTable[day][activeNum].endMin;
                isInitial = false;
            }
            else {
                buffer[0] = to24Hour(scheduleTable[day][activeNum].startHr, scheduleTable[day][activeNum].startPM);
                buffer[1] = scheduleTable[day][activeNum].startMin;
                isInitial = true;
            }
            return buffer;
        }
    }

    buffer[0] = 25;
    runningFlag = false;
    //isInitial = false;  //Recently added, might have bugs
    return buffer;
}



bool Schedule::compare(uint8_t hour1, uint8_t minute1, bool pm1, uint8_t hour2, uint8_t minute2, bool pm2)
{
    uint8_t _24Hour1 = hour1 + 12 * pm1;
    uint8_t _24Hour2 = hour2 + 12 * pm2;

    if ((_24Hour1 == _24Hour2 && minute1 < minute2) || _24Hour1 < _24Hour2)
    {
        return false;
    }
    return true;
}




void Schedule::save(uint8_t data[168])
{
    for (int i = 0, counter = 0; i < 7; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            data[counter] = scheduleTable[i][j].startHr;
            data[counter + 1] = scheduleTable[i][j].startMin;
            data[counter + 2] = scheduleTable[i][j].startPM;
            data[counter + 3] = scheduleTable[i][j].endHr;
            data[counter + 4] = scheduleTable[i][j].endMin;
            data[counter + 5] = scheduleTable[i][j].endPM;
            data[counter + 6] = scheduleTable[i][j].active;
            data[counter + 7] = scheduleTable[i][j].empty;
            counter += 8;
        }
    }
}

void Schedule::load(uint8_t data[168])
{
    for (int i = 0, counter = 0; i < 7; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            scheduleTable[i][j].startHr = data[counter];
            scheduleTable[i][j].startMin = data[counter + 1];
            scheduleTable[i][j].startPM = data[counter + 2];
            scheduleTable[i][j].endHr = data[counter + 3];
            scheduleTable[i][j].endMin = data[counter + 4];
            scheduleTable[i][j].endPM = data[counter + 5];
            scheduleTable[i][j].active = data[counter + 6];
            scheduleTable[i][j].empty = data[counter + 7];
            counter += 8;
        }
    }
}

uint8_t* Schedule::getDeadline(uint8_t buffer[3], bool hour24){
    if(hour24)
    {
        buffer[0] = deadline[0];
        buffer[1] = deadline[1];
        buffer[2] = 0;
    }
    else{
        uint8_t temp[2] = {0};
        to12Hour(deadline[0],temp);
        buffer[0] = temp[0];
        buffer[1] = deadline[1];
        buffer[2] = temp[1];
    }
    return buffer;
}

uint8_t Schedule::to24Hour(uint8_t hour, bool pm)
{
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


uint8_t* Schedule::to12Hour(uint8_t hour, uint8_t buffer[2]){
    if(hour == 0 || hour == 24){
        buffer[0] = 12;
        buffer[1] = 0;
    }
    else if(hour<=12){
        buffer[0] = hour;
        buffer[1] = (hour == 12)?1:0;
    }
    else{
        buffer[0] = hour-12;
        buffer[1] = 1;
    }
    return buffer;
}

void Schedule::setDeadline(uint8_t buffer[2]){
    deadline[0] = buffer[0];
    deadline[1] = buffer[1];
}


uint8_t Schedule::getActiveDay(void)
{
    return activeDay;
}
uint8_t Schedule::getActiveNum(void)
{
    return activeNum;
}

void Schedule::disable(void)
{
    scheduleFlag = 0;
}
void Schedule::enable(void)
{
    scheduleFlag = 1;
}
void Schedule::toggleSchedule(void)
{
    scheduleFlag = !scheduleFlag;
}
bool Schedule::isEnable(void)
{
    return scheduleFlag;
}

bool Schedule::isRunning(void)
{
    return (!isInitial && runningFlag) ? true : false;
}

bool Schedule::isReschedule(void){
    return reschedule;
}

bool Schedule::isDeadline(void){
    return deadlineFlag;
}

void Schedule::clearRescheduleFlag(void){
    reschedule = false;
}

void Schedule::clearDeadlineFlag(void){
    deadlineFlag = false;
}
void Schedule::clearRunningFlag(void){
    runningFlag = false;
}

void Schedule::enableDeadlineFlag(void){
    deadlineFlag = true;
}

void Schedule::enableRunningFlag(void){
    runningFlag = true;
}

