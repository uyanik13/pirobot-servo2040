# GPIO Test Tool for SERVO 2040

This tool provides a way to test the GPIO pins (A0, A1, A2) on the SERVO 2040 board via serial connection.

## Prerequisites

- Python 3.6 or higher
- PySerial library (`pip install pyserial`)
- SERVO 2040 board connected via USB

## Usage

```bash
python gpio_test.py [options]
```

### Options

- `-p, --port PORT`: Specify the serial port (e.g., /dev/ttyACM0)
- `-b, --baud RATE`: Set baud rate (default: 115200)
- `-d, --debug`: Enable debug output for detailed command/response logging
- `-i, --interactive`: Start in interactive mode to manually control GPIO pins

### Automatic Port Selection

If you don't specify a port, the script will:
1. List all available serial ports
2. Auto-select if only one port is found
3. Prompt you to choose if multiple ports are detected

## Interactive Mode

In interactive mode, you can control the GPIO pins using the following commands:

- `a0 [0/1]`: Set PIN A0 state (0=LOW, 1=HIGH)
- `a1 [0/1]`: Set PIN A1 state (0=LOW, 1=HIGH)
- `a2 [0/1]`: Set PIN A2 state (0=LOW, 1=HIGH)
- `read a0`: Read PIN A0 state
- `read a1`: Read PIN A1 state
- `read a2`: Read PIN A2 state
- `test`: Run the automated test sequence
- `q`: Quit the application

## Automated Test Sequence

Without interactive mode, the script runs an automated test sequence that:
1. Sets each pin HIGH and verifies its state
2. Sets each pin LOW and verifies its state
3. Resets all pins to LOW before exiting

## Debugging

Use the `--debug` flag to see detailed information about command and response bytes.

## Example

```bash
# Run in interactive mode with debug output
python gpio_test.py -i -d

# Specify port explicitly
python gpio_test.py -p /dev/ttyACM0

# Run automated test with high baud rate
python gpio_test.py -b 230400
``` 