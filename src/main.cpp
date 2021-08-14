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
//Increase to 5 soil sensors
//Option to adjust number of valve to sensor (1 sensor can control 2 valve, etc)

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
void vTaskSaveData(void *pvParameters);
void vTaskAlarm(void *pvParameters);


void vButton_ISR_Handler(void);
void vAlarm_ISR_Handler(void);

void debugFun(void);

SemaphoreHandle_t xButtonSemaphore,
	xValveSemaphore,
	xBlinkSemaphore;

TaskHandle_t xButtonTaskHandle = NULL,
			 xSaveTaskHandle = NULL,
			 xAlarmTaskHandle = NULL,
			 xSoilTaskHandle = NULL;

volatile uint8_t buttonPressed = 0,
				 idleCounter = 0;

volatile bool 	 needWater[3] = {false},
			  	 alarmFired = false,
			  	 isSensorEnabled = true;

//bool needWater = false;

void setup()
{

	Serial.begin(9600);

	DIDR0 = 0xFF;
	DIDR2 = 0xFF;
	ACSR &= ~_BV(ACIE);
	ACSR |= _BV(ACD);

	Interface.begin();
	Interface.printDisplay(MAIN_LCD);

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
	xTaskCreate(vTaskTemperature, "Temperature", 400, NULL, 1, NULL);
	xTaskCreate(vTaskRTC, "RTC", 400, NULL, 2, NULL);
	xTaskCreate(vTaskSoilSensor, "Soil", 256, NULL, 2, &xSoilTaskHandle);
	xTaskCreate(vTaskValve, "Valve", 256, NULL, 3, NULL);
	xTaskCreate(vTaskAlarm, "Alarm", 600, NULL, 3, &xAlarmTaskHandle);

	//xTaskCreate(TaskSaveData, "Save", 400, NULL, 4, &xSaveTaskHandle);


	//debugFun();

	vTaskStartScheduler();
}

