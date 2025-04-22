#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// Defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
  #define CFG_TUSB_MCU OPT_MCU_RP2040 // Raspberry Pi RP2040 (Servo 2040 based on RP2040)
#endif

#ifndef CFG_TUSB_OS
  #define CFG_TUSB_OS OPT_OS_NONE
#endif

// CFG_TUSB_DEBUG is defined by compiler in DEBUG build
#define CFG_TUSB_DEBUG 0

// USB port ayarları - hata düzeltme için eklendi
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE
#define CFG_TUSB_RHPORT1_MODE       OPT_MODE_NONE

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
  #define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
  #define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_ENABLED 1

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED OPT_MODE_FULL_SPEED

/* CDC FIFO size of TX and RX */
#define CFG_TUD_CDC               1
#define CFG_TUD_CDC_RX_BUFSIZE    512
#define CFG_TUD_CDC_TX_BUFSIZE    512

// CDC Endpoint transfer buffer size, more is faster
#define CFG_TUD_CDC_EP_BUFSIZE    64

//--------------------------------------------------------------------
// HOST CONFIGURATION - Not used in our device-only configuration
//--------------------------------------------------------------------

#define CFG_TUH_ENABLED 0

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */ 