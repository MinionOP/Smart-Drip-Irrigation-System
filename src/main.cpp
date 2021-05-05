
#include <Arduino.h>

//External Libraries
//If error after adding library on platformio. Try Ctrl+Shift+P -> Rebuild IntelliSense
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>

#include "UserInterface.h"
#include "DHT.h"
#include "RTClib.h"

//----------------------------------------------------
//Irrigation
#define MAX_SOIL_SENSOR 3
#define MAX_VALVE 3

#define AIR_VALUE 590
#define WATER_VALUE 305

#define SOIL_SENSOR_INTERVAL 10000
#define TEMPERATURE_INTERVAL 10000
#define RTC_INTERVAL 5000
#define IDLE_INTERVAL 100000

//----------------------------------------------------
//Digital pins for turning soil sensors ON/OFF
#define SOIL_SENSOR1_POWER_PIN 11
#define SOIL_SENSOR2_POWER_PIN 12
#define SOIL_SENSOR3_POWER_PIN 13

#define RELAY1_PIN 46

//Temperature Sensor pin
#define DHT_PIN 52
#define DHTTYPE DHT11

//Pin on arduino
//User input
const uint8_t BUTTON_UP_PIN = 8;
const uint8_t BUTTON_DOWN_PIN = 9;
const uint8_t BUTTON_SELECT_PIN = 10;

//----------------------------------------------------
// //Button Functions
// //Read inputs from user
void readButton(int currState, uint8_t buttonType);

//Account for debounce
unsigned long int debounceTimeArr[3] = {0}; //[Down,Up,Sel]
bool buttonState[3] = {1, 1, 1};
bool prevButtonState[3] = {1, 1, 1};
uint8_t debounceDelayToggle = 50;



//------------------------------Scheduling, Time, and Temperature----------------------------------------
struct periodicEvent
{
  unsigned long interval;   //Will run every x. Interval is in milliseconds
  unsigned long prevMillis; //Store prev millis
};

//Periodic Functions
enum periodicFunction
{
  PERIODIC_SOIL = 0,
  PERIODIC_RTC,
  PERIODIC_IDLE,
  PERIODIC_CURSOR_BLINK,
  PERIODIC_FUNC_NUM
};
periodicEvent event[PERIODIC_FUNC_NUM] = {
    SOIL_SENSOR_INTERVAL, 0, //Soil sensor
    RTC_INTERVAL, 0,         //RTC
    IDLE_INTERVAL, 0,        //Idle
    600, 0                   //Blinking cursor
};

//Digital pin used for temperature and type
DHT dht(DHT_PIN, DHTTYPE);
//Create rtc
RTC_DS3231 rtc;


void readSoilSensor(void);
void readTemperatureSensor(void);
//----------------------------------------------------------------------------------------

void runTimeEvents(void);

UserInterface interface(0x27, 20, 4);

//Setup code
void setup()
{

  //User interface buttons
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

  interface.begin();
  //Initalize digital pins for relay
  pinMode(RELAY1_PIN, OUTPUT);

  //Initalize digital pins to turn sensor ON/OFF
  //Initially keep the sensors OFF
  // for(int i=0;i<MAX_SOIL_SENSOR;i++){
  //   pinMode(valve[i].soilSensorPower,OUTPUT);
  //   digitalWrite(valve[i].soilSensorPower, LOW);
  // }

  pinMode(SOIL_SENSOR1_POWER_PIN, OUTPUT);
  digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);

  Serial.begin(9600);

  //Initalize temperature sensor
  dht.begin();
  delay(300);

  //Initalize real time clock
  if (!rtc.begin())
  {
    Serial.println("Error. Couldn't find RTC");
  }

  //Readjust time after shut down
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, setting the time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //Let everything settle down
  delay(1000);
  interface.printDisplay(MAIN_LCD);
}

//Run repeatedly
void loop()
{
  interface.database.setTime(rtc.now().twelveHour(),
                             rtc.now().minute(),
                             rtc.now().isPM());

  //Read buttons
  readButton(digitalRead(BUTTON_DOWN_PIN), DOWN);
  readButton(digitalRead(BUTTON_UP_PIN), UP);
  readButton(digitalRead(BUTTON_SELECT_PIN), SELECT);

  runTimeEvents();
}

