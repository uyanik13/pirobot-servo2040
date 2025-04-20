#!/usr/bin/env python3
import serial
import time
import sys

# Serial port settings
PORT = '/dev/ttyACM1'  # Updated to the correct port
BAUD_RATE = 115200     # Note: Baudrate doesn't matter for USB CDC

# Command constants
SET_CMD = 0x53 | 0x80  # 'S' with MSB set = 0xD3
GET_CMD = 0x47 | 0x80  # 'G' with MSB set = 0xC7

# GPIO indices for Servo 2040
GPIO_START_IDX = 0     # Starting index for GPIO pins
NUM_GPIO_PINS = 8      # Number of GPIO pins on the Servo 2040

def encode_value(value):
    """Encode a 14-bit value into two 7-bit bytes as per protocol"""
    low_byte = value & 0x7F
    high_byte = (value >> 7) & 0x7F
    return low_byte, high_byte

def decode_value(low_byte, high_byte):
    """Decode two 7-bit bytes into a 14-bit value as per protocol"""
    return (high_byte << 7) | low_byte

def send_set_command(ser, idx, value):
    """Send a SET command to set a pin value"""
    low_byte, high_byte = encode_value(value)
    cmd_bytes = bytearray([SET_CMD, idx, 1, low_byte, high_byte])
    print(f"Sending: {' '.join([f'0x{b:02x}' for b in cmd_bytes])}")
    ser.write(cmd_bytes)
    time.sleep(0.01)  # Small delay to ensure command is processed

def send_get_command(ser, idx):
    """Send a GET command to read a pin value"""
    cmd_bytes = bytearray([GET_CMD, idx, 1])
    print(f"Sending GET: {' '.join([f'0x{b:02x}' for b in cmd_bytes])}")
    ser.write(cmd_bytes)

def read_response(ser):
    """Read response (3 bytes: cmd echo, low byte, high byte)"""
    response = ser.read(3)
    if len(response) != 3:
        raise TimeoutError("Didn't receive full response from device")
    
    print(f"Response: {' '.join([f'0x{b:02x}' for b in response])}")
    return response[1], response[2]  # Return low byte and high byte

def set_pin_output(ser, pin_idx, value):
    """Set a GPIO pin as an output with the specified value (0 or 1)"""
    # GPIO pins operate in different modes:
    # To set as output, we first set the direction (mode 0)
    mode_idx = pin_idx
    send_set_command(ser, mode_idx, 0)  # Set as output (mode 0)
    
    # Then we set the output value (mode 1)
    value_idx = pin_idx + NUM_GPIO_PINS
    send_set_command(ser, value_idx, 1 if value else 0)
    
def set_pin_input(ser, pin_idx):
    """Set a GPIO pin as an input"""
    # Set direction to input (mode 0)
    mode_idx = pin_idx
    send_set_command(ser, mode_idx, 1)  # Set as input (mode 1)

def read_pin(ser, pin_idx):
    """Read the current value of a GPIO pin"""
    # The pin value is at offset NUM_GPIO_PINS
    value_idx = pin_idx + NUM_GPIO_PINS
    send_get_command(ser, value_idx)
    
    try:
        low_byte, high_byte = read_response(ser)
        value = decode_value(low_byte, high_byte)
        return value
    except Exception as e:
        print(f"Error reading pin {pin_idx}: {e}")
        return None

def test_output_sequence(ser):
    """Test a sequence of output patterns on all GPIO pins"""
    patterns = [
        [1, 0, 1, 0, 1, 0, 1, 0],  # Alternating
        [0, 1, 0, 1, 0, 1, 0, 1],  # Alternating flipped
        [1, 1, 1, 1, 0, 0, 0, 0],  # Half and half
        [1, 1, 0, 0, 1, 1, 0, 0],  # Pairs
        [1, 0, 0, 0, 0, 0, 0, 0],  # Single pin high, shift
        [0, 1, 0, 0, 0, 0, 0, 0],
        [0, 0, 1, 0, 0, 0, 0, 0],
        [0, 0, 0, 1, 0, 0, 0, 0],
        [0, 0, 0, 0, 1, 0, 0, 0],
        [0, 0, 0, 0, 0, 1, 0, 0],
        [0, 0, 0, 0, 0, 0, 1, 0],
        [0, 0, 0, 0, 0, 0, 0, 1],
        [1, 1, 1, 1, 1, 1, 1, 1],  # All high
        [0, 0, 0, 0, 0, 0, 0, 0],  # All low
    ]
    
    print("Testing GPIO output patterns...")
    
    # Set all pins as outputs
    for pin in range(NUM_GPIO_PINS):
        set_pin_output(ser, pin, 0)  # Initialize to 0
    
    # Run through each pattern
    for i, pattern in enumerate(patterns):
        print(f"Pattern {i+1}: {pattern}")
        
        # Set each pin according to the pattern
        for pin, value in enumerate(pattern):
            set_pin_output(ser, pin, value)
        
        time.sleep(0.5)  # Pause to observe
    
    # Reset all pins to 0
    for pin in range(NUM_GPIO_PINS):
        set_pin_output(ser, pin, 0)

def test_input_pins(ser):
    """Test reading from GPIO pins configured as inputs"""
    print("\nTesting GPIO inputs...")
    print("Connect pins to GND or 3.3V and observe readings.")
    print("Press Ctrl+C to exit this test.")
    
    # Set all pins as inputs
    for pin in range(NUM_GPIO_PINS):
        set_pin_input(ser, pin)
    
    try:
        while True:
            values = []
            for pin in range(NUM_GPIO_PINS):
                value = read_pin(ser, pin)
                values.append("1" if value else "0")
            
            print(f"GPIO Values: {''.join(values)}", end="\r")
            time.sleep(0.2)
    except KeyboardInterrupt:
        print("\nInput test stopped by user.")

def main():
    try:
        # Connect to the Servo 2040 board
        print(f"Trying to connect to {PORT}...")
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        print(f"Connected!")
        
        # Wait for connection to stabilize
        time.sleep(1)
        
        print("\n== GPIO Test for Servo 2040 ==")
        
        # Test output patterns
        test_output_sequence(ser)
        
        # Test input reading
        test_input_pins(ser)
            
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