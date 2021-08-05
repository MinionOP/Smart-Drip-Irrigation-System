#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constants
{
    constexpr uint8_t MAX_SOIL_SENSOR   = 3;
    constexpr uint8_t MAX_VALVE         = 3;

    constexpr uint16_t AIR_VALUE        = 590;  //Sensor value when exposed to the air                    
    constexpr uint16_t WATER_VALUE      = 305;  //Sensor value completely submerged in water

    //In milliseconds 
    constexpr uint32_t SOIL_SENSOR_INTERVAL  = 5000;   //5 seconds
    constexpr uint32_t TEMPERATURE_INTERVAL  = 300000;   //5 minute
    constexpr uint32_t RTC_INTERVAL          = 60000;   //60 seconds

    //Digital pins for turning soil sensors ON/OFF
    constexpr uint8_t SOIL_SENSOR1_POWER_PIN        = 11;
    constexpr uint8_t SOIL_SENSOR2_POWER_PIN        = 12;
    constexpr uint8_t SOIL_SENSOR3_POWER_PIN        = 13;
    constexpr uint8_t SOIL_SENSOR_POWER_INTERVAL    = 150;

    constexpr uint8_t RELAY1_PIN        = 46;
    constexpr uint8_t RELAY2_PIN        = 0;
    constexpr uint8_t RELAY3_PIN        = 0;

    constexpr uint8_t DHT_PIN           = 52;
    constexpr uint8_t CLOCK_LED_PIN     = 4;

    //Interrupt pins on Arduino Mega
    constexpr uint8_t BUTTON_UP_PIN         = 2;
    constexpr uint8_t BUTTON_DOWN_PIN       = 3;
    constexpr uint8_t BUTTON_SELECT_PIN     = 18;
    constexpr uint8_t BUTTON_DEBOUNCE       = 100;
    constexpr uint8_t CLOCK_INTERRUPT_PIN   = 19;
}

#endif