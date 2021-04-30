#include "UserInterface.h"
#include <LiquidCrystal_I2C.h>


UserInterface::UserInterface(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows):lcd(lcd_addr,lcd_cols,lcd_rows){
    AddMenuInfo(MAIN_LCD,0,4,0,0,false,0);
    AddMenuInfo(MENU_LCD,0,4,0,0,true,0);
    AddMenuInfo(VALVE_SENSOR_LCD,0,4,0,0,true,0);
    AddMenuInfo(CROP_LCD,2,10,0,0,true,0);
    AddMenuInfo(TIME_LCD,0,-1,-1,13,false,1);
    AddMenuInfo(VALVE_STATUS_LCD,0,4,0,0,true,0);
    AddMenuInfo(CROP_THRESHOLD_LCD,0,11,0,0,true,0);
    AddMenuInfo(THRESHOLD_NUM_LCD,7,-1,-1,10,false,1);
    AddMenuInfo(SCHEDULE_LCD,0,9,0,0,true,0);
    resetLCDValue();
    idleStatus = false;
    currentDisplay = MAIN_LCD;

}

void UserInterface::begin(void){
    lcd.begin();
    lcd.backlight();
}

void UserInterface::AddMenuInfo(uint8_t display, uint8_t _initialPosition, uint8_t _totalRows, uint8_t _totalPages, uint8_t _lastPosition, bool _isCircular, bool _cursorType){
    menu[display].initialPosition =  _initialPosition;
    menu[display].totalRows = _totalRows;
    menu[display].isCircular = _isCircular;
    if(_cursorType == 0){
        menu[display].cursorType = _cursorType;
        menu[display].totalPages = _totalRows/4;
        menu[display].lastPosition = (_totalRows%4)-1;
    }
    else{
        menu[display].cursorType = _cursorType;
        menu[display].totalPages = _totalPages;
        menu[display].lastPosition = _lastPosition; 
    }
}

void UserInterface::update(uint8_t type)
{
    switch (type)
    {
    case DOWN:
    {
        (!cursor[MODE]) ? updateCursor(-1) : updateBigNumber(-1);
        break;
    }
    case UP:
    {
        (!cursor[MODE]) ? updateCursor(1) : updateBigNumber(1);
        break;
    }
    case SELECT:
    {
        selectButton();
        break;
    }
    case SOIL:
    {
        if (currentDisplay == VALVE_SENSOR_LCD)
        {
            lcd.setCursor(18, 1);
            lcd.print(' ');lcd.setCursor(18, 2);
            lcd.print(' ');lcd.setCursor(18, 3);
            lcd.print(' ');

            lcd.setCursor(17, 1);
            lcd.print(database.getSoilSensor(0));lcd.setCursor(17, 2);
            lcd.print(database.getSoilSensor(1));lcd.setCursor(17, 3);
            lcd.print(database.getSoilSensor(2));
        }
        else if (currentDisplay == MAIN_LCD)
        {
            lcd.setCursor(3, 2);
            lcd.print(database.getSoilSensor(0));lcd.print("%");lcd.setCursor(10, 2);
            lcd.print(database.getSoilSensor(1));lcd.print("%");lcd.setCursor(17, 2);
            lcd.print(database.getSoilSensor(2));lcd.print("%");
        }
        break;
    }
    case VALVE:
    {
        if (currentDisplay == MAIN_LCD)
        {
            lcd.setCursor(3, 3);
            printValveStatusHelper(database.getValveStatus(0), 0);
            lcd.setCursor(10, 3);
            printValveStatusHelper(database.getValveStatus(1), 0);
            lcd.setCursor(17, 3);
            printValveStatusHelper(database.getValveStatus(2), 0);
        }
        else if (currentDisplay == VALVE_STATUS_LCD)
        {
        }
        break;
    }
    case TIME:
    {
        if (currentDisplay == MAIN_LCD)
        {
            printTimeHelper(database.getHour(), database.getMinute(), database.getPM(), 6, 0);
        }
        break;
    }
    case TEMPERATURE:
    {
        if(currentDisplay == MAIN_LCD){
            lcd.setCursor(11,1);
            lcd.print(database.getTemperature()); lcd.write(2); lcd.print("F");
        }
        break;
    }
    }
}

