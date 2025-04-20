#pragma once

#include "pico/stdlib.h"
#include "servo2040_defs.hpp"
#include "analogmux.hpp"
#include "analog.hpp"

/**
 * @brief Voltaj, akım ve dokunmatik sensörlerin yönetimini yapan sınıf
 */
class SensorManager {
public:
    /**
     * @brief Yapılandırıcı, analog ve çoklayıcı başlatır
     */
    SensorManager();
    
    /**
     * @brief Sistemi başlatır
     */
    void init();
    
    /**
     * @brief Sistemin voltajını ölçer
     * 
     * @return float Voltaj değeri (Volt)
     */
    float readVoltage();
    
    /**
     * @brief Sistemin çektiği akımı ölçer
     * 
     * @return float Akım değeri (Amper)
     */
    float readCurrent();
    
    /**
     * @brief Belirtilen dokunmatik sensörün değerini okur
     * 
     * @param sensor_idx Sensör indeksi (0-5)
     * @return float Sensör değeri (Volt)
     */
    float readTouchSensor(uint sensor_idx);
    
    /**
     * @brief Belirtilen analog pinin değerini okur
     * 
     * @param analog_pin Analog pin numarası
     * @return float Analog değeri (Volt)
     */
    float readAnalogPin(uint analog_pin);
    
    /**
     * @brief 14-bit değeri iki 7-bit byte'a kodlar 
     * (USB CDC iletişimi için gerekli)
     * 
     * @param value 14-bit değer
     * @param low_byte Düşük 7-bit
     * @param high_byte Yüksek 7-bit
     */
    void encodeValue(uint value, uint8_t &low_byte, uint8_t &high_byte);
    
    /**
     * @brief İki 7-bit byte'ı 14-bit değere dönüştürür
     * 
     * @param low_byte Düşük 7-bit
     * @param high_byte Yüksek 7-bit
     * @return uint 14-bit değer
     */
    uint decodeValue(uint8_t low_byte, uint8_t high_byte);
    
private:
    pimoroni::Analog _sensor_adc;        // Sensör analog-dijital dönüştürücüsü
    pimoroni::Analog _voltage_adc;       // Voltaj ölçümü ADC
    pimoroni::Analog _current_adc;       // Akım ölçümü ADC
    pimoroni::AnalogMux _mux;            // Analog çoklayıcı
    
    static constexpr float SHUNT_RESISTOR = servo_defs::SHUNT_RESISTOR;
    static constexpr float CURRENT_GAIN = servo_defs::CURRENT_GAIN;
    static constexpr float VOLTAGE_GAIN = servo_defs::VOLTAGE_GAIN;
    static constexpr float CURRENT_OFFSET = servo_defs::CURRENT_OFFSET;
    
    /**
     * @brief Sensör indeksinin geçerli olup olmadığını kontrol eder
     * 
     * @param sensor_idx Sensör indeksi
     * @return true Geçerli
     * @return false Geçersiz
     */
    bool _isValidSensorIdx(uint sensor_idx);
}; 