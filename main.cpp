#include <Arduino.h>

//External Libraries
#include <LiquidCrystal_I2C.h> //Taken from: https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library


//User Interface
#define MAX_DISPLAY 5
#define MAIN_LCD 0
#define MENU_LCD 1
#define VALVE_SENSOR_LCD 2
#define CROP_LCD 3
#define TIME_LCD 4 

#define MAX_PAGE 2

//User input
//Pin on arduino
const uint8_t BUTTON_UP_PIN = 8;
const uint8_t BUTTON_DOWN_PIN = 9;
const uint8_t BUTTON_SELECT_PIN = 10;

//Crop List
#define MAX_CROP 7
#define CORN    0
#define BEAN    1
#define COTTON  2
#define TOMATO  3
#define POTATO  4
#define toADD1  5
#define toADD2  6


//Hold the current cursor location
#define COLUMN  0
#define ROW     1


//----------------------------------------------------------------------------------------
//-------------------------------------User Interface-------------------------------------
//Functions to print on LCD


void printMainLCD(void);
void printMenuLCD(void);
void printValveSensorLCD(void);
void printTimeLCD(void);
void printCropLCD(void);

struct Menu{
  void (*print)();
  uint8_t startPos;
  uint8_t maxRows;
  int32_t maxPage;    //Total Page = maxRows/4
  int32_t lastPos;    //Last Position = (maxRows % 4)-1
  bool circular;
};

Menu menu[MAX_DISPLAY] = {  printMainLCD, 0, 3, menu[0].maxRows/4, (menu[0].maxRows%4)-1, false,         
                            printMenuLCD, 0, 3, menu[1].maxRows/4, (menu[1].maxRows%4)-1, true,        
                            printValveSensorLCD, 0, 4, menu[2].maxRows/4, (menu[2].maxRows%4)-1, true,
                            printCropLCD, 2, 10, menu[3].maxRows/4, (menu[3].maxRows%4)-1, false,
                            printTimeLCD, 0, 4, menu[4].maxRows/4, (menu[4].maxRows%4)-1, false
                            };

//Button Functions
void readButtons(void);
//Read inputs from user
void selectButton(void);

//Helper Funciton
void updateCursor(int8_t direction); //down = -1, up = 1;

//Variables
uint8_t currDisplay;
uint8_t currCursor[2]= {0,0}; //[Columns,Rows]
uint8_t hour = 9; uint8_t min=50;
uint8_t temperature = 85;
uint8_t page = 0;
uint8_t currValve = 0; //Testing

//Account for debounce
void debounce(int currState, uint8_t buttonType);
unsigned long int debounceTimeArr[3] = {0,0,0}; //[Down,Up,Sel]
int buttonState[3] = {HIGH, HIGH, HIGH};
int prevButtonState[3] = {HIGH, HIGH, HIGH};
uint8_t debounceDelay = 50;



//----------------------------------------------------------------------------------------
//------------------------------------------Crops-----------------------------------------
char cropList[MAX_CROP][10] = {"Corn", "Bean", "Cotton", "Tomato", "Potato", "toAdd", "toAdd"};
uint8_t currCrops[3] = {CORN,BEAN,COTTON}; //Valve 1-3
uint8_t currSensorValue[3] = {0,0,0};

//----------------------------------------------------------------------------------------

LiquidCrystal_I2C lcd(0x27,   //lcd_addr. Can be found by running an I2C scanner
                        20,   //Number of columns
                        4);   //Number of rows
                        

//Setup code
void setup() {
  lcd.begin();
  lcd.backlight();
  Serial.begin(9600);
  menu[MAIN_LCD].print();

  //User interface buttons
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN,INPUT_PULLUP);
}

//Run repeatedly
void loop() {
  readButtons();
}

void printMainLCD(void){
  lcd.clear();
  lcd.noCursor();

  lcd.print("Smart Irrigation"); lcd.setCursor(0,1);
  lcd.print("Time:"); lcd.setCursor(0,2);
  lcd.print("Current Temp:"); lcd.setCursor(0,3);
  

  //Update time on LCD
  if(hour>9){
    lcd.setCursor(5,1);}
  else{
      lcd.setCursor(6,1);}
  
  lcd.print(hour); lcd.print(":"); lcd.print(min);

  //Update temperature on LCD
  lcd.setCursor(14,2);
  lcd.print(temperature);

  currCursor[ROW] = 0;
  currDisplay = MAIN_LCD;
}
void printMenuLCD(void){
  lcd.clear();
  lcd.print(">Valve/Sensors"); lcd.setCursor(0,1);
  lcd.print(" Set Time"); lcd.setCursor(0,2);
  lcd.print(" Go to Main Display");

  currCursor[ROW] = 0;
  currDisplay = MENU_LCD;
}
void printValveSensorLCD(void){
  lcd.clear();
  lcd.print(">Go back"); 
  lcd.setCursor(14,0);
  lcd.print("Sensor");  
  
  lcd.setCursor(0,1);
  lcd.print(" Valve1:"); lcd.print(cropList[currCrops[0]]); lcd.setCursor(0,2);
  lcd.print(" Valve2:"); lcd.print(cropList[currCrops[1]]); lcd.setCursor(0,3);
  lcd.print(" Valve3:"); lcd.print(cropList[currCrops[2]]); 
  
  lcd.setCursor(16,1);
  lcd.print(currSensorValue[0]); lcd.print(" %"); lcd.setCursor(16,2);
  lcd.print(currSensorValue[1]); lcd.print(" %"); lcd.setCursor(16,3);
  lcd.print(currSensorValue[2]); lcd.print(" %");

  currCursor[ROW] = 0;
  page = 0;
  currDisplay = VALVE_SENSOR_LCD;
}
void printCropLCD(void){
  lcd.clear();
  switch(page){
    case 0:{
      lcd.print("Current Crop:"); lcd.print(cropList[currCrops[currValve]]); lcd.setCursor(0,1);
      lcd.print("Select:"); lcd.setCursor(1,2);
      lcd.print(cropList[0]); lcd.setCursor(1,3);
      lcd.print(cropList[1]);
      currCursor[ROW] = 2;
      break;
    }
    case 1:{
      lcd.setCursor(1,0);
      lcd.print(cropList[2]); lcd.setCursor(1,1);
      lcd.print(cropList[3]); lcd.setCursor(1,2);
      lcd.print(cropList[4]); lcd.setCursor(1,3);
      lcd.print(cropList[5]);
      currCursor[ROW] = 0;
      break;
    }
    case 2:{      
      lcd.setCursor(1,0);
      lcd.print(cropList[6]); lcd.setCursor(1,1);
      lcd.print("Go back");
      currCursor[ROW] = 0;
      break;
    }
  }
  currDisplay = CROP_LCD;
}
void printTimeLCD(void){

}

