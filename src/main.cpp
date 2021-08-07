#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <avr/sleep.h>
#include <semphr.h>

#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "RTClib.h"

#include "AT24C32_eeprom.h"
#include "constants.h"
#include "user_interface.h"

using namespace constants;

//TO DO:
//Disable and enable valve
//Save data to memory if power loss. Read mem and compare before writing to memory
//Change month/day/year , maybe just day of week
//Reduce Power: Test sleep mode in idle state
//				Add a conditional statement for all task if schedule is ENABLE but not active
//              Call a task every _____ to calculate how long it should stay asleep until next deadline if schedule is ENABLE
//
//Enable/Disable schedule: 	Run in fixed interval if schedule is DISABLED
//							Run in fixed interval at correct time if schedule is ENABLED
//							Attempt to use RTC to trigger an interrupt
//Implement code for a low power fan

//Current Consumption when idle:
/*
NONE					68.2mA-69mA
SLEEP_MODE_IDLE 		56mA
SLEEP_MODE_PWR_SAVE		28.9mA
SLEEP_MODE_ADC
SLEEP_MODE_STANDBY
SLEEP_MODE_PWR_DOWN
*/

//External Libraries
//If error after adding library on platformio. Try Ctrl+Shift+P -> Rebuild IntelliSense

//Ctrl+K, Ctrl+F: auto format selection
//Ctrl+K, Ctrl+0: Fold all
//Ctrl+K, Ctrl+J: Unfold all

DHT dht(DHT_PIN, DHT11);
RTC_DS3231 rtc;
//Need to find usable memory addr
//AT24C32 mem;

UserInterface Interface(0x27, 20, 4);

void vTaskButton(void *pvParameters);
void vTaskTemperature(void *pvParameters);
void vTaskRTC(void *pvParameters);
void vTaskSoilSensor(void *pvParameters);
void vTaskValve(void *pvParameters);
// void TaskBlink(void *pvParameters);
void vTaskSaveData(void *pvParameters);
void vTaskAlarm(void *pvParameters);

void vButtonUP_ISR_Handler(void);
void vButtonDOWN_ISR_Handler(void);
void vButtonSELECT_ISR_Handler(void);
void vAlarm_ISR_Handler(void);

void debugFun(void);

SemaphoreHandle_t xButtonSemaphore,
	xValveSemaphore,
	xBlinkSemaphore;

TaskHandle_t xButtonTaskHandle = NULL,
			 xSaveTaskHandle = NULL,
			 xAlarmTaskHandle = NULL,
			 xSoilTaskHandle = NULL;

uint8_t buttonPressed = 0;
volatile bool 	LEDtemp = LOW,
				needWater = false,
				alarmFired = false;
//bool needWater = false;


void setup()
{
	DIDR0 = 0xFF;
	DIDR2 = 0xFF;
	ACSR &= ~_BV(ACIE);
	ACSR |= _BV(ACD);


	Serial.begin(9600);
	Interface.begin();

	//Initalize real time clock
	if (!rtc.begin())
	{
		Serial.println("Error. Couldn't find RTC");
	}

	// //Readjust time after shut down
	if (rtc.lostPower())
	{
		Serial.println("RTC lost power, setting the time");
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}
	
	rtc.disable32K();
	rtc.disableAlarm(2);
	rtc.clearAlarm(1);
	rtc.writeSqwPinMode(DS3231_OFF);

	xValveSemaphore = xSemaphoreCreateBinary();

	//Stack 128, 256, 400, 512 
	xTaskCreate(vTaskButton, "Button", 400, NULL, 3, &xButtonTaskHandle);
	xTaskCreate(vTaskTemperature, "Temperature", 400, NULL, 2, NULL);
	xTaskCreate(vTaskRTC, "RTC", 400, NULL, 1, NULL);
	xTaskCreate(vTaskSoilSensor, "Soil", 256, NULL, 2, &xSoilTaskHandle);
	xTaskCreate(vTaskValve, "Valve", 128, NULL, 3, NULL);
	xTaskCreate(vTaskAlarm, "Alarm", 600, NULL, 3, &xAlarmTaskHandle);


	//xTaskCreate(TaskSaveData, "Save", 400, NULL, 4, &xSaveTaskHandle);

	Interface.printDisplay(MAIN_LCD);

	//debugFun();

	vTaskStartScheduler();
}


