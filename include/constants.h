#ifndef CONSTANTS_H
#define CONSTANTS_H



namespace constants
{
    constexpr uint8_t NUM_SOIL_SENSOR   = 3;
    constexpr uint8_t NUM_VALVE         = 3;

    constexpr uint16_t AIR_VALUE        = 590;  //Sensor value when exposed to the air                    
    constexpr uint16_t WATER_VALUE      = 305;  //Sensor value completely submerged in water

    constexpr uint32_t SOIL_SENSOR_INTERVAL  = 10000;   //10 seconds
    constexpr uint32_t RTC_INTERVAL          = 45000;   //45 seconds
    constexpr uint32_t IDLE_INTERVAL         = RTC_INTERVAL * 2;  //45 seconds
    //constexpr uint32_t BATTERY_INTERVAL      = 60000;   //1 minute
    constexpr uint32_t SAVE_INTERVAL         = 300000;   //5 minute


    //Digital pins for turning soil sensors ON/OFF
    constexpr uint8_t SOIL_SENSOR1_POWER_PIN        = 11;
    constexpr uint8_t SOIL_SENSOR2_POWER_PIN        = 12;
    constexpr uint8_t SOIL_SENSOR3_POWER_PIN        = 13;
    constexpr uint8_t SOIL_SENSOR_POWER_INTERVAL    = 150;

    constexpr uint8_t SOIL_SENSOR1_PIN  = A0;
    constexpr uint8_t SOIL_SENSOR2_PIN  = A1;
    constexpr uint8_t SOIL_SENSOR3_PIN  = A2;


    constexpr uint8_t RELAY_1_PIN        = 47;
    constexpr uint8_t RELAY_2_PIN        = 48;
    constexpr uint8_t RELAY_3_PIN        = 49;

    constexpr uint8_t VOLTAGE_SEN_PIN   = A3;

    constexpr uint8_t BUTTON_UP_PIN         = 8;
    constexpr uint8_t BUTTON_DOWN_PIN       = 9;
    constexpr uint8_t BUTTON_SELECT_PIN     = 10;
    constexpr uint8_t BUTTON_DEBOUNCE       = 130;

    //Interrupt pins on Arduino Mega
    constexpr uint8_t BUTTON_INT_PIN        = 2;
    constexpr uint8_t BUTTON_LED_INT_PIN    = 3;
    constexpr uint8_t CLOCK_INTERRUPT_PIN   = 19;

    //LED Task Array
    constexpr uint8_t MEMW_LED         = 37;    
    constexpr uint8_t CLOCK_LED        = 35;
    constexpr uint8_t VALVE_1_LED      = 33;
    constexpr uint8_t VALVE_2_LED      = 31;
    constexpr uint8_t VALVE_3_LED      = 29;

}

#endif