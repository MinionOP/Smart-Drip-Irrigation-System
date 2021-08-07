#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>


#include "database.h"
#include "custom_character.h"
#include "schedule.h"

enum Display
{
    MAIN_LCD = 0,
    MENU_LCD,
    VALVE_SENSOR_LCD,
    CROP_LCD,
    TIME_LCD,
    CROP_THRESHOLD_LCD,
    THRESHOLD_NUM_LCD,
    SCHEDULE_LCD,
    SCHEDULE_LCD2,
    SCHEDULE_LCD3,
    SCHEDULE_LCD4,
    DISPLAY_SIZE
};
enum CursorPosition
{
    COLUMN = 0,
    ROW,
    MODE,
    BLINK
};
enum Type
{
    SOIL,
    VALVE,
    UP,
    DOWN,
    SELECT,
    TIME,
    TEMPERATURE,
    SCHEDULE_STATUS
};

const uint32_t IDLE_INTERVAL = 90000; //90 seconds
const uint16_t BLINK_INTERVAL = 1500;

class UserInterface
{
public:
    Database database;
    Schedule schedule;

    UserInterface(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows);
    void begin(void);
    void update(uint8_t type);
    void printDisplay(uint8_t display);

    // bool blinkCheck(void);
    // bool blinkCursor(void);

    // bool isIdle(void);
    // void idleIncrement(uint32_t interval);
    // void idleResetCounter(void);
    // void wakeup(void);

    uint8_t getCursorPosition(uint8_t type);
    uint8_t getCurrentDisplay(void);

private:
    struct MenuInfo
    {
        uint8_t initialPos; //Row cursor will begin
        uint8_t totalRows;  //Max/Total number of rows
        uint8_t totalPages; //Total Page = maxRows/4
        uint8_t lastPos;    //Last Position = (maxRows % 4)-1
        bool isCircular;    //Circular if cursor loop back to top of list
        bool cursorType;    //Cursor Type '>' = 0, Cursor Type '_' = 1
    };

    MenuInfo menu[DISPLAY_SIZE];
    CustomCharacters customCharacter;
    LiquidCrystal_I2C lcd;

    typedef void (UserInterface::*displayTable)();
    displayTable interfaceTable[DISPLAY_SIZE];

    //void idle(void);
    //If UP button was pressed (direction = 1), DOWN (direction = -1)
    void updateCursor(int8_t direction);
    void updateCursor2(int direction, uint8_t set);
    void updateDigit(int8_t direction);
    void printDigit(uint8_t num, uint8_t col, uint8_t row);
    void selectButton(void);

    void createCustomSymbols(uint8_t set);

    void AddMenuInfo(uint8_t display, uint8_t _initialPos, uint8_t _totalRows, uint8_t _totalPages, uint8_t _lastPos, bool _isCircular, bool cursorType);
    void printMainLCD(void);
    void printMenuLCD(void);
    void printValveSensorLCD(void);
    void printCropLCD(void);
    void printTimeLCD(void);
    void printValveStatusLCD(void);
    void printCropThresholdLCD(void);
    void printThreshholdNumLCD(void);
    void printScheduleLCD(void);
    void printScheduleLCD2(void);
    void printScheduleLCD3(void);
    void printScheduleLCD4(void);
    void resetLCDValue(void);

    //Status = 0 if close
    void printValveStatus(uint8_t status, bool printType, uint8_t col, uint8_t row);
    //Print current date/time onto LCD
    void printTime(uint8_t hour, uint8_t minute, bool isPM, uint8_t col, uint8_t row);
    void printCrop(uint8_t num);
    void printDay(uint8_t num, uint8_t col, uint8_t row);
    void printSchedule(uint8_t day, uint8_t startNum, uint8_t endNum, uint8_t col, uint8_t row);

    void lcdClearRow(uint8_t row, uint8_t startPos, uint8_t endPos);
    void lcdClear(void);

    void lcdSetAndPrint(uint8_t col, uint8_t row, const char msg[20]); //Maybe add another argument const char msg2[20] = -1
    void lcdSetAndPrint(uint8_t col, uint8_t row, int msg);

    bool isLastPage(uint8_t _page);

    const char cursorArray[18][20] = {"---      ",
                                      "   ---   ",
                                      "        >",
                                      "---             ",
                                      "   ---          ",
                                      "       ---      ",
                                      "          ---   ",
                                      "             -- ",
                                      "               >",
                                      "--                 ",
                                      "   -               ",
                                      "    -              ",
                                      "     --            ",
                                      "          --       ",
                                      "             -     ",
                                      "              -    ",
                                      "               --  ",
                                      "                  -"};

    bool idleStatus = false;
    bool blinkAnimation = 0;
    bool blinkStatus = 0;
    bool skip = 1;
    bool skipMarker = 0;
    bool pressed = 0;
    bool toggleActiveTemp = 0;


    uint32_t idleCounter = 0;
    int globalBuffer[20] = {0};
    uint8_t testingCounter = 0;
    uint8_t currentDisplay;
    uint8_t tempArray[3] = {0};
    uint8_t page;
    uint8_t cursor[4] = {0};
};

#endif

//-------------------------For Reference---------------------------
/*
//----------MainLCD-------------
//Time: 5:20pm
//Unit Temp: 85F
//S1:52% S2:80% S3:90%
//V1:O   V2:C   V3:C
//------------------------------

//----------MenuLCD-------------
// Valve Setting
// Sensor Setting
// Set Time/Schedule
// Go to Main Display
//------------------------------

//--------ValveSensorLCD--------
// Go back      Sensor
// Valve1:Corn    85%
// Valve2:Bean    78% 
// Valve3:Corn    84%
//------------------------------

//Page 0
//----------CropLCD-------------
// Current Crop:Corn
// Select:
// Corn 
// Bean
//------------------------------

//Page 1
//----------CropLCD-------------
// Cotton
// Tomato
// Potato
// Tobacco
//------------------------------

//Page 2
//----------CropLCD-------------
// Papaya
// Go back
// 
// 
//------------------------------

//----------Schedule-------------
//>M  Monday:
// T  5:45pm - 5:00am
// W  6:00am - 7:00pm
// Th
//------------------------------
*/
//--------------------------------------------------------------
