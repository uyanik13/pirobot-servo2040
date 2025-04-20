#pragma once

#include "pico/stdlib.h"

// If we're using the servo namespace from the pimoroni library
#ifdef USE_SERVO_NAMESPACE
#include "servo2040.hpp"  // From Pimoroni
namespace servo_defs = servo::servo2040;
#else
// Define our own constants if not using the Pimoroni namespace

// Constants for Servo 2040
namespace servo_defs {
    // Servo pins
    constexpr uint SERVO_1 = 0;
    constexpr uint SERVO_2 = 1;
    constexpr uint SERVO_3 = 2;
    constexpr uint SERVO_4 = 3;
    constexpr uint SERVO_5 = 4;
    constexpr uint SERVO_6 = 5;
    constexpr uint SERVO_7 = 6;
    constexpr uint SERVO_8 = 7;
    constexpr uint SERVO_9 = 8;
    constexpr uint SERVO_10 = 9;
    constexpr uint SERVO_11 = 10;
    constexpr uint SERVO_12 = 11;
    constexpr uint SERVO_13 = 12;
    constexpr uint SERVO_14 = 13;
    constexpr uint SERVO_15 = 14;
    constexpr uint SERVO_16 = 15;
    constexpr uint SERVO_17 = 16;
    constexpr uint SERVO_18 = 17;
    constexpr uint NUM_SERVOS = 18;

    // LED and sensor pins
    constexpr uint LED_DATA = 18;
    constexpr uint NUM_LEDS = 6;

    constexpr uint I2C_INT = 19;
    constexpr uint I2C_SDA = 20;
    constexpr uint I2C_SCL = 21;

    constexpr uint USER_SW = 23;

    constexpr uint ADC_ADDR_0 = 22;
    constexpr uint ADC_ADDR_1 = 24;
    constexpr uint ADC_ADDR_2 = 25;

    constexpr uint ADC0 = 26;
    constexpr uint ADC1 = 27;
    constexpr uint ADC2 = 28;
    constexpr uint SHARED_ADC = 29;

    // Sensor addresses
    constexpr uint SENSOR_1_ADDR = 0b000;
    constexpr uint SENSOR_2_ADDR = 0b001;
    constexpr uint SENSOR_3_ADDR = 0b010;
    constexpr uint SENSOR_4_ADDR = 0b011;
    constexpr uint SENSOR_5_ADDR = 0b100;
    constexpr uint SENSOR_6_ADDR = 0b101;
    constexpr uint NUM_SENSORS = 6;

    constexpr uint VOLTAGE_SENSE_ADDR = 0b110;
    constexpr uint CURRENT_SENSE_ADDR = 0b111;

    // Analog calibration constants
    constexpr float SHUNT_RESISTOR = 0.003f;
    constexpr float CURRENT_GAIN = 69;
    constexpr float VOLTAGE_GAIN = 3.9f / 13.9f;
    constexpr float CURRENT_OFFSET = -0.02f;
}
#endif 