#!/usr/bin/env python3
import serial
import time
import sys
import binascii
import argparse

# Command constants - Bu değerler firmware'deki değerlerle eşleşmelidir
CMD_SET = 0x53 | 0x80  # 'S' with MSB set = 0xD3
CMD_GET = 0x47 | 0x80  # 'G' with MSB set = 0xC7

# Pin index definitions - should match the firmware indexes exactly
A0_IDX = 19  # Relay pin index
A1_IDX = 20  # A1 pin index
A2_IDX = 21  # A2 pin index

def print_hex(data):
    """Byte verisini okunabilir hex formatında yazdır"""
    if isinstance(data, bytes) or isinstance(data, bytearray):
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

def decode_value(low_byte, high_byte):
    """Decode two 7-bit bytes into a 14-bit value as per protocol"""
    return (high_byte << 7) | low_byte

def send_command(ser, cmd, idx, value=0):
    """Send a command to set or get a pin state
    For SET commands, value should be 0 or 1
    For GET commands, value is ignored"""
    
    # Clear buffers first
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    
    if cmd == CMD_SET:
        # Protocol requires 5 bytes for SET: command, index, count=1, low_byte, high_byte
        low_byte = value & 0x7F
        high_byte = (value >> 7) & 0x7F
        cmd_bytes = bytearray([cmd, idx, 1, low_byte, high_byte])
    else:  # CMD_GET
        # Protocol requires 3 bytes for GET: command, index, count=1
        cmd_bytes = bytearray([cmd, idx, 1])
    
    print(f"Gönderilen komut: {print_hex(cmd_bytes)}")
    ser.write(cmd_bytes)
    ser.flush()
    
    # Give device time to process
    time.sleep(0.1)

def read_response(ser, is_get=True):
    """Read and process response
    For GET, we expect 5 bytes: cmd, idx, count, low_byte, high_byte
    For SET, may return command echo or nothing"""
    
    if is_get:
        try:
            # Read 5 bytes for GET response
            response = ser.read(5)
            if len(response) != 5:
                print(f"Eksik yanıt: {len(response)} byte alındı, 5 bekleniyordu")
                return False
            
            # Extract and validate components
            cmd, idx, count, low_byte, high_byte = response
            if cmd != CMD_GET:
                print(f"Yanıt komutu eşleşmiyor: {print_hex(cmd)}, beklenen: {print_hex(CMD_GET)}")
                return False
            
            value = decode_value(low_byte, high_byte)
            print(f"Yanıt: Komut={print_hex(cmd)}, Idx={idx}, Değer={value}")
            return value > 0
        except Exception as e:
            print(f"Yanıt okuma hatası: {e}")
            return False
    else:
        # For SET, we may get no response or just an echo
        if ser.in_waiting:
            response = ser.read(ser.in_waiting)
            print(f"SET yanıtı: {print_hex(response)}")
        return True  # Assume success for SET commands

def set_pin(ser, idx, state):
    """Set a pin state (1=on, 0=off)"""
    send_command(ser, CMD_SET, idx, 1 if state else 0)
    time.sleep(0.2)  # Give firmware time to process
    return read_response(ser, is_get=False)

def get_pin(ser, idx):
    """Get a pin state"""
    send_command(ser, CMD_GET, idx)
    return read_response(ser)

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
            print(f"RELAY pini (A0/IDX {A0_IDX}) açılıyor...")
            if set_pin(ser, A0_IDX, True):
                print("Fan açıldı!")
            else:
                print("Fan açılamadı")
                
        elif choice == '2':
            print(f"RELAY pini (A0/IDX {A0_IDX}) kapatılıyor...")
            if set_pin(ser, A0_IDX, False):
                print("Fan kapatıldı!")
            else:
                print("Fan kapatılamadı")
                
        elif choice == '3':
            print(f"RELAY pini (A0/IDX {A0_IDX}) durumu okunuyor...")
            if get_pin(ser, A0_IDX):
                print("Fan AÇIK")
            else:
                print("Fan KAPALI")
                
        elif choice == '4':
            print(f"A1 pini (IDX {A1_IDX}) açılıyor...")
            if set_pin(ser, A1_IDX, True):
                print("A1 pin açıldı!")
            else:
                print("A1 pin açılamadı")
                
        elif choice == '5':
            print(f"A1 pini (IDX {A1_IDX}) kapatılıyor...")
            if set_pin(ser, A1_IDX, False):
                print("A1 pin kapatıldı!")
            else:
                print("A1 pin kapatılamadı")
                
        elif choice == '6':
            print(f"A2 pini (IDX {A2_IDX}) açılıyor...")
            if set_pin(ser, A2_IDX, True):
                print("A2 pin açıldı!")
            else:
                print("A2 pin açılamadı")
                
        elif choice == '7':
            print(f"A2 pini (IDX {A2_IDX}) kapatılıyor...")
            if set_pin(ser, A2_IDX, False):
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