#!/usr/bin/env python3
import serial
import time
import sys
import argparse

# Parse command-line arguments
parser = argparse.ArgumentParser(description='Monitor power readings from Servo 2040')
parser.add_argument('--voltage-factor', type=float, default=1.32, 
                    help='Voltage calibration factor (default: 1.32)')
parser.add_argument('--current-factor', type=float, default=3.5, 
                    help='Current calibration factor (default: 3.5)')
parser.add_argument('--current-offset', type=float, default=0.0,
                    help='Current offset adjustment (default: 0.0)')
parser.add_argument('--port', type=str, default='/dev/ttyACM0',
                    help='Serial port (default: /dev/ttyACM0)')

args = parser.parse_args()

# Serial port settings
PORT = args.port
BAUD_RATE = 115200     # Note: Baudrate doesn't matter for USB CDC

# Calibration factors
VOLTAGE_CALIBRATION_FACTOR = args.voltage_factor  # Default based on 7.6/5.76 ratio
CURRENT_CALIBRATION_FACTOR = args.current_factor
CURRENT_OFFSET = args.current_offset

# Command constants
GET_CMD = 0x47 | 0x80  # 'G' with MSB set = 0xC7

# Sensor indices
VOLTAGE_SENSOR_IDX = 29  # Index for voltage sensor (was 23)
CURRENT_SENSOR_IDX = 28  # Index for current sensor (was 24)

def decode_value(low_byte, high_byte):
    """Decode two 7-bit bytes into a 14-bit value as per protocol"""
    return (high_byte << 7) | low_byte

def send_command(ser, cmd, idx):
    """Send a command to get a sensor reading"""
    # Protocol requires 3 bytes: command, index, count of values to read
    cmd_bytes = bytearray([cmd, idx, 1])  # Added count=1 as required by protocol
    ser.write(cmd_bytes)

def read_response(ser):
    """Read response (5 bytes now: cmd echo, index, count, low byte, high byte)"""
    response = ser.read(5)  # Updated to read 5 bytes instead of 3
    if len(response) != 5:
        raise TimeoutError(f"Didn't receive full response from device (got {len(response)} bytes)")
    
    return response[3], response[4]  # Return low byte and high byte (now at positions 3 and 4)

def get_voltage(ser):
    """Get voltage reading in volts"""
    send_command(ser, GET_CMD, VOLTAGE_SENSOR_IDX)
    low_byte, high_byte = read_response(ser)
    raw_value = decode_value(low_byte, high_byte)
    
    # Hard calibration based on the command-line factor
    # Base formula is kept, but multiplied by calibration factor
    voltage = (raw_value / 4095.0 * 15.0 * 0.68) * VOLTAGE_CALIBRATION_FACTOR
    
    return voltage

def get_current(ser):
    """Get current reading in amps"""
    send_command(ser, GET_CMD, CURRENT_SENSOR_IDX)
    low_byte, high_byte = read_response(ser)
    raw_value = decode_value(low_byte, high_byte)
    
    # For current sensing, using a simpler and more directly calibratable approach
    # First convert to normalized value (0-1.0 range for ADC)
    normalized_value = raw_value / 4095.0
    
    # Apply offset to zero out at no current (0.5 is midpoint for bidirectional sensing)
    # Most current sensors read 0A at the middle of the ADC range
    zero_current_point = 0.48  # Slightly below middle, may need adjustment
    
    # Calculate with simple formula: (normalized - zero_point) * full_scale_amps * calibration
    # 10A is a typical full scale for servo controller current sensors
    full_scale_amps = 16.0  
    
    current = (normalized_value - zero_current_point) * full_scale_amps * CURRENT_CALIBRATION_FACTOR
    
    # If current is slightly negative when should be zero, clamp to zero
    if current < 0.025:
        current = 0.0
        
    return current

def add_current_offset_correction(current_reading, offset=0.0):
    """Apply additional offset correction to current reading if needed"""
    # Some current sensors drift over time or temperature
    # This allows additional correction
    return max(0, current_reading + offset)

