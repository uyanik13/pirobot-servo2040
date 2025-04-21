#include "pirobot_servo2040.hpp"
#include "pico/stdio_usb.h"

PirobotServo2040::PirobotServo2040() :
    _servoDriver(std::make_unique<ServoDriver>()),
    _sensorManager(std::make_unique<SensorManager>()),
    _ledManager(std::make_unique<LedManager>()),
    _gpioManager(std::make_unique<GPIOManager>()),
    _commProtocol(std::make_unique<CommProtocol>()) {
}

void PirobotServo2040::init() {
    // Stdio başlat (USB CDC için)
    stdio_init_all();
    
    // Alt sistemleri başlat
    _servoDriver->init();
    _sensorManager->init();
    _ledManager->init();
    _gpioManager->init();
    
    // VCP bağlantısı bekle
    _waitForVCPConnection();
}

void PirobotServo2040::run() {
    while (true) {
        // Komutları işle
        _parseAndProcessCommands();
    }
}

// old  logic
void PirobotServo2040::_parseAndProcessCommands() {
    int input = getchar_timeout_us(GETC_TIMEOUT_US);
    
    while (input != PICO_ERROR_TIMEOUT) {
        // Check for direct GPIO commands first
        // if (input == CommProtocol::SET_CMD || input == CommProtocol::GET_CMD) {
        //     uint8_t cmd = input;
        //     int pin_byte = getchar_timeout_us(GETC_TIMEOUT_US);
            
        //     if (pin_byte != PICO_ERROR_TIMEOUT) {
        //         int value_byte = getchar_timeout_us(GETC_TIMEOUT_US);
                
        //         if (value_byte != PICO_ERROR_TIMEOUT) {
        //             // A0/A1/A2 pinleri için özel işleme (GPIO 26, 27, 28)
        //             if (pin_byte == A0_GPIO_PIN || pin_byte == A1_GPIO_PIN || pin_byte == A2_GPIO_PIN) {
        //                 // GPIO komutu olarak işle
        //                 _gpioManager->handleCommand(cmd, pin_byte, value_byte);
        //                 input = getchar_timeout_us(GETC_TIMEOUT_US);
        //                 continue;  // Sonraki komuta geç
        //             }
        //         }
        //     }
        // }
        
        // Normal protocol processing for non-GPIO commands
        if (_commProtocol->processByte((uint8_t)input)) {
            // Tam bir komut paketi alındı
            auto& packet = _commProtocol->getCurrentPacket();
            
            // Komut tipine göre işle
            if (packet.type == CommProtocol::CommandType::SET) {
                _processSetCommand(packet);
            } else if (packet.type == CommProtocol::CommandType::GET) {
                _processGetCommand(packet);
            }
        }
        
        // Sonraki byte'ı al
        input = getchar_timeout_us(GETC_TIMEOUT_US);
    }
}

void PirobotServo2040::_processSetCommand(const CommProtocol::CommandPacket& packet) {
    uint startIdx = packet.startIdx;
    uint count = packet.count;
    
    // Paket doğrulama
    if (count == 0 || count > CommProtocol::MAX_VALUES) {
        return;  // Geçersiz değer sayısı
    }
    
    for (uint i = 0; i < count; i++, startIdx++) {
        uint value = packet.values[i];
        
        // Servo pozisyonu ayarla
        if (startIdx <= SERVO_IDX_MAX) {
            _servoDriver->moveServo(startIdx, value, false);
        }
        // RELAY pini - A0_IDX değerinde olmalı
        else if (startIdx == A0_IDX) {  // RELAY (A0)
            // GPIOManager ile A0 (RELAY) pini kontrolü
            bool state = value ? true : false;
            _gpioManager->setA0(state);
        }
        // A1 pini - A1_IDX değerinde olmalı
        else if (startIdx == A1_IDX) {  // A1
            bool state = value ? true : false;
            _gpioManager->setA1(state);
        }
        // A2 pini - A2_IDX değerinde olmalı
        else if (startIdx == A2_IDX) {  // A2
            bool state = value ? true : false;
            _gpioManager->setA2(state);
        }
        // LED komutları - LED_IDX_BASE ile LED_IDX_MAX arası
        else if (startIdx >= LED_IDX_BASE && startIdx <= LED_IDX_MAX) {
            uint ledIdx = startIdx - LED_IDX_BASE;
            
            // 14-bit değeri renk bileşenlerine ayır
            // RGB - 4-bit per channel
            uint8_t r = ((value >> 8) & 0x0F) << 4;  // 4-bit -> 8-bit
            uint8_t g = ((value >> 4) & 0x0F) << 4;  // 4-bit -> 8-bit
            uint8_t b = (value & 0x0F) << 4;         // 4-bit -> 8-bit
            
            // LED'i ayarla
            _ledManager->setLed(ledIdx, r, g, b);
        }
    }
}

