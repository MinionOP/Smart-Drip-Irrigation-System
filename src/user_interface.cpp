#include "user_interface.h"
#include <LiquidCrystal_I2C.h>

UserInterface::UserInterface(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows) : lcd(lcd_addr, lcd_cols, lcd_rows)
{
    //Remember to also update "printDisplay"
    AddMenuInfo(MAIN_LCD, 0, 4, 0, 0, false, 0);
    AddMenuInfo(MENU_LCD, 0, 4, 0, 0, true, 0);
    AddMenuInfo(VALVE_SENSOR_LCD, 0, 4, 0, 0, true, 0);
    AddMenuInfo(CROP_LCD, 2, 10, 0, 0, true, 0);
    AddMenuInfo(TIME_LCD, 0, -1, -1, 13, false, 1);
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
void UserInterface::update(uint8_t type)
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
                uint8_t soilTemp = database.soilSensor(i);
                uint8_t valveTemp = database.valveStatus(i);
                if (soilTemp != globalBuffer[i])
                {
                    globalBuffer[i] = soilTemp;
                    lcdSetAndPrint(16, i, globalBuffer[i]);
                    (globalBuffer[i] > 9) ? lcdSetAndPrint(18, i + 1, "%") : lcdSetAndPrint(17, i + 1, "%");
                }
                if (valveTemp != globalBuffer[i + 3])
                {
                    globalBuffer[i + 3] = valveTemp;
                    if (globalBuffer[i + 3] == LOW)
                    {
                        lcdSetAndPrint(4, i + 1, "x");
                    }
                    else
                    {
                        lcdSetAndPrint(4, i + 1, " ");
                    }
                }
            }
        }
        else if (currentDisplay == MAIN_LCD)
        {
            for (int i = 0, col_temp = 3; i < 3; i++)
            {
                uint8_t soilTemp = database.soilSensor(i);
                if (soilTemp != globalBuffer[i])
                {
                    globalBuffer[i] = soilTemp;
                    lcdSetAndPrint(col_temp, 2, soilTemp);
                    lcd.print("%");
                }
                col_temp = col_temp + 7;
            }
        }
        return;
    }
    case VALVE:
    {
        if (currentDisplay == MAIN_LCD)
        {
            for (int i = 0, col_temp = 3; i < 3; i++)
            {
                uint8_t valveTemp = database.valveStatus(i);
                if (valveTemp != globalBuffer[i + 3])
                {
                    printValveStatus(valveTemp, 0, col_temp, 3);
                    globalBuffer[i + 3] = valveTemp;
                }
                col_temp = col_temp + 7;
            }
            // lcd.setCursor(3, 3);
            // printValveStatus(database.valveStatus(0), 0,3,3);
            // lcd.setCursor(10, 3);
            // printValveStatus(database.valveStatus(1), 0,10,3);
            // lcd.setCursor(17, 3);
            // printValveStatus(database.valveStatus(2), 0,17,3);
        }

        return;
    }
    case TIME:
    {
        if (currentDisplay == MAIN_LCD)
        {
            uint8_t dayTemp = database.getDayOfWeek();
            if (dayTemp != globalBuffer[7])
            {
                printDay(dayTemp, 0, 0);
                globalBuffer[7] = dayTemp;
            }
            uint8_t col_temp = (dayTemp == 0 || dayTemp == 4 || dayTemp == 6) ? 7 : (dayTemp == 3 || dayTemp == 5) ? 9
                                                                                : (dayTemp == 1)                   ? 8
                                                                                                                   : 10;

            printTime(database.getHour(), database.getMinute(), database.getPM(), col_temp, 0);
        }
        return;
    }
    case TEMPERATURE:
    {
        if (currentDisplay == MAIN_LCD)
        {

            lcdSetAndPrint(11, 1, database.temperature());
            //lcd.write(2);
            lcd.print('F');
        }
        return;
    }
    case SCHEDULE_STATUS:
    {
        lcd.setCursor(18, 0);
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
    lcd.clear();

    //createCustomSymbols(0);

    lcd.setCursor(19, 0);
    //lcd.write(3);
    lcdSetAndPrint(0, 1, "Unit Temp:");
    lcdSetAndPrint(0, 2, "S1:");
    // lcd.print(database.soilSensor(0));
    // lcd.print("%");
    lcdSetAndPrint(7, 2, "S2:");
    // lcd.print(database.soilSensor(1));
    // lcd.print("%");
    lcdSetAndPrint(14, 2, "S3:");
    // lcd.print(database.soilSensor(2));
    // lcd.print("%");

    lcdSetAndPrint(0, 3, "V1:");
    // printValveStatus(database.valveStatus(0), 0);
    lcdSetAndPrint(7, 3, "V2:");

    // printValveStatus(database.valveStatus(1), 0);
    lcdSetAndPrint(14, 3, "V3:");
    // printValveStatus(database.valveStatus(2), 0);

    lcd.setCursor(18, 0);
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
        uint8_t valveTemp = database.valveStatus(i);
        if (soilTemp != globalBuffer[i] || currentDisplay != MAIN_LCD)
        {
            globalBuffer[i] = soilTemp;
            lcdSetAndPrint(col_temp, 2, soilTemp);
            lcd.print("%");
        }
        if (valveTemp != globalBuffer[i + 3] || currentDisplay != MAIN_LCD)
        {
            globalBuffer[i + 3] = valveTemp;
            printValveStatus(valveTemp, 0, col_temp, 3);
        }
        col_temp = col_temp + 7;
    }

    //Update time on LCD
    uint8_t dayTemp = database.getDayOfWeek();
    if (dayTemp != 9)
    {
        if (dayTemp != globalBuffer[7] || currentDisplay != MAIN_LCD)
        {
            printDay(dayTemp, 0, 0);
            globalBuffer[7] = dayTemp;
        }

        uint8_t col_temp = (dayTemp == 0 || dayTemp == 4 || dayTemp == 6) ? 7 : (dayTemp == 3 || dayTemp == 5) ? 9
                                                                            : (dayTemp == 1)                   ? 8
                                                                                                               : 10;

        printTime(database.getHour(), database.getMinute(), database.getPM(), col_temp, 0);
    }
    //Update temperature on LCD
    lcdSetAndPrint(11, 1, database.temperature());
    //lcd.write(2);
    lcd.print("F");

    resetLCDValue();
    currentDisplay = MAIN_LCD;

    if (idleStatus)
    {
        lcd.noBacklight();
    }
}
void UserInterface::printMenuLCD(void)
{

    lcd.clear();
    (!skipMarker) ? lcdSetAndPrint(0, 0, ">Valve Setting") : lcdSetAndPrint(0, 0, " Valve Setting");
    //lcdSetAndPrint(0, 1, " Sensor Setting");
    lcdSetAndPrint(0, 1, " Threshold Setting");
    lcdSetAndPrint(0, 2, " Set Time/Schedule");
    lcdSetAndPrint(0, 3, " Go to Main Display");

    page = 0;
    cursor[ROW] = 0;
    currentDisplay = MENU_LCD;
}
void UserInterface::printValveSensorLCD(void)
{

    lcd.clear();
    (!skipMarker) ? lcdSetAndPrint(0, 0, ">Go back") : lcdSetAndPrint(0, 0, " Go back");
    lcdSetAndPrint(14, 0, "Sensor");
    lcdSetAndPrint(0, 1, " V1| | ");
    printCrop(database.crop(0));
    lcdSetAndPrint(0, 2, " V2| | ");
    printCrop(database.crop(1));
    lcdSetAndPrint(0, 3, " V3| | ");
    printCrop(database.crop(2));

    for (int i = 0; i < 3; i++)
    {
        globalBuffer[i] = database.soilSensor(i);
        globalBuffer[i + 3] = database.valveStatus(i);
        lcdSetAndPrint(16, i + 1, globalBuffer[i]);
        (globalBuffer[i] > 9) ? lcdSetAndPrint(18, i + 1, "%") : lcdSetAndPrint(17, i + 1, "%");
        if (globalBuffer[i + 3] == LOW)
        {
            lcdSetAndPrint(4, i + 1, "x");
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
        lcdSetAndPrint(0, 0, "Curr Crop:");
        printCrop(database.crop(database.getSelectedCropNum()));
        lcdSetAndPrint(0, 1, "Select:");
        (!skipMarker) ? lcdSetAndPrint(0, 2, ">") : lcdSetAndPrint(0, 2, " ");
        printCrop(0);
        lcd.setCursor(1, 3);
        printCrop(1);
        lcd.setCursor(11, 3);
        lcd.print("v");
        //lcd.write(0);
        cursor[ROW] = 2;
        break;
    }
    case 1:
    {
        lcd.setCursor(1, 0);
        printCrop(2);
        lcd.setCursor(1, 1);
        printCrop(3);
        lcd.setCursor(1, 2);
        printCrop(4);
        lcd.setCursor(1, 3);
        printCrop(5);
        lcd.setCursor(11, 3);
        lcd.print("v");

        //lcd.write(0);
        cursor[ROW] = 0;
        break;
    }
    case 2:
    {
        lcd.setCursor(1, 0);
        printCrop(6);
        lcdSetAndPrint(1, 1, "Go back");
        lcd.setCursor(11, 0);
        //lcd.write(1);
        lcd.print("^");

        cursor[ROW] = 0;
        break;
    }
    }
    currentDisplay = CROP_LCD;
}
void UserInterface::printTimeLCD(void)
{
    createCustomSymbols(1);
    currentDisplay = TIME_LCD;
    lcd.clear();
    lcdSetAndPrint(0, 0, "Current Time:");
    lcdSetAndPrint(16, 3, "Done");

    printDigit(database.getHour() / 10, menu[currentDisplay].initialPos, 1);
    printDigit(database.getHour() - (database.getHour() / 10) * 10, menu[currentDisplay].initialPos + 3, 1);
    printDigit(database.getMinute() / 10, menu[currentDisplay].initialPos + 7, 1);
    printDigit(database.getMinute() - (database.getMinute() / 10) * 10, menu[currentDisplay].initialPos + 10, 1);
    lcdSetAndPrint(13, 2, (database.getPM()) ? ("pm") : ("am"));
    lcdSetAndPrint(6, 1, "o");
    lcdSetAndPrint(6, 2, "o");

    cursor[COLUMN] = menu[currentDisplay].initialPos;
    lcdSetAndPrint(cursor[COLUMN], 3, "---");
}

