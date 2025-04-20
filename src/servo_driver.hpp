#pragma once

#include <vector>
#include "pico/stdlib.h"
#include "servo2040_defs.hpp"
#include "servo_cluster.hpp"

/**
 * @brief Servo sürücü sınıfı, servo motorların kontrolünü sağlar
 */
class ServoDriver {
public:
    /**
     * @brief Yapılandırıcı, servo cluster'ı başlatır
     * 
     * @param start_pin İlk servo pini
     * @param end_pin Son servo pini
     */
    ServoDriver(uint start_pin = servo_defs::SERVO_1, 
                uint end_pin = servo_defs::SERVO_18);
    
    /**
     * @brief Sistemi başlatır ve tüm servoları etkinleştirir
     */
    void init();
    
    /**
     * @brief Belirli bir servoyu belirli bir pozisyona hareket ettirir
     * 
     * @param servo_pin Servo pin numarası
     * @param pulse_width PWM darbe genişliği (500-2500 μs arası)
     * @param wait_for_move Hareketin tamamlanması beklensin mi
     * @return Başarı/hata durumu
     */
    bool moveServo(uint servo_pin, uint pulse_width, bool wait_for_move = false);
    
    /**
     * @brief Servodan şu anki pozisyonu okur
     * 
     * @param servo_pin Servo pin numarası
     * @return uint Servo pozisyonu (pulse width - μs)
     */
    uint getServoPosition(uint servo_pin);
    
    /**
     * @brief Tüm servoları merkez pozisyona getirir
     * 
     * @param center_pos Merkez pozisyon (genellikle 1500 μs)
     */
    void centerAllServos(uint center_pos = 1500);
    
    /**
     * @brief Tüm servoları devre dışı bırakır
     */
    void disableAllServos();
    
    /**
     * @brief Tüm servoları etkinleştirir
     */
    void enableAllServos();

    /**
     * @brief Birden fazla servoyu aynı anda hareket ettirir
     * 
     * @param servo_pins Servo pin numaraları dizisi
     * @param pulse_widths PWM darbe genişliği değerleri dizisi
     * @param count Servo sayısı
     * @return Başarı/hata durumu
     */
    bool moveMultipleServos(const uint* servo_pins, const uint* pulse_widths, uint count);

    /**
     * @brief Tüm servoları hareket ettirir (hexapod bacak kontrolü için)
     * 
     * @param pulse_widths 18 servo için pwm darbe genişliği değerleri dizisi
     * @return Başarı/hata durumu
     */
    bool moveAllServos(const uint* pulse_widths);

    /**
     * @brief Birden fazla servoyu bir açı (derece) değerine göre hareket ettirir
     * 
     * @param servo_pins Servo pin numaraları dizisi
     * @param angles Açı değerleri dizisi (derece)
     * @param count Servo sayısı
     * @return Başarı/hata durumu
     */
    bool moveServosByAngle(const uint* servo_pins, const float* angles, uint count);

    /**
     * @brief Açı (derece) değerini PWM darbe genişliğine dönüştürür
     * 
     * @param angle Açı değeri (-90 ile 90 derece arası)
     * @param min_pulse En düşük darbe genişliği (500 μs)
     * @param max_pulse En yüksek darbe genişliği (2500 μs)
     * @return uint PWM darbe genişliği
     */
    uint angleToPulseWidth(float angle, uint min_pulse = 500, uint max_pulse = 2500);
    
private:
    servo::ServoCluster _servos;  // Servo kontrol nesnesi
    const uint _start_pin;        // İlk servo pini
    const uint _end_pin;          // Son servo pini
    const uint _servo_count;      // Toplam servo sayısı
    
    /**
     * @brief Verilen pin numarasının geçerli olup olmadığını kontrol eder
     * 
     * @param servo_pin Servo pin numarası
     * @return true Geçerli
     * @return false Geçersiz
     */
    bool _isValidPin(uint servo_pin);
}; 