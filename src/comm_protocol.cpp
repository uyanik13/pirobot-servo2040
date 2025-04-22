#include "comm_protocol.hpp"
#include "tusb.h"

CommProtocol::CommProtocol() {
    _resetPacketState();
}

bool CommProtocol::processByte(uint8_t byte) {
    // MSB=1 ise yeni bir komut başlat
    if (byte & 0x80) {
        _resetPacketState();
        _receivingPacket = true;
        
        // Komut tipini belirle
        if (byte == SET_CMD) {
            _currentPacket.type = CommandType::SET;
        } else if (byte == GET_CMD) {
            _currentPacket.type = CommandType::GET;
        } else {
            // Tanınmayan komut
            _receivingPacket = false;
            return false;
        }
        
        _byteCounter = 0;
        return false;  // Paket henüz tamamlanmadı
    }
    
    // Paket alınmıyorsa işleme
    if (!_receivingPacket) {
        return false;
    }
    
    // Paket verisini işle
    if (_byteCounter == 0) {
        // Başlangıç indeksi
        _currentPacket.startIdx = byte;
        _byteCounter++;
    } else if (_byteCounter == 1) {
        // Değer sayısı
        _currentPacket.count = byte;
        _byteCounter++;
        
        if (_currentPacket.type == CommandType::GET) {
            // GET komutu tamamlandı
            _receivingPacket = false;
            return true;
        }
        
        // SET komutu için değerleri beklemeye devam et
        _valueIdx = 0;
        _valueByteCounter = 0;
    } else {
        // Değerleri işle (her değer iki byte)
        if (_valueByteCounter == 0) {
            // Düşük 7-bit
            _currentPacket.values[_valueIdx] = byte & 0x7F;
            _valueByteCounter = 1;
        } else {
            // Yüksek 7-bit
            _currentPacket.values[_valueIdx] |= ((byte & 0x7F) << 7);
            _valueByteCounter = 0;
            _valueIdx++;
            
            // Tüm değerler alındı mı?
            if (_valueIdx >= _currentPacket.count) {
                _receivingPacket = false;
                return true;  // Paket tamamlandı
            }
        }
    }
    
    return false;  // Paket henüz tamamlanmadı
}

CommProtocol::CommandPacket& CommProtocol::getCurrentPacket() {
    return _currentPacket;
}

void CommProtocol::sendPacket(const CommandPacket& packet) {
    uint8_t cmd = (packet.type == CommandType::SET) ? SET_CMD : GET_CMD;
    uint8_t buffer[3 + 2 * MAX_VALUES]; // Header (3 bytes) + values (2 bytes each)
    uint16_t index = 0;
    
    // Komut headerı ekle
    buffer[index++] = cmd;
    buffer[index++] = packet.startIdx;
    buffer[index++] = packet.count;
    
    // SET komutu için değerleri ekle
    if (packet.type == CommandType::SET) {
        for (uint i = 0; i < packet.count; i++) {
            uint8_t low_byte, high_byte;
            encodeValue(packet.values[i], low_byte, high_byte);
            buffer[index++] = low_byte;
            buffer[index++] = high_byte;
        }
    }
    
    // Tüm buffer'ı bir seferde gönder
    if (tud_cdc_connected()) {
        tud_cdc_write(buffer, index);
        tud_cdc_write_flush();
    }
}

void CommProtocol::sendGetResponse(uint8_t startIdx, uint8_t count, const uint16_t* values) {
    uint8_t buffer[3 + 2 * MAX_VALUES]; // Header (3 bytes) + values (2 bytes each)
    uint16_t index = 0;
    
    // Yanıt headerı ekle
    buffer[index++] = GET_CMD;
    buffer[index++] = startIdx;
    buffer[index++] = count;
    
    // Değerleri ekle
    for (uint i = 0; i < count; i++) {
        uint8_t low_byte, high_byte;
        encodeValue(values[i], low_byte, high_byte);
        buffer[index++] = low_byte;
        buffer[index++] = high_byte;
    }
    
    // Tüm buffer'ı bir seferde gönder
    if (tud_cdc_connected()) {
        tud_cdc_write(buffer, index);
        tud_cdc_write_flush();
    }
}

void CommProtocol::encodeValue(uint16_t value, uint8_t& low_byte, uint8_t& high_byte) {
    low_byte = value & 0x7F;
    high_byte = (value >> 7) & 0x7F;
}

uint16_t CommProtocol::decodeValue(uint8_t low_byte, uint8_t high_byte) {
    return (low_byte & 0x7F) | ((high_byte & 0x7F) << 7);
}

void CommProtocol::_resetPacketState() {
    _receivingPacket = false;
    _byteCounter = 0;
    _valueByteCounter = 0;
    _valueIdx = 0;
} 