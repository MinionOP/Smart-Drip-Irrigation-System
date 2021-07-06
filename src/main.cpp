#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "RTClib.h"

#include "constants.h"
#include "user_interface.h"

using namespace constants;

//External Libraries
//If error after adding library on platformio. Try Ctrl+Shift+P -> Rebuild IntelliSense

//Ctrl+K, Ctrl+F: auto format selection

//DHT
DHT dht(DHT_PIN, DHT11);
//RTC
RTC_DS3231 rtc;

void readSoilSensor(void);
void readTemperatureSensor(void);
void runTimeEvents(void);

UserInterface Interface(0x27, 20, 4);

void TaskButton(void *pvParameters);
void TaskTemperature(void *pvParameters);
void TaskRTC(void *pvParameters);
void TaskSoilSensor(void *pvParameters);
void TaskValve(void *pvParameters);
void TaskBlink(void *pvParameters);


void vButtonUP_ISRHandler(void);
void vButtonDOWN_ISRHandler(void);
void vButtonSELECT_ISRHandler(void);

SemaphoreHandle_t 	xButtonSemaphore, 
					xValveSemaphore, 
					xBlinkSemaphore;


BaseType_t xHigherPriorityTaskWoken = pdFALSE;

bool needWater = false;
uint8_t buttonPressed = 0;

void setup()
{
	// //User interface buttons
	pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
	pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(BUTTON_UP_PIN), vButtonUP_ISRHandler, LOW);
	attachInterrupt(digitalPinToInterrupt(BUTTON_DOWN_PIN), vButtonDOWN_ISRHandler, LOW);
	attachInterrupt(digitalPinToInterrupt(BUTTON_SELECT_PIN), vButtonSELECT_ISRHandler, LOW);


	Interface.begin();
	Serial.begin(9600);

	xButtonSemaphore = xSemaphoreCreateBinary();
	xValveSemaphore = xSemaphoreCreateBinary();
	xBlinkSemaphore = xSemaphoreCreateBinary();

	xTaskCreate(TaskButton, "Button", 128, NULL, 2, NULL);
	xTaskCreate(TaskTemperature, "Temperature", 400, NULL, 1, NULL);
	xTaskCreate(TaskRTC, "RTC", 400, NULL, 0, NULL);
	xTaskCreate(TaskSoilSensor, "Soil", 128, NULL, 1, NULL);
	xTaskCreate(TaskValve, "Valve", 128, NULL, 2, NULL);
	xTaskCreate(TaskBlink, "Blink", 128, NULL, 0, NULL);

	Interface.printDisplay(MAIN_LCD);

	vTaskStartScheduler();
}

void loop() {}

void vButtonUP_ISRHandler(void)
{
	xHigherPriorityTaskWoken = pdFALSE;
	buttonPressed = UP;
	xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
}
void vButtonDOWN_ISRHandler(void)
{
	xHigherPriorityTaskWoken = pdFALSE;
	buttonPressed = DOWN;
	xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
}
void vButtonSELECT_ISRHandler(void)
{
	xHigherPriorityTaskWoken = pdFALSE;
	buttonPressed = SELECT;
	xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
}

void TaskButton(void *pvParameters)
{
	(void)pvParameters;
	while (true)
	{
		if (xSemaphoreTake(xButtonSemaphore, portMAX_DELAY))
		{
			if (Interface.isIdle())
			{
				Interface.wakeup();
			}
			else
			{
				Interface.update(buttonPressed);
				vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE));
				xSemaphoreTake(xButtonSemaphore, 0);
				if(Interface.blinkCheck()){
					xSemaphoreGive(xBlinkSemaphore);
				}
			}
		}
	}
	vTaskDelete(NULL);
}

void TaskTemperature(void *pvParameters)
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

void TaskRTC(void *pvParameters)
{
	(void)pvParameters;

	// //Initalize real time clock
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

	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xTimerFreq = pdMS_TO_TICKS(RTC_INTERVAL);

	while (true)
	{
		Interface.database.setTime(rtc.now().twelveHour(),
								   rtc.now().minute(),
								   rtc.now().isPM());

		Interface.update(TIME);
		Interface.idleIncrement(RTC_INTERVAL);
		xTaskDelayUntil(&xLastWakeTime, xTimerFreq);
	}
	vTaskDelete(NULL);
}

void TaskSoilSensor(void *pvParameters)
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
		//digitalWrite(SOIL_SENSOR1_POWER_PIN, HIGH);
		//Wait until power is stable
		vTaskDelay(pdMS_TO_TICKS(150));

		//Read Soil Sensors
		//uint16_t soilValue = analogRead(A0);
		uint16_t soilValue = 500;

		soilValue = soilValue < WATER_VALUE ? WATER_VALUE : soilValue;
		soilValue = soilValue > AIR_VALUE ? AIR_VALUE : soilValue;

		Interface.database.setSoilSensor(0, map(soilValue, AIR_VALUE, WATER_VALUE, 0, 100));
		//Turn off sensors
		digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);

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
		xTaskDelayUntil(&xLastWakeTime, xSoilSensorFreq);
	}
	vTaskDelete(NULL);
}

void TaskValve(void *pvParameters)
{
	(void)pvParameters;

	//Initalize digital pins for relay
	pinMode(RELAY1_PIN, OUTPUT);

	while (true)
	{
		if (xSemaphoreTake(xValveSemaphore, portMAX_DELAY))
		{
			if (needWater)
			{
				digitalWrite(RELAY1_PIN, HIGH);
				Interface.database.setValveStatus(0, OPEN);
			}
			else
			{
				digitalWrite(RELAY1_PIN, LOW);
				Interface.database.setValveStatus(0, CLOSE);
			}
			Interface.update(VALVE);
		}
	}
	vTaskDelete(NULL);
}

void TaskBlink(void *pvParameters){
	(void)pvParameters;
	while(true){
		if(xSemaphoreTake(xBlinkSemaphore, portMAX_DELAY)){
			while(Interface.blinkCheck()){
				Interface.blinkCursor();
				vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
			}
		}
	}
	vTaskDelete(NULL);
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
