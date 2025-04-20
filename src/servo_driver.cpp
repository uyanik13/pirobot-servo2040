#include "servo_driver.hpp"
#include <cmath>

ServoDriver::ServoDriver(uint start_pin, uint end_pin) :
    _servos(pio0, 0, start_pin, (end_pin - start_pin) + 1),
    _start_pin(start_pin),
    _end_pin(end_pin),
    _servo_count((end_pin - start_pin) + 1) {
}

void ServoDriver::init() {
    _servos.init();
    _servos.enable_all();
}

bool ServoDriver::moveServo(uint servo_pin, uint pulse_width, bool wait_for_move) {
    if (!_isValidPin(servo_pin)) {
        return false;
    }
    
    // Darbe genişliğini 500-2500 us arasında sınırla
    pulse_width = (pulse_width < 500) ? 500 : (pulse_width > 2500) ? 2500 : pulse_width;
    
    // Convert to the correct pin index (relative to start_pin)
    uint8_t servo_index = servo_pin - _start_pin;
    
    // Use the float version of pulse width
    _servos.pulse(servo_index, (float)pulse_width);
    
    return true;
}

uint ServoDriver::getServoPosition(uint servo_pin) {
    if (!_isValidPin(servo_pin)) {
        return 0;
    }
    
    // Convert to the correct pin index (relative to start_pin)
    uint8_t servo_index = servo_pin - _start_pin;
    
    return (uint)_servos.pulse(servo_index);
}

void ServoDriver::centerAllServos(uint center_pos) {
    for (uint i = 0; i < _servo_count; i++) {
        uint8_t servo_index = i;
        _servos.pulse(servo_index, (float)center_pos);
    }
}

void ServoDriver::disableAllServos() {
    _servos.disable_all();
}

void ServoDriver::enableAllServos() {
    _servos.enable_all();
}

bool ServoDriver::_isValidPin(uint servo_pin) {
    return (servo_pin >= _start_pin && servo_pin <= _end_pin);
}

// Yeni eklenen fonksiyonlar

bool ServoDriver::moveMultipleServos(const uint* servo_pins, const uint* pulse_widths, uint count) {
    bool success = true;
    
    for (uint i = 0; i < count; i++) {
        success &= moveServo(servo_pins[i], pulse_widths[i], false);
    }
    
    return success;
}

bool ServoDriver::moveAllServos(const uint* pulse_widths) {
    bool success = true;
    
    for (uint i = 0; i < _servo_count; i++) {
        uint servo_pin = _start_pin + i;
        success &= moveServo(servo_pin, pulse_widths[i], false);
    }
    
    return success;
}

bool ServoDriver::moveServosByAngle(const uint* servo_pins, const float* angles, uint count) {
    bool success = true;
    
    for (uint i = 0; i < count; i++) {
        uint pulse = angleToPulseWidth(angles[i]);
        success &= moveServo(servo_pins[i], pulse, false);
    }
    
    return success;
}

uint ServoDriver::angleToPulseWidth(float angle, uint min_pulse, uint max_pulse) {
    // Açıyı -90 ile 90 derece arasında sınırla
    angle = (angle < -90.0f) ? -90.0f : (angle > 90.0f) ? 90.0f : angle;
    
    // Açıyı 0-180 aralığına dönüştür (orijinal -90 +90 aralığını)
    float normalizedAngle = angle + 90.0f;
    
    // 0-180 aralığını min_pulse-max_pulse aralığına dönüştür
    float pulseRange = max_pulse - min_pulse;
    float pulseWidth = min_pulse + (normalizedAngle / 180.0f) * pulseRange;
    
    return (uint)pulseWidth;
} 