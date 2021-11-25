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


//Uncomment to use debugFun()
//#define DEBUGGING
//#define DEBUG_SOIL

//Uncomment for STACK debugging
// #define DEBUG_STACK
// #define BUTTON_STACK
// #define TEMPERATURE_STACK
// #define SOIL_STACK
// #define ALARM_STACK
// #define SAVE_STACK
// #define RTC_STACK

using namespace constants;

RTC_DS3231 rtc;
AT24C32 mem;

UserInterface Interface(0x27, 20, 4);


/**
 * @brief The button task is responsible for providing a smooth transition 
 *        between the buttons to the LCD. 
 * 		  
 *        The four pushbuttons: Up, Down, Select, and LED are initialize 
 * 		  by this task when powered. The 5k internal resistors in the Arduino are 
 *        used as pull-up resistors. 
 * 
 * 		  Task is delayed indefinitely until it is call by the button's interrupt routine
 */
void vTaskButton(void *pvParameters);

/**
 * @brief Read time/date and temperature from ds3231 and update LCD 
 */
void vTaskRTC(void *pvParameters);

/**
 * @brief Soil sensor task is responsible for reading moisture values
 * 		  and calling valve task. Task is 'blocked' if scheduler is enabled
 *        and is awake when scheduler is running or disabled
 * 
 * 		  The task is run every xSoilSensorFreq
 */
void vTaskSoilSensor(void *pvParameters);

// /**
//  * @brief Responsible for opening/closing valves
//  * 		  Task is delayed indefinitely until it is called
//  */
// void vTaskValve(void *pvParameters);

/**
 * @brief Task is responsible for navigating through the user defined schedule. 
 * 		  It is delayed indefinitely until user enabled the scheduler or called by the
 *  	  Alarm ISR. Once enabled, the task will get the closest available time 
 *        and arm the interrupt pin on the DS3231. 
 */
void vTaskAlarm(void *pvParameters);

/**
 * @brief Save crop's threshold into memory every xSaveFreq
 */
void vTaskSave(void *pvParameters);

/**
 * @brief 	An interrupt routine response when any of the three buttons are 
 * 			pressed. Determines which button was pressed before calling ButtonTask
 */
void vButton_ISR_Handler(void);

/**
 * @brief 	An interrupt routine response when the LED button is pressed.
 * 			Toggle LED status before calling writeAllLED
 */
void vButtonLED_ISR_Handler(void);

/**
 *  @brief  An interrupt routine response triggered by the DS3231 and will
 * 		    call AlarmTask
 */
void vAlarm_ISR_Handler(void);

/**
 * @brief Update LED status array
 * 
 * @param pin - LED pin
 * @param val - HIGH/LOW
 */
void digitalWriteLED(uint8_t pin, uint8_t val);

/**
 * @brief Turn off the LED status array 
 * 
 * @param turnOn 
 */
void writeAllLED(bool turnOn);


/**
 * @brief Responsible for opening/closing valves
 * 
 */
void valveFun(void);

#ifdef DEBUGGING
void debugFun(void);
#endif


TaskHandle_t xButtonTaskHandle = NULL,
			 xAlarmTaskHandle = NULL;


uint8_t relayArr[3] = {RELAY_1_PIN, RELAY_2_PIN, RELAY_3_PIN},
		valveLEDArr[3] = {VALVE_1_LED, VALVE_2_LED, VALVE_3_LED};

volatile uint8_t buttonPressed = 0,
				 idleCounter = 0;

volatile bool needWater[3] = {false},
			  alarmFired = false,
			  isSensorEnabled = true,
			  isLEDArrayEnabled = true,
			  ledArray[5] = {0};


