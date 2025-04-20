# RP2040 Servo Driver for Chica
This driver implements [Pimoroni's Servo 2040 board](https://shop.pimoroni.com/products/servo-2040?variant=39800591679571), a RP2040 based servo driver intended to work with Make Your Pet's [Chica Server](https://play.google.com/store/apps/details?id=com.makeyourpet.chicaserver&hl=en_US&gl=US) android app.

## Loading the Firmware Image
To load the firmware onto the servo 2040 board, perform the following steps:
1) **Read the warnings below**

2) Plug in the USB-C cable to your machine. Hold down the "boot/user" button, press the reset button at the same time, and let go of both buttons. The RP2040 should now appear as drive to the computer. 

3) Simply drag and drop the corresponding .uf2 image file to the RP2040 drive, the device will automatically reboot and start the loaded program.

# Hardware 
## Servo 2040

### [Schematics](https://cdn.shopify.com/s/files/1/0174/1800/files/servo2040_schematic.pdf?v=1648817752)

Details from the [Pimoroni's website](https://shop.pimoroni.com/products/servo-2040?variant=39800591679571):
> The servo 2040 is a standalone servo controller for making things with lots of moving parts. It has pre-soldered pin headers for plugging in up to 18 servos - enough for the leggiest of hexapod walkers or plenty of degrees of freedom for your robotic arms, legs or tentacles. Servos can be pretty power hungry, especially the chunky ones, so we've added some neat current monitoring functions so you can keep an eye on power consumption. 

>We've used RP2040 as the core of this board because of the flexibility of its Programmable IOs (PIOs). Traditionally, each servo needs to be connected to its own PWM capable channel on the microcontroller. RP2040 only has 16 PWM channels, but it's possible to drive up to 30 servos using the magic of PIOs (if you're canny with wiring). RP2040's PIOs are also super fast, so they can drive servos with sub microsecond resolution.

Keep in mind that the screw terminals for supplying external power (with reverse polarity protection) are rated for 10A max continuous current. 

### ***External Power Warning***:
This application requires an external power source greater than 5V to power the servos through the terminal block of the board.  **If you want to run servos with a higher voltage than 5V, you'll need to _cut the 'Separate USB and Ext. Power' trace on the back of the board_ to prevent the RP2040 or _your machine_ being damaged by the increased voltage.**

### ***Battery Power Warning***:
Although this application doesn't require it, the servo 2040 can be powered by a 5V battery through the 5V/GND pins, instead of through the USB-C port. **When sourcing power through the 5V pins, it's important to _use a data-only USB adapter/cable_ to program the board.** This will prevent the 5V battery and from backfeeding to the 5V source provide by the USB device. 

## Hexapod Robot Build
## Servos
The two most recommend servos to be used for this project are the [ZOSKAY 35kg coreless servos](https://www.amazon.com/dp/B07SBYZ4G5?_encoding=UTF8&ref_=cm_sw_r_cp_ud_dp_FHYWJWD1TXGTMJDHJMWC&th=1) [[Alternate Link]](https://www.aliexpress.us/item/2251832824472591.html?spm=a2g0o.order_detail.0.0.2e03f19c3p5o3j&gatewayAdapt=glo2usa4itemAdapt&_randl_shipto=US) and the [Feetech 35kg cored servos](https://www.robotshop.com/products/feetech-180-degrees-digital-servo-74v-35kg-cm-ft5330m). The current hexapod design will only work with the ZOSKAY servos out-of-the-box. 

## 3D Printed Chassis
All 3D printed designs, BOM, wiring guides, and build information for the hexapod can be found on Make Your Pet's [hexapod repository](https://github.com/MakeYourPet/hexapod).

# Software
The hexapod driver application is compliant with the Chica server application and follows the [Chica servo communication protocol specifications](https://docs.google.com/document/d/1mZwbWAyVBaSGiShjaIyb4V5swsjEJnEGjClIcWYX3S0/edit).

## Features
### Virtual Com Port
The RP2040 acts as a USB CDC device, and will be seen as a virtual com port (VCP) device to the host. Upon startup, the LEDs will perform a cyclic rainbow pattern until a VCP connection to the host device is made. Serial monitoring applications like TeraTerm and RealTerm can be used to interface with the board.

### Virtual Servo Power Relay
When the hexapod is powered down, the application will de-assert the servo power relay pin on the board to disable the physical power relay that's attached to the servo 2040 board. 

For redundancy, the application will also disable PWM signal outputs on all servos, which effectively disables the servos by removing torque. This has the added benefit of making a physical servo power relay for the hexapod optional. 

### Tools
The Chica server application requires servo calibration values as input to its config.txt file to improve servo positioning accuracy as demonstrated in MYP's [servo calibration video](https://www.youtube.com/watch?v=UMUeKFPptU4).

The ServoCalibration directory contains _servoCalibration.uf2_, along with the .stl files for the physical components needed for calibration (provided by MYP). This useful program streamlines the PWM value acquisition process for config.txt. A table will be produced at the end of the program, which you can copy or take a screenshot for later. All done without needing to buy a seperate servo calibrator! [A tutorial video for using servoCalibration.uf2 can be found here](https://youtu.be/w5ZRXiZLpTk).

# Development 
## Dependencies 
This application was witten in C++ to maximize speed and performance.

The [pico-SDK](https://github.com/raspberrypi/pico-sdk) and [pimoroni-pico
](https://github.com/pimoroni/pimoroni-pico) are required libraries needed for proper servo 2040 development and operation. It's recommended start development by running a "hello world" example using pico-SDK. Then slowly modifying CMake files and dependancies to migrate the pimoroni-pico library into your development enviroment (this is the tricky part!) and implementing the [C++ pimoroni servo2040 examples](https://github.com/pimoroni/pimoroni-pico/tree/main/examples/servo2040) one at a time.

Development on the servo 2040 can also be done using Micropython, but this is outside the scope of this repository. A tutorial for setting up a micropython development enviroment can be found [here](https://github.com/pimoroni/pimoroni-pico/blob/main/setting-up-micropython.md).

# Community & Feedback
This repository and the hexapod project is part of an active community constantly innovating hexapod robots. Please consider joining the [discord channel](https://discord.gg/vb8YWMfBuk) if you would like to make your own hexapod and become part of the community.

Bug reports, feature request, and general feedback for this repository would be greatly appreciated. Thank you! 



```bash
# Set required environment variables
export PICO_SDK_PATH=~/pico-sdk
export PIMORONI_PICO_PATH=~/pimoroni-pico
```

### Build Steps

```bash

# If CMake configuration changed, clean the build
rm -rf build

# Create build directory and configure
mkdir -p build
cd build
cmake ..

# Compile
make -j$(nproc)
 make -j4
```

cd /home/uyanik13/Desktop/SERVO_2040/servo-2040-code/build && make -j4

# Chica Servo2040 Sürücü Araçları

Bu repo, Chica Servo2040 kartı için basit sürücü ve test araçları içerir. Bu araçlar Servo2040 kart üzerindeki servoları kolayca kontrol etmenizi sağlar.

## Araçlar

### 1. Servo Sweep Test (`servo_sweep.py`)

Bu araç, belirtilen servo motorunu minimum ve maksimum pozisyonlar arasında adım adım hareket ettirir. Servo'nun tüm hareket aralığını test etmek için kullanılabilir.

#### Kullanım:

```bash
python servo_sweep.py [-h] [-p PORT] [-s SERVO] [-min MIN_POS] [-max MAX_POS] [-step STEP] [-d DELAY]
```

#### Parametreler:

- `-p, --port`: Seri port (varsayılan: /dev/ttyACM0)
- `-s, --servo`: Servo pin numarası (0-18 arası, varsayılan: 0)
- `-min, --min_pos`: Minimum pozisyon (500-2500 arası, varsayılan: 1000)
- `-max, --max_pos`: Maksimum pozisyon (500-2500 arası, varsayılan: 2000)
- `-step, --step`: Adım büyüklüğü (5-100 arası, varsayılan: 50)
- `-d, --delay`: Adımlar arası gecikme (saniye) (0.01-1.0 arası, varsayılan: 0.1)

### 2. Servo Yumuşak Hareket Testi (`servo_smooth.py`)

Bu araç, servo motorunu yumuşak geçişli hareketlerle kontrol eder. İki hareket modu sunar:

1. **Sine Modu**: İki pozisyon arasında yumuşak sinüs dalgası şeklinde hareket.
2. **Bounce Modu**: Merkez etrafında giderek azalan genlikle yaylanma hareketi.

#### Kullanım:

```bash
python servo_smooth.py [-h] [-p PORT] [-s SERVO] [-m {sine,bounce}] [--pos1 POS1] [--pos2 POS2] [--center CENTER] [--amplitude AMPLITUDE] [--cycles CYCLES] [--time TIME]
```

#### Parametreler:

- `-p, --port`: Seri port (varsayılan: /dev/ttyACM0)
- `-s, --servo`: Servo pin numarası (0-18 arası, varsayılan: 0)
- `-m, --mode`: Hareket modu (`sine` veya `bounce`, varsayılan: sine)
- `--pos1`: Birinci pozisyon (sine modu için, varsayılan: 1000)
- `--pos2`: İkinci pozisyon (sine modu için, varsayılan: 2000)
- `--center`: Merkez pozisyon (bounce modu için, varsayılan: 1500)
- `--amplitude`: Hareket genliği (bounce modu için, varsayılan: 300)
- `--cycles`: Yaylanma çevrim sayısı (bounce modu için, varsayılan: 5)
- `--time`: Hareket süresi (saniye) (varsayılan: 2.0)

## Kurulum

1. Python 3.x yüklü olmalıdır.
2. Gerekli kütüphaneleri yükleyin:

```bash
pip install pyserial
```

3. Servo2040 kartınızı USB ile bilgisayara bağlayın.
4. Scriptleri çalıştırın.

## Notlar

- Servo pozisyon değerleri genellikle 500-2500 arasında olmalıdır (1500 merkez).
- Çalıştırmadan önce servo motorların bağlantılarını kontrol edin.
- Programı durdurmak için Ctrl+C kullanın.


