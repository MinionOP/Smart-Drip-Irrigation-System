#include "user_interface.h"
#include <LiquidCrystal_I2C.h>

UserInterface::UserInterface(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows) : lcd(lcd_addr, lcd_cols, lcd_rows)
{
    //Remember to also update "printDisplay"
    AddMenuInfo(MAIN_LCD, 0, 4, 0, 0, false, 0);
    AddMenuInfo(MENU_LCD, 0, 4, 0, 0, true, 0);
    AddMenuInfo(VALVE_SENSOR_LCD, 0, 4, 0, 0, true, 0);
    AddMenuInfo(CROP_LCD, 0, 10, 0, 0, true, 0);
    AddMenuInfo(TIME_LCD, 0, 4, 0, 0, true, 0);
    AddMenuInfo(CROP_THRESHOLD_LCD, 0, 11, 0, 0, true, 0);
    AddMenuInfo(THRESHOLD_NUM_LCD, 7, -1, -1, 10, false, 1);
    AddMenuInfo(SCHEDULE_LCD, 0, 10, 0, 0, true, 0);
    AddMenuInfo(SCHEDULE_LCD2, 0, 4, 0, 0, true, 0);
    AddMenuInfo(SCHEDULE_LCD3, 0, 4, 0, 0, true, 0);
    AddMenuInfo(SCHEDULE_LCD4, 1, 4, 0, 0, true, 0);

    interfaceTable[MAIN_LCD] = &UserInterface::printMainLCD;
    interfaceTable[MENU_LCD] = &UserInterface::printMenuLCD;
    interfaceTable[VALVE_SENSOR_LCD] = &UserInterface::printValveSensorLCD;
    interfaceTable[CROP_LCD] = &UserInterface::printCropLCD;
    interfaceTable[TIME_LCD] = &UserInterface::printTimeLCD;
    interfaceTable[CROP_THRESHOLD_LCD] = &UserInterface::printCropThresholdLCD;
    interfaceTable[THRESHOLD_NUM_LCD] = &UserInterface::printThreshholdNumLCD;
    interfaceTable[SCHEDULE_LCD] = &UserInterface::printScheduleLCD;
    interfaceTable[SCHEDULE_LCD2] = &UserInterface::printScheduleLCD2;
    interfaceTable[SCHEDULE_LCD3] = &UserInterface::printScheduleLCD3;
    interfaceTable[SCHEDULE_LCD4] = &UserInterface::printScheduleLCD4;

    resetLCDValue();
    idleStatus = false;
    currentDisplay = MAIN_LCD;
}

void UserInterface::begin(void)
{
    lcd.begin();
    lcd.backlight();
}
void UserInterface::update(uint8_t type, uint8_t num)
{
    switch (type)
    {
    case DOWN:
    {
        (!cursor[MODE]) ? updateCursor(-1) : updateDigit(-1);
        return;
    }
    case UP:
    {
        (!cursor[MODE]) ? updateCursor(1) : updateDigit(1);
        return;
    }
    case SELECT:
    {
        selectButton();
        return;
    }
    case SOIL:
    {
        if (currentDisplay == VALVE_SENSOR_LCD)
        {
            for (int i = 0; i < 3; i++)
            {
                if (database.isValveAvailable(i))
                {
                    uint8_t soilTemp = database.soilSensor(i);
                    uint8_t valveTemp = database.valveStatus(i);
                    lcdSP(16, i + 1, soilTemp);

                    (soilTemp > 9) ? lcdSP(18, i + 1, "%") : lcdSP(17, i + 1, "%");
                    (valveTemp == 0) ? lcdSP(4, i + 1, "x") : lcdSP(4, i + 1, " ");
                }
            }
        }
        else if (currentDisplay == MAIN_LCD)
        {
            for (int i = 0, col_temp = 3; i < 3; i++)
            {
                uint8_t soilTemp = database.soilSensor(i);
                lcdSP(col_temp, 2, soilTemp);
                lcd.print('%');
                col_temp += 7;
            }
        }
        //lcdSP(0,0,"SOIL");

        return;
    }
    case VALVE:
    {
        if (currentDisplay == MAIN_LCD)
        {
            for (int i = 0, col_temp = 3; i < 3; i++)
            {
                if (database.isValveAvailable(i))
                {
                    uint8_t valveTemp = database.valveStatus(i);
                    printValveStatus(valveTemp, 0, col_temp, 3);
                }
                else
                {
                    lcdSP(col_temp, 3, 'X');
                }
                col_temp = col_temp + 7;
            }
        }
        //lcdSP(0,1,"VALE");

        return;
    }
    case TIME:
    {

        if (currentDisplay == MAIN_LCD)
        {
            uint8_t dayTemp = database.getDayOfWeek();
            printDay(dayTemp, 0, 0);
            uint8_t col_temp = (dayTemp == 0 || dayTemp == 4 || dayTemp == 6) ? 7 : (dayTemp == 3 || dayTemp == 5) ? 9
                                                                                : (dayTemp == 1)                   ? 8
                                                                                                                   : 10;

            printTime(database.getHour(), database.getMinute(), database.getPM(), col_temp, 0);
        }
        //lcdSP(0,2,"TIME");

        return;
    }
    case TEMPERATURE:
    {
        if (currentDisplay == MAIN_LCD)
        {

            uint8_t col_temp = (database.temperature() < 100) ? 13 : 12;
            lcdSP(col_temp, 1, '|');
            lcdSP(col_temp + 1, 1, database.temperature());
            lcd.print("F|");
        }

        //lcdSP(0,3,"TEMP");

        return;
    }
    case SCHEDULE_STATUS:
    {
        lcd.setCursor(18, 1);
        if (currentDisplay == MAIN_LCD)
        {
            if (schedule.isRunning())
            {
                lcd.print("SR");
            }
            else if (schedule.isEnable())
            {
                lcd.print("SE");
            }
            else
            {
                lcd.print("SD");
            }
        }
        //lcdSP(7,3,"ZCGa");

        return;
    }
    }
}

uint8_t UserInterface::getCurrentDisplay(void)
{
    return currentDisplay;
}
uint8_t UserInterface::getCursorPosition(uint8_t type)
{
    return cursor[type];
}