def get_average_readings(ser, num_samples=5, delay=0.05):
    """Get average voltage and current readings over multiple samples"""
    voltages = []
    currents = []
    
    for _ in range(num_samples):
        try:
            voltages.append(get_voltage(ser))
            # Apply the current offset correction
            currents.append(add_current_offset_correction(get_current(ser), CURRENT_OFFSET))
            time.sleep(delay)
        except Exception as e:
            print(f"Error during sampling: {e}")
    
    # Calculate averages, handling the case where no valid readings were obtained
    avg_voltage = sum(voltages) / len(voltages) if voltages else 0
    avg_current = sum(currents) / len(currents) if currents else 0
    
    return avg_voltage, avg_current

def estimate_battery_percentage(voltage):
    """Estimate battery percentage based on voltage
    Handles different battery types based on voltage range"""
    
    # Check if it's likely a 2S LiPo (7.4V nominal)
    if voltage > 6.0:
        # 2S LiPo: 6.6V (empty) to 8.4V (full)
        min_voltage = 6.6
        max_voltage = 8.4
    # Check if it's likely a 1S LiPo (3.7V nominal)
    elif voltage > 3.0:
        # 1S LiPo: 3.3V (empty) to 4.2V (full)
        min_voltage = 3.3
        max_voltage = 4.2
    # Assume it's a different power source
    else:
        # Default to 0-100% for 0-5V
        min_voltage = 0
        max_voltage = 5.0
    
    # Clamp voltage to valid range
    clamped_voltage = max(min_voltage, min(voltage, max_voltage))
    
    # Calculate percentage
    percentage = ((clamped_voltage - min_voltage) / (max_voltage - min_voltage)) * 100
    
    return round(percentage)

def main():
    try:
        # Connect to the servo controller
        print(f"Trying to connect to {PORT}...")
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        print(f"Connected!")
        
        # Wait for connection to stabilize
        time.sleep(1)
        
        print("\n== Power Monitor for Servo 2040 ==")
        print(f"Voltage calibration factor: {VOLTAGE_CALIBRATION_FACTOR:.2f}")
        print(f"Current calibration factor: {CURRENT_CALIBRATION_FACTOR:.2f}")
        print(f"Current offset adjustment: {CURRENT_OFFSET:.3f}")
        print("\nReading power values... (Press Ctrl+C to exit)")
        print("=" * 60)
        
        # Print header
        print(f"{'Voltage':^10} | {'Current':^10} | {'Power':^12} | {'Battery':^10} | {'Raw ADC':^10}")
        print("-" * 60)
        
        while True:
            # Get raw ADC value for debugging
            send_command(ser, GET_CMD, CURRENT_SENSOR_IDX)
            low_byte, high_byte = read_response(ser)
            raw_adc = decode_value(low_byte, high_byte)
            raw_percent = raw_adc / 4095.0 * 100
            
            # Get readings
            voltage, current = get_average_readings(ser)
            power = voltage * current
            battery_pct = estimate_battery_percentage(voltage)
            
            # Print with nice formatting (carriage return to refresh line)
            print(f"{voltage:7.2f}V | {current:7.3f}A | {power:9.2f}W | {battery_pct:7}% | {raw_percent:6.1f}%", end="\r")
            time.sleep(0.5)
            
    except serial.SerialException as e:
        print(f"\nError: {e}")
        print(f"Make sure the device is connected at {PORT}")
    except KeyboardInterrupt:
        print("\n\nProgram terminated by user")
        print("\nCalibration Commands:")
        print("---------------------")
        print("To adjust voltage reading:")
        if voltage > 0:
            print(f"  python3 power_test.py --voltage-factor={VOLTAGE_CALIBRATION_FACTOR * (8.4/voltage):.2f}")
        
        print("\nTo adjust current reading:")
        if current > 0.1:
            print(f"  python3 power_test.py --current-factor={CURRENT_CALIBRATION_FACTOR * (5.0/current):.2f}")
        else:
            print(f"  python3 power_test.py --current-factor=<desired_factor> --current-offset=<offset>")
        
        print("\nFor both voltage and current:")
        if voltage > 0 and current > 0.1:
            print(f"  python3 power_test.py --voltage-factor={VOLTAGE_CALIBRATION_FACTOR * (8.4/voltage):.2f} --current-factor={CURRENT_CALIBRATION_FACTOR * (5.0/current):.2f}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed")

if __name__ == "__main__":
    main() 