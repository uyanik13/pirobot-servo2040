#include "sensor_manager.hpp"

SensorManager::SensorManager() :
    _sensor_adc(servo::servo2040::SHARED_ADC),
    _voltage_adc(servo::servo2040::SHARED_ADC, VOLTAGE_GAIN),
    _current_adc(servo::servo2040::SHARED_ADC, CURRENT_GAIN, SHUNT_RESISTOR, CURRENT_OFFSET),
    _mux(servo::servo2040::ADC_ADDR_0, 
         servo::servo2040::ADC_ADDR_1, 
         servo::servo2040::ADC_ADDR_2,
         PIN_UNUSED, 
         servo::servo2040::SHARED_ADC) {
}

void SensorManager::init() {
    // Dokunmatik sensörleri pull-down ile yapılandır
    for (uint i = 0; i < servo::servo2040::NUM_SENSORS; i++) {
        _mux.configure_pulls(servo::servo2040::SENSOR_1_ADDR + i, false, true);
    }
}

float SensorManager::readVoltage() {
    _mux.select(servo::servo2040::VOLTAGE_SENSE_ADDR);
    return _voltage_adc.read_voltage();
}

float SensorManager::readCurrent() {
    _mux.select(servo::servo2040::CURRENT_SENSE_ADDR);
    return _current_adc.read_current();
}

float SensorManager::readTouchSensor(uint sensor_idx) {
    if (!_isValidSensorIdx(sensor_idx)) {
        return 0.0f;
    }
    
    uint address = servo::servo2040::SENSOR_1_ADDR + sensor_idx;
    _mux.select(address);
    return _sensor_adc.read_voltage();
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
    return (sensor_idx < servo::servo2040::NUM_SENSORS);
} 