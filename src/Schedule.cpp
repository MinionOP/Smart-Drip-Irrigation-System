#include "schedule.h"

Schedule::Schedule()
{
    for(int i=0;i<7;i++){
        for(int j=0;j<3;j++){
            scheduleOrder[i][j] = 3;
        }
    }
    update(0, 0, 1, 30, 1, 2, 30, 1);
    update(0, 1, 2, 25, 1, 3, 35, 1);
    update(0, 2, 11, 25, 0, 6, 35, 0);

    // update(0, 0, 7, 30, 1, 7, 35, 1);
    // update(0, 1, 7, 40, 1, 7, 45, 1);
    // update(0, 2, 7, 50, 1, 7, 55, 1);

    update(1, 0, 1, 30, 1, 6, 30, 1);
    update(1, 1, 2, 30, 1, 7, 15, 1);

    update(2, 0, 2, 30, 1, 6, 30, 1);
    update(2, 1, 5, 30, 0, 11, 15, 0);

    update(3, 0, 3, 30, 1, 6, 30, 1);
    update(3, 1, 11, 30, 1, 12, 30, 1);

    //Friday
    update(4, 0, 9, 0, 1, 9, 5, 1);
    update(4, 0, 9, 10, 1, 9, 15, 1);
    update(4, 0, 9, 20, 1, 9, 25, 1);

    //update(4, 1, 7, 30, 1, 12, 30, 1);

    update(5, 0, 5, 30, 1, 8, 45, 1);
    update(5, 1, 3, 30, 1, 8, 45, 1);

    update(6, 0, 6, 30, 1, 7, 30, 1);
    update(6, 1, 5, 30, 0, 7, 15, 0);

}

