# Hedef adı
set(OUTPUT_NAME servo2040-modular)

# Define project specific constants
add_definitions(
    -DSERVO2040_PROJECT_NAME="Servo2040 Modular Firmware"
    -DSERVO2040_PROJECT_VERSION="1.0.0"
)

# Add namespace definitions
add_definitions(
    -DUSE_SERVO_NAMESPACE
)

# Output directory for the built files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Yürütülebilir dosyayı tanımla
add_executable(${OUTPUT_NAME}
    main.cpp
    pirobot_servo2040.cpp
    servo_driver.cpp
    sensor_manager.cpp
    led_manager.cpp
    gpio_manager.cpp
    comm_protocol.cpp
    # Add the Pimoroni driver sources directly
    ${PIMORONI_PICO_PATH}/drivers/servo/servo.cpp
    ${PIMORONI_PICO_PATH}/drivers/servo/servo_cluster.cpp
    ${PIMORONI_PICO_PATH}/drivers/servo/servo_state.cpp
    ${PIMORONI_PICO_PATH}/drivers/servo/calibration.cpp
    ${PIMORONI_PICO_PATH}/drivers/pwm/pwm.cpp
    ${PIMORONI_PICO_PATH}/drivers/pwm/pwm_cluster.cpp
    ${PIMORONI_PICO_PATH}/drivers/plasma/ws2812.cpp
    ${PIMORONI_PICO_PATH}/drivers/plasma/apa102.cpp
    ${PIMORONI_PICO_PATH}/drivers/analogmux/analogmux.cpp
    ${PIMORONI_PICO_PATH}/drivers/analog/analog.cpp
)

# Generate PIO header files
pico_generate_pio_header(${OUTPUT_NAME} ${PIMORONI_PICO_PATH}/drivers/plasma/ws2812.pio)
pico_generate_pio_header(${OUTPUT_NAME} ${PIMORONI_PICO_PATH}/drivers/plasma/apa102.pio)
pico_generate_pio_header(${OUTPUT_NAME} ${PIMORONI_PICO_PATH}/drivers/pwm/pwm_cluster.pio)

# Bağımlılıkları ekle
target_link_libraries(${OUTPUT_NAME}
    pico_stdlib
    hardware_pio
    hardware_pwm
    hardware_dma
    hardware_adc
)

# Include paths for external libraries
target_include_directories(${OUTPUT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${PICO_SDK_PATH}/src/common/pico_stdlib/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_adc/include
    ${PIMORONI_PICO_PATH}/libraries/servo2040
    ${PIMORONI_PICO_PATH}/drivers/servo
    ${PIMORONI_PICO_PATH}/drivers/pwm
    ${PIMORONI_PICO_PATH}/drivers/plasma
    ${PIMORONI_PICO_PATH}/drivers/analogmux
    ${PIMORONI_PICO_PATH}/drivers/analog
)

# USB etkinleştir, UART devre dışı bırak
pico_enable_stdio_usb(${OUTPUT_NAME} 1)
pico_enable_stdio_uart(${OUTPUT_NAME} 0)

# UF2 dosyası oluşturma - sadece bu yeterli olmalı
pico_add_extra_outputs(${OUTPUT_NAME})