void loop()
{
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	portENTER_CRITICAL();
	sleep_enable();
#if defined(BODS) && defined(BODSE)
	sleep_bod_disable();
#endif
	portEXIT_CRITICAL();
	sleep_cpu();
	sleep_reset();
}

void vAlarm_ISR_Handler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	alarmFired = true;
	vTaskNotifyGiveFromISR(xAlarmTaskHandle, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR();
	}
}

void vButtonUP_ISR_Handler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	buttonPressed = UP;
	vTaskNotifyGiveFromISR(xButtonTaskHandle, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR();
	}
}
void vButtonDOWN_ISR_Handler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	buttonPressed = DOWN;
	vTaskNotifyGiveFromISR(xButtonTaskHandle, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR();
	}
}
void vButtonSELECT_ISR_Handler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	buttonPressed = SELECT;
	vTaskNotifyGiveFromISR(xButtonTaskHandle, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR();
	}
}


void vTaskButton(void *pvParameters)
{
	(void)pvParameters;
	//User interface buttons
	pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
	pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(BUTTON_UP_PIN), vButtonUP_ISR_Handler, LOW);
	attachInterrupt(digitalPinToInterrupt(BUTTON_DOWN_PIN), vButtonDOWN_ISR_Handler, LOW);
	attachInterrupt(digitalPinToInterrupt(BUTTON_SELECT_PIN), vButtonSELECT_ISR_Handler, LOW);

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// if (Interface.isIdle())
		// {
		// 	Interface.wakeup();
		// }
		// else
		// {
		bool prevScheduleState = Interface.schedule.isEnable();
		//Interface.idleResetCounter();
		Interface.update(buttonPressed);
		if (prevScheduleState != Interface.schedule.isEnable() && buttonPressed == SELECT)
		{
			if(Interface.schedule.isEnable()){
				xTaskNotifyGive(xAlarmTaskHandle);
			}
			else{
				//vTaskResume(xSoilTaskHandle);
			}
		}
		// }
		if(Interface.schedule.isEnable() && Interface.schedule.isReschedule()){
			Interface.schedule.clearRescheduleFlag();
			xTaskNotifyGive(xAlarmTaskHandle);
		}
		vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE));
		ulTaskNotifyTake(pdTRUE, 0);
	}
	vTaskDelete(xButtonTaskHandle);
}

void vTaskTemperature(void *pvParameters)
{
	(void)pvParameters;

	//Initalize temperature sensor
	dht.begin();

	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xTemperatureFreq = pdMS_TO_TICKS(TEMPERATURE_INTERVAL);

	while (true)
	{
		uint8_t temp = dht.readTemperature(true);
		if (isnan(temp))
		{
			//Failed to read from DHT sensor
			Interface.database.setTemperature(0);
		}
		else
		{
			Interface.database.setTemperature(temp);
			Interface.update(TEMPERATURE);
		}
		xTaskDelayUntil(&xLastWakeTime, xTemperatureFreq);
	}
	vTaskDelete(NULL);
}

