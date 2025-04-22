#include "sensor_manager.hpp"
#include "servo2040.hpp"
#include "common/pimoroni_common.hpp"

using namespace servo::servo2040;

namespace {
    constexpr float SHUNT_RESISTOR = servo::servo2040::SHUNT_RESISTOR;
    constexpr float CURRENT_GAIN = servo::servo2040::CURRENT_GAIN;
    constexpr float VOLTAGE_GAIN = servo::servo2040::VOLTAGE_GAIN;
    constexpr float CURRENT_OFFSET = servo::servo2040::CURRENT_OFFSET;
}

SensorManager::SensorManager() :
    _sensor_adc(SHARED_ADC),
    _voltage_adc(SHARED_ADC, VOLTAGE_GAIN),
    _current_adc(SHARED_ADC, CURRENT_GAIN, SHUNT_RESISTOR, CURRENT_OFFSET),
    _mux(ADC_ADDR_0, 
         ADC_ADDR_1, 
         ADC_ADDR_2,
         pimoroni::PIN_UNUSED, 
         SHARED_ADC) {
}

void SensorManager::init() {
    // Dokunmatik sensörleri pull-down ile yapılandır
    for (uint i = 0; i < NUM_SENSORS; i++) {
        _mux.configure_pulls(SENSOR_1_ADDR + i, false, true);
    }
}

float SensorManager::readVoltage() {
    _mux.select(VOLTAGE_SENSE_ADDR);
    
    // ADC stabilizasyonu için bekleme
    sleep_us(100);
    
    // Birkaç örnek alıp ortalamasını hesapla
    const int NUM_SAMPLES = 4;
    float total = 0.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        total += _voltage_adc.read_voltage();
        sleep_us(50);  // Örnekler arası kısa bekleme
    }
    
    return total / NUM_SAMPLES;
}

float SensorManager::readCurrent() {
    _mux.select(CURRENT_SENSE_ADDR);
    
    // ADC stabilizasyonu için bekleme
    sleep_us(100);
    
    // Birkaç örnek alıp ortalamasını hesapla
    const int NUM_SAMPLES = 4;
    float total = 0.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        total += _current_adc.read_current();
        sleep_us(50);  // Örnekler arası kısa bekleme
    }
    
    return total / NUM_SAMPLES;
}

float SensorManager::readTouchSensor(uint sensor_idx) {
    if (!_isValidSensorIdx(sensor_idx)) {
        return 0.0f;
    }
    
    uint address = SENSOR_1_ADDR + sensor_idx;
    _mux.select(address);
    
    // ADC stabilizasyonu için bekleme
    sleep_us(100);
    
    // Birkaç örnek alıp ortalamasını hesapla
    const int NUM_SAMPLES = 4;
    float total = 0.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        total += _sensor_adc.read_voltage();
        sleep_us(50);  // Örnekler arası kısa bekleme
    }
    
    return total / NUM_SAMPLES;
}

float SensorManager::readAnalogPin(uint analog_pin) {
    _mux.select(analog_pin);
    return _sensor_adc.read_voltage();
}

void SensorManager::encodeValue(uint value, uint8_t &low_byte, uint8_t &high_byte) {
    low_byte = value & 0x7F;
    high_byte = (value >> 7) & 0x7F;
}

uint SensorManager::decodeValue(uint8_t low_byte, uint8_t high_byte) {
    return (low_byte & 0x7F) | ((high_byte & 0x7F) << 7);
}

bool SensorManager::_isValidSensorIdx(uint sensor_idx) {
    return (sensor_idx < NUM_SENSORS);
} 