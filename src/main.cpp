#include "pirobot_servo2040.hpp"

/**
 * @brief Ana program başlangıç noktası
 * 
 * @return int
 */
int main() {
    // Ana uygulama nesnesini oluştur
    PirobotServo2040 app;
    
    // Sistemi başlat
    app.init();
    
    // Ana döngüyü çalıştır
    app.run();
    
    return 0;
} 