void setup()
{

	Serial.begin(9600);
	DIDR0 = 0xFF;
	DIDR2 = 0xFF;
	ACSR &= ~_BV(ACIE);
	ACSR |= _BV(ACD);

	//Initalize real time clock
	bool loadB = true;
	if (!rtc.begin())
	{
		loadB = false;	
	}

	//Readjust time after shut down
	if (rtc.lostPower())
	{
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}
	//Configure rtc and enable interrupt
	rtc.disable32K();
	rtc.disableAlarm(2);
	rtc.clearAlarm(1);
	rtc.writeSqwPinMode(DS3231_OFF);

	//Begin temperature sensor
	//dht.begin();

	//Configure an LED, a part of status array
	pinMode(CLOCK_LED, OUTPUT);

	//Initalize digital pins for valves' LED
	pinMode(VALVE_1_LED, OUTPUT);
	pinMode(VALVE_2_LED, OUTPUT);
	pinMode(VALVE_3_LED, OUTPUT);

	//Initalize digital pins for relay
	pinMode(RELAY_1_PIN, OUTPUT);
	pinMode(RELAY_2_PIN, OUTPUT);
	pinMode(RELAY_3_PIN, OUTPUT);

	//Relay is reverse. HIGH will turn off the relay.
	//Ensure valves are close in the event of power loss
	digitalWrite(RELAY_1_PIN, HIGH);
	digitalWrite(RELAY_2_PIN, HIGH);
	digitalWrite(RELAY_3_PIN, HIGH);

	uint8_t len = C_SAVE_SIZE + S_SAVE_SIZE,
			loadedDataArr[len] = {0};

	//Load previously saved memory into database
	if(loadB){
		mem.read(20, loadedDataArr, len);
		Interface.database.load(loadedDataArr);
		Interface.schedule.load(loadedDataArr + C_SAVE_SIZE);
	}

	
	//Initalize LCD and user interface
	Interface.begin();
	Interface.printDisplay(MAIN_LCD);

#if defined(DEBUGGING)
	//debugFun();
#else

	xTaskCreate(vTaskRTC, "RTC", 950, NULL, 2, NULL);
	xTaskCreate(vTaskSoilSensor, "Soil", 350, NULL, 3, NULL);
	xTaskCreate(vTaskButton, "Button", 450, NULL, 3, &xButtonTaskHandle);
	xTaskCreate(vTaskAlarm, "Alarm", 860, NULL, 4, &xAlarmTaskHandle);
	xTaskCreate(vTaskSave, "Save", 370, NULL, 5, NULL);

	vTaskStartScheduler();
#endif
}

/**
 * @brief   Idle task that will run when all other tasks are 
 * 			asleep, suspended, or waiting
 */