void vTaskSoilSensor(void *pvParameters)
{
	(void)pvParameters;

	// Initalize sensors' power pin
	//pinMode(SOIL_SENSOR1_POWER_PIN, OUTPUT);
	// pinMode(SOIL_SENSOR2_POWER_PIN, OUTPUT);
	// pinMode(SOIL_SENSOR3_POWER_PIN, OUTPUT);

	//Initially keep the sensors OFF
	//digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);
	// digitalWrite(SOIL_SENSOR2_POWER_PIN, LOW);
	// digitalWrite(SOIL_SENSOR3_POWER_PIN, LOW);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xSoilSensorFreq = pdMS_TO_TICKS(SOIL_SENSOR_INTERVAL);

	while (true)
	{
		// if(Interface.schedule.isEnable() && !Interface.schedule.isRunning()){
		// 	Interface.database.setValveStatus(0, CLOSE);
		// 	// Interface.database.setValveStatus(1, CLOSE);
		// 	// Interface.database.setValveStatus(2, CLOSE);

		// 	vTaskSuspend(xSoilTaskHandle);
		// }

		//digitalWrite(SOIL_SENSOR1_POWER_PIN, HIGH);
		//digitalWrite(SOIL_SENSOR2_POWER_PIN, HIGH);
		//digitalWrite(SOIL_SENSOR3_POWER_PIN, HIGH);
		//Wait until power is stable
		vTaskDelay(pdMS_TO_TICKS(150));
		// for(int i=0;i<3;i++){
		//Read Soil Sensors
		// uint16_t soilValue = analogRead((i == 0) ? A0 : (i == 1) ? A1
		// 														 : A2);
		uint16_t soilValue = 500;

		soilValue = max(soilValue, WATER_VALUE);
		soilValue = min(soilValue, AIR_VALUE);
		soilValue = map(soilValue, AIR_VALUE, WATER_VALUE, 0, 100);

		//Serial.println(soilValue);
		Interface.database.setSoilSensor(0, soilValue);
		//Turn off sensors
		//Remove outside later
		//digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);

		if (Interface.database.soilSensor(0) < Interface.database.cropThreshold(Interface.database.crop(0)))
		{
			needWater = true;
			xSemaphoreGive(xValveSemaphore);
		}
		else if (Interface.database.valveStatus(0) == HIGH)
		{
			needWater = false;
			xSemaphoreGive(xValveSemaphore);
		}
		Interface.update(SOIL);
			
		// }
		// digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);
		// digitalWrite(SOIL_SENSOR2_POWER_PIN, LOW);
		// digitalWrite(SOIL_SENSOR3_POWER_PIN, LOW);

		xTaskDelayUntil(&xLastWakeTime, xSoilSensorFreq);
	}
	vTaskDelete(NULL);
}

void vTaskValve(void *pvParameters)
{
	(void)pvParameters;

	//Initalize digital pins for relay
	//pinMode(RELAY1_PIN, OUTPUT);

	while (true)
	{
		if (xSemaphoreTake(xValveSemaphore, portMAX_DELAY))
		{
			if (needWater)
			{
				//digitalWrite(RELAY1_PIN, HIGH);
				Interface.database.setValveStatus(0, OPEN);
			}
			else
			{
				//digitalWrite(RELAY1_PIN, LOW);
				Interface.database.setValveStatus(0, CLOSE);
			}
			Interface.update(VALVE);
		}
	}
	vTaskDelete(NULL);
}

void vTaskRTC(void *pvParameters)
{
	(void)pvParameters;

	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xTimerFreq = pdMS_TO_TICKS(RTC_INTERVAL);
	//DS3231_A1_Day
	while (true)
	{
		Interface.database.setTime(rtc.now().twelveHour(),
								   rtc.now().minute(),
								   rtc.now().isPM());
		bool changed = Interface.database.setDayOfWeek(rtc.now().dayOfTheWeek());
		Interface.update(TIME);
		if(changed){
			xTaskNotifyGive(xAlarmTaskHandle);
		}
		// Interface.idleIncrement(RTC_INTERVAL);
		// if(Interface.isIdle()){
		// 	//Check if schedule is ENABLE and determine if controller should be on
		// 	//Call vTaskPoweringDown
		// }
		xTaskDelayUntil(&xLastWakeTime, xTimerFreq);
	}
	vTaskDelete(NULL);
}


void vTaskAlarm(void *pvParameters){
	(void)pvParameters;

	//Check if alarm is enable in setup()
	attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), vAlarm_ISR_Handler, FALLING);

	pinMode(CLOCK_LED_PIN,OUTPUT);
	pinMode(RELAY1_PIN, OUTPUT);

	uint8_t time[2] = {0},
			day = 0,
			myDay = 0;


	if(Interface.schedule.isEnable()){
		//If schedule is enabled on start up, enable rtc alarm
	}


	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		day = rtc.now().dayOfTheWeek();
		myDay = (day == 0) ? 6 : day - 1;
		//TODO: Consider when user enable schedule during time when it should turn on instantly.
		if (!Interface.schedule.isEnable())
		{
			continue;
		}

		if (alarmFired)
		{
			alarmFired = !alarmFired;
			Interface.schedule.next(myDay,time);
		}
		else{
			Interface.schedule.locateClosest(myDay,rtc.now().hour(),rtc.now().minute(),time);
		}

		Interface.update(SCHEDULE_STATUS);
		Serial.print(time[1]);

		if(time[0] == 25){
			digitalWrite(RELAY1_PIN, !digitalRead(RELAY1_PIN));
			continue;
		}

		rtc.clearAlarm(1);

		if (rtc.setAlarm1(
				DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), time[0], time[1], rtc.now().second()),
				DS3231_A1_Hour))
		{
			digitalWrite(CLOCK_LED_PIN, !digitalRead(CLOCK_LED_PIN));
		}
	}
	vTaskDelete(xAlarmTaskHandle);
}


