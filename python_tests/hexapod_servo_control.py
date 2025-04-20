#!/usr/bin/env python3
import serial
import time
import sys
import math
import numpy as np
import argparse
import os

# Serial port settings
PORT = '/dev/ttyACM0'  # Linux default, use COM port on Windows
BAUD_RATE = 115200     # Note: Baudrate doesn't matter for USB CDC

# Command constants - as specified in the protocol
SET_CMD = 0x53 | 0x80  # 'S' with MSB set = 0xD3
SERVO_1 = 0            # First servo index

# Servo range and center positions
MIN_PULSE = 500    # Minimum pulse width in μs
MAX_PULSE = 2500   # Maximum pulse width in μs
CENTER_PULSE = 1500  # Center position

def encode_value(value):
    """Encode a 14-bit value into two 7-bit bytes as per protocol"""
    low_byte = value & 0x7F
    high_byte = (value >> 7) & 0x7F
    return low_byte, high_byte

def send_command(ser, start_idx, count, values):
    """Send a SET command to control servos"""
    cmd = bytearray([SET_CMD, start_idx, count])
    
    # Add each value, properly encoded as per protocol
    for val in values:
        low_byte, high_byte = encode_value(val)
        cmd.extend([low_byte, high_byte])
    
    # Send command and print as hex for debugging
    ser.write(cmd)
    
    # Print only if verbose is enabled
    if args.verbose:
        print(f"Sent: {' '.join([f'0x{b:02x}' for b in cmd])}")

def angle_to_pulse(angle, center_offset=0):
    """Convert angle in degrees to PWM pulse width
    
    Args:
        angle: Angle in degrees (-90 to 90)
        center_offset: Offset from center position (1500) in μs
    
    Returns:
        PWM pulse width value (500-2500)
    """
    # Clamp angle to valid range
    angle = max(-90, min(90, angle))
    
    # Convert to 0-180 range for pulse width calculation
    norm_angle = angle + 90
    
    # Map 0-180 to 500-2500
    pulse_range = MAX_PULSE - MIN_PULSE
    pulse = MIN_PULSE + (norm_angle / 180.0) * pulse_range
    
    # Apply center offset
    pulse += center_offset
    
    # Clamp to valid range
    pulse = max(MIN_PULSE, min(MAX_PULSE, pulse))
    
    return int(pulse)

def center_all_servos(ser, center_values=None):
    """Center all servos to their center positions"""
    if center_values is None:
        # Default to 1500 for all servos
        center_values = [CENTER_PULSE] * 18
    
    send_command(ser, SERVO_1, 18, center_values)
    print("Tüm servolar merkez pozisyona getirildi")

def move_all_servos_by_angle(ser, angles, center_offsets=None):
    """Move all servos according to angle values"""
    if center_offsets is None:
        center_offsets = [0] * 18
    
    pulses = [angle_to_pulse(angle, offset) for angle, offset in zip(angles, center_offsets)]
    
    # Print angles and pulses
    if args.verbose:
        for i, (angle, pulse) in enumerate(zip(angles, pulses)):
            print(f"Servo {i+1}: Açı = {angle:.2f}°, Pulse = {pulse} μs")
    
    send_command(ser, SERVO_1, 18, pulses)

def set_kinematic_positions(ser, kinematic_data, center_offsets=None):
    """Set servo positions from kinematic angle data"""
    if center_offsets is None:
        center_offsets = [0] * 18
        
    # Convert angles to pulse values
    angles = [float(a) for a in kinematic_data]
    
    # Print angles if verbose
    if args.verbose:
        for i, angle in enumerate(angles):
            print(f"Servo {i+1}: Açı = {angle:.2f}°")
    
    # Move servos to specified angles
    move_all_servos_by_angle(ser, angles, center_offsets)
    print(f"Kinematik açılar servolara uygulandı: {len(angles)} servo")

def read_kinematic_positions(filename):
    """Read kinematic positions from a file"""
    try:
        # Önce doğrudan dosya adıyla dene
        try:
            with open(filename, 'r') as f:
                lines = f.readlines()
        except FileNotFoundError:
            # Eğer bulamazsa, python_tests dizininde ara
            script_dir = os.path.dirname(os.path.realpath(__file__))
            full_path = os.path.join(script_dir, os.path.basename(filename))
            with open(full_path, 'r') as f:
                lines = f.readlines()
            
        # Parse data (expecting 18 angles per line)
        positions = []
        
        for line in lines:
            # Clean up line and extract values
            line = line.strip()
            if not line or line.startswith('#'):
                continue
                
            # Try to parse values from the line
            try:
                # Split by spaces and/or commas
                values = line.replace(',', ' ').split()
                
                # Convert to floats and ensure we have 18 values
                angles = [float(v) for v in values]
                if len(angles) == 18:
                    positions.append(angles)
                else:
                    print(f"Uyarı: Satırda 18 açı değil, {len(angles)} değer var")
            except ValueError:
                print(f"Geçersiz satır atlanıyor: {line}")
                
        print(f"Dosya okundu: {len(positions)} hareket dizisi")
        return positions
    except Exception as e:
        print(f"Dosya okuma hatası: {e}")
        return []

