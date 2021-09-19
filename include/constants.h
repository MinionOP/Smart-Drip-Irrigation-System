#ifndef CONSTANTS_H
#define CONSTANTS_H



namespace constants
{
    constexpr uint8_t MAX_SOIL_SENSOR   = 3;

    constexpr uint8_t MAX_VALVE         = 3;
    constexpr uint16_t AIR_VALUE        = 590;  //Sensor value when exposed to the air                    
    constexpr uint16_t WATER_VALUE      = 305;  //Sensor value completely submerged in water

    constexpr uint32_t SOIL_SENSOR_INTERVAL  = 15000;   //15 seconds
    constexpr uint32_t TEMPERATURE_INTERVAL  = 300000;  //5 minute
    constexpr uint32_t RTC_INTERVAL          = 45000;   //45 seconds
    constexpr uint32_t IDLE_INTERVAL         = RTC_INTERVAL * 2;  //45 seconds
    constexpr uint32_t BATTERY_INTERVAL      = 60000;   //1 minute
    constexpr uint32_t SAVE_INTERVAL         = 600000;   //10 minute


    //Digital pins for turning soil sensors ON/OFF
    constexpr uint8_t SOIL_SENSOR1_POWER_PIN        = 11;
    constexpr uint8_t SOIL_SENSOR2_POWER_PIN        = 12;
    constexpr uint8_t SOIL_SENSOR3_POWER_PIN        = 13;
    constexpr uint8_t SOIL_SENSOR_POWER_INTERVAL    = 150;

    constexpr uint8_t SOIL_SENSOR1_PIN  = A0;
    constexpr uint8_t SOIL_SENSOR1_PIN  = A1;
    constexpr uint8_t SOIL_SENSOR1_PIN  = A2;


    constexpr uint8_t RELAY1_PIN        = 5;
    constexpr uint8_t RELAY2_PIN        = 0;
    constexpr uint8_t RELAY3_PIN        = 0;

    constexpr uint8_t DHT_PIN           = 52;
    constexpr uint8_t CLOCK_LED_PIN     = 4;
    constexpr uint8_t VOLTAGE_SEN_PIN   = A3;


    //Interrupt pins on Arduino Mega
    constexpr uint8_t BUTTON_UP_PIN         = 8;
    constexpr uint8_t BUTTON_DOWN_PIN       = 9;
    constexpr uint8_t BUTTON_SELECT_PIN     = 10;
    constexpr uint8_t BUTTON_INT_PIN        = 2;
    constexpr uint8_t BUTTON_DEBOUNCE       = 130;
    constexpr uint8_t CLOCK_INTERRUPT_PIN   = 19;

    //LED Task Array
    constexpr uint8_t MEMW_LED_PIN     = 47;    
}

#endif