uint8_t UserInterface::getCurrentDisplay(void){
    return currentDisplay;
}
uint8_t UserInterface::getCursorPosition(uint8_t type){
    return cursor[type];
}
bool UserInterface::isIdle(void){
    return idleStatus;
}
bool UserInterface::isBlinkEnable(void){
    return cursor[BLINK];
}

void UserInterface::printMainLCD(void){
    createCustomSymbols(0);
    lcd.clear();
    lcd.noCursor();

    lcd.print("Time:"); lcd.setCursor(19,0);
    lcd.write(3); lcd.setCursor(0,1);
    lcd.print("Unit Temp:"); lcd.setCursor(0,2);
    lcd.print("S1:");lcd.print(database.getSoilSensor(0));lcd.print("%");lcd.setCursor(7,2);
    lcd.print("S2:");lcd.print(database.getSoilSensor(1));lcd.print("%");lcd.setCursor(14,2);
    lcd.print("S3:");lcd.print(database.getSoilSensor(2));lcd.print("%");


    lcd.setCursor(0,3);
    lcd.print("V1:");printValveStatusHelper(database.getValveStatus(0),0);lcd.setCursor(7,3); 
    lcd.print("V2:");printValveStatusHelper(database.getValveStatus(1),0);lcd.setCursor(14,3);
    lcd.print("V3:");printValveStatusHelper(database.getValveStatus(2),0);

    //Update time on LCD
    printTimeHelper(database.getHour(), database.getMinute(), database.getPM(),6,0);

    //Update temperature on LCD
    lcd.setCursor(11,1);
    lcd.print(database.getTemperature()); lcd.write(2); lcd.print("F");

    resetLCDValue();
    currentDisplay = MAIN_LCD;
}
void UserInterface::printMenuLCD(void){
    lcd.clear();
    lcd.print(">Valve Setting"); lcd.setCursor(0,1);
    lcd.print(" Sensor Setting"); lcd.setCursor(0,2);
    lcd.print(" Set Time/Schedule"); lcd.setCursor(0,3);
    lcd.print(" Go to Main Display");

    page = 0;
    cursor[ROW] = 0;
    currentDisplay = MENU_LCD;
}
void UserInterface::printValveSensorLCD(void){
    lcd.clear();
    lcd.print(">Go back"); 
    lcd.setCursor(14,0);
    lcd.print("Sensor");

    lcd.setCursor(0,1);
    //Name of crops of valve
    lcd.print(" Valve1:"); printCropHelper(database.getCrop(0)); lcd.setCursor(0,2);
    lcd.print(" Valve2:"); printCropHelper(database.getCrop(1)); lcd.setCursor(0,3);
    lcd.print(" Valve3:"); printCropHelper(database.getCrop(2)); 

    //valve[2].soilMoistureValue
    lcd.setCursor(17,1);
    lcd.print(database.getSoilSensor(0)); lcd.print(" %"); lcd.setCursor(17,2);
    lcd.print(database.getSoilSensor(1)); lcd.print(" %"); lcd.setCursor(17,3);
    lcd.print(database.getSoilSensor(2)); lcd.print(" %");

    cursor[ROW] = 0;
    page = 0;
    currentDisplay = VALVE_SENSOR_LCD;
}
void UserInterface::printCropLCD(void){
    lcd.clear();
    switch(page){
    case 0:{
        lcd.print("Current Crop:"); printCropHelper(database.getCrop(database.getCurrentCrop())); lcd.setCursor(0,1);
        lcd.print("Select:"); lcd.setCursor(0,2);
        lcd.print('>');
        printCropHelper(0); lcd.setCursor(1,3);
        printCropHelper(1); lcd.setCursor(11,3);
        lcd.write(0);
        cursor[ROW] = 2;
        break;
    }
    case 1:{
        lcd.setCursor(1,0);
        printCropHelper(2); lcd.setCursor(1,1);
        printCropHelper(3); lcd.setCursor(1,2);
        printCropHelper(4); lcd.setCursor(1,3);
        printCropHelper(5); lcd.setCursor(11,3);
        lcd.write(0);
        cursor[ROW] = 0;
        break;
    }
    case 2:{      
        lcd.setCursor(1,0);
        printCropHelper(6); lcd.setCursor(1,1);
        lcd.print("Go back"); lcd.setCursor(11,0);
        lcd.write(1);
        cursor[ROW] = 0;
        break;
    }
    }
    currentDisplay = CROP_LCD;
}
void UserInterface::printTimeLCD(void){
    createCustomSymbols(1);
    currentDisplay = TIME_LCD;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Changing Time");lcd.setCursor(16,3);
    lcd.print("Done");

    printBigNum(database.getHour()/10,menu[currentDisplay].initialPosition,1);
    printBigNum(database.getHour() - (database.getHour()/10)*10,menu[currentDisplay].initialPosition+3,1);
    printBigNum(database.getMinute()/10,menu[currentDisplay].initialPosition+7,1);
    printBigNum(database.getMinute() - (database.getMinute()/10)*10,menu[currentDisplay].initialPosition+10,1);
    lcd.setCursor(13,2);
    (database.getPM())?lcd.print("pm"):lcd.print("am");
    lcd.setCursor(6,1); lcd.print('o');
    lcd.setCursor(6,2); lcd.print('o');

    cursor[COLUMN] = menu[currentDisplay].initialPosition;
    lcd.setCursor(cursor[COLUMN], 3);
    lcd.print("---");
}
void UserInterface::printValveStatusLCD(void){
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(" Go back"); lcd.setCursor(1,1);
    lcd.print(" Valve1:"); printValveStatusHelper(database.getValveStatus(0),1); lcd.setCursor(0,2);
    lcd.print(" Valve2:"); printValveStatusHelper(database.getValveStatus(1),1); lcd.setCursor(0,3);
    lcd.print(" Valve3:"); printValveStatusHelper(database.getValveStatus(2),1); 
    currentDisplay = VALVE_STATUS_LCD;
}
void UserInterface::printCropThresholdLCD(void){
    lcd.clear();
    switch(page){
    case 0:{
        lcd.print(">Select to restore"); lcd.setCursor(0,1);
        lcd.print(" default settings"); lcd.setCursor(0,2);
        lcd.print(" Crop"); lcd.setCursor(9,2); 
        lcd.print("Threshold"); lcd.setCursor(1,3);
        printCropHelper(0); lcd.setCursor(12,3); 
        lcd.print(database.getCropThreshold(0)); lcd.print('%'); lcd.setCursor(18,3);
        lcd.write(0);
        cursor[ROW] = 0;
        break;
    }
    case 1:{
        lcd.setCursor(1,0);
        printCropHelper(1); lcd.setCursor(12,0); lcd.print(database.getCropThreshold(1)); lcd.print('%'); lcd.setCursor(1,1);
        printCropHelper(2); lcd.setCursor(12,1); lcd.print(database.getCropThreshold(2)); lcd.print('%'); lcd.setCursor(1,2);
        printCropHelper(3); lcd.setCursor(12,2); lcd.print(database.getCropThreshold(3)); lcd.print('%'); lcd.setCursor(1,3);
        printCropHelper(4); lcd.setCursor(12,3); lcd.print(database.getCropThreshold(4)); lcd.print('%'); lcd.setCursor(18,3);
        lcd.write(0);
        cursor[ROW] = 0;
        break;
    }
    case 2:{      
        lcd.setCursor(1,0);
        printCropHelper(5); lcd.setCursor(12,0); lcd.print(database.getCropThreshold(5)); lcd.print('%'); lcd.setCursor(1,1);
        printCropHelper(6); lcd.setCursor(12,1); lcd.print(database.getCropThreshold(6)); lcd.print('%'); lcd.setCursor(1,2);
        lcd.print("Go back"); lcd.setCursor(18,0);
        lcd.write(1);
        cursor[ROW] = 0;
        break;
    }
    }
    currentDisplay = CROP_THRESHOLD_LCD;
}
void UserInterface::printThreshholdNumLCD(void){
    createCustomSymbols(1);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Curr Thresh: "); lcd.print(database.getCropThreshold(database.getCurrentCrop())); lcd.print("%"); lcd.setCursor(0,3);
    printCropHelper(database.getCurrentCrop()); lcd.setCursor(16,3);
    lcd.print("Done");
    printBigNum(database.getCropThreshold(database.getCurrentCrop()),7,1);
    currentDisplay = THRESHOLD_NUM_LCD;
    cursor[COLUMN] = menu[currentDisplay].initialPosition;
    lcd.setCursor(cursor[COLUMN], 3);
    lcd.print("---");
}
void UserInterface::printScheduleLCD(void){
    lcd.clear();
    switch(page){
    case 0:{
        lcd.print(">M"); lcd.setCursor(0, 1);
        lcd.print(" T"); lcd.setCursor(0, 2);
        lcd.print(" W"); lcd.setCursor(0, 3);
        lcd.print(" Th");
        cursor[ROW] = 0;
        break;
    }
    case 1:{
        lcd.print(" F"); lcd.setCursor(0, 1);
        lcd.print(" S"); lcd.setCursor(0, 2);
        lcd.print(" Su"); lcd.setCursor(0, 3);
        lcd.print(" Change time");
        cursor[ROW] = 0;
        break;
    }
    case 2:{
        lcd.print(" Go back");
        cursor[ROW] = 0;
        break;
    }
    }
    currentDisplay = SCHEDULE_LCD;
}
void UserInterface::printDisplay(uint8_t display){
    switch(display){
        case MAIN_LCD:{printMainLCD(); break;}
        case MENU_LCD:{printMenuLCD(); break;}
        case VALVE_SENSOR_LCD:{printValveSensorLCD(); break;}
        case CROP_LCD:{printCropLCD(); break;}
        case TIME_LCD:{printTimeLCD(); break;}
        case VALVE_STATUS_LCD:{printValveStatusLCD(); break;}
        case CROP_THRESHOLD_LCD:{printCropThresholdLCD(); break;}
        case THRESHOLD_NUM_LCD:{printThreshholdNumLCD(); break;}
        case SCHEDULE_LCD:{printScheduleLCD(); break;}
    }
}


