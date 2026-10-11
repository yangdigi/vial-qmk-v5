#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define FW_VER_DATE     DQAB
#define CONTACT(x,y)    x##y
#define CONTACT2(x,y)   CONTACT(x,y)
#define FW_VER          CONTACT2(VIAL_, FW_VER_DATE)
#define VENDOR_ID       0x9D5B
#define PRODUCT_ID      0x24EF
#define DEVICE_VER      0x0001
#define MANUFACTURER    KBDFans
#if CONSOLE_ENABLE
#define PRODUCT         Evoli Keyboard Uni Debug (FW_VER)
#else
#define PRODUCT         Evoli Keyboard Uni (FW_VER)
#endif

#define USB_MAX_POWER_CONSUMPTION 350


/* key matrix size */
#define MATRIX_ROWS 7
#define MATRIX_COLS 16
/* 32bit for Layout Options */
#define VIA_EEPROM_LAYOUT_OPTIONS_SIZE 2 //default 1,  not enough for ec_ap and indicator color
#define VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT 4 //default AP Level 5.
//#define FORCE_NKRO //32B

#define DEBOUNCE_DN 2
#define DEBOUNCE_UP 2

#define RGBLIGHT_EFFECT_BREATHING
#define RGBLIGHT_EFFECT_RAINBOW_MOOD
#define RGBLIGHT_EFFECT_RAINBOW_SWIRL
#define RGBLIGHT_EFFECT_SNAKE
#define RGBLIGHT_EFFECT_KNIGHT
#define RGBLIGHT_EFFECT_CHRISTMAS
#define RGBLIGHT_EFFECT_STATIC_GRADIENT
#define RGBLIGHT_EFFECT_RGB_TEST
#define RGBLIGHT_EFFECT_ALTERNATING
#define RGBLIGHT_EFFECT_TWINKLE

#define RGBLIGHT_LIMIT_VAL 192
#define RGBLIGHT_SLEEP
#define RGB_DI_PIN B15
#define RGBLED_NUM 16
/* key combination for command */
#define IS_COMMAND() ( \
    (get_mods() == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT))) || \
    (get_mods() == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_LCTRL) | MOD_BIT(KC_RSHIFT))) \
)

