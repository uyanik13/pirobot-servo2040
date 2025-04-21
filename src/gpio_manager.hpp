#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "servo2040.hpp"
#include <cstdint>

using namespace servo;

// GPIO Pin Definitions
static constexpr uint8_t A0_GPIO_PIN = 26;  // RELAY pin
static constexpr uint8_t A1_GPIO_PIN = 27;  // A1 pin
static constexpr uint8_t A2_GPIO_PIN = 28;  // A2 pin

// GPIO Masks
static constexpr uint32_t GPIO_A0_MASK = (1u << A0_GPIO_PIN);
static constexpr uint32_t GPIO_A1_MASK = (1u << A1_GPIO_PIN);
static constexpr uint32_t GPIO_A2_MASK = (1u << A2_GPIO_PIN);
static constexpr uint32_t GPIO_OUTPUT_MASK = 0xFFFFFFFF;
static constexpr uint32_t GPIO_LOW_MASK = 0x00;

// Command constants
static constexpr uint8_t CMD_SET_PIN = 0xD3;  // Set pin command
static constexpr uint8_t CMD_GET_PIN = 0xC7;  // Get pin command

/**
 * @brief GPIO pinlerini (A0, A1, A2) yöneten sınıf
 */
class GPIOManager {
public:
    GPIOManager();
    void init();
    void setA0(bool state);
    void setA1(bool state);
    void setA2(bool state);
    bool getA0();
    bool getA1();
    bool getA2();
    void handleCommand(uint8_t cmd, uint8_t pin, uint8_t value);
    
private:
    void _setPin(uint8_t pin, bool state);
    bool _getPin(uint8_t pin);
}; 