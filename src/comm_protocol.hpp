#pragma once

#include <cstdint>
#include <vector>
#include "pico/stdlib.h"

/**
 * @brief CDC USB protokolü için komut ve yanıt yapılarını tanımlayan sınıf
 */
class CommProtocol {
public:
    // Komut sabitleri
    static constexpr uint8_t SET_CMD = 0x53 | 0x80;  // 'S' with MSB set = 0xD3
    static constexpr uint8_t GET_CMD = 0x47 | 0x80;  // 'G' with MSB set = 0xC7
    
    // Maksimum değer sayısı
    static constexpr uint MAX_VALUES = 32;
    
    /**
     * @brief Komut türleri
     */
    enum class CommandType {
        SET,  // Değerleri ayarla
        GET   // Değerleri oku
    };
    
    /**
     * @brief Komut paketi yapısı
     */
    struct CommandPacket {
        CommandType type;     // Komut türü
        uint8_t startIdx;     // Başlangıç indeksi
        uint8_t count;        // Değer sayısı
        uint16_t values[MAX_VALUES];  // Değerler dizisi (sadece SET komutu için)
        
        CommandPacket() : type(CommandType::SET), startIdx(0), count(0) {
            for (uint i = 0; i < MAX_VALUES; i++) {
                values[i] = 0;
            }
        }
    };
    
    /**
     * @brief Yapılandırıcı
     */
    CommProtocol();
    
    /**
     * @brief USB seri port üzerinden alınan bir byte'ı işler
     * 
     * @param byte Alınan byte
     * @return true Tam bir paket alındı
     * @return false Paket henüz tamamlanmadı
     */
    bool processByte(uint8_t byte);
    
    /**
     * @brief En son alınan komut paketini alır
     * 
     * @return CommandPacket& Komut paketi referansı
     */
    CommandPacket& getCurrentPacket();
    
    /**
     * @brief Komut paketini seri porta yazar
     * 
     * @param packet Gönderilecek paket
     */
    void sendPacket(const CommandPacket& packet);
    
    /**
     * @brief GET komutu yanıtını gönderir
     * 
     * @param startIdx Başlangıç indeksi
     * @param count Değer sayısı
     * @param values Değerler dizisi
     */
    void sendGetResponse(uint8_t startIdx, uint8_t count, const uint16_t* values);
    
    /**
     * @brief 14-bit değeri iki 7-bit byte'a kodlar
     * 
     * @param value 14-bit değer
     * @param low_byte Düşük 7-bit
     * @param high_byte Yüksek 7-bit
     */
    void encodeValue(uint16_t value, uint8_t& low_byte, uint8_t& high_byte);
    
    /**
     * @brief İki 7-bit byte'ı 14-bit değere dönüştürür
     * 
     * @param low_byte Düşük 7-bit
     * @param high_byte Yüksek 7-bit
     * @return uint16_t 14-bit değer
     */
    uint16_t decodeValue(uint8_t low_byte, uint8_t high_byte);
    
private:
    CommandPacket _currentPacket;   // Mevcut komut paketi
    bool _receivingPacket;          // Paket alınıyor bayrağı
    uint8_t _byteCounter;           // Paket içinde alınan byte sayısı
    uint8_t _valueByteCounter;      // Değerler dizisinde alınan byte sayısı
    uint8_t _valueIdx;              // Şu anki değer indeksi
    
    /**
     * @brief Paket işleme durumunu sıfırlar
     */
    void _resetPacketState();
}; 