void UserInterface::AddMenuInfo(uint8_t display, uint8_t _initialPos, uint8_t _totalRows, uint8_t _totalPages, uint8_t _lastPos, bool _isCircular, bool _cursorType)
{
    //Remember to also update "printDisplay"
    menu[display].initialPos = _initialPos;
    menu[display].totalRows = _totalRows;
    menu[display].isCircular = _isCircular;
    if (_cursorType == 0)
    {
        menu[display].cursorType = _cursorType;
        menu[display].totalPages = _totalRows / 4;
        menu[display].lastPos = (_totalRows % 4) - 1;
    }
    else
    {
        menu[display].cursorType = _cursorType;
        menu[display].totalPages = _totalPages;
        menu[display].lastPos = _lastPos;
    }
}
void UserInterface::printMainLCD(void)
{
    currentDisplay = MAIN_LCD;
    lcd.clear();

    lcd.setCursor(19, 0);
    lcdSP(0, 1, "Battery:");
    lcdSP(0, 2, "S1:");
    lcdSP(7, 2, "S2:");
    lcdSP(14, 2, "S3:");
    lcdSP(0, 3, "V1:");
    lcdSP(7, 3, "V2:");
    lcdSP(14, 3, "V3:");


    if(isInitialBootup){
        lcdSP(18,1,"SD");
        isInitialBootup = false;
        return;
    }


    lcd.setCursor(18, 1);
    if (schedule.isRunning())
    {
        lcd.print("SR");
    }
    else if (schedule.isEnable())
    {
        lcd.print("SE");
    }
    else
    {
        lcd.print("SD");
    }

    for (int i = 0, col_temp = 3; i < 3; i++)
    {
        uint8_t soilTemp = database.soilSensor(i);
        lcdSP(col_temp, 2, soilTemp);
        lcd.print("%");
        if (database.isValveAvailable(i))
        {
            uint8_t valveTemp = database.valveStatus(i);

            printValveStatus(valveTemp, 0, col_temp, 3);
        }
        else
        {
            lcdSP(col_temp, 3, 'X');
        }
        col_temp = col_temp + 7;
    }

    uint8_t dayTemp = database.getDayOfWeek();
    if (dayTemp != 9)
    {
        printDay(dayTemp, 0, 0);
        uint8_t col_temp = (dayTemp == 0 || dayTemp == 4 || dayTemp == 6) ? 7 : (dayTemp == 3 || dayTemp == 5) ? 9
                                                                            : (dayTemp == 1)                   ? 8
                                                                                                               : 10;

        printTime(database.getHour(), database.getMinute(), database.getPM(), col_temp, 0);
    }

    uint8_t col_temp = (database.temperature() < 100) ? 13 : 12;
    lcdSP(col_temp, 1, '|');
    lcdSP(col_temp + 1, 1, database.temperature());
    lcd.print("F|");

    resetLCDValue();
}
void UserInterface::printMenuLCD(void)
{

    lcd.clear();
    //(!skipMarker) ? lcdSP(0, 0, ">Valve Setting") : lcdSP(0, 0, " Valve Setting");
    if (currentDisplay == MENU_LCD)
    {
        lcdSP(0, 0, " Valve Setting");
    }
    else
    {
        lcdSP(0, 0, ">Valve Setting");
    }

    //lcdSP(0, 1, " Sensor Setting");
    lcdSP(0, 1, " Edit Crop Values");
    lcdSP(0, 2, " Set Time/Schedule");
    lcdSP(0, 3, " Go to Main Display");

    //debugging-----------------------------------------
    // uint8_t buffer[8] = {0};

    // schedule.getInfo(schedule.getActiveDay(), schedule.getActiveNum(), buffer);

    // lcd.setCursor(0,0);
    // for(int i=0;i<8;i++){
    //     lcd.print(buffer[i]);
    //     lcd.print(' ');
    // }
    // lcdSP(0,1,schedule.getActiveDay());
    // lcdSP(0,2,schedule.getActiveNum());

    //debugging-----------------------------------------

    page = 0;
    cursor[ROW] = 0;
    currentDisplay = MENU_LCD;
}
void UserInterface::printValveSensorLCD(void)
{

    lcd.clear();
    (!skipMarker) ? lcdSP(0, 0, ">Go back") : lcdSP(0, 0, " Go back");
    lcdSP(14, 0, "Sensor");
    lcdSP(0, 1, " V1");
    lcdSP(0, 2, " V2");
    lcdSP(0, 3, " V3");

    for (int i = 0; i < 3; i++)
    {
        if (database.isValveAvailable(i))
        {
            uint8_t soilTemp = database.soilSensor(i);
            uint8_t valveTemp = database.valveStatus(i);

            lcdSP(16, i + 1, soilTemp);
            printCrop(database.crop(i), 7, i + 1);
            (valveTemp == 0) ? lcdSP(3, i + 1, "|x|") : lcdSP(3, i + 1, "| |");
            (soilTemp > 9) ? lcdSP(18, i + 1, "%") : lcdSP(17, i + 1, "%");
        }
        else
        {
            lcdSP(5, i + 1, "--Disabled--");
        }
    }

    cursor[ROW] = 0;
    page = 0;
    currentDisplay = VALVE_SENSOR_LCD;
}
void UserInterface::printCropLCD(void)
{
    lcd.clear();
    switch (page)
    {
    case 0:
    {
        // lcdSP(0, 0, "Curr Crop:");
        // printCrop(database.crop(database.getSelectedCropNum()));
        // lcdSP(0, 1, "Select:");
        // (!skipMarker) ? lcdSP(0, 2, ">") : lcdSP(0, 2, " ");
        // printCrop(0);
        // lcd.setCursor(1, 3);
        // printCrop(1);
        // lcd.setCursor(11, 3);
        // lcd.print("v");
        // //lcd.write(0);
        // cursor[ROW] = 2;
        // break;
        database.isValveAvailable(database.getSelectedValveNum()) ? lcdSP(0, 0, " Disable Valve") : lcdSP(0, 0, " Enable Valve");
        (!skipMarker) ? lcdSP(0, 0, ">") : lcdSP(0, 0, " ");
        lcdSP(0, 1, " Select: |Current");
        lcdSP(9, 2, "|crop is");
        lcdSP(9, 3, "|        v");
        printCrop(database.crop(database.getSelectedCropNum()), 10, 3);
        printCrop(0, 1, 2);
        printCrop(1, 1, 3);

        //lcd.write(0);
        cursor[ROW] = 0;
        break;
    }
    case 1:
    {
        lcd.setCursor(1, 0);
        printCrop(2, 1, 0);
        printCrop(3, 1, 1);
        printCrop(4, 1, 2);
        printCrop(5, 1, 3);
        lcd.setCursor(18, 3);
        lcd.print("v");

        //lcd.write(0);
        cursor[ROW] = 0;
        break;
    }
    case 2:
    {
        printCrop(6, 1, 0);
        lcdSP(1, 1, "Go back");
        lcd.setCursor(18, 0);
        //lcd.write(1);
        lcd.print("^");

        cursor[ROW] = 0;
        break;
    }
    }
    currentDisplay = CROP_LCD;
}
void UserInterface::printCropThresholdLCD(void)
{
    lcd.clear();
    switch (page)
    {
    case 0:
    {
        (!skipMarker) ? lcdSP(0, 0, ">Select to restore") : lcdSP(0, 0, " Select to restore");
        lcdSP(0, 1, " default settings");
        lcdSP(0, 2, " Crop");
        lcdSP(9, 2, "Threshold");
        printCrop(0, 1, 3);
        lcdSP(12, 3, database.cropThreshold(0));
        lcd.print('%');
        lcd.setCursor(18, 3);
        lcd.print("v");

        //lcd.write(0);
        break;
    }
    case 1:
    {
        printCrop(1, 1, 0);
        lcdSP(12, 0, database.cropThreshold(1));
        lcd.print('%');
        printCrop(2, 1, 1);
        lcdSP(12, 1, database.cropThreshold(2));
        lcd.print('%');
        printCrop(3, 1, 2);
        lcdSP(12, 2, database.cropThreshold(3));
        lcd.print('%');
        printCrop(4, 1, 3);
        lcdSP(12, 3, database.cropThreshold(4));
        lcd.print('%');
        lcd.setCursor(18, 3);
        //lcd.write(0);
        lcd.print("v");

        break;
    }
    case 2:
    {
        printCrop(5, 1, 0);
        lcdSP(12, 0, database.cropThreshold(5));
        lcd.print('%');
        printCrop(6, 1, 1);
        lcdSP(12, 1, database.cropThreshold(6));
        lcd.print('%');
        lcdSP(1, 2, "Go back");
        lcd.setCursor(18, 0);
        //lcd.write(1);
        lcd.print("^");

        break;
    }
    }
    cursor[ROW] = 0;
    currentDisplay = CROP_THRESHOLD_LCD;
}
void UserInterface::printThreshholdNumLCD(void)
{
    createCustomSymbols(1);
    lcd.clear();
    lcdSP(0, 0, "Curr Thresh: ");
    lcd.print(database.cropThreshold(database.getSelectedCropNum()));
    lcd.print("%");
    printCrop(database.getSelectedCropNum(), 0, 3);
    lcdSP(16, 3, "Done");
    printDigit(database.cropThreshold(database.getSelectedCropNum()), 7, 1);
    currentDisplay = THRESHOLD_NUM_LCD;
    cursor[COLUMN] = menu[currentDisplay].initialPos;
    lcdSP(cursor[COLUMN], 3, "---");
}