void PirobotServo2040::_processGetCommand(const CommProtocol::CommandPacket& packet) {
    uint startIdx = packet.startIdx;
    uint count = packet.count;
    uint16_t values[CommProtocol::MAX_VALUES] = {0}; // Yanıt değerleri için geçici dizi
    
    // Paket doğrulama
    if (count == 0 || count > CommProtocol::MAX_VALUES) {
        return;  // Geçersiz değer sayısı
    }
    
    for (uint i = 0; i < count; i++, startIdx++) {
        // Servo pozisyonu oku
        if (startIdx <= SERVO_IDX_MAX) {
            values[i] = _servoDriver->getServoPosition(startIdx);
        }
        // A0 durumunu oku
        else if (startIdx == A0_IDX) {  // A0/
            values[i] = _gpioManager->getA0() ? 1 : 0;
        }
        // A1 durumunu oku
        else if (startIdx == A1_IDX) {  // A1
            values[i] = _gpioManager->getA1() ? 1 : 0;
        }
        // A2 durumunu oku
        else if (startIdx == A2_IDX) {  // A2
            values[i] = _gpioManager->getA2() ? 1 : 0;
        }
        // Dokunmatik sensör değeri oku
        else if (startIdx >= TOUCH_START_IDX && startIdx <= TOUCH_END_IDX) {  // TS1-TS6
            uint sensorIdx = startIdx - TOUCH_START_IDX;
            float sensor_voltage = _sensorManager->readTouchSensor(sensorIdx);
            // Voltajı 10-bit değere dönüştür (0-1023 arası)
            values[i] = (uint16_t)(sensor_voltage * 310.303f);
        }
        // Akım değeri oku
        else if (startIdx == CURRENT_IDX) {  // CURR
            float current = _sensorManager->readCurrent();
            // Akımı 10-bit değere dönüştür (0-1023 arası, orta değer = 512 -> 0A)
            values[i] = (uint16_t)(current / 0.0814f) + 512;
        }
        // Voltaj değeri oku
        else if (startIdx == VOLTAGE_IDX) {  // VOLT
            float voltage = _sensorManager->readVoltage();
            // Voltajı 10-bit değere dönüştür (0-1023 arası)
            values[i] = (uint16_t)(voltage * 310.303f);
        }
        else {
            values[i] = 0;  // Geçersiz indeks, 0 döndür
        }
    }
    
    // Yanıtı gönder
    _commProtocol->sendGetResponse(packet.startIdx, packet.count, values);
}

void PirobotServo2040::_waitForVCPConnection() {
    // VCP bağlantısı kurulana kadar bekle, LED animasyonu göster
    while (!stdio_usb_connected()) {
        _ledManager->pendingConnectionAnimation();
    }
    
    // Bağlantı kuruldu, bağlantı durumunu göster
    _ledManager->setConnectedStatus(true);
    sleep_ms(1000);  // Kısa bir süre göster
    
    // LEDleri temizle
    _ledManager->clearAllLeds();
    
    // Tüm servolar için merkez pozisyona git
    // _servoDriver->centerAllServos(1500);
}