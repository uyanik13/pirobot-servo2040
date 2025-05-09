cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)
# Pull in Pimoroni libraries
include(pimoroni_pico_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

function(add_resource target file)
  get_filename_component(NAME ${ARGV1} NAME_WE)
  set(FILENAME ${ARGV1})

  add_custom_command(
    OUTPUT ${NAME}.o

    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}
            ${CMAKE_CURRENT_BINARY_DIR}

    COMMAND arm-none-eabi-ld -r -b binary -o ${NAME}.o ${FILENAME}
    DEPENDS ${FILENAME}
  )

  target_sources(${ARGV0} PRIVATE ${NAME}.o)
endfunction(add_resource)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${PIMORONI_PICO_PATH}
  ${PIMORONI_PICO_PATH}/drivers/servo
  ${PIMORONI_PICO_PATH}/drivers/pwm
  ${PIMORONI_PICO_PATH}/drivers/plasma
  ${PIMORONI_PICO_PATH}/drivers/analogmux
  ${PIMORONI_PICO_PATH}/drivers/analog
  ${PIMORONI_PICO_PATH}/libraries/servo2040
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

# Using external repos instead of local directories
# add_subdirectory(common)
# add_subdirectory(drivers)
# add_subdirectory(libraries)
add_subdirectory(src)
