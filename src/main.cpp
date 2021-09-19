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

DHT dht(DHT_PIN, DHT11);
RTC_DS3231 rtc;
AT24C32 mem;

UserInterface Interface(0x27, 20, 4);

void vTaskButton(void *pvParameters);
void vTaskTemperature(void *pvParameters);
void vTaskRTC(void *pvParameters);
void vTaskSoilSensor(void *pvParameters);
void vTaskValve(void *pvParameters);
void vTaskSaveData(void *pvParameters);
void vTaskAlarm(void *pvParameters);
void vTaskSave(void *pvParameters);


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

	
	uint8_t len = C_SAVE_SIZE,
			loadedDataArr[len] = {0};

	mem.read(20,loadedDataArr, len);
	Interface.database.load(loadedDataArr);


	//debugFun();

	xValveSemaphore = xSemaphoreCreateBinary();

	xTaskCreate(vTaskTemperature, "Temperature", 400, NULL, 1, NULL);
	xTaskCreate(vTaskRTC, "RTC", 400, NULL, 2, NULL);
	xTaskCreate(vTaskSoilSensor, "Soil", 256, NULL, 2, &xSoilTaskHandle);
	xTaskCreate(vTaskValve, "Valve", 256, NULL, 3, NULL);
	xTaskCreate(vTaskButton, "Button", 400, NULL, 3, &xButtonTaskHandle);
	xTaskCreate(vTaskAlarm, "Alarm", 1000, NULL, 4, &xAlarmTaskHandle);
	xTaskCreate(vTaskSave, "Save", 400, NULL, 5, &xSaveTaskHandle);


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
			vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE));
			ulTaskNotifyTake(pdTRUE, 0);
			continue;
		}

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
		if (Interface.database.isRTCFlagEnable())
		{
			Interface.database.clearRTCFlag();
			uint8_t time[2] = {Interface.database.get24Hour(),
							   Interface.database.getMinute()};

			uint8_t calenderDate[3] = {8, 13, 21};

			Interface.database.getCalenderDate(calenderDate);
			// Interface.printToLCD(0, 2, time, 2);
			// Interface.printToLCD(10, 2, calenderDate, 3);

			rtc.adjust(DateTime(calenderDate[2] + 2000, //year
								calenderDate[0],		//month
								calenderDate[1],		//day
								time[0],				//24-hour
								time[1],				//minute
								0)						//second
			);

			Interface.database.setDayOfWeek(rtc.now().dayOfTheWeek());

			if (Interface.schedule.isEnable())
			{
				toUpdateAlarm = true;
			}
		}
		if (toUpdateAlarm)
		{
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
		//Turn off sensors
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
	pinMode(RELAY1_PIN, OUTPUT);
	//pinMode(RELAY2_PIN, OUTPUT);
	//pinMode(RELAY3_PIN, OUTPUT);

	digitalWrite(RELAY1_PIN, HIGH);
	//digitalWrite(RELAY2_PIN, HIGH);
	//digitalWrite(RELAY3_PIN, HIGH);

	int relayArr[3] = {RELAY1_PIN, RELAY2_PIN, RELAY3_PIN};

	while (true)
	{
		if (xSemaphoreTake(xValveSemaphore, portMAX_DELAY))
		{
			for (int i = 0; i < MAX_SOIL_SENSOR; i++)
			{
				if (needWater[i])
				{
					if(Interface.database.isValveAvailable(i) && i == 0) //1 relay testing)
						digitalWrite(relayArr[i], LOW);
					Interface.database.setValveStatus(i, OPEN);
				}
				else
				{
					if(i == 0)
						digitalWrite(relayArr[i], HIGH);
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
		{
			idleCounter++;
			if(idleCounter == IDLE_INTERVAL/RTC_INTERVAL){
				Interface.idle();
				idleCounter = 0;
			}
		}
		else{
			if(Interface.isLCDOn()){
				Interface.idle();
			}
		}
		xTaskDelayUntil(&xLastWakeTime, xTimerFreq);
	}
	vTaskDelete(NULL);
}

void vTaskAlarm(void *pvParameters)
{
	(void)pvParameters;

	pinMode(CLOCK_INTERRUPT_PIN,INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), vAlarm_ISR_Handler, FALLING);

	int debugLED1 = 46;

	pinMode(CLOCK_LED_PIN, OUTPUT);
	pinMode(debugLED1, OUTPUT);

	uint8_t time[2] = {0},
			day = 0,
			myDay = 0;


	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		day = rtc.now().dayOfTheWeek();
		//myDay = Interface.database.getDayOfWeek();
		myDay = (day == 0) ? 6 : day - 1;
		//TODO: Consider when user enable schedule during time when it should turn on instantly.
		if (!Interface.schedule.isEnable())
		{
			continue;
		}

		if (alarmFired)
		{
			alarmFired = false;
			Interface.schedule.next(myDay, time);
		}
		else
		{
			Interface.schedule.locateClosest(myDay, rtc.now().hour(), rtc.now().minute(), time);
			uint8_t buf[5] = {9,9,9,9,9};
			Interface.printToLCD(0,0,buf,5);
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


		//-------------------------------------------------
		Interface.printToLCD(0, 2, time, 2);
		Interface.printToLCD(0, 3, buffer, 1);

		if (time[0] == 25)
		{
			digitalWrite(debugLED1, !digitalRead(debugLED1));
			continue;
		}

		rtc.clearAlarm(1);

		if (rtc.setAlarm1(
				DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), time[0], time[1], 0),
				DS3231_A1_Hour))
		{
			digitalWrite(CLOCK_LED_PIN, !digitalRead(CLOCK_LED_PIN));
		}


	}
	vTaskDelete(xAlarmTaskHandle);
}

