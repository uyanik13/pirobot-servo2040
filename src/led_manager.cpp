#include "led_manager.hpp"

LedManager::LedManager() :
    _led_bar(servo_defs::NUM_LEDS, pio1, 0, servo_defs::LED_DATA) {
}

void LedManager::init() {
    _led_bar.start();
    
    // Test pattern to ensure LEDs are working
    setAllLeds(255, 0, 0);  // Red
    sleep_ms(200);
    setAllLeds(0, 255, 0);  // Green
    sleep_ms(200);
    setAllLeds(0, 0, 255);  // Blue
    sleep_ms(200);
    
    clearAllLeds();
}

bool LedManager::setLed(uint index, uint8_t r, uint8_t g, uint8_t b) {
    if (!_isValidIndex(index)) {
        return false;
    }
    
    _led_bar.set_rgb(index, r, g, b);
    return true;
}

bool LedManager::setLedHSV(uint index, float h, float s, float v) {
    if (!_isValidIndex(index)) {
        return false;
    }
    
    // v değerini global parlaklık değeriyle çarp
    _led_bar.set_hsv(index, h, s, v * BRIGHTNESS);
    return true;
}

void LedManager::setAllLeds(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < servo_defs::NUM_LEDS; i++) {
        _led_bar.set_rgb(i, r, g, b);
    }
}

void LedManager::clearAllLeds() {
    _led_bar.clear();
}

void LedManager::pendingConnectionAnimation() {
    static float offset = 0.0f;
    const uint updates = 10;  // Slow down the animation for better visibility
    
    offset += 0.01f;  // Increase speed slightly
    
    // Tüm LEDleri güncelle
    for (uint i = 0; i < servo_defs::NUM_LEDS; i++) {
        float hue = (float)i / (float)servo_defs::NUM_LEDS;
        _led_bar.set_hsv(i, hue + offset, 1.0f, BRIGHTNESS * 1.5f);  // Make brighter
    }
    
    sleep_ms(1000 / updates);
}

void LedManager::setConnectedStatus(bool connected) {
    if (connected) {
        // Bağlantı kuruldu, yeşil LED
        setAllLeds(0, 64, 0);
    } else {
        // Bağlantı kesildi, kırmızı LED
        setAllLeds(64, 0, 0);
    }
}

bool LedManager::_isValidIndex(uint index) {
    return (index < servo_defs::NUM_LEDS);
} 