void UserInterface::printTimeLCD(void)
{

    uint8_t hour = globalBuffer[0] * 10 + globalBuffer[1],
            min = globalBuffer[2] * 10 + globalBuffer[3];
    bool pm = globalBuffer[4];
    
    if(currentDisplay == TIME_LCD){
        printTime(hour, min, pm, 1, 1);
    }
    lcd.clear();
    (!skipMarker) ? lcdSP(0, 0, ">") : lcdSP(0, 0, " ");
    currentDisplay = TIME_LCD;

    lcdSP(1, 0, globalBuffer[10]);
    lcdSP(2, 0, globalBuffer[11]);
    lcd.print('/');
    lcdSP(4, 0, globalBuffer[12]);
    lcdSP(5, 0, globalBuffer[13]);
    lcd.print("/20");
    lcdSP(9, 0, globalBuffer[14]);
    lcdSP(10, 0, globalBuffer[15]);

    printTime(hour, min, pm, 1, 1);
    lcdSP(1, 3, "Go back");

    cursor[COLUMN] = menu[currentDisplay].initialPos;
}

void UserInterface::printScheduleLCD(void)
{

    if (currentDisplay == SCHEDULE_LCD && cursor[ROW] == 0 && page == 0 && pressed == 1)
    {
        (schedule.isEnable()) ? lcdSP(8, 2, "ENABLED ") : lcdSP(8, 2, "DISABLED");
        return;
    }

    lcd.clear();
    currentDisplay = SCHEDULE_LCD;
    switch (page)
    {
    case 0:
    {
        //(!skipMarker) ? lcdSP(0, 0, ">M |") : lcdSP(0, 0, " M |");
        (!skipMarker) ? lcdSP(0, 0, ">Enable/Disable") : lcdSP(0, 0, " Enable/Disable");
        (schedule.isEnable()) ? lcdSP(8, 2, "ENABLED ") : lcdSP(8, 2, "DISABLED");

        lcdSP(0, 1, " M | Scheduling");

        lcdSP(0, 2, " T | is");
        lcdSP(0, 3, " W |");
        lcd.setCursor(19, 3);
        //lcd.write(0);
        lcd.print("v");

        break;
    }
    case 1:
    {
        lcdSP(0, 0, " Th|");
        lcdSP(0, 1, " F |");
        lcdSP(0, 2, " S |");
        lcdSP(0, 3, " Su|");
        lcdSP(19, 3, "v");

        break;
    }
    case 2:
    {
        lcdSP(0, 0, " Adjust Date/Time");
        lcdSP(0, 1, " Go back");
        lcdSP(19, 0, "^");

        break;
    }
    }
    cursor[ROW] = 0;
}
void UserInterface::printScheduleLCD2(void)
{
    lcd.clear();
    uint8_t date = database.getSelectedDate(0);
    currentDisplay = SCHEDULE_LCD2;
    (pressed) ? lcdSP(0, 0, ">  Go back") : lcdSP(2, 0, " Go back");
    printSchedule(date, 0, 3, 3, 1);

    lcdSP((date == 3 || date == 5 || date == 6) ? (18) : 19, 0,
                   (date == 0) ? "M" : (date == 1) ? "T"
                                   : (date == 2)   ? "W"
                                   : (date == 3)   ? "Th"
                                   : (date == 4)   ? "F"
                                   : (date == 5)   ? "Sa"
                                   : (date == 6)   ? "Su"
                                                   : "X");

    cursor[ROW] = 0;
}
void UserInterface::printScheduleLCD3(void)
{
    if (currentDisplay == SCHEDULE_LCD3)
    {
        toggleActiveTemp ? lcdSP(0, 0, ">Disable |") : lcdSP(0, 0, ">Enable  |");
        toggleActiveTemp ? lcdSP(19, 0, "*") : lcdSP(19, 0, " ");

        return;
    }
    lcd.clear();
    currentDisplay = SCHEDULE_LCD3;
    if (pressed)
    {
        toggleActiveTemp ? lcdSP(0, 0, ">Disable |") : lcdSP(0, 0, ">Enable  |");
        toggleActiveTemp ? lcdSP(19, 0, "*") : lcdSP(19, 0, " ");
    }
    else
    {
        toggleActiveTemp ? lcdSP(0, 0, " Disable |") : lcdSP(0, 0, " Enable  |");
        toggleActiveTemp ? lcdSP(19, 0, "*") : lcdSP(19, 0, " ");
    }

    uint8_t day = database.getSelectedDate(0),
            num = database.getSelectedDate(1);

    lcdSP(0, 1, " Delete  |");
    lcdSP(0, 2, " Edit    |");
    lcdSP(0, 3, " Go back |");
    (num != 2) ? printDay(day, 11, 0) : printDay(day, 10, 0);
    printSchedule(day, num, num + 1, 11, 1);

    cursor[ROW] = 0;
}
void UserInterface::printScheduleLCD4(void)
{
    lcd.clear();
    currentDisplay = SCHEDULE_LCD4;
    (pressed) ? lcdSP(0, 1, ">") : lcdSP(0, 1, " ");

    uint8_t day = database.getSelectedDate(0),
            num = database.getSelectedDate(1);

    printDay(day, 1, 0);
    printSchedule(day, num, num + 1, 1, 1);
    lcdSP(0, 3, " Go back");

    cursor[ROW] = 1;
}