//Periodic functions
void runTimeEvents(void)
{
  //Turn off lcd backlight if idle
  if (interface.isIdle() == false)
  {
    if (millis() - event[PERIODIC_IDLE].prevMillis > event[PERIODIC_IDLE].interval)
    {
      event[PERIODIC_IDLE].prevMillis = millis();
      interface.idle();
    }
  }

  if(interface.isBlinkEnable() && (interface.getCurrentDisplay() == THRESHOLD_NUM_LCD || interface.getCurrentDisplay() == TIME_LCD) && interface.getCursorPosition(COLUMN) != 15){
    if(millis() - event[PERIODIC_CURSOR_BLINK].prevMillis > event[PERIODIC_CURSOR_BLINK].interval){
      event[PERIODIC_CURSOR_BLINK].prevMillis = millis();
      interface.blinkCursor();
    }
  }

  //Soil sensor
  if (millis() - event[SOIL].prevMillis > event[SOIL].interval)
  {
    event[SOIL].prevMillis = millis();
    readSoilSensor();
    if (interface.getCurrentDisplay() == MAIN_LCD)
    {
      readTemperatureSensor();
    }
  }

  //RTC
  if (millis() - event[PERIODIC_RTC].prevMillis > event[PERIODIC_RTC].interval)
  {
    event[PERIODIC_RTC].prevMillis = millis();
    interface.update(TIME);
  }
}

// //Read Sensors
void readSoilSensor(void)
{
  //Turn on sensors
  digitalWrite(SOIL_SENSOR1_POWER_PIN, HIGH);
  //Wait until power is stable
  delay(150);
  //Read Soil Sensors;
  //int temp = analogRead(A0);
  //Serial.println(temp);
  //interface.database.setSoilSensor(0, map(temp, AIR_VALUE, WATER_VALUE, 0, 100));
  interface.database.setSoilSensor(0, map(analogRead(A0), AIR_VALUE, WATER_VALUE, 0, 100));
  //Turn off sensors
  digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);

  if (interface.database.getSoilSensor(0) < interface.database.getCropThreshold(interface.database.getCrop(0)))
  {
    digitalWrite(RELAY1_PIN, HIGH);
    interface.database.setValveStatus(0, OPEN);
    interface.update(VALVE);
  }
  else if (interface.database.getValveStatus(0) == HIGH)
  {
    digitalWrite(RELAY1_PIN, LOW);
    interface.database.setValveStatus(0, CLOSE);
    interface.update(VALVE);
  }

  interface.update(SOIL);
}

void readTemperatureSensor(void)
{
  //Read temperature. True for fahrenheit
  uint8_t temp = dht.readTemperature(true);
  if (isnan(temp))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    interface.database.setTemperature(0);
  }
  else
  {
    interface.database.setTemperature(temp);
    interface.update(TEMPERATURE);
  }
}

//Read user input
void readButton(int currState, uint8_t buttonType)
{
  if (currState != prevButtonState[buttonType])
  {
    debounceTimeArr[buttonType] = millis();
  }
  //Serial.println("Current State: " + String(currState) + "    buttonState: " + String(buttonState[buttonType]));
  if ((millis() - debounceTimeArr[buttonType] > debounceDelayToggle) && (currState != buttonState[buttonType]))
  {
    //Serial.println("Current State: " + String(currState) + "    buttonState: " + String(buttonState[buttonType]));
    event[PERIODIC_IDLE].prevMillis = millis();
    buttonState[buttonType] = currState;
    if (interface.isIdle())
    {
      interface.wakeup();
    }
    else if (buttonState[buttonType] == LOW)
    {
      interface.update(buttonType);
    }
  }
  prevButtonState[buttonType] = currState;
}

//Credits
/*
This application uses Open Source components. You can find the links to their 
open source projects along with licenses information below. We acknowledge and 
are grateful to these developers for their contributions to open source.


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
