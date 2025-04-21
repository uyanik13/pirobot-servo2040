#!/usr/bin/env python3
import serial
import time
import sys
import argparse
import os
from datetime import datetime

# Parse command-line arguments
parser = argparse.ArgumentParser(description='Monitor microswitch sensors on Servo 2040')
parser.add_argument('--port', type=str, default='/dev/ttyACM0',
                    help='Serial port (default: /dev/ttyACM0)')
parser.add_argument('--threshold', type=float, default=0.5,
                    help='Switch detection threshold voltage (default: 0.5V)')
parser.add_argument('--active-high', action='store_true',
                    help='Switches are active high (closed = high voltage, default is active low)')

args = parser.parse_args()

# Serial port settings
PORT = args.port
BAUD_RATE = 115200     # Note: Baudrate doesn't matter for USB CDC

# Switch sensor settings
SWITCH_THRESHOLD = args.threshold
ACTIVE_HIGH = args.active_high  # Active high (switch closed = high voltage)
                                # Active low (switch closed = low voltage or ground)
NUM_SWITCHES = 6     # Servo 2040 has 6 touch sensors we can use for switches
TOUCH_SENSOR_BASE_IDX = 22  # Base index for touch sensors in the protocol

# Command constants
GET_CMD = 0x47 | 0x80  # 'G' with MSB set = 0xC7

# ANSI colors for terminal output
COLORS = {
    'red': '\033[91m',
    'green': '\033[92m',
    'yellow': '\033[93m',
    'blue': '\033[94m',
    'magenta': '\033[95m',
    'cyan': '\033[96m',
    'white': '\033[97m',
    'reset': '\033[0m',
    'bold': '\033[1m',
    'background_green': '\033[42m',
    'background_red': '\033[41m'
}

def decode_value(low_byte, high_byte):
    """Decode two 7-bit bytes into a 14-bit value as per protocol"""
    return (high_byte << 7) | low_byte

def send_command(ser, cmd, idx):
    """Send a command to get a sensor reading"""
    # Protocol requires 3 bytes: command, index, count of values to read
    cmd_bytes = bytearray([cmd, idx, 1])
    ser.write(cmd_bytes)

def read_response(ser):
    """Read response (5 bytes: cmd echo, index, count, low byte, high byte)"""
    response = ser.read(5)
    if len(response) != 5:
        raise TimeoutError(f"Didn't receive full response from device (got {len(response)} bytes)")
    
    return response[3], response[4]  # Return low byte and high byte

def read_sensor(ser, sensor_idx):
    """Read a sensor value in volts"""
    idx = TOUCH_SENSOR_BASE_IDX + sensor_idx
    send_command(ser, GET_CMD, idx)
    
    try:
        low_byte, high_byte = read_response(ser)
        raw_value = decode_value(low_byte, high_byte)
        
        # Convert raw value to voltage (0-1023 -> 0-3.3V)
        voltage = raw_value / 310.303  # Based on the scaling in firmware
        
        return voltage
    except Exception as e:
        print(f"Error reading sensor {sensor_idx}: {e}")
        return 0.0

def is_switch_closed(voltage):
    """Determine if a switch is closed based on voltage threshold and active mode"""
    if ACTIVE_HIGH:
        return voltage > SWITCH_THRESHOLD
    else:
        return voltage < SWITCH_THRESHOLD

def get_bar_graph(value, max_value, width=20):
    """Create a bar graph representation of a value"""
    filled = int(value / max_value * width)
    return '█' * filled + '░' * (width - filled)

def main():
    try:
        # Connect to the Servo 2040 board
        print(f"Trying to connect to {PORT}...")
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        print(f"Connected!")
        
        # Wait for connection to stabilize
        time.sleep(1)
        
        print("\n== Microswitch Monitor for Servo 2040 ==")
        print(f"Switch {'HIGH' if ACTIVE_HIGH else 'LOW'} mode with threshold: {SWITCH_THRESHOLD}V")
        print("Press Ctrl+C to exit")
        
        # Keep track of last state for change detection
        last_state = [False] * NUM_SWITCHES
        switch_press_count = [0] * NUM_SWITCHES
        
        # For calculating press/release rate
        last_time = time.time()
        
        while True:
            # Clear screen (Unix/Linux/MacOS)
            os.system('clear' if os.name != 'nt' else 'cls')
            
            current_time = time.time()
            elapsed = current_time - last_time
            last_time = current_time
            
            print(f"{COLORS['bold']}== Microswitch Monitor for Servo 2040 =={COLORS['reset']}")
            print(f"Switch mode: {COLORS['green']}ACTIVE {'HIGH' if ACTIVE_HIGH else 'LOW'}{COLORS['reset']} with threshold: {SWITCH_THRESHOLD}V")
            print("Press Ctrl+C to exit\n")
            
            # Read all switches
            readings = []
            states = []
            
            for i in range(NUM_SWITCHES):
                voltage = read_sensor(ser, i)
                readings.append(voltage)
                is_closed = is_switch_closed(voltage)
                states.append(is_closed)
                
                # Calculate press and display
                if is_closed and not last_state[i]:
                    switch_press_count[i] += 1
                
                # Print sensor reading with color based on switch state
                bar = get_bar_graph(voltage, 3.3)  # 3.3V is max for RP2040
                state_color = COLORS['background_green'] if is_closed else COLORS['background_red']
                state_text = "CLOSED" if is_closed else "OPEN"
                
                print(f"Switch {i+1}: {voltage:.2f}V {bar} {state_color} {state_text} {COLORS['reset']} [Count: {switch_press_count[i]}]")
                
                # Print notification if state changed
                if is_closed != last_state[i]:
                    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                    state_msg = f"{COLORS['green']}CLOSED{COLORS['reset']}" if is_closed else f"{COLORS['red']}OPENED{COLORS['reset']}"
                    print(f"  {COLORS['yellow']}[{timestamp}] Switch {i+1}: {state_msg}{COLORS['reset']}")
            
            # Show a summary
            print("\nSwitch Status Summary:")
            active_switches = [i+1 for i, state in enumerate(states) if state]
            if active_switches:
                print(f"Closed switches: {', '.join(map(str, active_switches))}")
            else:
                print("All switches are open")
            
            # Show total counts
            print("\nTotal Switch Activations:")
            for i in range(NUM_SWITCHES):
                print(f"Switch {i+1}: {switch_press_count[i]} presses")
            
            # Update last state
            last_state = states.copy()
            
            # Short delay for UI updates
            time.sleep(0.1)
            
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