void loop()
{
#if !defined(DEBUGGING) && !defined(DEBUG_STACK) && !defined(DEBUG_SOIL)
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	portENTER_CRITICAL();
	sleep_enable();
#if defined(BODS) && defined(BODSE)
	sleep_bod_disable();
#endif
	portEXIT_CRITICAL();
	sleep_cpu();
	sleep_reset();
#endif
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


void vButtonLED_ISR_Handler(void)
{
	isLEDArrayEnabled = !isLEDArrayEnabled;
	writeAllLED(isLEDArrayEnabled);
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
#if defined(DEBUG_STACK) && defined(BUTTON_STACK)
	UBaseType_t uxHighWaterMark;
#endif

	pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
	pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
	pinMode(BUTTON_LED_INT_PIN, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(BUTTON_INT_PIN), vButton_ISR_Handler, FALLING);
	attachInterrupt(digitalPinToInterrupt(BUTTON_LED_INT_PIN), vButtonLED_ISR_Handler, FALLING);

	bool prevScheduleState, toUpdateValve, toUpdateAlarm;

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

		prevScheduleState = Interface.schedule.isEnable();
		toUpdateValve = false;
		toUpdateAlarm = false;

		Interface.updateLCD(buttonPressed);

		// Check if any valves were disabled and update the interface accordingly
		if (Interface.getCurrentDisplay() == VALVE_SENSOR_LCD)
		{
			for (int i = 0; i < 3; i++)
			{
				if (Interface.database.getValveFlag(i) == 1)
				{
					toUpdateValve = true;
					needWater[i] = false;
					Interface.database.clearValveFlag(i);
				}
			}
		}

		if (toUpdateValve)
		{
			valveFun();
		}

		//Check if scheduler was recently enabled or disabled
		if (prevScheduleState != Interface.schedule.isEnable() && buttonPressed == SELECT)
		{
			if (Interface.schedule.isEnable())
			{
				toUpdateAlarm = true;
			}
			else
			{
				//Scheduler was disabled. Clear everything and set soil task to be periodically
				rtc.clearAlarm(1);
				digitalWriteLED(CLOCK_LED, LOW);
				if(!isSensorEnabled){
					isSensorEnabled = true;
				}
			}
		}

		if (Interface.schedule.isEnable() && Interface.schedule.isReschedule())
		{
			//Scheduler is currently enabled and something was changed. Update Alarm.
			Interface.schedule.clearRescheduleFlag();
			toUpdateAlarm = true;
		}

		//Check if device's time/date was recently changed
		if (Interface.database.isRTCFlagEnable())
		{
			Interface.database.clearRTCFlag();
			uint8_t time[2] = {Interface.database.get24Hour(),
							   Interface.database.getMinute()};

			uint8_t calenderDate[3] = {0};

			Interface.database.getCalenderDate(calenderDate);
			Interface.database.setDayOfWeek(calenderDate[1]);

			//Update date/time
			rtc.adjust(DateTime(calenderDate[2] + 2000, //year
								calenderDate[0],		//month
								calenderDate[1],		//day
								time[0],				//24-hour
								time[1],				//minute
								0)						//second
			);

			//vTaskDelay(pdMS_TO_TICKS(50));

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
#if defined(DEBUG_STACK) && defined(BUTTON_STACK)
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		Serial.println("BUTTON: " + String(uxHighWaterMark));
#endif
		ulTaskNotifyTake(pdTRUE, 0);
	}
	vTaskDelete(xButtonTaskHandle);
}


void vTaskSoilSensor(void *pvParameters)
{
	(void)pvParameters;
#if defined(DEBUG_STACK) && defined(SOIL_STACK)
	UBaseType_t uxHighWaterMark;
#endif

	// Initalize sensors' power pin
	pinMode(SOIL_SENSOR1_POWER_PIN, OUTPUT);
	pinMode(SOIL_SENSOR2_POWER_PIN, OUTPUT);
	pinMode(SOIL_SENSOR3_POWER_PIN, OUTPUT);

	//Turn the sensors OFF until they are needed
	digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);
	digitalWrite(SOIL_SENSOR2_POWER_PIN, LOW);
	digitalWrite(SOIL_SENSOR3_POWER_PIN, LOW);

	// digitalWrite(SOIL_SENSOR1_POWER_PIN, HIGH);
	// digitalWrite(SOIL_SENSOR2_POWER_PIN, HIGH);
	// digitalWrite(SOIL_SENSOR3_POWER_PIN, HIGH);


	uint8_t s_pin[NUM_SOIL_SENSOR] = {SOIL_SENSOR1_PIN, SOIL_SENSOR2_PIN, SOIL_SENSOR3_PIN};

	//Marker, Data
	//3->S1 
	//2 = S2
	//1 = S3
	uint16_t sensor_lowerBound[NUM_SOIL_SENSOR] = {580, 995, 300},
			 sensor_upperBound[NUM_SOIL_SENSOR] = {1000, 1023, 600},
			 lowestBound = 200,
			 highestBound = 1300,
			 moisture_value;

	bool toUpdate;

	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xSoilSensorFreq = pdMS_TO_TICKS(SOIL_SENSOR_INTERVAL);

	while (true)
	{

		if(!isSensorEnabled){
			xTaskDelayUntil(&xLastWakeTime, xSoilSensorFreq);
			continue;
		}
		//Turn on soil sensors
		digitalWrite(SOIL_SENSOR1_POWER_PIN, HIGH);
		digitalWrite(SOIL_SENSOR2_POWER_PIN, HIGH);
		digitalWrite(SOIL_SENSOR3_POWER_PIN, HIGH);

		//Wait until power is stable
		vTaskDelay(pdMS_TO_TICKS(300));

		for (int i = 0; i < NUM_SOIL_SENSOR; i++)
		{
			if (!Interface.database.isValveAvailable(i))
				continue;

			//Read Soil Sensors
			moisture_value = analogRead(s_pin[i]);

#if defined(DEBUG_SOIL)
			Serial.print("V: " + String(i) + " A: " + String(moisture_value));
#endif
			toUpdate = false;

			if (moisture_value < sensor_lowerBound[i])
			{
				if (moisture_value > lowestBound)
				{
					sensor_lowerBound[i] = moisture_value;
				}
				else
				{
					moisture_value = sensor_lowerBound[i];
				}
			}

			if (moisture_value > sensor_upperBound[i])
			{
				if (moisture_value < highestBound)
				{
					sensor_upperBound[i] = moisture_value;
				}
				else
				{
					moisture_value = sensor_upperBound[i];
				}
			}

			//Calculate moisture value and save to database
			moisture_value = max(moisture_value, sensor_lowerBound[i]);
			moisture_value = min(moisture_value, sensor_upperBound[i]);
			moisture_value = map(moisture_value, sensor_upperBound[i], sensor_lowerBound[i], 0, 100);

#if defined(DEBUG_SOIL)
			Serial.println(" d: " + String(moisture_value));
			if(i == 2){
				Serial.println(" ");
			}
#endif
			Interface.database.setSoilSensor(i, moisture_value);

			//Check if any valves need to be updated
			if (moisture_value < Interface.database.cropThreshold(Interface.database.crop(i)) &&
				Interface.database.valveStatus(i) == CLOSE)
			{
				needWater[i] = true;
				toUpdate = true;
			}
			else if (moisture_value > Interface.database.cropThreshold(Interface.database.crop(i)) &&
					 Interface.database.valveStatus(i) == OPEN)
			{
				needWater[i] = false;
				toUpdate = true;
			}

			if (toUpdate)
			{
				valveFun();
			}
		}

		//Turn off sensors
		digitalWrite(SOIL_SENSOR1_POWER_PIN, LOW);
		digitalWrite(SOIL_SENSOR2_POWER_PIN, LOW);
		digitalWrite(SOIL_SENSOR3_POWER_PIN, LOW);

		Interface.updateLCD(SOIL);

#if defined(DEBUG_STACK) && defined(SOIL_STACK)
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		Serial.println("SOIL: " + String(uxHighWaterMark));
#endif
		xTaskDelayUntil(&xLastWakeTime, xSoilSensorFreq);
	}
	vTaskDelete(NULL);
}


