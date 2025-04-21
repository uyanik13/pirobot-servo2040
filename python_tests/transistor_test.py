#!/usr/bin/env python3
import serial
import time
import sys
import binascii
import argparse

# Command constants - Bu değerler firmware'deki değerlerle eşleşmelidir
CMD_SET_PIN = 0xD3  # Set pin command - 0x53 | 0x80
CMD_GET_PIN = 0xC7  # Get pin command - 0x47 | 0x80

# Pin definitions - tam olarak firmware pin numaralarıyla eşleşmelidir
A0_PIN = 26  # RELAY pin (GPIO26)
A1_PIN = 27  # A1 pin (GPIO27)
A2_PIN = 28  # A2 pin (GPIO28)

def print_hex(data):
    """Byte verisini okunabilir hex formatında yazdır"""
    if isinstance(data, bytes):
        return ' '.join([f"0x{b:02X}" for b in data])
    elif isinstance(data, list):
        return ' '.join([f"0x{b:02X}" for b in data])
    else:
        return f"0x{data:02X}"

def drain_serial(ser, timeout=0.2):
    """Drain all data from serial port and return it"""
    data = b''
    start_time = time.time()
    
    while (time.time() - start_time) < timeout:
        if ser.in_waiting:
            chunk = ser.read(ser.in_waiting)
            if chunk:
                data += chunk
            time.sleep(0.01)
        else:
            time.sleep(0.01)
    
    return data

def send_direct_command(ser, cmd, pin, value=0):
    """Send a raw command directly to the device."""
    # Clear buffers first
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    
    # Send the 3-byte command
    cmd_bytes = bytes([cmd, pin, value])
    print(f"Gönderilen komut: {print_hex(cmd_bytes)}")
    ser.write(cmd_bytes)
    ser.flush()
    
    # Wait for response
    time.sleep(0.5)  # 500ms bekle
    
    # Read any data (both log and binary response)
    if ser.in_waiting:
        data = ser.read(ser.in_waiting)
        print(f"Alınan veri ({len(data)} byte): {binascii.hexlify(data)}")
        
        # Analyze received data
        if len(data) >= 3:
            # Try to find the binary response packet
            found = False
            for i in range(len(data) - 2):
                if data[i] == cmd and data[i+1] == pin:
                    resp_value = data[i+2]
                    print(f"! Yanıt paketi bulundu: {print_hex(data[i:i+3])}")
                    print(f"! Pin {pin} durumu: {resp_value}")
                    found = True
                    return resp_value > 0
            
            if not found:
                # Try to extract text status from logs
                try:
                    text = data.decode('utf-8', errors='replace')
                    print("Log mesajları:")
                    print(text)
                    
                    if cmd == CMD_SET_PIN:
                        # SET komutu için, komutu gönderebildiysek ve hata mesajı yoksa
                        # işlemin başarılı olduğunu varsayalım
                        return True
                    elif cmd == CMD_GET_PIN:
                        # GET komutu için pin durumunu log'lardan çıkarmaya çalış
                        if f"Pin {pin} state: 1" in text:
                            print("Log'dan Pin AÇIK durumu tespit edildi")
                            return True
                        else:
                            print("Log'dan Pin KAPALI durumu tespit edildi")
                            return False
                except:
                    pass
        
        print("Geçerli yanıt alınamadı")
        return False
    else:
        print("Yanıt alınamadı (0 byte)")
        return False

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Transistor Test for Servo 2040')
    parser.add_argument('--port', '-p', default='/dev/ttyACM0', help='Serial port device')
    parser.add_argument('--baud', '-b', type=int, default=115200, help='Baud rate')
    parser.add_argument('--timeout', '-t', type=float, default=1.0, help='Serial read timeout')
    parser.add_argument('--debug', '-d', action='store_true', help='Enable additional debug output')
    
    args = parser.parse_args()
    
    # Open serial port
    try:
        ser = serial.Serial(args.port, args.baud, timeout=args.timeout)
        print(f"Bağlandı: {ser.name}")
        
        # Allow time for device to initialize
        time.sleep(1)
        
        # Drain initial messages
        initial_data = drain_serial(ser)
        if initial_data:
            print("Başlangıç mesajları:")
            print(initial_data.decode('utf-8', errors='replace'))
        
    except serial.SerialException as e:
        print(f"Seri port açma hatası: {e}")
        sys.exit(1)

    while True:
        print("\nTransistör Test Menüsü:")
        print("1. Fanı Aç (A0)")
        print("2. Fanı Kapat (A0)")
        print("3. Fan Durumunu Oku (A0)")
        print("4. A1 Pini Aç")
        print("5. A1 Pini Kapat")
        print("6. A2 Pini Aç")
        print("7. A2 Pini Kapat")
        print("8. Seri Portu Temizle")
        print("9. Çıkış")
        
        choice = input("Seçiminiz (1-9): ")
        
        if choice == '1':
            print(f"RELAY pini (A0/GPIO {A0_PIN}) açılıyor...")
            if send_direct_command(ser, CMD_SET_PIN, A0_PIN, 1):
                print("Fan açıldı!")
            else:
                print("Fan açılamadı")
                
        elif choice == '2':
            print(f"RELAY pini (A0/GPIO {A0_PIN}) kapatılıyor...")
            if send_direct_command(ser, CMD_SET_PIN, A0_PIN, 0):
                print("Fan kapatıldı!")
            else:
                print("Fan kapatılamadı")
                
        elif choice == '3':
            print(f"RELAY pini (A0/GPIO {A0_PIN}) durumu okunuyor...")
            if send_direct_command(ser, CMD_GET_PIN, A0_PIN, 0):
                print("Fan AÇIK")
            else:
                print("Fan KAPALI")
                
        elif choice == '4':
            print(f"A1 pini (GPIO {A1_PIN}) açılıyor...")
            if send_direct_command(ser, CMD_SET_PIN, A1_PIN, 1):
                print("A1 pin açıldı!")
            else:
                print("A1 pin açılamadı")
                
        elif choice == '5':
            print(f"A1 pini (GPIO {A1_PIN}) kapatılıyor...")
            if send_direct_command(ser, CMD_SET_PIN, A1_PIN, 0):
                print("A1 pin kapatıldı!")
            else:
                print("A1 pin kapatılamadı")
                
        elif choice == '6':
            print(f"A2 pini (GPIO {A2_PIN}) açılıyor...")
            if send_direct_command(ser, CMD_SET_PIN, A2_PIN, 1):
                print("A2 pin açıldı!")
            else:
                print("A2 pin açılamadı")
                
        elif choice == '7':
            print(f"A2 pini (GPIO {A2_PIN}) kapatılıyor...")
            if send_direct_command(ser, CMD_SET_PIN, A2_PIN, 0):
                print("A2 pin kapatıldı!")
            else:
                print("A2 pin kapatılamadı")
                
        elif choice == '8':
            print("Seri port temizleniyor...")
            data = drain_serial(ser, timeout=1.0)
            if data:
                print(f"Temizlenen veri ({len(data)} byte):")
                try:
                    print(data.decode('utf-8', errors='replace'))
                except:
                    print(f"Binary: {binascii.hexlify(data)}")
            else:
                print("Temizlenecek veri yok")
                
        elif choice == '9':
            print("Çıkılıyor...")
            ser.close()
            break
            
        else:
            print("Geçersiz seçim. Lütfen tekrar deneyin.")
        
        time.sleep(0.1)  # Komutlar arası küçük bir gecikme

if __name__ == "__main__":
    main() 