void UserInterface::printCropThresholdLCD(void)
{
    lcd.clear();
    switch (page)
    {
    case 0:
    {
        (!skipMarker) ? lcdSetAndPrint(0, 0, ">Select to restore") : lcdSetAndPrint(0, 0, " Select to restore");
        lcdSetAndPrint(0, 1, " default settings");
        lcdSetAndPrint(0, 2, " Crop");
        lcdSetAndPrint(9, 2, "Threshold");
        lcd.setCursor(1, 3);
        printCrop(0);
        lcdSetAndPrint(12, 3, database.cropThreshold(0));
        lcd.print('%');
        lcd.setCursor(18, 3);
        lcd.print("v");

        //lcd.write(0);
        break;
    }
    case 1:
    {
        lcd.setCursor(1, 0);
        printCrop(1);
        lcdSetAndPrint(12, 0, database.cropThreshold(1));
        lcd.print('%');
        lcd.setCursor(1, 1);
        printCrop(2);
        lcdSetAndPrint(12, 1, database.cropThreshold(2));
        lcd.print('%');
        lcd.setCursor(1, 2);
        printCrop(3);
        lcdSetAndPrint(12, 2, database.cropThreshold(3));
        lcd.print('%');
        lcd.setCursor(1, 3);
        printCrop(4);
        lcdSetAndPrint(12, 3, database.cropThreshold(4));
        lcd.print('%');
        lcd.setCursor(18, 3);
        //lcd.write(0);
        lcd.print("v");

        break;
    }
    case 2:
    {
        lcd.setCursor(1, 0);
        printCrop(5);
        lcdSetAndPrint(12, 0, database.cropThreshold(5));
        lcd.print('%');
        lcd.setCursor(1, 1);
        printCrop(6);
        lcdSetAndPrint(12, 1, database.cropThreshold(6));
        lcd.print('%');
        lcdSetAndPrint(1, 2, "Go back");
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
    lcdSetAndPrint(0, 0, "Curr Thresh: ");
    lcd.print(database.cropThreshold(database.getSelectedCropNum()));
    lcd.print("%");
    lcd.setCursor(0, 3);
    printCrop(database.getSelectedCropNum());
    lcdSetAndPrint(16, 3, "Done");
    printDigit(database.cropThreshold(database.getSelectedCropNum()), 7, 1);
    currentDisplay = THRESHOLD_NUM_LCD;
    cursor[COLUMN] = menu[currentDisplay].initialPos;
    lcdSetAndPrint(cursor[COLUMN], 3, "---");
}
void UserInterface::printScheduleLCD(void)
{
    // lcd.clear();
    // currentDisplay = SCHEDULE_LCD;
    // switch (page)
    // {
    // case 0:
    // {
    //     (!skipMarker) ? lcdSetAndPrint(0, 0, ">M |") : lcdSetAndPrint(0, 0, " M |");
    //     if (pressed || skipMarker)
    //     {
    //         printDay(0, 4, 0);
    //         lcd.print(':');
    //         printSchedule(0, 0, 3, 4, 1);
    //     }
    //     lcdSetAndPrint(0, 1, " T |");
    //     lcdSetAndPrint(0, 2, " W |");
    //     lcdSetAndPrint(0, 3, " Th|");

    //     lcd.setCursor(19, 3);
    //     //lcd.write(0);
    //     lcd.print("v");

    //     break;
    // }
    // case 1:
    // {
    //     lcdSetAndPrint(0, 0, " F |");
    //     lcdSetAndPrint(0, 1, " S |");
    //     lcdSetAndPrint(0, 2, " Su|");
    //     lcdSetAndPrint(0, 3, " Edit time");

    //     lcd.setCursor(19, 3);
    //     //lcd.write(0);
    //     lcd.print("v");

    //     break;
    // }
    // case 2:
    // {
    //     lcd.print(" Go back");
    //     lcd.setCursor(19, 0);
    //     //lcd.write(1);
    //     lcd.print("^");
    //     break;
    // }
    // }

    // cursor[ROW] = 0;

    if (currentDisplay == SCHEDULE_LCD && cursor[ROW] == 0 && page == 0 && pressed == 1)
    {
        (schedule.isEnable()) ? lcdSetAndPrint(8, 2, "ENABLED ") : lcdSetAndPrint(8, 2, "DISABLED");
        return;
    }

    lcd.clear();
    currentDisplay = SCHEDULE_LCD;
    switch (page)
    {
    case 0:
    {
        //(!skipMarker) ? lcdSetAndPrint(0, 0, ">M |") : lcdSetAndPrint(0, 0, " M |");
        (!skipMarker) ? lcdSetAndPrint(0, 0, ">Enable/Disable") : lcdSetAndPrint(0, 0, " Enable/Disable");
        (schedule.isEnable()) ? lcdSetAndPrint(8, 2, "ENABLED ") : lcdSetAndPrint(8, 2, "DISABLED");

        lcdSetAndPrint(0, 1, " M | Scheduling");

        lcdSetAndPrint(0, 2, " T | is");
        lcdSetAndPrint(0, 3, " W |");
        lcd.setCursor(19, 3);
        //lcd.write(0);
        lcd.print("v");

        break;
    }
    case 1:
    {
        lcdSetAndPrint(0, 0, " Th|");
        lcdSetAndPrint(0, 1, " F |");
        lcdSetAndPrint(0, 2, " S |");
        lcdSetAndPrint(0, 3, " Su|");
        lcdSetAndPrint(19, 3, "v");

        break;
    }
    case 2:
    {
        lcdSetAndPrint(0, 0, " Adjust Date/Time");
        lcdSetAndPrint(0, 1, " Go back");
        lcdSetAndPrint(19, 0, "^");

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
    (pressed) ? lcdSetAndPrint(0, 0, ">  Go back") : lcdSetAndPrint(2, 0, " Go back");
    printSchedule(date, 0, 3, 3, 1);

    lcdSetAndPrint((date == 3 || date == 5 || date == 6) ? (18) : 19, 0,
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
        toggleActiveTemp ? lcdSetAndPrint(0, 0, ">Disable |") : lcdSetAndPrint(0, 0, ">Enable  |");
        toggleActiveTemp ? lcdSetAndPrint(19, 0, "*") : lcdSetAndPrint(19, 0, " ");

        return;
    }
    lcd.clear();
    currentDisplay = SCHEDULE_LCD3;
    if (pressed)
    {
        toggleActiveTemp ? lcdSetAndPrint(0, 0, ">Disable |") : lcdSetAndPrint(0, 0, ">Enable  |");
        toggleActiveTemp ? lcdSetAndPrint(19, 0, "*") : lcdSetAndPrint(19, 0, " ");
    }
    else
    {
        toggleActiveTemp ? lcdSetAndPrint(0, 0, " Disable |") : lcdSetAndPrint(0, 0, " Enable  |");
        toggleActiveTemp ? lcdSetAndPrint(19, 0, "*") : lcdSetAndPrint(19, 0, " ");
    }

    uint8_t day = database.getSelectedDate(0),
            num = database.getSelectedDate(1);

    lcdSetAndPrint(0, 1, " Delete  |");
    lcdSetAndPrint(0, 2, " Edit    |");
    lcdSetAndPrint(0, 3, " Go back |");
    (num != 2) ? printDay(day, 11, 0) : printDay(day, 10, 0);
    printSchedule(day, num, num + 1, 11, 1);

    cursor[ROW] = 0;
}

void UserInterface::printScheduleLCD4(void)
{
    lcd.clear();
    currentDisplay = SCHEDULE_LCD4;
    (pressed) ? lcdSetAndPrint(0, 1, ">") : lcdSetAndPrint(0, 1, " ");

    uint8_t day = database.getSelectedDate(0),
            num = database.getSelectedDate(1);

    printDay(day, 1, 0);
    printSchedule(day, num, num + 1, 1, 1);
    lcdSetAndPrint(0, 3, " Go back");

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
    lcd.setCursor(col, row);
    lcd.print(hour, DEC);
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
                lcdSetAndPrint(col - 1, row, "*");
            }
            uint8_t temp = (buffer[0] < 10) ? col + 1 : col;
            if (buffer[0] < 10 && currentDisplay == SCHEDULE_LCD4)
            {
                lcdSetAndPrint(col, row, "0");
            }
            printTime(buffer[0], buffer[1], buffer[2], temp, row);
            switch (currentDisplay)
            {
            case SCHEDULE_LCD:
            {
                lcdSetAndPrint(11, row, "-");
                break;
            }
            case SCHEDULE_LCD2:
            {
                lcdSetAndPrint(10, row, "-");
                break;
            }
            case SCHEDULE_LCD3:
            {
                lcdSetAndPrint(13, 2, "to");
                col = 3;
                row = 3;
                break;
            }
            case SCHEDULE_LCD4:
            {
                lcdSetAndPrint(9, row, "-");
                col = col + 2;
                break;
            }
            }
            uint8_t col_temp = col;
            if (buffer[3] < 10)
            {
                if (currentDisplay != SCHEDULE_LCD3)
                    lcdSetAndPrint(col_temp + 8, row, "0");
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

bool UserInterface::blinkCheck(void)
{
    if ((currentDisplay == THRESHOLD_NUM_LCD || currentDisplay == TIME_LCD) && !blinkStatus)
    {
        blinkStatus = 1;
        return true;
    }
    return false;
}
bool UserInterface::blinkCursor(void)
{
    if (currentDisplay == THRESHOLD_NUM_LCD || currentDisplay == TIME_LCD)
    {
        if (cursor[BLINK] && cursor[COLUMN] != 15)
        {
            blinkAnimation = !blinkAnimation;
            lcd.setCursor(cursor[COLUMN], 3);
            if (currentDisplay == TIME_LCD && cursor[COLUMN] == 13)
            {
                (blinkAnimation ? lcd.print("  ") : lcd.print("--"));
            }
            else
            {
                (blinkAnimation ? lcd.print("   ") : lcd.print("---"));
            }
        }
        return true;
    }
    blinkStatus = !blinkStatus;
    return false;
}

bool UserInterface::isIdle(void)
{
    return idleStatus;
}
void UserInterface::idleIncrement(uint32_t interval)
{
    if (IDLE_INTERVAL < interval)
    {
        return;
    }
    if (!idleStatus)
    {
        idleCounter++;
        if (idleCounter >= IDLE_INTERVAL / interval)
        {
            idleCounter = 0;
            idle();
        }
    }
}
void UserInterface::idleResetCounter(void)
{
    idleCounter = 0;
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
        for (int i = 0; i < 20; i++)
        {
            globalBuffer[i] = -1;
        }
        printMainLCD();
    }
}

void UserInterface::wakeup(void)
{
    if (idleStatus)
    {
        lcd.backlight();
        idleStatus = false;
    }
    idleCounter = 0;
}

void UserInterface::resetLCDValue(void)
{
    for (int i = 0; i < 20; i++)
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
                if (currentDisplay == SCHEDULE_LCD4 && cursor[ROW] == 2)
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
            lcdSetAndPrint(0, row, " ");
            row = row + directionTemp;
            if (page == 0)
            {
                if (currentDisplay == CROP_THRESHOLD_LCD && (row == 1 || row == 2))
                {
                    row = (directionTemp == 1) ? 3 : 0;
                }

                if (row < menu[currentDisplay].initialPos)
                {
                    page = menu[currentDisplay].totalPages;
                    printDisplay(currentDisplay);
                    lcdSetAndPrint(0, menu[currentDisplay].lastPos, ">");
                    cursor[ROW] = menu[currentDisplay].lastPos;
                    return;
                }
            }
            if (page == menu[currentDisplay].totalPages && row > menu[currentDisplay].lastPos)
            {
                page = 0;
                printDisplay(currentDisplay);
                lcdSetAndPrint(0, menu[currentDisplay].initialPos, ">");
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
        lcdSetAndPrint(0, cursor[ROW], ">");

        if (currentDisplay == SCHEDULE_LCD)
        {

            // if (!(cursor[ROW] == menu[currentDisplay].lastPos && page == menu[currentDisplay].totalPages))
            // {
            //     lcdSetAndPrint(4, 0, "               ");
            //     lcdSetAndPrint(4, 1, "               ");
            //     lcdSetAndPrint(4, 2, "               ");

            //     if (page == 0)
            //     {
            //         lcdSetAndPrint(4, 3, "               ");
            //     }

            //     if (!(cursor[ROW] == 3 && page == 1))
            //     {
            //         printDay(cursor[ROW] + page * 4, 4, 0);
            //         lcd.print(':');
            //         uint8_t iteration = (page == 1) ? (2) : 3;
            //         printSchedule(cursor[ROW] + page * 4, 0, iteration, 4, 1);
            //     }
            // }

            if (page == 2 || (cursor[ROW] == 0 && page == 0))
            {
                if (cursor[ROW] == 0 && page == 0)
                {
                    lcdSetAndPrint(4, 1, " Scheduling     ");
                    lcdSetAndPrint(4, 2, " is             ");
                    (schedule.isEnable()) ? lcdSetAndPrint(8, 2, "ENABLED ") : lcdSetAndPrint(8, 2, "DISABLED");
                    lcdSetAndPrint(5, 3, "               ");
                }
                return;
            }

            lcdSetAndPrint(4, 1, "               ");
            lcdSetAndPrint(4, 2, "               ");
            lcdSetAndPrint(4, 3, "               ");
            if (page == 1)
            {
                lcdSetAndPrint(4, 0, "               ");
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

    if (currentDisplay != SCHEDULE_LCD4)
    {
        if ((cursor[COLUMN] == menu[currentDisplay].initialPos && direction == -1) || (cursor[COLUMN] == 15 && direction == 1))
        {
            return;
        }
    }
    else
    {
        if ((cursor[COLUMN] == 2 && direction == -1) || (cursor[COLUMN] == 19 && direction == 1))
        {
            return;
        }
    }

    uint8_t col_temp = 0,
            row_temp = 0,
            iteration = 0,
            counter = 0;
    uint8_t arr[20] = {7, 10, 15,
                       0, 3, 7, 10, 13, 15,
                       2, 4, 5, 6, 12, 14, 15, 16, 19};

    if (set == 0)
    {
        iteration = 3;
        col_temp = 7;
        row_temp = 3;
    }
    else if (set == 1)
    {
        col_temp = 0;
        row_temp = 3;
        iteration = 6;
        counter = 3;
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
    if (currentDisplay == THRESHOLD_NUM_LCD || currentDisplay == TIME_LCD)
    {
        tempPos = (cursor[COLUMN] == menu[currentDisplay].initialPos) ? 0 : 1;
        if (!((tempArray[tempPos] == 0 && direction == -1) || (tempArray[tempPos] == 9 && direction == 1)))
        {
            tempArray[tempPos] += direction;
            printDigit(tempArray[tempPos], cursor[COLUMN], 1);
        }
    }
    else if (currentDisplay == SCHEDULE_LCD4)
    {
        int _hour, _minute, _pm, mark = -1;
        uint8_t pos = (cursor[COLUMN] < 9) ? 0 : 5;

        _hour = globalBuffer[pos] * 10 + globalBuffer[pos + 1];
        _minute = globalBuffer[pos + 2] * 10 + globalBuffer[pos + 3];
        _pm = globalBuffer[pos + 4];

        if (cursor[COLUMN] == 2 || cursor[COLUMN] == 12)
        {
            _hour = _hour + direction;
            _hour = mark = (_hour <= 0) ? 12 : (_hour > 12) ? 1
                                                            : _hour;
            globalBuffer[pos] = _hour / 10;
            globalBuffer[pos + 1] = _hour % 10;

            lcdSetAndPrint(cursor[COLUMN] - 1, 1, globalBuffer[pos]);
            lcdSetAndPrint(cursor[COLUMN], 1, globalBuffer[pos + 1]);
            return;
        }
        else if (cursor[COLUMN] == 6 || cursor[COLUMN] == 16)
        {
            _pm = (_pm > 0) ? 0 : 1;
            globalBuffer[pos + 4] = _pm;
            lcdSetAndPrint(cursor[COLUMN], 1, (_pm > 0) ? "pm" : "am");
            return;
        }
        else
        {
            uint8_t adder = (cursor[COLUMN] == 4 || cursor[COLUMN] == 14) ? 10 : (cursor[COLUMN] == 5 || cursor[COLUMN] == 15) ? 5
                                                                                                                               : 0;
            _minute = _minute + adder * direction;
            _minute = mark = (_minute < 0) ? 55 : (_minute >= 60) ? 0
                                                                  : (_minute);

            globalBuffer[pos + 2] = _minute / 10;
            globalBuffer[pos + 3] = _minute % 10;

            uint8_t col_temp = (adder == 10) ? (cursor[COLUMN]) : (cursor[COLUMN] - 1);

            lcdSetAndPrint(col_temp, 1, globalBuffer[pos + 2]);
            lcdSetAndPrint(col_temp + 1, 1, globalBuffer[pos + 3]);
            return;
        }
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
        if (page == menu[currentDisplay].totalPages && cursor[ROW] == menu[currentDisplay].lastPos)
        {
            printValveSensorLCD();
            return;
        }
        if (page == 0)
        {
            database.setCrop(database.getSelectedValveNum(), cursor[ROW] - 2);
        }
        //Increment by 4 after page 1
        else if (page == 1)
        {
            database.setCrop(database.getSelectedValveNum(), cursor[ROW] + 2);
        }
        else if (page == 2)
        {
            database.setCrop(database.getSelectedValveNum(), cursor[ROW] + 6);
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
        if (cursor[COLUMN] == 15)
        {
            //Recreate symbols
            //createCustomSymbols(0);
            //Fixing cursor when lcd is switch to back to previous menu
            //-----------------------------------
            uint8_t temp = cursor[ROW];
            printScheduleLCD();
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
                for (int i = 0, counter = 0; i < 6; i++)
                {
                    if (timeArr[i] > 9 && (i != 2 && i != 5))
                    {
                        globalBuffer[counter] = globalBuffer[counter + 10] = timeArr[i] / 10;
                        globalBuffer[counter + 1] = globalBuffer[counter + 1 + 10] = timeArr[i] % 10;

                        counter = counter + 2;
                    }
                    else if (timeArr[i] <= 9 && (i != 2 && i != 5))
                    {
                        globalBuffer[counter] = globalBuffer[counter + 10] = 0;
                        globalBuffer[counter + 1] = globalBuffer[counter + 10 + 1] = timeArr[i];
                        counter = counter + 2;
                    }
                    else
                    {
                        globalBuffer[counter] = globalBuffer[counter + 10] = timeArr[i];
                        counter = counter + 1;
                    }
                }
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
        uint8_t day = database.getSelectedDate(0);
        uint8_t num = database.getSelectedDate(1);
        bool toUpdate = 0;
        if (cursor[ROW] == 3)
        {
            //--------------------------------------------------------------//
            /*
            Will produce an error if time is invalid
            Example: 0:00am - 0:00pm, need to verify. do later
            */
            //--------------------------------------------------------------//

            if (toggleActiveTemp != schedule.isDayActive(day, num))
            {
                if (schedule.isDayActive(day, num))
                {
                    schedule.disable(day, num);
                    // lcdSetAndPrint(9,2,"disable");
                }
                else
                {
                    toUpdate = true;
                    // lcdSetAndPrint(9,2,"update");
                }
            }
            // lcdSetAndPrint(3,1,schedule.isDayActive(day,num));
            // lcdSetAndPrint(2,1,toggleActiveTemp);

            for (int i = 0; i < 10; i++)
            {
                if (globalBuffer[i] != globalBuffer[i + 10])
                {
                    toUpdate = true;
                    break;
                }
            }

            for (int i = 0; i < 10; i++)
            {
                lcdSetAndPrint(i, 0, globalBuffer[i]);
                lcdSetAndPrint(i, 1, globalBuffer[i + 10]);
            }
            if (toUpdate == true)
            {
                lcdSetAndPrint(9, 2, "true");
            }
            else
            {
                lcdSetAndPrint(9, 2, "false");
            }

            if (toUpdate == true)
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
                    lcdSetAndPrint(9, 3, "Failed");
                    break;
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

            lcdSetAndPrint(19, 1, "D");
            lcdSetAndPrint(0, 1, " ");
            lcdSetAndPrint(1, 2, "--");
            menu[currentDisplay].cursorType = 1;
            cursor[COLUMN] = 2;
        }
        else if (cursor[COLUMN] == 19 && menu[currentDisplay].cursorType == 1)
        {


            //TODO: Adding time validation
            //---------------------------
            uint8_t hour1 = globalBuffer[0] * 10 + globalBuffer[1],
                    min1  = globalBuffer[2] * 10 + globalBuffer[3],
                    pm1   = globalBuffer[4],
                    hour2 = globalBuffer[5] * 10 + globalBuffer[6],
                    min2  = globalBuffer[7] * 10 + globalBuffer[8],
                    pm2   = globalBuffer[9];

            if (hour1 == 0 || hour2 == 0 || (hour1 == hour2 && min1 == min2 && pm1 == pm2))
            {
                lcdSetAndPrint(11, 3, "Invalid");
            }
            else{
                //Restore default cursor
                lcdSetAndPrint(19, 1, " ");
                lcdSetAndPrint(19, 2, " ");
                lcdSetAndPrint(0, 1, ">");
                lcdSetAndPrint(11,3, "       ");
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

void UserInterface::lcdClearRow(uint8_t row, uint8_t startPos = 0, uint8_t endPos = 19)
{
    if (startPos < 0 || startPos > 20 || endPos < 0 || endPos > 20 || startPos > endPos)
    {
        return;
    }
    for (int i = startPos; i <= endPos; i++)
    {
        lcd.setCursor(i, row);
        lcd.print(" ");
    }
}
void UserInterface::lcdSetAndPrint(uint8_t col, uint8_t row, const char msg[20])
{
    lcd.setCursor(col, row);
    lcd.print(msg);
}
void UserInterface::lcdSetAndPrint(uint8_t col, uint8_t row, int msg)
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
void UserInterface::printCrop(uint8_t num)
{
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

void UserInterface::lcdClear(void)
{
    for (int i = 0; i < 4; i++)
    {
        lcd.setCursor(0, i);
        lcd.print(F("                   "));
    }
    lcd.setCursor(0, 0);
}