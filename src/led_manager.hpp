#pragma once

#include "pico/stdlib.h"
#include "servo2040_defs.hpp"
#include "ws2812.hpp"

/**
 * @brief LED yönetim sınıfı - Servo2040 üzerindeki 6 RGB LEDi kontrol eder
 */
class LedManager {
public:
    /**
     * @brief Yapılandırıcı, LED çubuğunu başlatır
     */
    LedManager();
    
    /**
     * @brief LED çubuğunu başlatır
     */
    void init();
    
    /**
     * @brief Belirli bir LEDi belirtilen renkte ayarlar
     * 
     * @param index LED indeksi (0-5)
     * @param r Kırmızı (0-255)
     * @param g Yeşil (0-255)
     * @param b Mavi (0-255)
     * @return true Başarılı
     * @return false Geçersiz indeks
     */
    bool setLed(uint index, uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * @brief Belirli bir LEDi HSV renk uzayında ayarlar
     * 
     * @param index LED indeksi (0-5)
     * @param h Ton (0.0-1.0)
     * @param s Doygunluk (0.0-1.0)
     * @param v Parlaklık (0.0-1.0)
     * @return true Başarılı
     * @return false Geçersiz indeks
     */
    bool setLedHSV(uint index, float h, float s, float v);
    
    /**
     * @brief Tüm LEDleri belirtilen renke ayarlar
     * 
     * @param r Kırmızı (0-255)
     * @param g Yeşil (0-255)
     * @param b Mavi (0-255)
     */
    void setAllLeds(uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * @brief Tüm LEDleri temizler (kapatır)
     */
    void clearAllLeds();
    
    /**
     * @brief VCP bağlantısı beklerken çalışan gökkuşağı animasyonu
     */
    void pendingConnectionAnimation();
    
    /**
     * @brief Bağlantı durumuna göre LED durumunu ayarlar
     * 
     * @param connected Bağlantı durumu (true: bağlı, false: bağlı değil)
     */
    void setConnectedStatus(bool connected);
    
private:
    plasma::WS2812 _led_bar;               // LED çubuğu nesnesi
    static constexpr float BRIGHTNESS = 0.3f;  // Genel parlaklık seviyesi
    
    /**
     * @brief LED indeksinin geçerli olup olmadığını kontrol eder
     * 
     * @param index LED indeksi
     * @return true Geçerli
     * @return false Geçersiz
     */
    bool _isValidIndex(uint index);
}; 