void vTaskRTC(void *pvParameters)
{
	(void)pvParameters;

#if defined(DEBUG_STACK) && defined(RTC_STACK)
	UBaseType_t uxHighWaterMark;
#endif

	bool initialPowerUp = true,
		 changed;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xTimerFreq = pdMS_TO_TICKS(RTC_INTERVAL);

	while (true)
	{
		if (initialPowerUp)
		{
			vTaskDelay(pdMS_TO_TICKS(200));
			initialPowerUp = false;
		}
		Interface.database.setCalenderDate(rtc.now().month(), rtc.now().day(), rtc.now().year());
		Interface.database.setTime(rtc.now().twelveHour(),
								   rtc.now().minute(),
								   rtc.now().isPM());
		changed = Interface.database.setDayOfWeek(rtc.now().dayOfTheWeek());
		Interface.updateLCD(RTC);
#if defined(DEBUG_STACK) && defined(RTC_STACK)
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		Serial.println("RTC: " + String(uxHighWaterMark));
#endif

		if (changed)
		{
			xTaskNotifyGive(xAlarmTaskHandle);
		}

#if !defined(DEBUGGING) && !defined(DEBUG_STACK) && !defined(DEBUG_SOIL)
		if (!Interface.isIdle())
		{
			idleCounter++;
			if (idleCounter == IDLE_INTERVAL / RTC_INTERVAL)
			{
				Interface.idle();
				idleCounter = 0;
			}
		}
		else
		{
			if (Interface.isLCDOn())
			{
				Interface.idle();
			}
		}
#endif

		//Read temperature and update LCD
		float t = rtc.getTemperature() * (1.8) + 32;
		Interface.database.setTemperature((uint8_t)t);
		Interface.updateLCD(TEMPERATURE);


#if defined(DEBUG_STACK) && defined(RTC_STACK)
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		Serial.println("RTC: " + String(uxHighWaterMark));
#endif
		xTaskDelayUntil(&xLastWakeTime, xTimerFreq);
	}
	vTaskDelete(NULL);
}


