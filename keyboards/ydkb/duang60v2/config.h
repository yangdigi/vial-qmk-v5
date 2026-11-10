#pragma once

#include "config_common.h"
#include "config_ble51.h"

/* USB Device descriptor parameter */
#define FW_VER_DATE     DQ29
#define VENDOR_ID       0x9D5B 
#define PRODUCT_ID      0x2162 
#define DEVICE_VER      0x0001
#define MANUFACTURER    YDKB
#define PRODUCT         Duang60v2 (FW_VER)


#define MATRIX_ROWS 9  //595
#define MATRIX_COLS 8

#define TAPPING_TOGGLE  2

#define DEFAULT_6KRO // macOS's Capslock switching between Chinese and English has compatibility issues with NKRO

#define ws2812_PORTREG  PORTE
#define ws2812_DDRREG   DDRE
#define ws2812_pin  PE2
#define ws2812_pin2 PE6
#define RGBLED_NUM 4     // Number of LEDs
#define RGBLIGHT_MODES 14 //less rgblight mode to save some space for vial 

/* BT Power Control */
#define BT_POWERED    (~PORTD & (1<<5))
#define bt_power_init()    do { DDRD |= (1<<5); PORTD &= ~(1<<5); } while(0)
#define bt_power_reset()    do {PORTD |= (1<<5); WAIT_MS(100); PORTD &= ~(1<<5);} while(0)
#define turn_off_bt()    do { PORTD |= (1<<5); UCSR1B = (1<<RXCIE1 | 1<<RXEN1); } while(0)
#define turn_on_bt()    do { PORTD &= ~(1<<5); if (UCSR1B == (1<<RXCIE1 | 1<<RXEN1)) WAIT_MS(200); UCSR1B = (1<<RXCIE1 | 1<<RXEN1 | 1<<TXEN1); } while(0)

#define BLE_NAME "Duang60v2 BLE"
#define BLE_BATTERY_SERVICE
#define BLE_LIGHT_ON (~PORTC & (1<<7)) // RGB Power IO
#define HARDWARE_BT_SWITCH

#define UPDATE_BATTERY_WHEN_CHARGING
#define BATTERY_CHARGING (~PINC & (1<<6))
#define CHARGING_FIX_VALUE 50
#define CHARGING_STATE_INIT()    do { DDRC &= ~(1<<6); PORTC |= (1<<6);} while(0)


#define BLE51_NO_BATTERY_VOLTAGE
#define BLE51_NO_ULTRA_LOW_BATTERY
/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */
/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT  //930B
//#define NO_ACTION_MACRO
//#define NO_ACTION_FUNCTION
#define NO_DEFAULT_COMMAND