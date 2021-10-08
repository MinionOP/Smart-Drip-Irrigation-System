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
    SCHEDULE_STATUS,
    DEADLINE
};

const uint16_t BLINK_INTERVAL = 1500;

class UserInterface
{
public:

    
    Database database;
    Schedule schedule;

    /**
     * @brief Construct a new User Interface object
     * 
     * @param lcd_addr - i2c address, dependent on manufacturer
     * @param lcd_cols - number of columns
     * @param lcd_rows - number of rows
     */
    UserInterface(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows);

    /**
     * @brief Prompt the LCD to turn on. Ready to operate.
     * 
     */
    void begin(void);

    /**
     * @brief Print the updated value to LCD
     * 
     * @param type - from enum "Type"
     * @param num  - the updated value
     */
    void updateLCD(uint8_t type, uint8_t num = 10);

    /**
     * @brief Clear LCD and print a new display
     * 
     * @param display - from enum "Display"
     */
    void printDisplay(uint8_t display);

    /**
     * @brief Prints an array of integers to LCD
     * 
     * @param col - column initial position
     * @param row  - row initial position
     * @param buffer - array of intergers
     * @param len - length of array input
     */
    void printToLCD(uint8_t col, uint8_t row, uint8_t buffer[20], uint8_t len);

    /**
     * @brief Return LCD's idle status
     * 
     * @return true if 'idle'
     * @return false if 'not idle'
     */
    bool isIdle(void);

    /**
     * @brief Return LCD's backlight status
     * 
     * @return true if backlight is on
     * @return false if backlight is off
     */
    bool isLCDOn(void);

    /**
     * @brief Check if backlight is disbaled. If true then it will 
     *         turn on LCD's backlight and set idle status to false
     * 
     */
    void wakeup(void);

    /**
     * @brief Turn off LCD's backlight and set idle to true
     * 
     */
    void idle(void);

    /**
     * @brief Get the current display on the LCD
     * 
     * @return uint8_t type value that is mapped to enum "Display"
     */
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

    void printValveStatus(uint8_t status, bool printType, uint8_t col, uint8_t row);
    void printTime(uint8_t hour, uint8_t minute, bool isPM, uint8_t col, uint8_t row);
    void printCrop(uint8_t num, uint8_t col, uint8_t row);
    void printDay(uint8_t num, uint8_t col, uint8_t row);
    void printSchedule(uint8_t day, uint8_t startNum, uint8_t endNum, uint8_t col, uint8_t row);


    void lcdSP(uint8_t col, uint8_t row, const char msg[20]); //Maybe add another argument const char msg2[20] = -1
    void lcdSP(uint8_t col, uint8_t row, int msg);
    void lcdSP(uint8_t col, uint8_t row, char msg);

    void reformatToGB(uint8_t dataArr[], uint8_t len, bool reformatDate = false);
    bool isLastPage(uint8_t _page);
    

    bool isInitialBootup = true,
         idleStatus = false,
         blinkAnimation = 0,
         blinkStatus = 0,
         skipMarker = 0,
         pressed = 0,
         toggleActiveTemp = 0,
         noDisplay = false;

    int globalBuffer[25] = {0};

    uint8_t testingCounter = 0,
            currentDisplay,
            tempArray[3] = {0},
            page,
            cursor[4] = {0};

    const char cursorArray[24][20] = {"---      ",
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
                                      "                  -",
                                      "--                 ",
                                      "   -               ",
                                      "    -              ",
                                      "        -          ",
                                      "         -         ",
                                      "            -      "};

};

#endif

//-------------------------For Reference---------------------------
/*
//----------MainLCD-------------
    Time: 5:20pm
    Battery:       85F|SR
    S1:52% S2:80% S3:90%
    V1:O   V2:C   V3:C
//------------------------------

//----------MenuLCD-------------
    Valve Setting
    Sensor Setting
    Set Time/Schedule
    Go to Main Display
//------------------------------

//--------ValveSensorLCD--------
    Go back      Sensor
    V2|X| Corn    85%
    V1| |Bean    78% 
    ------Disabled-------
//------------------------------

//Page 0
//----------CropLCD-------------
    Current Crop:Corn
    Select:
    Corn 
    Bean
//------------------------------

//Page 1
//----------CropLCD-------------
    Cotton
    Tomato
    Potato
    Tobacco
//------------------------------

//Page 2
//----------CropLCD-------------
    Papaya
    Go back
 

//------------------------------

//----------Schedule-------------
>Enable/Disable Schedule
 M  Monday:
 T  5:45pm - 5:00am
 W  6:00am - 7:00pm
//------------------------------
*/
//--------------------------------------------------------------
