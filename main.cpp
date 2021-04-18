#include <Arduino.h>

//External Libraries
//If error after adding library on platformio. Try Ctrl+Shift+P -> Rebuild IntelliSense
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "RTClib.h"


//User Interface
#define MAX_DISPLAY 6
#define MAIN_LCD 0
#define MENU_LCD 1
#define VALVE_SENSOR_LCD 2
#define CROP_LCD 3
#define TIME_LCD 4 
#define OPEN_CLOSE_VALVE 5

//----------------------------
//Irrigation 
#define MAX_SOIL_SENSOR 3
#define MAX_VALVE       3

#define AIR_VALUE       590
#define WATER_VALUE     314

//Turn on valve if soil moisture falls below threshold
#define CORN_THRESHOLD        80
#define BEAN_THRESHOLD        80
#define COTTON_THRESHOLD      80
#define TOMATO_THRESHOLD      80
#define POTATO_THRESHOLD      80
#define TOBACCO_THRESHOLD     80
#define PAPAYA_THRESHOLD      80

//Crop List
#define MAX_CROP  7
#define CORN      0
#define BEAN      1
#define COTTON    2
#define TOMATO    3
#define POTATO    4
#define TOBACCO   5
#define PAPAYA    6


//----------------------------
//Hold the current cursor location
#define COLUMN  0
#define ROW     1

//Valve Closed/Open
#define CLOSE 0
#define OPEN 1


//----------------------------
//Digital pins for turning soil sensors ON/OFF
#define SOIL_SENSOR1_POWER  11
#define SOIL_SENSOR2_POWER  12
#define SOIL_SENSOR3_POWER  13

//Temperature Sensor pin
#define DHT_PIN 52
#define DHTTYPE DHT11

//Pin on arduino
//User input
const uint8_t BUTTON_UP_PIN = 8;
const uint8_t BUTTON_DOWN_PIN = 9;
const uint8_t BUTTON_SELECT_PIN = 10;


//MOISTURE_SENSOR1_PIN = A0


//----------------------------------------------------------------------------------------
//-------------------------------------User Interface-------------------------------------
//Functions to print on LCD
//Layout of displays can be found under the reference
void printMainLCD(void);
void printMenuLCD(void);
void printValveSensorLCD(void);
void printTimeLCD(void);
void printCropLCD(void);

struct Menu{
  void (*print)();
  uint8_t startPos;   //Row cursor will begin
  uint8_t maxRows;    //Max/Total number of rows
  int32_t maxPage;    //Total Page = maxRows/4
  int32_t lastPos;    //Last Position = (maxRows % 4)-1
  bool circular;      //Circular if cursor loop back to top of list
};

Menu menu[MAX_DISPLAY] = {  printMainLCD, 0, 4, menu[0].maxRows/4, (menu[0].maxRows%4)-1, false,         
                            printMenuLCD, 0, 4, menu[1].maxRows/4, (menu[1].maxRows%4)-1, true,        
                            printValveSensorLCD, 0, 4, menu[2].maxRows/4, (menu[2].maxRows%4)-1, true,
                            printCropLCD, 2, 10, menu[3].maxRows/4, (menu[3].maxRows%4)-1, false,
                            printTimeLCD, 0, 4, menu[4].maxRows/4, (menu[4].maxRows%4)-1, false
                            };


//Calling function will update the value on LCD without reprinting the entire display
//Type:
 //Soil Sensor = 0, Temperature Sensor = 1, Valve = 2
void updateValueLCD(uint8_t type);  


//Button Functions
//Read inputs from user
void selectButton(void);
void updateCursor(int8_t direction); //down = -1, up = 1;

//Variables
//Keep track of what display is currently being shown on the LCD
//Example: If the LCD is currently on menu then currDisplay = MENU_LCD
uint8_t currDisplay;

//Hold the current position of the cursor
uint8_t currCursor[2]= {0,0}; //[Columns,Rows]
//Hold the current page. Page will be 0 if we only need 4 rows for a display
uint8_t page = 0;

//Hold the current valve that is being configured
uint8_t currValve = 0;

//Temporary values used for testing the display
uint8_t hour = 9; uint8_t min=50;
int temperature = 0;

//Account for debounce
void debounce(int currState, uint8_t buttonType);
unsigned long int debounceTimeArr[3] = {0,0,0}; //[Down,Up,Sel]
int buttonState[3] = {HIGH, HIGH, HIGH};
int prevButtonState[3] = {HIGH, HIGH, HIGH};
uint8_t debounceDelay = 50;

//------------------------------Scheduling, Time, and Temperature----------------------------------------
//Will run every x seconds
//In milliseconds
unsigned long soilSensorInterval = 3000;  
unsigned long tempSensorInterval = 10000;  
unsigned long rtcInterval = 5000;

unsigned long currMillis;

unsigned long soilPrevMillis = 0;
unsigned long rtcPrevMillis = 0;

