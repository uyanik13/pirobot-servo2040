#include "gpio_manager.hpp"
#include "hardware/gpio.h"

// Command constants are now defined in the header file

GPIOManager::GPIOManager() {
}

void GPIOManager::init() {
    // A0/RELAY pini başlat
    gpio_init(A0_GPIO_PIN);
    gpio_set_dir(A0_GPIO_PIN, GPIO_OUT);
    gpio_put(A0_GPIO_PIN, 0);
    
    // A1 pini başlat
    gpio_init(A1_GPIO_PIN);
    gpio_set_dir(A1_GPIO_PIN, GPIO_OUT);
    gpio_put(A1_GPIO_PIN, 0);
    
    // A2 pini başlat
    gpio_init(A2_GPIO_PIN);
    gpio_set_dir(A2_GPIO_PIN, GPIO_OUT);
    gpio_put(A2_GPIO_PIN, 0);
}

void GPIOManager::setA0(bool state) {
    gpio_put(A0_GPIO_PIN, state);
}

bool GPIOManager::getA0() {
    return gpio_get(A0_GPIO_PIN);
}

void GPIOManager::setA1(bool state) {
    gpio_put(A1_GPIO_PIN, state);
}

bool GPIOManager::getA1() {
    return gpio_get(A1_GPIO_PIN);
}

void GPIOManager::setA2(bool state) {
    gpio_put(A2_GPIO_PIN, state);
}

bool GPIOManager::getA2() {
    return gpio_get(A2_GPIO_PIN);
}

// Private helper methods for pin control
void GPIOManager::_setPin(uint8_t pin, bool state) {
    gpio_put(pin, state);
}

bool GPIOManager::_getPin(uint8_t pin) {
    return gpio_get(pin);
}

void GPIOManager::handleCommand(uint8_t cmd, uint8_t pin, uint8_t value) {
    // Check if pin is valid (A0, A1, or A2)
    if (pin == A0_GPIO_PIN || pin == A1_GPIO_PIN || pin == A2_GPIO_PIN) {
        if (cmd == CMD_SET_PIN) {
            // Set pin state
            _setPin(pin, value > 0);
        }
    }
}