void readButtons(void){
  debounce(digitalRead(BUTTON_DOWN_PIN), 0);
  debounce(digitalRead(BUTTON_UP_PIN), 1);
  debounce(digitalRead(BUTTON_SELECT_PIN), 2);
}
void selectButton(){
  switch(currDisplay){
    case MAIN_LCD:{
      printMenuLCD();
      break;}
    case MENU_LCD:{
      if(currCursor[ROW] == 0){printValveSensorLCD();}
      else if(currCursor[ROW] == 1){printTimeLCD();}
      else if(currCursor[ROW] == 2){printMainLCD();}
      break;}
    case VALVE_SENSOR_LCD:{
      if(currCursor[ROW] == 0){
        printMenuLCD();
      }
      else{
        currValve = currCursor[ROW] - 1;
        page = 0;        
        printCropLCD();
      }
      break;}
    case CROP_LCD:{
      if(page == menu[currDisplay].maxPage && currCursor[ROW] == menu[currDisplay].lastPos){
        printValveSensorLCD(); 
        break;
      }
      if(page == 0){currCrops[currValve] = currCursor[ROW]-2;}
      //Increment by 4 after page 1
      else if(page == 1){currCrops[currValve] = currCursor[ROW]+2;}
      else if(page == 2){currCrops[currValve] = currCursor[ROW]+6;}
      printValveSensorLCD();
      break;}
  }
}


void debounce(int currState, uint8_t buttonType){
  if(currState != prevButtonState[buttonType]){
    debounceTimeArr[buttonType] = millis();
  }
  //Serial.println((String)buttonType + " " + (String)(millis() - debounceTimeArr[buttonType]));
  if(millis() - debounceTimeArr[buttonType] > debounceDelay && currState != buttonState[buttonType]){
    buttonState[buttonType] = currState;
    if(buttonState[buttonType] == LOW){
      if(buttonType == 0){
        updateCursor(-1);
      }
      else if(buttonType == 1){
        updateCursor(1);
      }
      else if(buttonType == 2){
        selectButton();
      }
      //Serial.println((String)currCursor[ROW] + " D: " + (String)currDisplay + " P: "+ (String)page);
    }  
  }
  prevButtonState[buttonType] = currState;
}
void updateCursor(int8_t direction){
  lcd.setCursor(0, currCursor[ROW]);
  lcd.print(' ');
  if(currDisplay == MAIN_LCD){
    menu[MENU_LCD].print();
    return;
  }
  if(menu[currDisplay].maxRows <5){
    if(currCursor[ROW]==0 && direction == 1){
      currCursor[ROW] = menu[currDisplay].maxRows-1;
    }
    else{
      currCursor[ROW] = (currCursor[ROW]-direction)%menu[currDisplay].maxRows;
    }
  }
  else{
    if(page > 0 && currCursor[ROW] ==  0 && direction == 1){
    page--;
    menu[currDisplay].print();
    currCursor[ROW] = 3;
    }
    else if(page < menu[currDisplay].maxPage && currCursor[ROW] == 3 && direction == -1){
      page++;
      menu[currDisplay].print();
      currCursor[ROW] = 0;
    }
    else if(  !(page == 0 && currCursor[ROW] == menu[currDisplay].startPos && direction == 1) &&
              !((page == menu[currDisplay].maxPage) && (currCursor[ROW] == menu[currDisplay].lastPos) && direction == -1)){
      currCursor[ROW] = currCursor[ROW]-direction;
    }
  }
  lcd.setCursor(0,currCursor[ROW]);
  lcd.print('>');
}

//--------------------------Reference---------------------------
/*
//----------MainLCD-------------
//Smart Irrigation
//Time: 5:20 pm
//Current Temp: 85 F
//------------------------------

//----------MenuLCD-------------
// Valve/Sensors
// Adjust Current Time
// Go back to main display
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
// toAdd
//------------------------------

//Page 2
//----------CropLCD-------------
// toAdd
// Go back
// 
// 
//------------------------------
*/
//--------------------------------------------------------------