void UserInterface::printValveStatusHelper(uint8_t status, bool printType){
    if(!printType){
        if(status == 0){
            lcd.print("C");
        }
        else{
            lcd.print("O");
        }
    }
    else{
        if(status == 0){
            lcd.print("Close");
        }
        else{
            lcd.print("Open");
        }
    }
}
void UserInterface::printTimeHelper(uint8_t hour, uint8_t minute, bool isPM, uint8_t col, uint8_t row){
    lcd.setCursor(col,row);
    lcd.print(hour, DEC);
    lcd.print(':');

    if(minute <10){
        lcd.print('0');
    }
    lcd.print(minute, DEC);
    if(isPM){
        lcd.print("pm ");
    }
    else{
        lcd.print("am ");
    }
}
void UserInterface::printSoilSensorHelper(uint8_t num, uint8_t col, uint8_t row){
    // lcd.setCursor(col+1,row); lcd.print(' ');
    // lcd.setCursor(col,row);
    // lcd.print(valve[0].soilMoistureValue);
}

void UserInterface::printCropHelper(uint8_t num){
    switch(num){
        case 0:{lcd.print("Corn"); break;}
        case 1:{lcd.print("Bean");break;}
        case 2:{lcd.print("Cotton");break;}
        case 3:{lcd.print("Tomato");break;}
        case 4:{lcd.print("Potato");break;}
        case 5:{lcd.print("Tobacco");break;}
        case 6:{lcd.print("Papaya");break;}
    }
}
void UserInterface::printBigNum(uint8_t num, uint8_t col, uint8_t row){
  if(num < 100 && num >= 0){
    uint8_t numOfLoop = 1;
    uint8_t temp = 0;
    if(num > 9){
      temp = num;
      numOfLoop = 2;
      num = num/10;
    }
    for(int i=0;i<numOfLoop;i++){
      lcd.setCursor(col,row);
      for(int i=0;i<3;i++){
        lcd.write(customCharacter.bigNums[num][i]);
      }
      lcd.setCursor(col,row+1);
      for(int i=3;i<6;i++){
        lcd.write(customCharacter.bigNums[num][i]);
      }
      num = temp%(num*10);
      col = col+3;
    }
  }
}