//Digital pin used for temperature and type
DHT dht(DHT_PIN, DHTTYPE);

//Create rtc
RTC_DS3231 rtc;

//----------------------------------------------------------------------------------------
//---------------------------------------Irrigation---------------------------------------
struct Valve{
  bool valveStatus;         //Check valve current status (Opened or closed)
  uint8_t currCrop;         //Current crop set for valve
  int soilMoistureValue;    //Moisture sensor value
  uint8_t soilSensorPower;  //Moisture sensor power (Save power)
};

Valve valve[MAX_VALVE] = {CLOSE, CORN, 0, SOIL_SENSOR1_POWER,
                          CLOSE, BEAN, 0, SOIL_SENSOR2_POWER,
                          CLOSE, COTTON, 0, SOIL_SENSOR3_POWER
                          };

char cropList[MAX_CROP][10] = {"Corn", "Bean", "Cotton", "Tomato", "Potato", "Tobacco", "Papaya"};

void readSoilSensor(void);
void readTemperatureSensor(void);
void readValve(void);

//Helper functions
//pos = current valve. type(0) = 'C' or 'O'. type(1) = 'Close' or 'Open'
void printValveStatusHelper(uint8_t pos, bool type);
//print current time/date onto lcd
void printTimeHelper(DateTime&);

//----------------------------------------------------------------------------------------
LiquidCrystal_I2C lcd(0x27,   //lcd_addr. Can be found by running an I2C scanner
                        20,   //Number of columns
                        4);   //Number of rows
                        

//Setup code
void setup() {
  lcd.begin();
  lcd.backlight();

   //User interface buttons
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN,INPUT_PULLUP);

  //Initalize digital pins to turn sensor ON/OFF
	//Initially keep the sensors OFF
  for(int i=0;i<MAX_SOIL_SENSOR;i++){
    pinMode(valve[i].soilSensorPower,OUTPUT);
    digitalWrite(valve[i].soilSensorPower, LOW);
  }

  //Moisture sensor pins
  pinMode(A0, INPUT); 
  pinMode(A1, INPUT); 
  pinMode(A2, INPUT); 
  
  Serial.begin(9600);

  //Initalize temperature sensor
  dht.begin();
  delay(300);


  if(!rtc.begin()){
    Serial.println("Error. Couldn't find RTC");
  }

  //Readjust time after shut down
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //Let everything settle down
  delay(1000);

  menu[MAIN_LCD].print();
}


//Run repeatedly
void loop() {
  DateTime now = rtc.now();
  //Read buttons
  debounce(digitalRead(BUTTON_DOWN_PIN), 0);
  debounce(digitalRead(BUTTON_UP_PIN), 1);
  debounce(digitalRead(BUTTON_SELECT_PIN), 2);

  currMillis = millis();
  if(currMillis - soilPrevMillis > soilSensorInterval){
    soilPrevMillis = currMillis;
    readSoilSensor();
    if(currDisplay == MAIN_LCD){
      readTemperatureSensor();
    }
  }


  currMillis = millis();
  if(currMillis - rtcPrevMillis > rtcInterval){
    rtcPrevMillis = currMillis;
    if(currDisplay == MAIN_LCD){
      if(hour>9){
        lcd.setCursor(5,1);
        }
      else{
        lcd.setCursor(6,1);
        }
      printTimeHelper(now);
    }
  }
}

