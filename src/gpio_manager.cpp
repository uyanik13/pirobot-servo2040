#include "gpio_manager.hpp"
#include "tusb.h"
#include <stdio.h>

// Command constants are now defined in the header file

GPIOManager::GPIOManager() {
    // USB CDC başlatma işlemi sınıf başlatılırken gerçekleşmemeli
    // init() fonksiyonunda yapılmalı
}

void GPIOManager::init() {
    // TinyUSB başlatma
    tusb_init();
    
    // USB CDC bağlantısı kurulana kadar bekle
    while (!tud_cdc_connected()) {
        sleep_ms(100);
    }
    
    // Initialize GPIO pins
    gpio_init(A0_GPIO_PIN);
    gpio_init(A1_GPIO_PIN);
    gpio_init(A2_GPIO_PIN);

    // Set pins as outputs
    gpio_set_dir(A0_GPIO_PIN, GPIO_OUT);
    gpio_set_dir(A1_GPIO_PIN, GPIO_OUT);
    gpio_set_dir(A2_GPIO_PIN, GPIO_OUT);

    // Set initial states to LOW
    gpio_put(A0_GPIO_PIN, 0);
    gpio_put(A1_GPIO_PIN, 0);
    gpio_put(A2_GPIO_PIN, 0);
    
    sleep_ms(500); // Bağlantının kararlı hale gelmesi için bekle
}

void GPIOManager::setA0(bool state) {
    _setPin(A0_GPIO_PIN, state);
}

void GPIOManager::setA1(bool state) {
    _setPin(A1_GPIO_PIN, state);
}

void GPIOManager::setA2(bool state) {
    _setPin(A2_GPIO_PIN, state);
}

bool GPIOManager::getA0() {
    return _getPin(A0_GPIO_PIN);
}

bool GPIOManager::getA1() {
    return _getPin(A1_GPIO_PIN);
}

bool GPIOManager::getA2() {
    return _getPin(A2_GPIO_PIN);
}

void GPIOManager::_setPin(uint8_t pin, bool state) {
    gpio_put(pin, state);

    // USB CDC bağlantısını kontrol et
    if (!tud_cdc_connected()) {
        return;
    }
    
    // Send all bytes in a single operation
    uint8_t response[3] = {CMD_SET_PIN, pin, static_cast<uint8_t>(state ? 1 : 0)};
    for (int i = 0; i < 3; i++) {
        tud_cdc_write_char(response[i]);
    }
    tud_cdc_write_flush();
}

bool GPIOManager::_getPin(uint8_t pin) {
    bool state = gpio_get(pin);

    // USB CDC bağlantısını kontrol et
    if (!tud_cdc_connected()) {
        return state;
    }
    
    // Send all bytes in a single operation
    uint8_t response[3] = {CMD_GET_PIN, pin, static_cast<uint8_t>(state ? 1 : 0)};
    for (int i = 0; i < 3; i++) {
        tud_cdc_write_char(response[i]);
    }
    tud_cdc_write_flush();

    return state;
}

void GPIOManager::handleCommand(uint8_t cmd, uint8_t pin, uint8_t value) {
    // Check if pin is valid (A0, A1, or A2)
    if (pin == A0_GPIO_PIN || pin == A1_GPIO_PIN || pin == A2_GPIO_PIN) {
        if (cmd == CMD_SET_PIN) {
            // Set pin state
            bool pin_state = value > 0;
            gpio_put(pin, pin_state);
            
            uint8_t response[3] = {cmd, pin, static_cast<uint8_t>(pin_state ? 1 : 0)};
            for (int i = 0; i < 3; i++) {
                tud_cdc_write_char(response[i]);
            }
            tud_cdc_write_flush();
        } 
        else if (cmd == CMD_GET_PIN) {
            // Get pin state
            bool pin_state = gpio_get(pin);
            
            uint8_t response[3] = {cmd, pin, static_cast<uint8_t>(pin_state ? 1 : 0)};
            for (int i = 0; i < 3; i++) {
                tud_cdc_write_char(response[i]);
            }
            tud_cdc_write_flush();
        }
    } 
    else {
        uint8_t response[3] = {cmd, pin, 0};
        for (int i = 0; i < 3; i++) {
            tud_cdc_write_char(response[i]);
        }
        tud_cdc_write_flush();
    }
}