void vTaskAlarm(void *pvParameters)
{
	(void)pvParameters;
#if defined(DEBUG_STACK) && defined(ALARM_STACK)
	UBaseType_t uxHighWaterMark;
#endif
	pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), vAlarm_ISR_Handler, FALLING);

	uint8_t time[2] = {0}, day = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		day = rtc.now().dayOfTheWeek();
		day = (day == 0) ? 6 : day - 1;
		if (!Interface.schedule.isEnable())
		{
			Interface.updateLCD(SCHEDULE_STATUS);
			digitalWriteLED(CLOCK_LED, LOW);
			Interface.schedule.clearDeadlineFlag();
			continue;
		}

		digitalWriteLED(CLOCK_LED, HIGH);
		if (alarmFired)
		{
			alarmFired = false;
			Interface.schedule.next(day, time);
		}
		else
		{
			Interface.schedule.locateClosest(day, rtc.now().hour(), rtc.now().minute(), time);
		}

		Interface.updateLCD(SCHEDULE_STATUS);
		if (!Interface.schedule.isRunning() && isSensorEnabled)
		{
			for (int i = 0; i < 3; i++)
				needWater[i] = false;
			valveFun();
			isSensorEnabled = false;
		}
		else if (Interface.schedule.isRunning())
		{
			isSensorEnabled = true;
		}

		if (time[0] >= 25 || time[1] > 60 || time[1] % 5 != 0)
		{
			Interface.schedule.clearDeadlineFlag();
			Interface.updateLCD(DEADLINE);

#if defined(DEBUG_STACK) && defined(ALARM_STACK)
			uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
			Serial.println("ALARM TASK: " + String(uxHighWaterMark));
#endif
			continue;
		}

		rtc.clearAlarm(1);
		if (rtc.setAlarm1(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), time[0], time[1], 0), DS3231_A1_Hour))
		{
			Interface.schedule.setDeadline(time);
			Interface.updateLCD(DEADLINE);
		}
#if defined(DEBUG_STACK) && defined(ALARM_STACK)
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		Serial.println("ALARM TASK: " + String(uxHighWaterMark));
#endif
	}
	vTaskDelete(xAlarmTaskHandle);
}



void vTaskSave(void *pvParameters)
{
#if defined(DEBUG_STACK) && defined(SAVE_STACK)
	UBaseType_t uxHighWaterMark;
#endif
	(void)pvParameters;
	pinMode(MEMW_LED, OUTPUT);

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

		//If memory was written when this function last ran, turn off LED
		if (written)
		{
			digitalWrite(MEMW_LED, LOW);
			written = false;
		}

		Interface.database.save(data + CROP_START);
		Interface.schedule.save(data + SCH_START);
		addr_p = 20;

		//Compare current values with values stored in memory. 
		for (int i = 0; i < LEN; i++)
		{
			uint8_t x = mem.read(addr_p);
			if (x != data[i])
			{
				digitalWriteLED(MEMW_LED, HIGH);
				mem.write(addr_p, data[i]);
				written = true;
			}
			addr_p++;
		}
#if defined(DEBUG_STACK) && defined(SAVE_STACK)
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		Serial.println("SAVE TASK: " + String(uxHighWaterMark));
#endif
	}

	vTaskDelete(NULL);
}