void vTaskSave(void *pvParameters)
{
	(void)pvParameters;

	pinMode(MEMW_LED_PIN, OUTPUT);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xSaveFreq = pdMS_TO_TICKS(SAVE_INTERVAL);

	uint8_t CROP_START = 0,
			CROP_END = CROP_START + C_SAVE_SIZE,
			SCH_START = CROP_END,
			SCH_END = SCH_START + S_SAVE_SIZE,
			LEN = SCH_END;

	uint8_t data[LEN] = {0};
	bool written = false;
	unsigned int addr_p;

	while (true)
	{
		xTaskDelayUntil(&xLastWakeTime, xSaveFreq);

		Interface.database.save(data + CROP_START);
		Interface.database.save(data + SCH_START);
		addr_p = 20;
		
		for (int i = CROP_START; i < CROP_END; i++)
		{
			uint8_t x = mem.read(addr_p);
			if (x != data[i])
			{
				mem.write(addr_p, data[i]);
				written = true;
			}
			addr_p++;
		}

		if (written)
		{
			digitalWrite(MEMW_LED_PIN, HIGH);
			written = false;
		}
		else if (digitalRead(MEMW_LED_PIN))
		{
			digitalWrite(MEMW_LED_PIN, LOW);
		}

	}

	vTaskDelete(NULL);
}

//Comment out loop when debugging
void debugFun(void)
{
	/*-----------------EEPROM-------------------------------------------
	uint8_t CROP_START    = 0,
			CROP_END      = CROP_START + C_SAVE_SIZE,
			SCH_START     = CROP_END,
			SCH_END       = SCH_START + S_SAVE_SIZE,
			LEN 		  = SCH_END;

	uint8_t ADDR_START	  = 20;		

	uint8_t data[LEN] = {0};
	
	Interface.database.save(data + CROP_START);
	Interface.schedule.save(data + SCH_START);

	// uint8_t data2[100];
	// uint8_t data3[100];

	// for (int i = 0; i < 10; i++)
	// {
	// 	data2[i] = 5;
	// 	data3[i] = 5;
	// }
	// Serial.println("");
	// unsigned int addr_p = ADDR_START;
	// for(int i=0;i<10;i++){
	// 	uint8_t x = mem.read(addr_p);
	// 	Serial.print("addr[" + (String)addr_p + "] " + x + "\t");
	// 	if(x != data[i]){
	// 		mem.write((unsigned int)addr_p,data[i]);
	// 		Serial.println("[" + (String)addr_p + "] " + mem.read(addr_p) + " " + x + " " + data[i]);
	// 	}
	// 	else{
	// 		Serial.println("");
	// 	}
	// 	addr_p++;
	// }

	//mem.write(20,4);
	uint8_t data2[100] = {0},
			data3[100] = {0};

	Serial.println("---------");

	for(int i=0;i<100;i++){
		data3[i] = mem.read(i);
		Serial.println("[" + (String)i + "] " + data3[i]);
	}
	mem.read(0,data2,100);
	Serial.println("---------");
	for(int i=0;i<100;i++){
		Serial.println("[" + (String)i + "] " + data2[i]);
	}

	for(int i=0;i<100;i++){
		if(data2[i] != data3[i]){
			Serial.println("ERROR");
		}
	}

	
	//mem.write(20,1);
	//Serial.println(mem.read(20));
	// mem.read(0,data2,100);
	// mem.read(200,data3,100);


	// for(int i=0;i<100;i++){
	// 	Serial.println((String)i + " " + (String)data2[i]);
	// }

	// Serial.println("----");
	// for(int i=0;i<100;i++){
	// 	Serial.println((String)i + " " + (String)data3[i]);
	// }

	// for (int i = 0, c = 0; i < LEN; i++)
	// {
	// 	if (i < CROP_END)
	// 	{
	// 		Serial.println(data[i]);
	// 		//Serial.println("");
	// 	}
	// 	else
	// 	{
	// 		Serial.print(data[i]);
	// 		Serial.print(" ");
	// 		c++;
	// 		if (c == 8)
	// 		{
	// 			Serial.println("");
	// 			c = 0;
	// 		}
	// 	}
	//}

	//-----------------------------------------------------------*/

	uint8_t time[2] = {0};
	Serial.println();
	Interface.schedule.locateClosest(TUESDAY,12,40,time);
	Serial.println((String)time[0] + ":" + (String)time[1]);
	uint8_t counter=  0;
	while(time[0] != 25 && counter <10){
		Interface.schedule.next(TUESDAY,time);
		Serial.println((String)time[0] + ":" + (String)time[1]);
		counter++;
	}


	/*-----------------------------------------------------
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
	// _eeprom.read(20,dataBuffer,5);
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
	//--------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------*/

}