void debugFun(void){
// uint8_t _startHr = 7,
	// 		_startMin = 30,
	// 		_endHr = 12,
	// 		_endMin = 30;
	// bool	_startPM = 1,
	// 		_endPM = 1;

	// uint8_t _to24Hour1 = Interface.schedule.to24Hour(_startHr, _startPM);
	// uint8_t _to24Hour2 = Interface.schedule.to24Hour(_endHr, _endPM);

	// uint16_t startPos = _to24Hour1 * 12 + (_startMin/5),
	//        	 endPos = _to24Hour2 * 12 + (_endMin/5);

	// int duration = 	((int)(endPos - startPos) > 0)?(endPos-startPos):(288-startPos + endPos);

	// Serial.println("o " + String(_to24Hour1));
	// Serial.println("t " + String(_to24Hour2));

	// Serial.println("\nd " + String(duration));
	// Serial.println("s " + String(startPos));
	// Serial.println("e " + String(endPos));

	//--------------------------------------------------------------------------------
	// uint8_t timelineArr[288];
	// //Interface.schedule.clear(0,1,1);
	// for (int i = 0; i < 1; i++)
	// {
	// 	Interface.schedule.getTimeline(i, timelineArr);
	// 	Serial.println("\n\n d:" + String(i));
	// 	for (int i = 0, counter = 0; i < 288; i++)
	// 	{
	// 		if (i % 12 == 0)
	// 		{
	// 			Serial.print("\n" + String(counter) + "\t");
	// 			counter++;
	// 		}
	// 		Serial.print(String(timelineArr[i]) + " ");
	// 	}
	// }
	//--------------------------------------------------------------------------------

	// uint8_t temp[5] = {9, 10,11,12,13};
	// _eeprom.write(20, temp, 5);

	//uint8_t data = 0, data2 = 0;
	// uint8_t dataBuffer[5] = {0};
	//  _eeprom.read(20,dataBuffer,5);

	// Serial.println(dataBuffer[0]);
	// Serial.println(dataBuffer[1]);
	// Serial.println(dataBuffer[2]);
	// Serial.println(dataBuffer[3]);
	// Serial.println(dataBuffer[4]);

	// uint8_t buffer[189] = {99};
	// uint8_t buffer2[189] = {99};

	// Interface.schedule.save(buffer);

	// for(int i=0;i<168;i++){
	// 	if(i%8 == 0 || i==0){
	// 		Serial.println("");
	// 	}
	// 	Serial.print(buffer[i]);
	// 	Serial.print(" ");
	// }



	//Debugging Alarm
	// Serial.println();
	// uint8_t alarm[2] = {0},
	// 		buffer[8] = {0};

	// for(int i=0;i<3;i++){
	// 	Interface.schedule.getInfo(MONDAY,i,buffer);
	// 	for(int j=0;j<8;j++){
	// 		Serial.print((String)buffer[j] + " ");
	// 	}
	// 	Serial.println();
	// }


	// Interface.schedule.locateClosest(MONDAY, 2, 55, alarm);
	// for (int i = 0; i < 2; i++) {
	// 	Serial.print((String)alarm[i] + " ");
	// }
	// for (int i = 0; i < 8; i++) {
	// 	Interface.schedule.next(MONDAY, alarm);
	// 	for (int i = 0; i < 2; i++) {
	// 		Serial.print((String)alarm[i] + " ");
	// 	}
	// }
}

// void TaskSave(void *pvParameters)
// {
// 	(void)pvParameters;
// 	while (true)
// 	{
// 		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
// 		Serial.println("t");

// 	}
// 	vTaskDelete(xSaveTaskHandle);
// }

// void TaskBlink(void *pvParameters)
// {
// 	(void)pvParameters;
// 	while (true)
// 	{
// 		if (xSemaphoreTake(xBlinkSemaphore, portMAX_DELAY))
// 		{
// 			while (Interface.blinkCursor())
// 			{
// 				vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
// 			}
// 		}
// 	}
// 	vTaskDelete(NULL);
// }

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