def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Hexapod Servo Control')
    parser.add_argument('--port', default=PORT, help=f'Serial port (default: {PORT})')
    parser.add_argument('--verbose', '-v', action='store_true', help='Enable verbose output')
    parser.add_argument('--center', action='store_true', help='Center all servos')
    parser.add_argument('--angles', nargs='+', type=float, help='Set specific angles for all 18 servos')
    parser.add_argument('--file', type=str, help='Read angles from a file')
    parser.add_argument('--offsets', nargs='+', type=int, help='Center offsets for servos (18 values)')
    parser.add_argument('--delay', type=float, default=0.1, help='Delay between movements in seconds (default: 0.1)')
    parser.add_argument('--cycle-delay', type=float, default=0.5, help='Delay between cycles in seconds (default: 0.5)')
    return parser.parse_args()

def main():
    global args
    args = parse_arguments()
    
    # Validate offsets if provided
    center_offsets = None
    if args.offsets:
        if len(args.offsets) != 18:
            print(f"UYARI: 18 offset değeri bekleniyordu, {len(args.offsets)} verildi. Varsayılan değerler kullanılacak.")
        else:
            center_offsets = args.offsets
    
    try:
        # Connect to the servo controller
        print(f"{args.port} portuna bağlanılıyor...")
        ser = serial.Serial(args.port, BAUD_RATE, timeout=1)
        print(f"Bağlantı başarılı!")
        
        # Wait for connection to stabilize
        time.sleep(1)
        
        if args.center:
            center_all_servos(ser, center_offsets)
            time.sleep(1)
        
        if args.angles:
            if len(args.angles) != 18:
                print(f"HATA: 18 açı değeri bekleniyordu, {len(args.angles)} verildi.")
            else:
                set_kinematic_positions(ser, args.angles, center_offsets)
        
        if args.file:
            print(f"Dosya okunuyor: '{args.file}'")
            positions = read_kinematic_positions(args.file)
            if positions:
                print(f"{len(positions)} hareket uygulanacak...")
                
                # Sürekli çalışacak döngü
                print("Kinematik hareket dizisi - Ctrl+C ile durdurana kadar devam edecek...")
                try:
                    cycle_count = 0
                    while True:
                        cycle_count += 1
                        print(f"\n--- Döngü #{cycle_count} başlıyor ---")
                        
                        for i, pos in enumerate(positions):
                            print(f"Hareket {i+1}/{len(positions)} uygulanıyor")
                            set_kinematic_positions(ser, pos, center_offsets)
                            time.sleep(args.delay)  # Use the specified delay
                        
                        # Döngü sonunda belirtilen gecikme kadar bekle
                        print(f"Döngü #{cycle_count} tamamlandı. {args.cycle_delay} saniye bekleniyor...")
                        time.sleep(args.cycle_delay)
                except KeyboardInterrupt:
                    print("\nHareket dizisi kullanıcı tarafından durduruldu.")
                
                # Return to center
                # center_all_servos(ser, center_offsets)
            else:
                print(f"HATA: Dosyadan hiç hareket pozisyonu okunamadı.")
                print(f"Lütfen dosya yolunu kontrol edin veya dosyanın doğru formatında olduğundan emin olun.")
                script_dir = os.path.dirname(os.path.realpath(__file__))
                print(f"Şu anki script dizini: {script_dir}")
                print(f"Aranan dosya: {args.file}")
                print(f"Alternatif: {os.path.join(script_dir, os.path.basename(args.file))}")
        
        # If no specific command was given, show help
        if not (args.center or args.sine or args.wave or args.calibrate or args.angles or args.file):
            print("\nKomut belirtilmedi. Kullanım örnekleri:")
            print("  python hexapod_servo_control.py --center")
            print("  python hexapod_servo_control.py --sine --verbose")
            print("  python hexapod_servo_control.py --wave")
            print("  python hexapod_servo_control.py --calibrate")
            print("  python hexapod_servo_control.py --angles 0 10 20 -10 -20 -30 15 25 35 -15 -25 -35 5 15 25 -5 -15 -25")
            print("  python hexapod_servo_control.py --file kinematic_positions.txt")
            print("  python3 python_tests/hexapod_servo_control.py --file kinematic_positions.txt --delay 0.025 --cycle-delay 0.1")
            print("  python hexapod_servo_control.py --offsets 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 --center")
        
    except serial.SerialException as e:
        print(f"Hata: {e}")
        print(f"{args.port} portuna bağlanamadı. Cihazın bağlı olduğundan emin olun.")
    except KeyboardInterrupt:
        print("\nProgram kullanıcı tarafından sonlandırıldı")
    finally:
        if 'ser' in locals() and ser.is_open:
            # Center servos before closing
            
            ser.close()
            print("Seri bağlantı kapatıldı")

if __name__ == "__main__":
    main() 