void valveFun(void){
	for (int i = 0; i < NUM_VALVE; i++)
	{
		if (needWater[i] && Interface.database.isValveAvailable(i))
		{
#if !defined(DEBUG_SOIL)
			digitalWrite(relayArr[i], LOW);
#endif
			digitalWriteLED(valveLEDArr[i], HIGH);
			Interface.database.setValveStatus(i, OPEN);
		}
		else
		{
#if !defined(DEBUG_SOIL)
			digitalWrite(relayArr[i], HIGH);
#endif
			digitalWriteLED(valveLEDArr[i], LOW);
			Interface.database.setValveStatus(i, CLOSE);
		}
	}
	Interface.updateLCD(VALVE);
}



void digitalWriteLED(uint8_t pin, uint8_t val)
{
	uint8_t i = (pin == VALVE_1_LED) ? 0 : (pin == VALVE_2_LED) ? 1
									   : (pin == VALVE_3_LED)	? 2
									   : (pin == MEMW_LED)		? 3
																: 4;
	ledArray[i] = val;
	if (isLEDArrayEnabled)
	{
		digitalWrite(pin, val);
	}
}


void writeAllLED(bool turnOn)
{
	if (turnOn == false)
	{
		digitalWrite(VALVE_1_LED, LOW);
		digitalWrite(VALVE_2_LED, LOW);
		digitalWrite(VALVE_3_LED, LOW);
		digitalWrite(MEMW_LED, LOW);
		digitalWrite(CLOCK_LED, LOW);
	}
	else
	{
		digitalWrite(VALVE_1_LED, ledArray[0]);
		digitalWrite(VALVE_2_LED, ledArray[1]);
		digitalWrite(VALVE_3_LED, ledArray[2]);
		digitalWrite(MEMW_LED, ledArray[3]);
		digitalWrite(CLOCK_LED, ledArray[4]);
	}
}

#ifdef DEBUGGING
void debugFun(void)
{

	uint8_t CROP_START = 0,
			CROP_END = CROP_START + C_SAVE_SIZE,
			SCH_START = CROP_END,
			SCH_END = SCH_START + S_SAVE_SIZE,
			LEN = SCH_END;

	uint8_t data[LEN] = {0};
	bool written = false;
	unsigned int addr_p = 20;

	Interface.database.save(data + CROP_START);
	Interface.schedule.save(data + SCH_START);

	for(int i=0;i<CROP_END;i++){
		Serial.println(String(i) + ") " + data[i]);
	}
	Serial.println();

	for(int i=SCH_START, j = 0;i<SCH_END;i++){
		Serial.print(data[i]);
		Serial.print(" ");
		j++;
		if(j%8 == 0 ){
			Serial.println(" ");
		}
	}


	mem.read(20, data, LEN);
	for(int i=0;i<LEN;i++){
		Serial.println(String(i) + ") " + data[i]);
	}

	// uint8_t time[2] = {0};
	// Serial.println();
	// Serial.println(Interface.schedule.isRunning());
	// Interface.schedule.locateClosest(SATURDAY, 10, 40, time);
	// Serial.println(Interface.schedule.isRunning());
	// Serial.println((String)time[0] + ":" + (String)time[1]);
	// uint8_t counter = 0;

	// while (time[0] != 25 && counter < 10)
	// {
	// 	Interface.schedule.next(SATURDAY, time);
	// 	Serial.println((String)time[0] + ":" + (String)time[1]);
	// 	counter++;
	// }
	// Serial.println(Interface.schedule.isRunning());

}
#endif