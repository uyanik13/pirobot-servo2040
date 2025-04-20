#!/usr/bin/env python3
import serial
import time
import sys

# Serial port settings
PORT = '/dev/ttyACM0'  # Linux default, use COM port on Windows
BAUD_RATE = 115200     # Note: Baudrate doesn't matter for USB CDC

# Command constants - as specified in the protocol
SET_CMD = 0x53 | 0x80  # 'S' with MSB set = 0xD3
SERVO_1 = 0            # First servo index

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
    print(f"Sending: {' '.join([f'0x{b:02x}' for b in cmd])}")
    ser.write(cmd)
    time.sleep(0.1)

def main():
    try:
        # Connect to the servo controller
        print(f"Trying to connect to {PORT}...")
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        print(f"Connected!")
        
        # Wait for connection to stabilize
        time.sleep(1)
        
        # Firmware'de artık RELAY kullanılmıyor, servolar her zaman aktif
        
        # Center the servo
        print("\nCentering servo (1500)...")
        send_command(ser, SERVO_1, 1, [1500])
        time.sleep(1)
        
        print("\nMoving servo between positions...")
        
        # Loop through positions
        for i in range(3):  # Repeat 3 times
            print("\nMoving to 1000...")
            send_command(ser, SERVO_1, 1, [1000])
            time.sleep(1)
            
            print("Moving to 1500...")
            send_command(ser, SERVO_1, 1, [1500])
            time.sleep(1)
            
            print("Moving to 2000...")
            send_command(ser, SERVO_1, 1, [2000])
            time.sleep(1)
            
            print("Moving back to 1500...")
            send_command(ser, SERVO_1, 1, [1500])
            time.sleep(1)
        
        print("\nTest complete! Servo should have moved through all positions.")
        
    except serial.SerialException as e:
        print(f"Error: {e}")
        print(f"Make sure the device is connected at {PORT}")
    except KeyboardInterrupt:
        print("\nProgram terminated by user")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed")

if __name__ == "__main__":
    main() 