#!/bin/bash

# Renk kodları
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logo göster
echo -e "${BLUE}"
echo "  _____  _      ____  ____   ____ _______    _____ ______ _______      ______  ___   _  _   ___   "
echo " |  __ \(_)    |  _ \|  _ \ / __ \__   __|  / ____|  ____|  __ \ \    / / __ \|__ \ | || | / _ \  "
echo " | |__) |_ _ __| |_) | |_) | |  | | | |    | (___ | |__  | |__) \ \  / / |  | |  ) || || || | | | "
echo " |  ___/| | '__|  _ <|  _ <| |  | | | |     \___ \|  __| |  _  / \ \/ /| |  | | / / |__   _| | | | "
echo " | |    | | |  | |_) | |_) | |__| | | |     ____) | |____| | \ \  \  / | |__| |/ /_    | | | |_| | "
echo " |_|    |_|_|  |____/|____/ \____/  |_|    |_____/|______|_|  \_\  \/   \____/____|   |_|  \___/  "
echo -e "${NC}"
echo -e "${YELLOW}Build Script for Servo 2040 Firmware${NC}\n"

# Proje kök dizini
PROJECT_ROOT=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="${PROJECT_ROOT}/build"
FIRMWARE_DIR="${PROJECT_ROOT}/firmware"
BIN_DIR="${BUILD_DIR}/bin"
SRC_DIR="${PROJECT_ROOT}/src"

# İlerleme çubuğu fonksiyonu
progress_bar() {
    local width=50
    local percentage=$1
    local filled=$(printf "%.0f" $(echo "$percentage * $width / 100" | bc -l))
    local empty=$((width - filled))
    
    printf "\r["
    printf "%${filled}s" | tr ' ' '#'
    printf "%${empty}s" | tr ' ' ' '
    printf "] %3d%%" $percentage
}

# Dizinleri kontrol et
echo -e "${BLUE}[i]${NC} Dizinler kontrol ediliyor..."
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}[!]${NC} Build dizini bulunamadı, oluşturuluyor: ${BUILD_DIR}"
    mkdir -p "$BUILD_DIR"
fi

if [ ! -d "$FIRMWARE_DIR" ]; then
    echo -e "${YELLOW}[!]${NC} Firmware dizini bulunamadı, oluşturuluyor: ${FIRMWARE_DIR}"
    mkdir -p "$FIRMWARE_DIR"
fi

# Gerekli ortam değişkenlerini kontrol et
echo -e "${BLUE}[i]${NC} Ortam değişkenleri kontrol ediliyor..."
if [ -z "$PICO_SDK_PATH" ]; then
    echo -e "${RED}[!]${NC} PICO_SDK_PATH tanımlanmamış. Lütfen tanımlayın:"
    echo "export PICO_SDK_PATH=~/pico-sdk"
    exit 1
fi

if [ -z "$PIMORONI_PICO_PATH" ]; then
    echo -e "${RED}[!]${NC} PIMORONI_PICO_PATH tanımlanmamış. Lütfen tanımlayın:"
    echo "export PIMORONI_PICO_PATH=~/pimoroni-pico"
    exit 1
fi

# CMake yapılandırma
echo -e "${BLUE}[i]${NC} CMake yapılandırılıyor..."
cd "$BUILD_DIR" || { echo -e "${RED}[!]${NC} Build dizinine geçilemedi"; exit 1; }

cmake .. > cmake.log 2>&1 || { 
    echo -e "${RED}[!]${NC} CMake yapılandırma hatası:"; 
    cat cmake.log; 
    exit 1; 
}

# Derleme
echo -e "${BLUE}[i]${NC} Proje derleniyor..."
NUM_CORES=$(nproc)
echo -e "${YELLOW}[!]${NC} $NUM_CORES çekirdek kullanılıyor"

# Progress bar simülasyonu
for i in {1..20}; do
    progress_bar $((i * 5))
    sleep 0.1
done
echo -e "\n"

# Gerçek build işlemi
if make -j"$NUM_CORES" > build.log 2>&1; then
    echo -e "${GREEN}[✓]${NC} Derleme başarılı!"
else
    echo -e "${RED}[!]${NC} Derleme hatası:"
    cat build.log
    exit 1
fi

# UF2 dosyasını bul ve kopyala
UF2_FILE=$(find "$BUILD_DIR" -name "*.uf2" | head -n 1)

if [ -n "$UF2_FILE" ]; then
    echo -e "${BLUE}[i]${NC} UF2 firmware dosyası bulundu: $UF2_FILE"
    cp "$UF2_FILE" "$FIRMWARE_DIR/" || { 
        echo -e "${RED}[!]${NC} UF2 dosyası kopyalanamadı"; 
        exit 1; 
    }
    echo -e "${GREEN}[✓]${NC} UF2 firmware dosyası başarıyla kopyalandı: ${FIRMWARE_DIR}/$(basename "$UF2_FILE")"
else
    echo -e "${RED}[!]${NC} UF2 dosyası bulunamadı!"
    exit 1
fi

# Derlenmiş .bin dosyalarını kopyala
if [ -d "$BIN_DIR" ]; then
    for f in "$BIN_DIR"/*.bin "$BIN_DIR"/*.elf; do
        if [ -f "$f" ]; then
            cp "$f" "$FIRMWARE_DIR/" || { 
                echo -e "${RED}[!]${NC} $(basename "$f") dosyası kopyalanamadı"; 
                continue; 
            }
            echo -e "${GREEN}[✓]${NC} Firmware dosyası başarıyla kopyalandı: ${FIRMWARE_DIR}/$(basename "$f")"
        fi
    done
fi

echo -e "\n${GREEN}[✓]${NC} Derleme ve dosya kopyalama tamamlandı!"
echo -e "${YELLOW}[!]${NC} Firmware dosyaları: $FIRMWARE_DIR"
ls -la "$FIRMWARE_DIR"

echo -e "\n${BLUE}[i]${NC} Servo 2040 kartına firmware yüklemek için:"
echo -e "${YELLOW}1. BOOT tuşuna basılı tutarken RESET tuşuna basın"
echo -e "2. RP2040 sürücüsü bilgisayarınıza takıldığında, .uf2 dosyasını sürücüye sürükleyip bırakın${NC}" 