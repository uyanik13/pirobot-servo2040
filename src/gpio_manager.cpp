#include "gpio_manager.hpp"

GPIOManager::GPIOManager() {
}

void GPIOManager::init() {
    // A0, A1 ve A2 pinlerini başlat
    gpio_init(A0_GPIO_PIN);
    gpio_init(A1_GPIO_PIN);
    gpio_init(A2_GPIO_PIN);
    
    // Pinleri çıkış olarak ayarla
    gpio_set_dir(A0_GPIO_PIN, GPIO_OUT);
    gpio_set_dir(A1_GPIO_PIN, GPIO_OUT);
    gpio_set_dir(A2_GPIO_PIN, GPIO_OUT);
    
    // Başlangıçta tüm pinleri LOW olarak ayarla
    gpio_put(A0_GPIO_PIN, false);
    gpio_put(A1_GPIO_PIN, false);
    gpio_put(A2_GPIO_PIN, false);
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

void GPIOManager::_setPin(uint pin, bool state) {
    gpio_put(pin, state);
}

bool GPIOManager::_getPin(uint pin) {
    return gpio_get(pin);
} 