void UserInterface::printDisplay(uint8_t display)
{
    (this->*interfaceTable[display])();
}
void UserInterface::printValveStatus(uint8_t status, bool printType, uint8_t col, uint8_t row)
{
    lcd.setCursor(col, row);
    if (!printType)
    {
        if (status == 0)
        {
            lcd.print("C");
        }
        else
        {
            lcd.print("O");
        }
    }
    else
    {
        if (status == 0)
        {
            lcd.print("Close");
        }
        else
        {
            lcd.print("Open");
        }
    }
}
void UserInterface::printTime(uint8_t hour, uint8_t minute, bool isPM, uint8_t col, uint8_t row)
{
    if (currentDisplay == TIME_LCD && hour < 10)
    {
        lcdSP(1, 1, '0');
        lcdSP(2, 1, hour);
    }
    else
    {    
        lcd.setCursor(col, row);
        lcd.print(hour, DEC);
    }
    lcd.print(':');

    if (minute < 10)
    {
        lcd.print('0');
    }
    lcd.print(minute, DEC);
    if (isPM)
    {
        lcd.print("pm ");
    }
    else
    {
        lcd.print("am ");
    }
}
void UserInterface::printSchedule(uint8_t day, uint8_t startNum, uint8_t endNum, uint8_t col, uint8_t row)
{
    if (endNum - startNum > 3 || endNum > 3)
    {
        return;
    }
    uint8_t buffer[8];

    for (int i = startNum; i < endNum; i++)
    {
        if (currentDisplay != SCHEDULE_LCD4 && currentDisplay != SCHEDULE_LCD3)
        {
            // Serial.print("1 ");
            schedule.getInfo(day, i, buffer);
            // for(int i=0;i<8;i++){
            //     Serial.print((String)buffer[i] + " ");
            // }
            // Serial.println();
        }
        else
        {
            // for(int i=0;i<8;i++){
            //     Serial.print((String)globalBuffer[i] + " ");
            // }
            // Serial.println();

            buffer[0] = globalBuffer[0] * 10 + globalBuffer[1];
            buffer[1] = globalBuffer[2] * 10 + globalBuffer[3];
            buffer[2] = globalBuffer[4];
            buffer[3] = globalBuffer[5] * 10 + globalBuffer[6];
            buffer[4] = globalBuffer[7] * 10 + globalBuffer[8];
            buffer[5] = globalBuffer[9];
            buffer[6] = schedule.isDayActive(database.getSelectedDate(0), database.getSelectedDate(1));
            buffer[7] = false;
            //Serial.print("2 ");
        }

        if (buffer[7] == false)
        {
            if (buffer[6] == true && currentDisplay == SCHEDULE_LCD2)
            {
                lcdSP(col - 1, row, "*");
            }
            uint8_t temp = (buffer[0] < 10) ? col + 1 : col;
            if (buffer[0] < 10 && currentDisplay == SCHEDULE_LCD4)
            {
                lcdSP(col, row, "0");
            }
            printTime(buffer[0], buffer[1], buffer[2], temp, row);
            switch (currentDisplay)
            {
            case SCHEDULE_LCD:
            {
                lcdSP(11, row, "-");
                break;
            }
            case SCHEDULE_LCD2:
            {
                lcdSP(10, row, "-");
                break;
            }
            case SCHEDULE_LCD3:
            {
                lcdSP(13, 2, "to");
                col = 3;
                row = 3;
                break;
            }
            case SCHEDULE_LCD4:
            {
                lcdSP(9, row, "-");
                col = col + 2;
                break;
            }
            }
            uint8_t col_temp = col;
            if (buffer[3] < 10)
            {
                if (currentDisplay != SCHEDULE_LCD3)
                    lcdSP(col_temp + 8, row, "0");
                col_temp++;
            }
            printTime(buffer[3], buffer[4], buffer[5], col_temp + 8, row);
            row += 1;
        }
        else if (currentDisplay == SCHEDULE_LCD2 || currentDisplay == SCHEDULE_LCD)
        {
            lcd.setCursor((currentDisplay == SCHEDULE_LCD) ? col + 1 : col, row);
            lcd.print("No entry found");
            row += 1;
        }
    }
}
void UserInterface::printDigit(uint8_t num, uint8_t col, uint8_t row)
{
    if (num < 100 && num >= 0)
    {
        uint8_t numOfLoop = 1;
        uint8_t temp = 0;
        if (num > 9)
        {
            temp = num;
            numOfLoop = 2;
            num = num / 10;
        }
        for (int i = 0; i < numOfLoop; i++)
        {
            lcd.setCursor(col, row);
            for (int i = 0; i < 3; i++)
            {
                lcd.write(customCharacter.bigNums[num][i]);
            }
            lcd.setCursor(col, row + 1);
            for (int i = 3; i < 6; i++)
            {
                lcd.write(customCharacter.bigNums[num][i]);
            }
            num = temp % (num * 10);
            col = col + 3;
        }
    }
}

void UserInterface::createCustomSymbols(uint8_t set)
{
    if (customCharacter.getCharacterSet() == set)
    {
        return;
    }
    customCharacter.setCharacterSet(set);
    switch (set)
    {
    case 0:
    {
        for (int i = 0; i < customCharacter.getSize(); i++)
        {
            lcd.createChar(i, customCharacter.getCharacter(i));
        }
        return;
    }
    case 1:
    {
        for (int i = 0; i < 8; i++)
        {
            lcd.createChar(i, customCharacter.getCharacter(i));
        }
        return;
    }
    }
}

bool UserInterface::isIdle(void)
{
    return idleStatus;
}
void UserInterface::idle(void)
{
    if (skip == 1)
    {
        skip = 0;
    }
    else
    {
        idleStatus = true;
        menu[currentDisplay].cursorType = 0;
        printMainLCD();
        lcd.noBacklight();
        if (schedule.isEnable())
        {
            lcd.noDisplay();
            noDisplay = true;
        }
    }
}
void UserInterface::wakeup(void)
{
    lcd.backlight();
    if (noDisplay)
    {
        lcd.display();
        noDisplay = false;
    }
    idleStatus = false;
}
void UserInterface::resetLCDValue(void)
{
    for (int i = 0; i < 25; i++)
    {
        globalBuffer[i] = -1;
    }
    cursor[ROW] = 0;
    cursor[COLUMN] = 0;
    cursor[MODE] = 0;
    cursor[BLINK] = 1;
    page = 0;
}

