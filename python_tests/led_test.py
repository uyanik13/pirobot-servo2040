#!/usr/bin/env python3
import serial
import time
import sys

# Serial port settings
PORT = '/dev/ttyACM0'  # Update to match your device
BAUD_RATE = 115200     # Note: Baudrate doesn't matter for USB CDC

# Command constants
SET_CMD = 0x53 | 0x80  # 'S' with MSB set = 0xD3
GET_CMD = 0x47 | 0x80  # 'G' with MSB set = 0xC7

# LED Command Constants - Based on the firmware protocol
LED_CMD = 32          # LED command base index
NUM_LEDS = 6          # Servo2040 has 6 RGB LEDs

def encode_value(value):
    """Encode a 14-bit value into two 7-bit bytes as per protocol"""
    low_byte = value & 0x7F
    high_byte = (value >> 7) & 0x7F
    return low_byte, high_byte

def send_led_command(ser, led_index, r, g, b):
    """Send a command to set a single LED color"""
    # Create RGB value (assuming 8-bit per channel)
    # Pack into 14-bit value as per protocol (4 bits per color + 2 unused)
    r_value = (r >> 4) & 0x0F  # 4 bits
    g_value = (g >> 4) & 0x0F  # 4 bits
    b_value = (b >> 4) & 0x0F  # 4 bits
    
    value = (r_value << 8) | (g_value << 4) | b_value
    
    # Encode for protocol
    low_byte, high_byte = encode_value(value)
    
    # Send command to specific LED index
    cmd = bytearray([SET_CMD, LED_CMD + led_index, 1, low_byte, high_byte])
    
    print(f"LED {led_index}: RGB({r},{g},{b}) → {value:04x} → {cmd.hex()}")
    ser.write(cmd)
    time.sleep(0.05)  # Small delay to ensure command is processed

def set_all_leds(ser, r, g, b):
    """Set all LEDs to the same color"""
    for i in range(NUM_LEDS):
        send_led_command(ser, i, r, g, b)

def main():
    try:
        # Connect to the servo controller
        print(f"Trying to connect to {PORT}...")
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        print(f"Connected!")
        
        # Wait for connection to stabilize
        time.sleep(1)
        
        # First clear all LEDs
        print("\nClearing all LEDs...")
        set_all_leds(ser, 0, 0, 0)
        time.sleep(0.5)
        
        # Test each color separately (R, G, B)
        print("\nTesting basic colors...")
        
        # Red
        print("\nSetting all LEDs to RED")
        set_all_leds(ser, 255, 0, 0)
        time.sleep(1)
        
        # Green
        print("\nSetting all LEDs to GREEN")
        set_all_leds(ser, 0, 255, 0)
        time.sleep(1)
        
        # Blue
        print("\nSetting all LEDs to BLUE")
        set_all_leds(ser, 0, 0, 255)
        time.sleep(1)
        
        # White (all on)
        print("\nSetting all LEDs to WHITE")
        set_all_leds(ser, 255, 255, 255)
        time.sleep(1)
        
        # Test individual LEDs
        print("\nTesting each LED individually...")
        
        # First turn off all LEDs
        set_all_leds(ser, 0, 0, 0)
        time.sleep(0.5)
        
        # Colors for each LED
        colors = [
            (255, 0, 0),    # Red
            (0, 255, 0),    # Green
            (0, 0, 255),    # Blue
            (255, 255, 0),  # Yellow
            (255, 0, 255),  # Magenta
            (0, 255, 255),  # Cyan
        ]
        
        # Light each LED one by one
        for i in range(NUM_LEDS):
            r, g, b = colors[i]
            send_led_command(ser, i, r, g, b)
            time.sleep(0.5)
        
        time.sleep(2)  # Keep them on to observe
        
        # Clean up - turn off all LEDs
        print("\nTurning all LEDs off...")
        set_all_leds(ser, 0, 0, 0)
        
        print("\nLED test complete!")
        
    except serial.SerialException as e:
        print(f"Error: {e}")
        print(f"Make sure the device is connected at {PORT}")
    except KeyboardInterrupt:
        print("\nProgram terminated by user")
    finally:
        if 'ser' in locals() and ser.is_open:
            # Turn off all LEDs before exiting
            try:
                set_all_leds(ser, 0, 0, 0)
            except:
                pass
            ser.close()
            print("Serial connection closed")

if __name__ == "__main__":
    main() 