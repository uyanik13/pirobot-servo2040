#pragma once

#include "pico/stdlib.h"

/**
 * @brief GPIO pinlerini (A0, A1, A2) yöneten sınıf
 */
class GPIOManager {
public:
    // GPIO pin tanımlamaları
    static constexpr uint A0_GPIO_PIN = 26;  // RELAY olarak kullanılır
    static constexpr uint A1_GPIO_PIN = 27;
    static constexpr uint A2_GPIO_PIN = 28;
    
    /**
     * @brief Yapılandırıcı
     */
    GPIOManager();
    
    /**
     * @brief GPIO pinlerini başlatır
     */
    void init();
    
    /**
     * @brief A0 pinini (RELAY) ayarlar
     * 
     * @param state Durum (true: HIGH, false: LOW)
     */
    void setA0(bool state);
    
    /**
     * @brief A1 pinini ayarlar
     * 
     * @param state Durum (true: HIGH, false: LOW)
     */
    void setA1(bool state);
    
    /**
     * @brief A2 pinini ayarlar
     * 
     * @param state Durum (true: HIGH, false: LOW)
     */
    void setA2(bool state);
    
    /**
     * @brief A0 pininin durumunu okur
     * 
     * @return bool Pin durumu
     */
    bool getA0();
    
    /**
     * @brief A1 pininin durumunu okur
     * 
     * @return bool Pin durumu
     */
    bool getA1();
    
    /**
     * @brief A2 pininin durumunu okur
     * 
     * @return bool Pin durumu
     */
    bool getA2();
    
private:
    /**
     * @brief Belirtilen pini belirtilen duruma ayarlar
     * 
     * @param pin Pin numarası
     * @param state Durum (true: HIGH, false: LOW)
     */
    void _setPin(uint pin, bool state);
    
    /**
     * @brief Belirtilen pinin durumunu okur
     * 
     * @param pin Pin numarası
     * @return bool Pin durumu
     */
    bool _getPin(uint pin);
}; 