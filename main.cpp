#include <Arduino.h>

//External Libraries
#include <LiquidCrystal_I2C.h> //Taken from: https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library


//User Interface
#define MAIN_LCD 0
#define MENU_LCD 1
#define VALVE_SENSOR_LCD 2
#define CROP_LCD 3
#define TIME_LCD 4 

#define MAX_PAGE 2

//User input
//Pin on arduino
const uint8_t BUTTON_UP = 8;
const uint8_t BUTTON_DOWN = 9;
const uint8_t BUTTON_SELECT = 10;

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
void printCropLCD(uint8_t currValve, uint8_t page);

//Button Functions
void readButton(void);

//Read inputs from user
void downButton(void);
void upButton(void);
void selectButton(void);

//Helper Funciton
void printCursor(uint8_t row, char c);

//Variables
volatile uint8_t currDisplayTracker;
volatile uint8_t currCursor[2]= {0,0}; //[Columns,Rows]
uint8_t hour = 9; uint8_t min=50;
uint8_t temperature = 85;
volatile uint8_t page = 0;
uint8_t currValve = 0; //Testing

//Account for debounce
void debounce(int currState, uint8_t buttonType);

uint8_t debounceDelay = 50;
unsigned long int debounceTimeArr[3] = {0,0,0}; //[Down,Up,Sel]
int buttonState[3] = {HIGH, HIGH, HIGH};
int prevButtonState[3] = {HIGH, HIGH, HIGH};


//----------------------------------------------------------------------------------------
//------------------------------------------Crops-----------------------------------------
char cropList[MAX_CROP][10] = {"Corn", "Bean", "Cotton", "Tomato", "Potato", "toAdd", "toAdd"};
volatile uint8_t currCrops[3] = {CORN,CORN,CORN}; //Valve 1-3
volatile uint8_t currSensorValue[3] = {0,0,0};

//----------------------------------------------------------------------------------------

LiquidCrystal_I2C lcd(0x27,   //lcd_addr. Can be found by running an I2C scanner
                        20,   //Number of columns
                        4);   //Number of rows
                        

//Setup code
void setup() {
  lcd.begin();
  Serial.begin(9600);
  lcd.backlight();
  printMainLCD();

  //User interface buttons
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_SELECT,INPUT_PULLUP);
}

int counter = 0;  //Use for testing

//Run repeatedly
void loop() {
  readButton();
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
  currDisplayTracker = MAIN_LCD;
}
void printMenuLCD(void){
  lcd.clear();
  lcd.print(">Valve/Sensors"); lcd.setCursor(0,1);
  lcd.print(" Set Time"); lcd.setCursor(0,2);
  lcd.print(" Go to Main Display");

  currCursor[ROW] = 0;
  currDisplayTracker = MENU_LCD;
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
  currDisplayTracker = VALVE_SENSOR_LCD;
}
void printCropLCD(uint8_t currValve, uint8_t page){
  lcd.clear();
  switch(page){
    case 0:{
      lcd.print("Current Crop:"); lcd.print(cropList[currCrops[currValve]]); lcd.setCursor(0,1);
      lcd.print("Select:"); lcd.setCursor(0,2);
      lcd.print(">"); lcd.setCursor(1,2);
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
  currDisplayTracker = CROP_LCD;
}
void printTimeLCD(void){

}

void readButton(void){
  debounce(digitalRead(BUTTON_DOWN), 0);
  debounce(digitalRead(BUTTON_UP), 1);
  debounce(digitalRead(BUTTON_SELECT), 2);
}
void downButton(){
  switch(currDisplayTracker){
    case MAIN_LCD:{
      printMenuLCD();
      break;}
    case MENU_LCD:{
      printCursor(currCursor[ROW],' ');
      currCursor[ROW] = (currCursor[ROW]+1)%3;
      printCursor(currCursor[ROW],'>');
      break;}
    case VALVE_SENSOR_LCD:{
      printCursor(currCursor[ROW],' ');
      currCursor[ROW] = (currCursor[ROW]+1)%4;
      printCursor(currCursor[ROW],'>');
      break;}
    case CROP_LCD:{
      if(page == MAX_PAGE && currCursor[ROW] == 1){break;}
      if(currCursor[ROW] == 3){
        currCursor[ROW] = 0;
        page++;
        printCropLCD(currValve, page);
        printCursor(currCursor[ROW],'>');
      }
      else{
        printCursor(currCursor[ROW],' ');
        currCursor[ROW] = currCursor[ROW]+1;
        printCursor(currCursor[ROW],'>');
      }
      break;}
    case TIME_LCD:{
      break;}
  }
}
void upButton(){
  switch(currDisplayTracker){
    case MAIN_LCD:{
      printMenuLCD();
      break;}
    case MENU_LCD:{
      printCursor(currCursor[ROW],' ');
      if(currCursor[ROW] <=0){
        currCursor[ROW] = 2;
      }
      else{
        currCursor[ROW] = (currCursor[ROW]-1);
      }
      printCursor(currCursor[ROW],'>');
      break;}
    case VALVE_SENSOR_LCD:{
      printCursor(currCursor[ROW],' ');
      if(currCursor[ROW] <=0){
        currCursor[ROW] = 3;
      }
      else{
        currCursor[ROW] = (currCursor[ROW]-1);
      }
      printCursor(currCursor[ROW],'>');
      break;}
    case CROP_LCD:{
      if(page == 0 && currCursor[ROW] == 2){break;}
      if(currCursor[ROW] == 0){
        page--;
        printCropLCD(currValve, page);
        printCursor(currCursor[ROW], ' ');
        currCursor[ROW] = 3;
        printCursor(currCursor[ROW],'>');
      }
      else{
        printCursor(currCursor[ROW],' ');
        currCursor[ROW] = currCursor[ROW] - 1;
        printCursor(currCursor[ROW], '>');
      }
      break;}
    case TIME_LCD:{
      break;}
  }
}
void selectButton(){
  switch(currDisplayTracker){
    case MAIN_LCD:{
      printMenuLCD();
      break;}
    case MENU_LCD:{
      if(currCursor[ROW] == 0){printValveSensorLCD();}
      else if(currCursor[ROW] == 1){printTimeLCD();}
      else if(currCursor[ROW] == 2){printMainLCD();}
      break;}
    case VALVE_SENSOR_LCD:{
      if(currCursor[ROW] == 0){printMenuLCD();}
      else{
        currValve = currCursor[ROW] - 1;
        printCropLCD(currCrops[currCursor[ROW] - 1],0);
      }
      break;}
    case CROP_LCD:{
      if(page == 2 && currCursor[ROW] == 1){printValveSensorLCD(); break;}
      if(page == 0){currCrops[currValve] = currCursor[ROW]-2;}
      else if(page == 1){currCrops[currValve] = currCursor[ROW]+2;}
      else if(page == 2){currCrops[currValve] = currCursor[ROW]+5;}
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
        downButton();
      }
      else if(buttonType == 1){
        upButton();
      }
      else if(buttonType == 2){
        selectButton();
      }
      //Serial.println((String)currCursor[ROW] + " D: " + (String)currDisplayTracker + " P: "+ (String)page);
    }  
  }
  prevButtonState[buttonType] = currState;
}

void printCursor(uint8_t row, char c){
  lcd.setCursor(0,row);
  lcd.print(c);
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
