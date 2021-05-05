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



#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

#include "Database.h"
#include "CustomCharacter.h"
#include "Schedule.h"

enum Display{MAIN_LCD = 0,MENU_LCD,VALVE_SENSOR_LCD,CROP_LCD,TIME_LCD,VALVE_STATUS_LCD,CROP_THRESHOLD_LCD,THRESHOLD_NUM_LCD,SCHEDULE_LCD,DISPLAY_SIZE};
enum CursorPosition{COLUMN = 0,ROW,MODE,BLINK};
enum Type{SOIL,VALVE,UP,DOWN,SELECT,TIME,TEMPERATURE};



class UserInterface
{
public:
    Database database;

    UserInterface(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows);
    void begin(void);
    void update(uint8_t type);
    void printDisplay(uint8_t display);

    bool isBlinkEnable(void);
    void blinkCursor(void);

    bool isIdle(void);
    void idle(void);
    void wakeup(void);

    uint8_t getCursorPosition(uint8_t type);
    uint8_t getCurrentDisplay(void);

private:
    struct MenuInfo
    {
        uint8_t initialPosition; //Row cursor will begin
        uint8_t totalRows;       //Max/Total number of rows
        uint8_t totalPages;      //Total Page = maxRows/4
        uint8_t lastPosition;    //Last Position = (maxRows % 4)-1
        bool isCircular;         //Circular if cursor loop back to top of list
        bool cursorType;         //Cursor Type '>' = 0, Cursor Type '_' = 1
    };

    MenuInfo menu[DISPLAY_SIZE];
    CustomCharacters customCharacter;
    Schedule schedule;
    LiquidCrystal_I2C lcd;

    uint8_t tempArray[3] = {0};
    uint8_t page;
    uint8_t cursor[4] = {0};
    uint8_t currentDisplay;

    bool idleStatus = false;
    bool blinkStatus = 0;
    bool skip = 1;

    void updateCursor(int8_t direction);
    void updateBigNumber(int8_t direction);
    void printBigNum(uint8_t num, uint8_t col, uint8_t row);
    void selectButton(void);

    void createCustomSymbols(uint8_t set);

    void AddMenuInfo(uint8_t display, uint8_t _initialPosition, uint8_t _totalRows, uint8_t _totalPages, uint8_t _lastPosition, bool _isCircular, bool cursorType);
    void printMainLCD(void);
    void printMenuLCD(void);
    void printValveSensorLCD(void);
    void printCropLCD(void);
    void printTimeLCD(void);
    void printValveStatusLCD(void);
    void printCropThresholdLCD(void);
    void printThreshholdNumLCD(void);
    void printScheduleLCD(void);
    void resetLCDValue(void);

    //Status = 0 if close
    void printValveStatusHelper(uint8_t status, bool printType);
    //Print current date/time onto LCD
    void printTimeHelper(uint8_t hour, uint8_t minute, bool isPM, uint8_t col, uint8_t row);
    void printCropHelper(uint8_t num);
    void printDayHelper(uint8_t num);
    void printScheduleHelper(uint8_t day, uint8_t startNum, uint8_t endNum, uint8_t col, uint8_t row);





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
