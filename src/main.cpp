#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "tusb.h"
#include "pirobot_servo2040.hpp"

/**
 * @brief Ana program başlangıç noktası
 * 
 * @return int
 */
int main() {
    // Tusb başlat
    tusb_init();
    
    // Ana uygulamayı oluştur ve başlat
    PirobotServo2040 pirobot;
    pirobot.init();
    
    // Ana döngüyü çalıştır
    pirobot.run();
    
    return 0;
} 