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

void PirobotServo2040::_parseAndProcessCommands() {
    int input = getchar_timeout_us(GETC_TIMEOUT_US);
    
    while (input != PICO_ERROR_TIMEOUT) {
        // Byte'ı protokol işleyiciye gönder
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
        // RELAY pini - 19 değerinde olmalı
        else if (startIdx == 19) {  // RELAY (A0)
            // RELAY komutları servo2040'ta göz ardı edilir
            // Servolar her zaman etkin
        }
        // A1 pini - 20 değerinde olmalı
        else if (startIdx == 20) {  // A1
            bool state = value ? true : false;
            _gpioManager->setA1(state);
        }
        // A2 pini - 21 değerinde olmalı
        else if (startIdx == 21) {  // A2
            bool state = value ? true : false;
            _gpioManager->setA2(state);
        }
        // LED komutları - 32-37 arası (6 LED)
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
        // Dokunmatik sensör değeri oku - 19 ile başlar
        else if (startIdx >= 19 && startIdx <= 24) {  // TS1-TS6 (19-24)
            uint sensorIdx = startIdx - 19;
            float sensor_voltage = _sensorManager->readTouchSensor(sensorIdx);
            // Voltajı 10-bit değere dönüştür (0-1023 arası)
            values[i] = (uint16_t)(sensor_voltage * 310.303f);
        }
        // Akım değeri oku - 25
        else if (startIdx == 25) {  // CURR
            float current = _sensorManager->readCurrent();
            // Akımı 10-bit değere dönüştür (0-1023 arası, orta değer = 512 -> 0A)
            values[i] = (uint16_t)(current / 0.0814f) + 512;
        }
        // Voltaj değeri oku - 26
        else if (startIdx == 26) {  // VOLT
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