void loop()
{
	if (!Interface.isIdle() && idleCounter == IDLE_INTERVAL / RTC_INTERVAL)
	{
		Interface.idle();
		idleCounter = 0;
	}
	
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

void vButton_ISR_Handler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	idleCounter = 0;
	if (digitalRead(BUTTON_SELECT_PIN) == LOW)
	{
		buttonPressed = SELECT;
	}
	else if (digitalRead(BUTTON_UP_PIN) == LOW)
	{
		buttonPressed = UP;
	}
	else
	{
		buttonPressed = DOWN;
	}
	vTaskNotifyGiveFromISR(xButtonTaskHandle, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR();
	}
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

void vTaskButton(void *pvParameters)
{
	(void)pvParameters;
	//User interface buttons
	pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
	pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(BUTTON_INT_PIN), vButton_ISR_Handler, FALLING);

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (Interface.isIdle())
		{
			Interface.wakeup();
		}
		else
		{
			bool prevScheduleState = Interface.schedule.isEnable(),
				 toUpdateValve = false,
				 toUpdateAlarm = false;			
			Interface.update(buttonPressed);

			if (Interface.getCurrentDisplay() == VALVE_SENSOR_LCD)
			{
				for (int i = 0; i < 3; i++)
				{
					if (Interface.database.getValveFlag(i) == 1)
					{
						toUpdateValve = true;
						needWater[i] = 0;
						Interface.database.clearValveFlag(i);
					}
				}
			}

			if (toUpdateValve)
			{
				xSemaphoreGive(xValveSemaphore);
			}

			if (prevScheduleState != Interface.schedule.isEnable() && buttonPressed == SELECT)
			{
				if (Interface.schedule.isEnable())
				{
					toUpdateAlarm = true;
				}
				else
				{
					rtc.clearAlarm(1);
					vTaskResume(xSoilTaskHandle);
					isSensorEnabled = true;
				}
			}

			if (Interface.schedule.isEnable() && Interface.schedule.isReschedule())
			{
				Interface.schedule.clearRescheduleFlag();
				toUpdateAlarm = true;
			}
			if(Interface.database.isRTCFlagEnable()){
				Interface.database.clearRTCFlag();
				uint8_t time[2] = {Interface.database.get24Hour(),
								   Interface.database.getMinute()};

				uint8_t calenderDate[3] = {8,13,21};

				Interface.database.getCalenderDate(calenderDate);
				// Interface.printToLCD(0, 2, time, 2);
				// Interface.printToLCD(10, 2, calenderDate, 3);

				rtc.adjust(DateTime(calenderDate[2]+2000, 	//year
									calenderDate[0], 		//month
									calenderDate[1], 		//day
									time[0], 				//24-hour
									time[1], 				//minute
									0)						//second
									);

				Interface.database.setDayOfWeek(rtc.now().dayOfTheWeek());

				if(Interface.schedule.isEnable()){
					toUpdateAlarm = true;
				}
			}
			if(toUpdateAlarm){
				xTaskNotifyGive(xAlarmTaskHandle);
			}
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
		// if (Interface.schedule.isEnable() && !Interface.schedule.isRunning())
		// {
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
		bool toUpdate = false;

		soilValue = max(soilValue, WATER_VALUE);
		soilValue = min(soilValue, AIR_VALUE);
		soilValue = map(soilValue, AIR_VALUE, WATER_VALUE, 0, 100);

		//Serial.println(soilValue);
		Interface.database.setSoilSensor(0, soilValue);
		//Turn off sensors
		//Remove outside later
		//digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);

		if (Interface.database.soilSensor(0) < Interface.database.cropThreshold(Interface.database.crop(0)) &&
			Interface.database.valveStatus(0) == CLOSE)
		{
			needWater[0] = true;
			toUpdate = true;
		}
		else if (Interface.database.soilSensor(0) > Interface.database.cropThreshold(Interface.database.crop(0)) &&
				 Interface.database.valveStatus(0) == OPEN)
		{
			needWater[0] = false;
			toUpdate = true;
		}

		if(toUpdate){
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
	//pinMode(RELAY2_PIN, OUTPUT);
	//pinMode(RELAY3_PIN, OUTPUT);


	while (true)
	{
		if (xSemaphoreTake(xValveSemaphore, portMAX_DELAY))
		{
			for (int i = 0; i < MAX_SOIL_SENSOR; i++)
			{
				if (needWater[i])
				{
					//digitalWrite(RELAY1_PIN, HIGH);
					Interface.database.setValveStatus(i, OPEN);
				}
				else
				{
					//digitalWrite(RELAY1_PIN, LOW);
					Interface.database.setValveStatus(i, CLOSE);
				}
			}
			Interface.update(VALVE);
		}
	}
	vTaskDelete(NULL);
}

void vTaskRTC(void *pvParameters)
{
	(void)pvParameters;
	bool initialPowerUp = true;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xTimerFreq = pdMS_TO_TICKS(RTC_INTERVAL);
	while (true)
	{
		if(initialPowerUp){
			vTaskDelay(pdMS_TO_TICKS(200));
			initialPowerUp = false;
		}
		Interface.database.setCalenderDate(rtc.now().month(), rtc.now().day(), rtc.now().year());
		Interface.database.setTime(rtc.now().twelveHour(),
								   rtc.now().minute(),
								   rtc.now().isPM());
		bool changed = Interface.database.setDayOfWeek(rtc.now().dayOfTheWeek());
		Interface.update(TIME);
		if (changed)
		{
			xTaskNotifyGive(xAlarmTaskHandle);
		}

		if (!Interface.isIdle())
			idleCounter++;
		xTaskDelayUntil(&xLastWakeTime, xTimerFreq);
	}
	vTaskDelete(NULL);
}

void vTaskAlarm(void *pvParameters)
{
	(void)pvParameters;

	//Check if alarm is enable in setup()
	attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), vAlarm_ISR_Handler, FALLING);

	pinMode(CLOCK_LED_PIN, OUTPUT);
	pinMode(RELAY1_PIN, OUTPUT);

	uint8_t time[2] = {0},
			day = 0,
			myDay = 0;

	if (Interface.schedule.isEnable())
	{
		//If schedule is enabled on start up, enable rtc alarm
	}

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		day = rtc.now().dayOfTheWeek();
		myDay = (day == 0) ? 6 : day - 1;
		//TODO: Consider when user enable schedule during time when it should turn on instantly.
		//Instead of circling back in reorder table, just stop arming interrupt in schedule.next
		if (!Interface.schedule.isEnable())
		{
			continue;
		}

		if (alarmFired)
		{
			alarmFired = !alarmFired;
			Interface.schedule.next(myDay, time);
		}
		else
		{
			Interface.schedule.locateClosest(myDay, rtc.now().hour(), rtc.now().minute(), time);
		}

		Interface.update(SCHEDULE_STATUS);

		if (!Interface.schedule.isRunning() && isSensorEnabled)
		{	
			vTaskSuspend(xSoilTaskHandle);
			for(int i=0;i<3;i++)
				needWater[i] = false;
			xSemaphoreGive(xValveSemaphore);
			isSensorEnabled = false;
		}
		else{
			vTaskResume(xSoilTaskHandle);
			isSensorEnabled = true;
		}

		//-------------------------------------------------
		uint8_t buffer[5];
		buffer[0] = myDay;

		Interface.printToLCD(0, 2, time, 2);
		Interface.printToLCD(0, 3, buffer, 1);
		//-------------------------------------------------

		if (time[0] == 25)
		{
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

void debugFun(void)
{
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