bool Schedule::update(uint8_t day, uint8_t num, uint8_t _startHr, uint8_t _startMin, bool _startPM, uint8_t _endHr, uint8_t _endMin, bool _endPM)
{
    if (
        (_startMin > 55) ||
        ( _endMin > 55) ||
        (_startMin != 0 && _startMin % 5 != 0) ||
        (_endMin != 0 && _endMin % 5 != 0) ||
        (_startHr == 0 || _endHr == 0) ||
        (_startHr == _endHr && _startMin == _endMin && _startPM == _endPM)
        )
    {
        return false;
    }
 
    if (!scheduleTable[day][num].empty)
    {
        clear(day, num);
    }
    // uint16_t startPos = to24Hour(_startHr, _startPM) * 12 + (_startMin / 5),
    //          endPos = to24Hour(_endHr, _endPM) * 12 + (_endMin / 5);

    // int duration = ((int)(endPos - startPos) > 0) ? (endPos - startPos) : (288 - startPos + endPos);

    // for (int i = 0, counter = 0; i <= duration; i++)
    // {
    //     if (i + startPos >= 288)
    //     {
    //         timeline[day][counter]++;
    //         counter++;
    //     }
    //     else
    //     {
    //         timeline[day][i + startPos]++;
    //     }
    // }
    uint16_t startPos = to24Hour(_startHr, _startPM) * 12 + (_startMin / 5),
             endPos = to24Hour(_endHr, _endPM) * 12 + (_endMin / 5),
             s2 = 0,
             e2 = 0;
    uint8_t  type = 0;

    bool setToInActive = false;

    for (int i = 0, x = 3; i < 3; i++)
    {
        if (scheduleTable[day][i].empty)
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
            if (startPos <= s2 && endPos >= e2)
            {
                scheduleTable[day][i].active = false;
                continue;
            }
            else if (startPos >= s2 && endPos <= e2)
            {
                setToInActive = true;
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
            if (startPos >= e2 && endPos >= e2)
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
                setToInActive = true;
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
            if (startPos <= s2 && endPos >= e2)
            {
                scheduleTable[day][i].active = false;
                continue;
            }
            else if (startPos >= s2 && endPos <= e2)
            {
                setToInActive = true;
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

        if (setToInActive)
        {
            break;
        }
        x = 3;
    }

    scheduleTable[day][num].startHr = _startHr;
    scheduleTable[day][num].startMin = _startMin;
    scheduleTable[day][num].startPM = _startPM;
    scheduleTable[day][num].endHr = _endHr;
    scheduleTable[day][num].endMin = _endMin;
    scheduleTable[day][num].endPM = _endPM;
    scheduleTable[day][num].active = (setToInActive)?false:true;
    scheduleTable[day][num].empty = false;

    reorder(day, num, _startHr, _startMin, _startPM);


    return true;
}

bool Schedule::disable(uint8_t day, uint8_t num)
{
    // bool temp = erase(day,num);
    // if(!temp){
    //     return false;
    // }
    scheduleTable[day][num].active = false;
    return true;
}

bool Schedule::clear(uint8_t day, uint8_t num)
{
    // bool temp = erase(day,num);
    // if(!temp){
    //     return false;
    // }
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

// bool Schedule::erase(uint8_t day, uint8_t num)
// {
//     if (scheduleTable[day][num].empty)
//     {
//         return true;
//     }
//     uint8_t _startHr = scheduleTable[day][num].startHr,
//             _startMin = scheduleTable[day][num].startMin,
//             _endHr = scheduleTable[day][num].endHr,
//             _endMin = scheduleTable[day][num].endMin;
//     bool    _startPM = scheduleTable[day][num].startPM,
//             _endPM = scheduleTable[day][num].endPM;

//     uint16_t startPos = to24Hour(_startHr, _startPM) * 12 + (_startMin / 5),
//              endPos = to24Hour(_endHr, _endPM) * 12 + (_endMin / 5);
//     int duration = ((int)(endPos - startPos) > 0) ? (endPos - startPos) : (288 - startPos + endPos);

//     for (int i = 0, counter = 0; i <= duration; i++)
//     {
//         if (i + startPos >= 288)
//         {
//             timeline[day][counter]--;
//             counter++;
//         }
//         else
//         {
//             timeline[day][i + startPos]--;
//         }
//     }
//     return true;
// }

// bool Schedule::isAvailable(uint8_t day, uint8_t hour, uint8_t min, uint8_t pm){
//     hour = to24Hour(hour,pm);
//     uint8_t time = hour*12 + min/5;
//     if(min%5 == 0){
//         if(timeline[day][time])
//             return true;
//     }
//     else{
//         if (timeline[day][time] && timeline[day][(time + 1) % 288])
//         {
//             return true;
//         }
//     }
//     return false;
// }


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

// uint16_t Schedule::timelineSize(uint8_t day)
// {
//     uint16_t counter = 0;
//     for (int i = 0; i < 288; i++)
//     {
//         if (timeline[day][i])
//         {
//             counter++;
//         }
//     }
//     return counter;
// }
void Schedule::clearOne(uint8_t day, uint8_t num)
{
    scheduleTable[day][num].active = 0;
}

void Schedule::reorder(uint8_t day, uint8_t num, uint8_t hour, uint8_t min, bool pm)
{
    uint16_t a1 = to24Hour(hour, pm) + min,
             num2 = 0,
             buf[3] = {0};

    uint8_t  counter = 0;

    for (int i = 0; i < 3; i++)
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
        buf[i] = to24Hour(scheduleTable[day][num2].startHr, scheduleTable[day][num2].startPM) + scheduleTable[day][num2].startMin;

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



uint8_t* Schedule::locateClosest(uint8_t day, uint8_t hour, uint8_t min, uint8_t buffer[2]) {
    uint16_t a1 = hour*12 + min / 5,
             a2,
             x = 288;
    uint8_t  pos = 0,
             counter = 0,
             s = 0;
    bool     b1 = true,
             b2 = true;

    for (int i = 0; i < 3; i++) {
        if(!scheduleTable[day][i].active) {
            counter++;
        }
        else{
            s = i;
        }
        if (counter == 3) {
            buffer[0] = 25;
            return buffer;
        }
    }

    if (counter == 2)
    {
        pos = s;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            if (!scheduleTable[day][i].active)
                continue;
            a2 = to24Hour(scheduleTable[day][i].startHr, scheduleTable[day][i].startPM) * 12 + scheduleTable[day][i].startMin / 5;
            if (a1 > a2)
            {
                a2 = to24Hour(scheduleTable[day][i].endHr, scheduleTable[day][i].endPM) * 12 + scheduleTable[day][i].endMin / 5;
                if (a1 > a2 || a2 == a1)
                {
                    continue;
                }
                b1 = false;
            }

            uint16_t diff = (a2 > a1) ? (a2 - a1) : 0;
            if (diff < x)
            {
                b2 = (!b1) ? false : true;
                pos = i;
                x = diff;
            }
        }
    }

    if(b2){
        buffer[0] = to24Hour(scheduleTable[day][pos].startHr, scheduleTable[day][pos].startPM);
	    buffer[1] = scheduleTable[day][pos].startMin;
        isInitial = true;
    }
    else{
        buffer[0] = to24Hour(scheduleTable[day][pos].endHr, scheduleTable[day][pos].endPM);
	    buffer[1] = scheduleTable[day][pos].endMin; 
        isInitial = false;
    }
   
    activeNum = pos;

    return buffer;
}

uint8_t* Schedule::next(uint8_t day, uint8_t buffer[2]) {
    if (isInitial) {
        isInitial = !isInitial;
        buffer[0] = to24Hour(scheduleTable[day][activeNum].endHr, scheduleTable[day][activeNum].endPM);
        buffer[1] = scheduleTable[day][activeNum].endMin;
        return buffer;
    }
    isInitial = true;
    for (int i = 0, i2 = 0, counter = 0; i < 3; i++, i2++) {
        if (scheduleOrder[day][i] == activeNum && scheduleTable[day][scheduleOrder[day][i]].active) {
            activeNum = scheduleOrder[day][(i + 1) % 3];
            while (activeNum == 3) {
                activeNum = scheduleOrder[day][(i2 + 1) % 3];
                if (counter == 3) {
                    buffer[0] = 25;
                    return buffer;
                }
                counter++;
                i2++;
            }
			buffer[0] = to24Hour(scheduleTable[day][activeNum].startHr, scheduleTable[day][activeNum].startPM);
			buffer[1] = scheduleTable[day][activeNum].startMin;
            return buffer;
        }
    }
    buffer[0] = 25;
    return buffer;
}

bool Schedule::isRunning(void){
    return (!isInitial)?true:false;
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

// uint8_t *Schedule::getTimeline(uint8_t day, uint8_t buffer[288])
// {
//     for (int i = 0; i < 288; i++)
//     {
//         buffer[i] = timeline[day][i];
//     }
//     return buffer;
// }


void Schedule::save(uint8_t data[168]){
    for(int i=0, counter = 0;i<7;i++){
        for(int j=0;j<3;j++){
            data[counter] = scheduleTable[i][j].startHr;
            data[counter + 1] = scheduleTable[i][j].startMin;
            data[counter + 2] = scheduleTable[i][j].startPM;
            data[counter + 3] = scheduleTable[i][j].endHr;
            data[counter + 4] = scheduleTable[i][j].endMin;
            data[counter + 5] = scheduleTable[i][j].endPM;
            data[counter + 6] = scheduleTable[i][j].active;
            data[counter + 7] = scheduleTable[i][j].empty;
            counter +=8;
        }
    }
}

void Schedule::load(uint8_t data[168]){
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

void Schedule::disable(void){
    scheduleFlag = 0;
}
void Schedule::enable(void){
    scheduleFlag = 1;
}
void Schedule::toggleSchedule(void){
    scheduleFlag = !scheduleFlag;
}
bool Schedule::isEnable(void){
    return scheduleFlag;
}