void UserInterface::updateCursor(int8_t direction)
{
    skipMarker = 1;
    pressed = 0;

    if (menu[currentDisplay].cursorType == 0)
    {
        //Set to currect cursor location
        lcd.setCursor(0, cursor[ROW]);
        //Erase old cursor
        lcd.print(' ');
        if (currentDisplay == MAIN_LCD)
        {
            printMenuLCD();
            return;
        }
        if (menu[currentDisplay].totalRows < 5)
        {
            if (cursor[ROW] == menu[currentDisplay].initialPos && direction == 1)
            {
                cursor[ROW] = menu[currentDisplay].totalRows - 1;
            }
            else if (cursor[ROW] == 3 && direction == -1)
            {
                cursor[ROW] = menu[currentDisplay].initialPos;
            }
            else
            {
                cursor[ROW] = (cursor[ROW] - direction) % menu[currentDisplay].totalRows;
                if ((currentDisplay == SCHEDULE_LCD4 || currentDisplay == TIME_LCD) && cursor[ROW] == 2)
                {
                    cursor[ROW] = (direction == -1) ? 3 : 1;
                }
            }
        }
        //-----------------------------------------------------------------------------------------------------------------------
        //More than 4 rows, will be more than 1 page
        else
        {
            int row = cursor[ROW];
            int directionTemp = direction * -1;
            lcdSP(0, row, " ");
            row = row + directionTemp;
            if (page == 0)
            {
                if (currentDisplay == CROP_THRESHOLD_LCD && (row == 1 || row == 2))
                {
                    row = (directionTemp == 1) ? 3 : 0;
                }

                if (currentDisplay == CROP_LCD && (row == 1))
                {
                    row = (directionTemp == 1) ? 2 : 0;
                }

                if (row < menu[currentDisplay].initialPos)
                {
                    page = menu[currentDisplay].totalPages;
                    printDisplay(currentDisplay);
                    lcdSP(0, menu[currentDisplay].lastPos, ">");
                    cursor[ROW] = menu[currentDisplay].lastPos;
                    return;
                }
            }
            if (page == menu[currentDisplay].totalPages && row > menu[currentDisplay].lastPos)
            {
                page = 0;
                printDisplay(currentDisplay);
                lcdSP(0, menu[currentDisplay].initialPos, ">");
                cursor[ROW] = menu[currentDisplay].initialPos;
                return;
            }
            uint8_t temp = page;
            page = (row > 3) ? (page + 1) : (row < 0) ? (page - 1)
                                                      : page;

            if (page != temp)
            {
                printDisplay(currentDisplay);
            }
            cursor[ROW] = (row < 0) ? (3) : (row > 3) ? (0)
                                                      : row;
        }
        lcdSP(0, cursor[ROW], ">");

        if (currentDisplay == SCHEDULE_LCD)
        {

            if (page == 2 || (cursor[ROW] == 0 && page == 0))
            {
                if (cursor[ROW] == 0 && page == 0)
                {
                    lcdSP(4, 1, " Scheduling     ");
                    lcdSP(4, 2, " is             ");
                    (schedule.isEnable()) ? lcdSP(8, 2, "ENABLED ") : lcdSP(8, 2, "DISABLED");
                    lcdSP(5, 3, "               ");
                }
                return;
            }

            lcdSP(4, 1, "               ");
            lcdSP(4, 2, "               ");
            lcdSP(4, 3, "               ");
            if (page == 1)
            {
                lcdSP(4, 0, "               ");
            }

            uint8_t iteration = (page == 1) ? (3) : 2;
            uint8_t rowTemp = (page == 0) ? (1) : 0;
            printDay((cursor[ROW] + page * 4) - 1, 4, rowTemp);
            lcd.print(':');
            printSchedule((cursor[ROW] + page * 4) - 1, 0, iteration, 4, rowTemp + 1);
        }
    }
    //-----------------------------------------------------------------------------------------------------------------------
    else
    {
        if (currentDisplay == THRESHOLD_NUM_LCD)
        {
            updateCursor2(direction, 0);
        }
        else if (currentDisplay == TIME_LCD)
        {
            updateCursor2(direction, 1);
        }
        else
        {
            updateCursor2(direction, 2);
        }
    }
}
void UserInterface::updateCursor2(int direction, uint8_t set)
{

    if (currentDisplay == THRESHOLD_NUM_LCD)
    {
        if ((cursor[COLUMN] == menu[currentDisplay].initialPos && direction == -1) || (cursor[COLUMN] == 15 && direction == 1))
        {
            return;
        }
    }
    else if (currentDisplay == SCHEDULE_LCD4)
    {
        if ((cursor[COLUMN] == 2 && direction == -1) || (cursor[COLUMN] == 19 && direction == 1))
        {
            return;
        }
    }
    else if (currentDisplay == TIME_LCD)
    {
        if (cursor[ROW] == 1)
        {
            if ((cursor[COLUMN] == 2 && direction == -1) || (cursor[COLUMN] == 12 && direction == 1))
            {
                return;
            }
        }
        else if (cursor[ROW] == 0)
        {
            if ((cursor[COLUMN] == 2 && direction == -1) || (cursor[COLUMN] == 13 && direction == 1))
            {
                return;
            }
        }
    }

    uint8_t col_temp = 0,
            row_temp = 0,
            iteration = 0,
            counter = 0;
    uint8_t arr[26] = {7, 10, 15,
                       0, 3, 7, 10, 13, 15,
                       2, 4, 5, 6, 12, 14, 15, 16, 19,
                       2, 4, 5, 9, 10, 13};

    if (set == 0)
    {
        iteration = 3;
        col_temp = 7;
        row_temp = 3;
    }
    else if (set == 1)
    {
        if (cursor[ROW] == 1)
        {
            col_temp = 1;
            row_temp = 2;
            iteration = 5;
            counter = 9;
        }
        else
        {
            col_temp = 1;
            row_temp = 1;
            iteration = 6;
            counter = 18;
        }
    }
    else
    {
        col_temp = 1;
        row_temp = 2;
        iteration = 9;
        counter = 9;
    }

    lcd.setCursor(col_temp, row_temp);

    for (int i = 0; i < iteration; i++)
    {
        if (cursor[COLUMN] == arr[counter + i])
        {
            lcd.print((direction == -1) ? cursorArray[counter + i - 1] : cursorArray[counter + i + 1]);
            cursor[COLUMN] = (direction == -1) ? arr[counter + i - 1] : arr[counter + i + 1];
            return;
        }
    }
}
void UserInterface::updateDigit(int8_t direction)
{
    uint8_t tempPos = 0;
    if (currentDisplay == THRESHOLD_NUM_LCD)
    {
        tempPos = (cursor[COLUMN] == menu[currentDisplay].initialPos) ? 0 : 1;
        if (!((tempArray[tempPos] == 0 && direction == -1) || (tempArray[tempPos] == 9 && direction == 1)))
        {
            tempArray[tempPos] += direction;
            printDigit(tempArray[tempPos], cursor[COLUMN], 1);
        }
    }
    else if (currentDisplay == SCHEDULE_LCD4 || (currentDisplay == TIME_LCD && cursor[ROW] == 1))
    {
        int _hour, _minute, _pm;
        uint8_t pos = (cursor[COLUMN] < 9 || currentDisplay == TIME_LCD) ? 0 : 5;

        _hour = globalBuffer[pos] * 10 + globalBuffer[pos + 1];
        _minute = globalBuffer[pos + 2] * 10 + globalBuffer[pos + 3];
        _pm = globalBuffer[pos + 4];

        if (cursor[COLUMN] == 2 || cursor[COLUMN] == 12)
        {
            _hour = _hour + direction;
            _hour = (_hour <= 0)   ? 12
                    : (_hour > 12) ? 1
                                   : _hour;
            globalBuffer[pos] = _hour / 10;
            globalBuffer[pos + 1] = _hour % 10;

            lcdSP(cursor[COLUMN] - 1, 1, globalBuffer[pos]);
            lcdSP(cursor[COLUMN], 1, globalBuffer[pos + 1]);
            return;
        }
        else if (cursor[COLUMN] == 6 || cursor[COLUMN] == 16)
        {
            _pm = (_pm > 0) ? 0 : 1;
            globalBuffer[pos + 4] = _pm;
            lcdSP(cursor[COLUMN], 1, (_pm > 0) ? "pm" : "am");
            return;
        }
        else
        {
            uint8_t adder = (cursor[COLUMN] == 4 || cursor[COLUMN] == 14)                                        ? 10
                            : (currentDisplay == SCHEDULE_LCD4 && (cursor[COLUMN] == 5 || cursor[COLUMN] == 15)) ? 5
                                                                                                                 : 1;
            _minute = _minute + adder * direction;
            _minute = (_minute < 0 && currentDisplay == SCHEDULE_LCD4) ? 55
                      : (_minute < 0)                                  ? 59
                      : (_minute >= 60)                                ? 0
                                                                       : (_minute);

            globalBuffer[pos + 2] = _minute / 10;
            globalBuffer[pos + 3] = _minute % 10;

            uint8_t col_temp = (adder == 10) ? (cursor[COLUMN]) : (cursor[COLUMN] - 1);

            lcdSP(col_temp, 1, globalBuffer[pos + 2]);
            lcdSP(col_temp + 1, 1, globalBuffer[pos + 3]);
            return;
        }
    }
    else if(currentDisplay == TIME_LCD && cursor[ROW] == 0){

        uint8_t t, max, min, x, constant = 1, col = cursor[COLUMN];

        uint8_t _month = globalBuffer[10] * 10 + globalBuffer[11],
                _year = globalBuffer[14] * 10 + globalBuffer[15],
                _day = globalBuffer[12] * 10 + globalBuffer[13],
                daysInMonthTable[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        if(cursor[COLUMN] == 2){
            t = 10;
            max = 12;
            min = 1;
        }
        else if(cursor[COLUMN] == 4 || cursor[COLUMN] == 5){
            t = 12;
            max = (_month != 2)      ? daysInMonthTable[_month]
                  : (_year % 4 == 0) ? 29
                                     : 28;
            min = 1;
        }
        else{
            t = 14;
            max = 99;
            min = 0;   
        }

        if(cursor[COLUMN] == 9 || cursor[COLUMN] == 4){
            constant = 10;
        }

        x = globalBuffer[t]*10 + globalBuffer[t+1];
        x = x + constant * direction;
        x = (x > max) ? min : x;
        x = (x < min) ? max : x;

        globalBuffer[t] = x/10;
        globalBuffer[t+1] = x%10;

        if(t == 10){
            uint8_t temp = (_month != 2)      ? daysInMonthTable[x]
                           : (_year % 4 == 0) ? 29
                                              : 28;
            if(_day > temp){
                _day = temp;
                globalBuffer[12] = _day/10;
                globalBuffer[13] = _day%10;
                lcdSP(4,0,globalBuffer[12]);
                lcdSP(5,0,globalBuffer[13]);
            }
        }


        if (constant == 1)
        {
            lcdSP(col - 1, 0, globalBuffer[t]);
            lcdSP(col, 0, globalBuffer[t + 1]);
        }
        else
        {
            lcdSP(col, 0, globalBuffer[t]);
            lcdSP(col + 1, 0, globalBuffer[t + 1]);
        }

        return;
    }
}
void UserInterface::selectButton(void)
{
    skipMarker = 0;
    pressed = 1;
    switch (currentDisplay)
    {
    case MAIN_LCD:
    {
        printMenuLCD();
        return;
    }
    case MENU_LCD:
    {
        if (cursor[ROW] == 0)
        {
            printValveSensorLCD();
        }
        else if (cursor[ROW] == 1)
        {
            printCropThresholdLCD();
        }
        else if (cursor[ROW] == 2)
        {
            printScheduleLCD();
        }
        else if (cursor[ROW] == 3)
        {
            printMainLCD();
        }
        break;
    }
    case VALVE_SENSOR_LCD:
    {
        if (cursor[ROW] == 0)
        {
            printMenuLCD();
        }
        else
        {
            database.setSelectedValveNum(cursor[ROW] - 1);
            database.setSelectedCropNum(cursor[ROW] - 1);
            page = 0;
            printCropLCD();
        }
        break;
    }
    case CROP_LCD:
    {
        uint8_t selValve = database.getSelectedValveNum();
        if (page == menu[currentDisplay].totalPages && cursor[ROW] == menu[currentDisplay].lastPos)
        {
            printValveSensorLCD();
            return;
        }
        if (page == 0 && cursor[ROW] == 0)
        {
            database.setValveAvailability(selValve, !database.isValveAvailable(selValve));
        }
        else if (page == 0)
        {
            database.setCrop(selValve, cursor[ROW] - 2);
        }
        //Increment by 4 after page 1
        else if (page == 1)
        {
            database.setCrop(selValve, cursor[ROW] + 2);
        }
        else if (page == 2)
        {
            database.setCrop(selValve, cursor[ROW] + 6);
        }
        printValveSensorLCD();
        break;
    }
    case CROP_THRESHOLD_LCD:
    {
        if (page == menu[currentDisplay].totalPages && cursor[ROW] == menu[currentDisplay].lastPos)
        {
            printMenuLCD();
            break;
        }
        else if (page == 0 && cursor[ROW] == 0)
        {
            database.setCropThreshold(0, 80);
            database.setCropThreshold(1, 80);
            database.setCropThreshold(2, 80);
            database.setCropThreshold(3, 80);
            database.setCropThreshold(4, 80);
            database.setCropThreshold(5, 80);
            database.setCropThreshold(6, 80);
            printCropThresholdLCD();
            break;
        }

        if (page == 0)
        {
            database.setSelectedCropNum(cursor[ROW] - 3);
        }
        //Increment 4 by  after page 1
        else if (page == 1)
        {
            database.setSelectedCropNum(cursor[ROW] + 1);
        }
        else if (page == 2)
        {
            database.setSelectedCropNum(cursor[ROW] + 5);
        }
        printThreshholdNumLCD();
        tempArray[0] = database.cropThreshold(database.getSelectedCropNum()) / 10;
        tempArray[1] = database.cropThreshold(database.getSelectedCropNum()) - tempArray[0] * 10;
        break;
    }
    case THRESHOLD_NUM_LCD:
    {
        if (cursor[COLUMN] == 15)
        {
            //Recreate symbols
            //createCustomSymbols(0);
            //Save new threshold value into crop array
            database.setCropThreshold(database.getSelectedCropNum(), tempArray[0] * 10 + tempArray[1]);
            //Fixing cursor when lcd is switch to back to threshold menu
            //-----------------------------------
            uint8_t temp = cursor[ROW];
            printCropThresholdLCD();
            if (page == 0)
            {
                lcd.setCursor(0, 0);
                lcd.print(' ');
            }
            cursor[ROW] = temp;
            lcd.setCursor(0, cursor[ROW]);
            lcd.print('>');
            //-----------------------------------
        }
        else if (!cursor[MODE])
        {
            //Disable blinking cursor and print cursor
            //-----------------------------------
            cursor[BLINK] = 0;
            if (blinkAnimation)
            {
                lcd.setCursor(cursor[COLUMN], 3);
                lcd.print("---");
                blinkAnimation = !blinkAnimation;
            }
            //-----------------------------------
            //Toggle cursorMode
            cursor[MODE] = !cursor[MODE];
        }
        else
        {
            //Enable blinking cursor
            cursor[BLINK] = 1;
            //Toggle cursorMode
            cursor[MODE] = !cursor[MODE];
        }
        break;
    }
    case SCHEDULE_LCD:
    {
        if (page == menu[currentDisplay].totalPages && cursor[ROW] == menu[currentDisplay].lastPos)
        {
            printMenuLCD();
        }
        else if (page == 0 && cursor[ROW] == 0)
        {
            schedule.toggleSchedule();
            printScheduleLCD();
        }
        else if (page == 2 && cursor[ROW] == 0)
        {
            uint8_t timeArr[3] = {database.getHour(), database.getMinute(), database.getPM()},
                    dateArr[3] = {0};

            database.getCalenderDate(dateArr);
            reformatToGB(timeArr, 3);
            reformatToGB(dateArr, 3, true);
            printTimeLCD();
        }
        else
        {
            database.setSelectedDate((cursor[ROW] + page * 4) - 1, 0);
            printScheduleLCD2();
        }
        break;
    }
    case TIME_LCD:
    {
        if (cursor[ROW] == 3)
        {
            bool toUpdate = false;
            for (int i = 0; i < 16; i++)
            {
                // if (i < 5)
                // {
                //     lcdSP(i, 0, globalBuffer[i]);
                //     lcdSP(i, 1, globalBuffer[i+5]);

                // }
                // else if (i >= 10)
                // {
                //     lcdSP(i, 2, globalBuffer[i]);
                //     lcdSP(i, 3, globalBuffer[i+6]);

                // }
                if ((i < 5) && (globalBuffer[i] != globalBuffer[i + 5]))
                {
                    toUpdate = true;
                }
                if ((i >= 10) && (globalBuffer[i] != globalBuffer[i + 6]))
                {
                    toUpdate = true;
                }
            }

            if (toUpdate)
            {
                database.setHour(globalBuffer[0] * 10 + globalBuffer[1]);
                database.setMinute(globalBuffer[2] * 10 + globalBuffer[3]);
                database.setPM(globalBuffer[4]);

                database.setCalenderDate(globalBuffer[10] * 10 + globalBuffer[11],
                                         globalBuffer[12] * 10 + globalBuffer[13],
                                         globalBuffer[14] * 10 + globalBuffer[15]);

                database.enableRTCFlag();
            }
            page = 0;
            printScheduleLCD();
        }
        else if (cursor[ROW] == 0 && menu[currentDisplay].cursorType == 0)
        {
            lcdSP(13, 0, 'D');
            lcdSP(0, 0, ' ');
            lcdSP(1, 1, "--      ");
            menu[currentDisplay].cursorType = 1;
            cursor[COLUMN] = 2;
        }
        else if (cursor[ROW] == 1 && menu[currentDisplay].cursorType == 0)
        {
            lcdSP(11, 1, 'D');
            lcdSP(0, 1, " ");
            lcdSP(1, 2, "--");
            menu[currentDisplay].cursorType = 1;
            cursor[COLUMN] = 2;
        }
        else if (cursor[ROW] == 1 && cursor[COLUMN] == 12 && menu[currentDisplay].cursorType == 1)
        {
            lcdSP(11, 1, "  ");
            lcdSP(11, 2, "  ");
            lcdSP(0, 1, '>');
            //lcdSP(11, 3, "       ");
            menu[currentDisplay].cursorType = 0;
            cursor[COLUMN] = 0;
        }
        else if(cursor[ROW] == 0 && cursor[COLUMN] == 13 && menu[currentDisplay].cursorType == 1){
            lcdSP(13,0,' ');
            lcdSP(13,1,' ');
            lcdSP(0,0,'>');
            menu[currentDisplay].cursorType = 0;
            cursor[COLUMN] = 0;
            printTimeLCD();
            
        }
        else if ((cursor[ROW] == 1 || cursor[ROW] == 0) && menu[currentDisplay].cursorType == 1)
        {
            cursor[MODE] = !cursor[MODE];
        }

        break;
    }
    case SCHEDULE_LCD2:
    {
        if (cursor[ROW] == 0)
        {
            page = 0;
            printScheduleLCD();
        }
        else
        {
            uint8_t timeArr[8];
            uint8_t day = database.getSelectedDate(0);
            uint8_t num = cursor[ROW] - 1;

            database.setSelectedDate(num, 1);

            toggleActiveTemp = schedule.isDayActive(day, num);

            schedule.getInfo(day, num, timeArr);
            if (timeArr[7] == false)
            {
                reformatToGB(timeArr, 6);
            }
            else
            {
                for (int i = 0; i < 10; i++)
                {
                    globalBuffer[i] = 0;
                }
            }

            printScheduleLCD3();
        }
        break;
    }
    case SCHEDULE_LCD3:
    {
        uint8_t day = database.getSelectedDate(0),
                num = database.getSelectedDate(1);
        bool toUpdateSchedule = 0;
        if (cursor[ROW] == 3)
        {

            for (int i = 0; i < 10; i++)
            {
                if (globalBuffer[i] != globalBuffer[i + 10])
                {
                    toUpdateSchedule = true;
                    break;
                }
            }

            if (toUpdateSchedule == true)
            {
                lcdSP(9, 2, "true");
            }
            else
            {
                lcdSP(9, 2, "false");
            }

            if (toUpdateSchedule == true)
            {
                bool boolValue = schedule.update(database.getSelectedDate(0),            //Selected day
                                                 database.getSelectedDate(1),            //Selected number
                                                 globalBuffer[0] * 10 + globalBuffer[1], //Start: hour
                                                 globalBuffer[2] * 10 + globalBuffer[3], //Start: minute
                                                 globalBuffer[4],                        //Start: PM or AM
                                                 globalBuffer[5] * 10 + globalBuffer[6], //End: hour
                                                 globalBuffer[7] * 10 + globalBuffer[8], //End: minute
                                                 globalBuffer[9]                         //End: PM or AM

                );
                if (boolValue == false)
                {
                    lcdSP(9, 3, "Failed");
                    break;
                }
            }

            if (toggleActiveTemp != schedule.isDayActive(day, num))
            {
                if (toggleActiveTemp == 0)
                {
                    schedule.disableDay(day, num);
                }
                else if (toggleActiveTemp == 1)
                {
                    schedule.enableDay(day, num);
                }
            }

            printScheduleLCD2();
        }
        else if (cursor[ROW] == 0)
        {
            toggleActiveTemp = !toggleActiveTemp;
            printScheduleLCD3();
        }
        else if (cursor[ROW] == 1)
        {
            schedule.clear(day, num);
            printScheduleLCD2();
        }
        else
        {
            printScheduleLCD4();
        }
        break;
    }
    case SCHEDULE_LCD4:
    {
        if (cursor[ROW] == 3)
        {
            printScheduleLCD3();
        }
        else if (cursor[ROW] == 1 && menu[currentDisplay].cursorType == 0)
        {

            lcdSP(19, 1, "D");
            lcdSP(0, 1, " ");
            lcdSP(1, 2, "--");
            menu[currentDisplay].cursorType = 1;
            cursor[COLUMN] = 2;
        }
        else if (cursor[COLUMN] == 19 && menu[currentDisplay].cursorType == 1)
        {
            //TODO: Adding time validation
            //---------------------------
            uint8_t hour1 = globalBuffer[0] * 10 + globalBuffer[1],
                    min1 = globalBuffer[2] * 10 + globalBuffer[3],
                    pm1 = globalBuffer[4],
                    hour2 = globalBuffer[5] * 10 + globalBuffer[6],
                    min2 = globalBuffer[7] * 10 + globalBuffer[8],
                    pm2 = globalBuffer[9];

            if (hour1 == 0 || hour2 == 0 || (hour1 == hour2 && min1 == min2 && pm1 == pm2))
            {
                lcdSP(11, 3, "Invalid");
            }
            else
            {
                //Restore default cursor
                lcdSP(19, 1, " ");
                lcdSP(19, 2, " ");
                lcdSP(0, 1, ">");
                lcdSP(11, 3, "       ");
                menu[currentDisplay].cursorType = 0;
                cursor[COLUMN] = 0;
            }
        }
        else
        {
            cursor[MODE] = !cursor[MODE];
        }
        break;
    }
    }
}

void UserInterface::reformatToGB(uint8_t dataArr[], uint8_t len, bool reformatDate)
{
    uint8_t x = (reformatDate) ? 6 : (len == 3) ? 5
                                                : 10;

    for (int i = 0, counter = (!reformatDate) ? 0 : 10; i < len; i++)
    {
        if (dataArr[i] > 9 && ((i != 2 && i != 5) || reformatDate))
        {
            globalBuffer[counter] = globalBuffer[counter + x] = dataArr[i] / 10;
            globalBuffer[counter + 1] = globalBuffer[counter + 1 + x] = dataArr[i] % 10;

            counter = counter + 2;
        }
        else if (dataArr[i] <= 9 && ((i != 2 && i != 5) || reformatDate))
        {
            globalBuffer[counter] = globalBuffer[counter + x] = 0;
            globalBuffer[counter + 1] = globalBuffer[counter + x + 1] = dataArr[i];
            counter = counter + 2;
        }
        else
        {
            globalBuffer[counter] = globalBuffer[counter + x] = dataArr[i];
            counter = counter + 1;
        }
    }
}

void UserInterface::lcdSP(uint8_t col, uint8_t row, const char msg[20])
{
    lcd.setCursor(col, row);
    lcd.print(msg);
}
void UserInterface::lcdSP(uint8_t col, uint8_t row, int msg)
{
    lcd.setCursor(col, row);
    lcd.print(msg);
}
void UserInterface::lcdSP(uint8_t col, uint8_t row, char msg)
{
    lcd.setCursor(col, row);
    lcd.print(msg);
}

void UserInterface::printDay(uint8_t num, uint8_t col, uint8_t row)
{
    lcd.setCursor(col, row);
    switch (num)
    {
    case 0:
    {
        lcd.print("Monday");
        break;
    }
    case 1:
    {
        lcd.print("Tuesday");
        break;
    }
    case 2:
    {
        lcd.print("Wednesday");
        break;
    }
    case 3:
    {
        lcd.print("Thursday");
        break;
    }
    case 4:
    {
        lcd.print("Friday");
        break;
    }
    case 5:
    {
        lcd.print("Saturday");
        break;
    }
    case 6:
    {
        lcd.print("Sunday");
        break;
    }
    }
}
void UserInterface::printCrop(uint8_t num, uint8_t col, uint8_t row)
{
    lcd.setCursor(col, row);
    switch (num)
    {
    case 0:
    {
        lcd.print("Corn");
        break;
    }
    case 1:
    {
        lcd.print("Bean");
        break;
    }
    case 2:
    {
        lcd.print("Cotton");
        break;
    }
    case 3:
    {
        lcd.print("Tomato");
        break;
    }
    case 4:
    {
        lcd.print("Potato");
        break;
    }
    case 5:
    {
        lcd.print("Tobacco");
        break;
    }
    case 6:
    {
        lcd.print("Papaya");
        break;
    }
    }
}
