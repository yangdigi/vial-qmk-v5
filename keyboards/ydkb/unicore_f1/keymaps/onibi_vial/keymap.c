#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  {
    {KC_ESCAPE, KC_GRAVE, KC_1, KC_F1, KC_2, KC_F2, KC_F3, KC_3},
    {KC_LGUI, KC_A, KC_Q, KC_A, KC_LCTRL, KC_LSHIFT, KC_CAPSLOCK, KC_TAB},
    {KC_F4, KC_4, KC_F5, KC_5, KC_F6, KC_F7, KC_6, KC_7},
    {KC_X, KC_R, KC_D, KC_E, KC_LALT, KC_Z, KC_S, KC_W},
    {KC_T, KC_Y, KC_F8, KC_8, KC_U, KC_F9, KC_9, KC_I},
    {KC_N, KC_SPACE, KC_H, KC_B, KC_V, KC_G, KC_C, KC_F},
    {KC_F10, KC_0, KC_O, KC_F11, KC_MINUS, KC_P, KC_EQUAL, KC_LBRACKET},
    {KC_SCOLON, KC_L, KC_DOT, KC_RALT, KC_COMMA, KC_M, KC_K, KC_J},
    {KC_F12, KC_NO, KC_BSPACE, KC_RBRACKET, KC_CALC, KC_BSLASH, KC_NO, KC_NO},
    {KC_ENTER, KC_ENTER, KC_RSHIFT, MO(1), KC_RCTRL, KC_RGUI, KC_SLASH, KC_QUOTE},
    {KC_PSCREEN, KC_INSERT, KC_DELETE, KC_SCROLLLOCK, KC_END, KC_HOME, KC_NO, KC_NO},
    {KC_PGDOWN, KC_PGUP, KC_PAUSE, KC_RIGHT, KC_DOWN, KC_LEFT, KC_UP, KC_NO},
    {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
    {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO}
  },
  {
    {QK_BOOT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_TRNS, RGB_TOG, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {RGB_RMOD, RGB_MOD, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {MAGIC_TOGGLE_NKRO, KC_TRNS, RGB_VAI, KC_TRNS, KC_TRNS, RGB_SAI, KC_CALC, RGB_HUI},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
    {KC_MPRV, KC_TRNS, KC_VOLU, KC_TRNS, KC_VOLD, KC_MUTE, KC_TRNS, KC_TRNS},
    {KC_TRNS, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_NO},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_MNXT, KC_MPLY},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_NO},
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO},
    {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
    {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO}
  },
};

#include "led.h"
#include "rgblight.h"
extern uint8_t indicator_color_config[3];
extern uint8_t indicator_state;
void rgb_extra_process(LED_TYPE *rgbled) {
    if (indicator_color_config[2] & 1) {
        if (indicator_state & 1) {
            for (uint8_t i=2; i<6; i++) {
                rgbled[i] = rgbled[0];
            }
        }
    }
}