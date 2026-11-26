#pragma once

#undef  PRODUCT_ID
#define PRODUCT_ID    0x2601
#undef  PRODUCT
#define PRODUCT    Tofu TKL x Lucy (FW_VER)
#define MAX_ROWS 12
#define DYNAMIC_KEYMAP_LAYER_COUNT    8
#define FLASH_KEYMAP_COUNT    2
#define VIAL_KEYBOARD_UID    {0x2E, 0xE6, 0x0E, 0x23, 0x34, 0xEF, 0x99, 0x37}
#define VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT 0x0031

#undef  DEBOUNCE_DN
#undef  DEBOUNCE_UP
#define DEBOUNCE_DN 1
#define DEBOUNCE_UP 3

#undef  RGBLIGHT_LIMIT_VAL
#define RGBLIGHT_LIMIT_VAL    128
#undef  RGBLED_NUM
#define RGBLED_NUM    1
#define PHY_INDICATOR_NUM    2
#define INDICATOR_FUNCT    {(1<<USB_LED_CAPS_LOCK), (1<<USB_LED_SCROLL_LOCK)}
#define INDICATOR_VAL    128
//#define RGB_EXTRA_PROCESS_ENABLE


#define _USE_I2C1

#ifdef OLED_ENABLE
#define OLED_DISPLAY_128X32
#define OLED_IC OLED_IC_SSD1306
#define OLED_DISPLAY_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#endif
// I2C config
#define I2C_DRIVER I2CD1
#define I2C1_SCL_PIN B6     // SCL pin assignment
#define I2C1_SDA_PIN B7     // SDA pin assignment
#define I2C1_CLOCK_SPEED 400000
#define I2C1_DUTY_CYCLE FAST_DUTY_CYCLE_2

#define OLED_TIMEOUT 60000000 
#define OLED_UPDATE_INTERVAL 50