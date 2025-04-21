#pragma once

#include "pico/stdlib.h"
#include <cstdint>
#include <memory>

#include "servo_driver.hpp"
#include "sensor_manager.hpp"
#include "led_manager.hpp"
#include "gpio_manager.hpp"
#include "comm_protocol.hpp"

/**
 * @brief Ana uygulama sınıfı - Servo2040 için servo kontrolü, sensör okuma ve iletişim
 */
class PirobotServo2040 {
public:
    /**
     * @brief Yapılandırıcı
     */
    PirobotServo2040();
    
    /**
     * @brief Sistemi başlatır
     */
    void init();
    
    /**
     * @brief Ana döngüyü çalıştırır
     */
    void run();
    
private:
    // Alt sistemler
    std::unique_ptr<ServoDriver> _servoDriver;       // Servo kontrolü
    std::unique_ptr<SensorManager> _sensorManager;   // Sensör yönetimi
    std::unique_ptr<LedManager> _ledManager;         // LED yönetimi
    std::unique_ptr<GPIOManager> _gpioManager;       // GPIO yönetimi
    std::unique_ptr<CommProtocol> _commProtocol;     // İletişim protokolü
    
    // Komut sabitleri
    static constexpr uint SERVO_IDX_MAX = 18;       // Servo indeksi üst sınırı
    static constexpr uint TOUCH_SENSOR_IDX_MAX = 6; // Dokunmatik sensör indeksi üst sınırı
    static constexpr uint GETC_TIMEOUT_US = 100;    // getchar_timeout_us için zaman aşımı
    
    // Komut indeks sabitleri
    static constexpr uint A0_IDX = 19;              // A0 komutu indeksi
    static constexpr uint A1_IDX = 20;              // A1 komutu indeksi
    static constexpr uint A2_IDX = 21;              // A2 komutu indeksi
    static constexpr uint TOUCH_START_IDX = 22;     // Dokunmatik sensör başlangıç indeksi
    static constexpr uint TOUCH_END_IDX = 27;       // Dokunmatik sensör bitiş indeksi
    static constexpr uint CURRENT_IDX = 28;         // Akım ölçüm indeksi
    static constexpr uint VOLTAGE_IDX = 29;         // Voltaj ölçüm indeksi
    static constexpr uint LED_IDX_BASE = 32;        // LED komutları başlangıç indeksi
    static constexpr uint LED_IDX_MAX = 37;         // LED komutları bitiş indeksi (dahil)
    static constexpr uint NUM_LEDS = 6;             // LED sayısı
    
    /**
     * @brief USB CDC veri alımını ve komut çözümlemesini işler
     */
    void _parseAndProcessCommands();
    
    /**
     * @brief Alınan SET komutunu işler
     * 
     * @param packet Komut paketi
     */
    void _processSetCommand(const CommProtocol::CommandPacket& packet);
    
    /**
     * @brief Alınan GET komutunu işler
     * 
     * @param packet Komut paketi
     */
    void _processGetCommand(const CommProtocol::CommandPacket& packet);
    
    /**
     * @brief VCP bağlantı kurulana kadar bekler
     */
    void _waitForVCPConnection();
}; 