//User Interface functions
//Print different display
void printMainLCD(void){
  lcd.clear();
  lcd.noCursor();

  lcd.print("Smart Irrigation"); lcd.setCursor(0,1);
  lcd.print("Time:"); lcd.setCursor(0,2);
  lcd.print("Current Temp:"); lcd.setCursor(0,3);
  lcd.print("V1:");printValveStatusHelper(0,0);lcd.setCursor(7,3); //Change later when valve are use
  lcd.print("V2:");printValveStatusHelper(1,0);lcd.setCursor(14,3);
  lcd.print("V3:");printValveStatusHelper(2,0);

  //Update time on LCD
  if(hour>9){
    lcd.setCursor(5,1);}
  else{
      lcd.setCursor(6,1);}
  

  //Update temperature on LCD
  lcd.setCursor(14,2);
  lcd.print(temperature); lcd.print("F");

  currCursor[ROW] = 0;
  currDisplay = MAIN_LCD;
}
void printMenuLCD(void){
  lcd.clear();
  lcd.print(">Sensor Setting"); lcd.setCursor(0,1);
  lcd.print(" Open/Close Valve"); lcd.setCursor(0,2);
  lcd.print(" Set Schedule"); lcd.setCursor(0,3);
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
  lcd.print(" Valve1:"); lcd.print(cropList[valve[0].currCrop]); lcd.setCursor(0,2);
  lcd.print(" Valve2:"); lcd.print(cropList[valve[1].currCrop]); lcd.setCursor(0,3);
  lcd.print(" Valve3:"); lcd.print(cropList[valve[2].currCrop]); 
  
  lcd.setCursor(17,1);
  lcd.print(valve[0].soilMoistureValue); lcd.print(" %"); lcd.setCursor(17,2);
  lcd.print(valve[1].soilMoistureValue); lcd.print(" %"); lcd.setCursor(17,3);
  lcd.print(valve[2].soilMoistureValue); lcd.print(" %");

  currCursor[ROW] = 0;
  page = 0;
  currDisplay = VALVE_SENSOR_LCD;
}
void printCropLCD(void){
  lcd.clear();
  switch(page){
    case 0:{
      lcd.print("Current Crop:"); lcd.print(cropList[valve[currValve].currCrop]); lcd.setCursor(0,1);
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


void updateValueLCD(uint8_t type){
  switch(type){
    case 0:{
      lcd.setCursor(18,1); lcd.print(' ');
      lcd.setCursor(18,2); lcd.print(' ');
      lcd.setCursor(18,3); lcd.print(' ');

      lcd.setCursor(17,1);
      lcd.print(valve[0].soilMoistureValue); lcd.setCursor(17,2);
      lcd.print(valve[1].soilMoistureValue); lcd.setCursor(17,3);
      lcd.print(valve[2].soilMoistureValue);
    }
    case 1:{

    }
    case 2:{

    }
  }
}
//Read Sensors
void readSoilSensor(void){
  //Turn on sensors
  digitalWrite(valve[0].soilSensorPower,HIGH);
  //Wait until power is stable
  delay(200);  

  //Read Soil Sensors;
  valve[0].soilMoistureValue = analogRead(A0);
  valve[0].soilMoistureValue = map(valve[0].soilMoistureValue,AIR_VALUE, WATER_VALUE, 0, 100);
  //Serial.println(soilMoistureValue[0]);
  //Turn off sensors
  digitalWrite(valve[0].soilSensorPower,LOW);
  if(currDisplay == VALVE_SENSOR_LCD){
    updateValueLCD(0);
  }

}
void readTemperatureSensor(void){
  temperature = dht.readTemperature(true);
  if (isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }
  //Serial.println(temperature);
  if(currDisplay == MAIN_LCD){
    lcd.setCursor(14,2);
    lcd.print(temperature); lcd.print("F");
  }
}
//Read user input
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
void selectButton(){
  switch(currDisplay){
    case MAIN_LCD:{
      printMenuLCD();
      break;}
    case MENU_LCD:{
      if(currCursor[ROW] == 0){printValveSensorLCD();}
      else if(currCursor[ROW] == 1){}
      else if(currCursor[ROW] == 2){printTimeLCD();}
      else if(currCursor[ROW] == 3){printMainLCD();}
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
      if(page == 0){valve[currValve].currCrop = currCursor[ROW]-2;}
      //Increment by 4 after page 1
      else if(page == 1){valve[currValve].currCrop = currCursor[ROW]+2;}
      else if(page == 2){valve[currValve].currCrop = currCursor[ROW]+6;}
      printValveSensorLCD();
      break;}
  }
}


//Print current status of valve(Opened or closed) onto LCD
void printValveStatusHelper(uint8_t pos, bool type){
  if(!type){
    if(valve[pos].valveStatus == CLOSE){
      lcd.print("C");
    }
    else{
      lcd.print("O");
    }
  }
  else{
    if(valve[pos].valveStatus == CLOSE){
      lcd.print("Close");
    }
    else{
      lcd.print("Open");
    }
  }
}
//Print current date/time onto LCD
void printTimeHelper(DateTime& now){
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  if(now.isPM()){
    lcd.print("pm");
  }
  else{
    lcd.print("am");
  }
}

//Credits
/*
This application uses Open Source components. You can find the source code of their 
open source projects along with license information below. We acknowledge and are 
grateful to these developers for their contributions to open source.


ArduinomLiquidCrystal I2C https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library

Adafruit Unified Sensor https://github.com/adafruit/Adafruit_Sensor
Apache License Version 2.0, January 2004 https://github.com/adafruit/Adafruit_Sensor/blob/master/LICENSE.txt

DHT sensor https://github.com/adafruit/DHT-sensor-library
Copyright (c) 2020 Adafruit Industries
License (MIT) https://github.com/adafruit/DHT-sensor-library/blob/master/license.txt

RTClib https://github.com/adafruit/RTClib
Copyright (c) 2019 Adafruit Industries
License (MIT) https://github.com/adafruit/RTClib/blob/master/license.txt

*/



//--------------------------Reference---------------------------
/*
//----------MainLCD-------------
//Smart Irrigation
//Time: 5:20pm
//Current Temp: 85F
//V1:C   V2:O   V3:C
//------------------------------

//----------MenuLCD-------------
// Sensor Setting
// Open/Close Valve
// Set Schedule
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
*/
//--------------------------------------------------------------