void UserInterface::createCustomSymbols(uint8_t set){
    customCharacter.setCharacterSet(set);
    switch(set){
    case 0:{
      for(int i=0;i<customCharacter.getSize();i++){
        lcd.createChar(i,customCharacter.getCharacter(i));
        }
      break;
      }
    case 1:{
      for(int i=0;i<8;i++){
        lcd.createChar(i, customCharacter.getCharacter(i));
        }
      break;
      }
    }
}

void UserInterface::blinkCursor(void){
    blinkStatus = !blinkStatus;
    lcd.setCursor(cursor[COLUMN],3);
    (blinkStatus?lcd.print("   "):lcd.print("---"));
}
void UserInterface::idle(void){
    if(skip == 1){
        skip = 0;
    }
    else{
        printMainLCD();
        lcd.noBacklight();
        idleStatus = true;
    }
}
void UserInterface::wakeup(void){
    lcd.backlight();
    idleStatus = false;
}
void UserInterface::resetLCDValue(void){
    cursor[ROW] = 0;
    cursor[COLUMN] = 0;
    cursor[MODE] = 0;
    cursor[BLINK] = 1;
    page = 0;
}


void UserInterface::updateCursor(int8_t direction){
  if(menu[currentDisplay].cursorType == 0){
    //Set to currect cursor location
    lcd.setCursor(0, cursor[ROW]);
    //Erase old cursor
    lcd.print(' ');
    //If current display is on the main page, print menu display and return
    if(currentDisplay == MAIN_LCD){
      printMenuLCD();
      return;
    }
    //If there are only 4 rows in the display, don't have to worry about pages
    if(menu[currentDisplay].totalRows <5){
      //If UP button was pressed (direction = 1)
      if(cursor[ROW]==0 && direction == 1){
        //Move current cursor up 1 row
        cursor[ROW] = menu[currentDisplay].totalRows-1;
      }
      //Else if DOWN button was pressed (direction = 0)
      else{
        //Move current cursor down 1 row
        cursor[ROW] = (cursor[ROW]-direction)%menu[currentDisplay].totalRows;
      }
    }
    //More than 4 rows, will be more than 1 page
    else{
      //Special case for threshold lcd
      if(currentDisplay == CROP_THRESHOLD_LCD){
        if(page == 0 && cursor[ROW] == 0 && direction == -1){
          cursor[ROW] = 2;
        }
        //Set current cursor position to row 1
        else if(page == 0 && cursor[ROW] == 3 && direction == 1){
          cursor[ROW] = 1;
        }
      }
      if(page > 0 && cursor[ROW] ==  0 && direction == 1){
        page--;
        printDisplay(currentDisplay);
        cursor[ROW] = 3;
        lcd.setCursor(0,menu[currentDisplay].initialPosition);
        lcd.print(' ');
      }
      else if(page < menu[currentDisplay].totalPages && cursor[ROW] == 3 && direction == -1){
        page++;
        printDisplay(currentDisplay);
        cursor[ROW] = 0;
      }
      else if((page == 0 && cursor[ROW] == menu[currentDisplay].initialPosition && direction == 1 && menu[currentDisplay].isCircular)){
        page = menu[currentDisplay].totalPages;
        printDisplay(currentDisplay);
        cursor[ROW] = menu[currentDisplay].lastPosition;
      }
      else if(page == menu[currentDisplay].totalPages && cursor[ROW] == menu[currentDisplay].lastPosition && direction == -1 && menu[currentDisplay].isCircular){
        page = 0;
        printDisplay(currentDisplay);
        cursor[ROW] = menu[currentDisplay].initialPosition;
      }
      else if(  !(page == 0 && cursor[ROW] == menu[currentDisplay].initialPosition && direction == 1) &&
                !((page == menu[currentDisplay].totalPages) && (cursor[ROW] == menu[currentDisplay].lastPosition) && direction == -1)){
        cursor[ROW] = cursor[ROW]-direction;
      }
    }
    lcd.setCursor(0,cursor[ROW]);
    lcd.print('>');
  }
  else{
    if(currentDisplay == THRESHOLD_NUM_LCD){
      if((cursor[COLUMN] == menu[currentDisplay].initialPosition && direction == -1) || 
          (cursor[COLUMN] == 15 && direction == 1)){
        return;
      }
      if(cursor[COLUMN] == menu[currentDisplay].lastPosition && direction == 1){
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print("   ");
        cursor[COLUMN] = 15;
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print('>');
      }
      else if(cursor[COLUMN] == 15){
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print(" ");
        cursor[COLUMN] = menu[currentDisplay].lastPosition;
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print("---");
      }
      else{  
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print("   ");
        cursor[COLUMN] = cursor[COLUMN] + direction * 3;
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print("---");
      }
    }
    else{
      if((cursor[COLUMN] == menu[currentDisplay].initialPosition && direction == -1) || 
          (cursor[COLUMN] == 15 && direction == 1)){
        return;
      }
      lcd.setCursor(cursor[COLUMN],3);
      if(cursor[COLUMN] == menu[currentDisplay].lastPosition && direction == 1){
        lcd.print("  ");
        cursor[COLUMN] = 15;
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print('>');
      }
      else if((cursor[COLUMN] == 3 && direction == 1) || (cursor[COLUMN] == 7 && direction == -1)){
        lcd.print("   ");
        cursor[COLUMN] = cursor[COLUMN] + direction * 4;
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print("---");
      }
      else if((cursor[COLUMN] == 15 && direction == -1) || (cursor[COLUMN] == menu[currentDisplay].lastPosition-3 && direction == 1)){
        lcd.print(" ");
        cursor[COLUMN] = menu[currentDisplay].lastPosition;
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print("--");
      }
      else{
        lcd.print("   ");
        cursor[COLUMN] = cursor[COLUMN] + direction * 3;
        lcd.setCursor(cursor[COLUMN],3);
        lcd.print("---");
      }

    }
  }
}
void UserInterface::updateBigNumber(int8_t direction){
    uint8_t tempPos = 0;
    if (currentDisplay == THRESHOLD_NUM_LCD)
    {
        tempPos = (cursor[COLUMN] == menu[currentDisplay].initialPosition) ? 0 : 1;
        if (!((tempArray[tempPos] == 0 && direction == -1) || (tempArray[tempPos] == 9 && direction == 1)))
        {
            tempArray[tempPos] += direction;
            printBigNum(tempArray[tempPos], cursor[COLUMN], 1);
        }
    }
}
void UserInterface::selectButton(void)
{
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
            database.setCurrentValve(cursor[ROW]-1);
            page = 0;
            printCropLCD();
        }
        break;
    }
    case CROP_LCD:
    {
        if (page == menu[currentDisplay].totalPages && cursor[ROW] == menu[currentDisplay].lastPosition)
        {
            printValveSensorLCD();
            return;
        }
        if (page == 0)
        {
            database.setCrop(database.getCurrentValve(), cursor[ROW] - 2);
        }
        //Increment by 4 after page 1
        else if (page == 1)
        {
            database.setCrop(database.getCurrentValve(),cursor[ROW] + 2);
        }
        else if (page == 2)
        {
            database.setCrop(database.getCurrentValve(),cursor[ROW] + 6);
        }
        printValveSensorLCD();
        break;
    }
    case CROP_THRESHOLD_LCD:
    {
        if (page == menu[currentDisplay].totalPages && cursor[ROW] == menu[currentDisplay].lastPosition)
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

            break;
        }
        if (page == 0)
        {
            database.setCurrentCrop(cursor[ROW] - 3);
        }
        //Increment 4 by  after page 1
        else if (page == 1)
        {
            database.setCurrentCrop(cursor[ROW] + 1);
        }
        else if (page == 2)
        {
            database.setCurrentCrop(cursor[ROW] + 5);
        }
        printThreshholdNumLCD();
        tempArray[0] = database.getCropThreshold(database.getCurrentCrop()) / 10;
        tempArray[1] = database.getCropThreshold(database.getCurrentCrop()) - tempArray[0] * 10;
        break;
    }
    case THRESHOLD_NUM_LCD:
    {
        if (cursor[COLUMN] == 15)
        {
            //Recreate symbols
            createCustomSymbols(0);
            //Save new threshold value into crop array
            database.setCropThreshold(database.getCurrentCrop(),tempArray[0] * 10 + tempArray[1]);
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
            if (blinkStatus)
            {
                lcd.setCursor(cursor[COLUMN], 3);
                lcd.print("---");
                blinkStatus = !blinkStatus;
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
        if (page == menu[currentDisplay].totalPages && cursor[ROW] == menu[currentDisplay].lastPosition)
        {
            printMenuLCD();
        }
        else if (page == 1 && cursor[ROW] == 3)
        {
            printTimeLCD();
        }
        break;
    }
    case TIME_LCD:
    {
        if (cursor[COLUMN] == 15)
        {
            //Recreate symbols
            createCustomSymbols(0);
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
            if (blinkStatus)
            {
                lcd.setCursor(cursor[COLUMN], 3);
                lcd.print("---");
                blinkStatus = !blinkStatus;
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
    }
}
