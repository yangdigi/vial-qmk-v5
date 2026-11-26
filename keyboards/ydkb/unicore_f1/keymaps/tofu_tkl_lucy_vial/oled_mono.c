#include QMK_KEYBOARD_H
#include "eeconfig.h"
#include "oled_lucy.h"
#include "bongocat.h"

static uint8_t is_booting = 1;
static bool is_idle = 1;
static uint32_t last_keypress_timer;

#define OLED_MODE_TOG USER04

typedef enum {
    OLED_MODE_LETTERS = 0,
    OLED_MODE_BONGO   = 1,
} oled_mode_t;

#define OLED_EEPROM_MAGIC 0xBCu
static uint8_t oled_mode = OLED_MODE_BONGO;

static void oled_config_load(void) {
    uint32_t raw = eeconfig_read_user();
    oled_mode = (((raw >> 24) & 0xFF) == OLED_EEPROM_MAGIC) ? ((raw & 0x01) ? OLED_MODE_BONGO
                                                                            : OLED_MODE_LETTERS)
                                                           : OLED_MODE_BONGO;
}

static void oled_config_save(void) {
    eeconfig_update_user(((uint32_t)OLED_EEPROM_MAGIC << 24) | (uint32_t)oled_mode);
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    setPinOutput(B5);
    writePinLow(B5);
    wait_ms(10);

    oled_config_load();
    return OLED_ROTATION_180;
}

void render_lucy_boot(void) {
    for (uint8_t i=0; i<4; i++) {
        oled_set_cursor(0, i);
        oled_write_raw_P(&lucy_boot[0]+i*128, 128);
    }
}

void render_lucy_text(void) {
    for (uint8_t i=0; i<4; i++) {
        oled_set_cursor(0, i);
        oled_write_raw_P(&lucy_text[0]+i*128, 128);
    }
}

#if 0
bool oled_task_user() {
    render_lucy_boot();
    //render_logo();
    return false;
}
#endif

#ifdef OLED_ENABLE

#define OLED_BUF_SIZE 12
#define TOP_OFFSET_PX 1
static char oled_string_buf[OLED_BUF_SIZE] = "";
static uint8_t oled_buf_idx = 0;

void draw_fine_string(const char *str, uint8_t start_col, uint8_t page) {
    uint8_t current_col = start_col;

    while (*str) {
        char c = *str;
        int16_t font_idx = -1;

        if (c >= 'A' && c <= 'Z') font_idx = c - 'A';
        else if (c == '|') font_idx = 26;

        if (font_idx != -1) {
            uint8_t p1_buf[12];
            uint8_t p2_buf[12];
            uint8_t p3_buf[12];

            for (uint8_t i = 0; i < 12; i++) {
                uint8_t top_half = pgm_read_byte(&fine_font_12x16[font_idx][i]);      
                uint8_t bot_half = pgm_read_byte(&fine_font_12x16[font_idx][i + 12]); 

                p1_buf[i] = top_half << TOP_OFFSET_PX;
                p2_buf[i] = (top_half >> (8-TOP_OFFSET_PX)) | (bot_half << TOP_OFFSET_PX);
                p3_buf[i] = (bot_half >> (8-TOP_OFFSET_PX));
            }

            uint8_t c_col = current_col;

            oled_set_cursor(c_col, page);
            oled_write_raw((const char*)p1_buf, 12);

            oled_set_cursor(c_col, page + 1);
            oled_write_raw((const char*)p2_buf, 12);

            oled_set_cursor(c_col, page + 2);
            oled_write_raw((const char*)p3_buf, 12);

        } else if (c == ' ') {
            static const uint8_t blank[12] = {0};
            uint8_t c_col = current_col;
            oled_set_cursor(c_col, page);
            oled_write_raw((const char*)blank, 12);
            oled_set_cursor(c_col, page + 1);
            oled_write_raw((const char*)blank, 12);
            oled_set_cursor(c_col, page + 2);
            oled_write_raw((const char*)blank, 12);
        }

        current_col += 2;
        str++;
    }
}

// ===== BongoCat 打字动画 =====
#define BONGO_TAP_MS        700
#define BONGO_READY_MS      1400
#define BONGO_TAP_FRAME_MS   140
#define BONGO_IDLE_FRAME_MS  300
#define BONGO_IDLE_FRAMES      5

static uint32_t bongo_anim_timer;
static uint8_t  bongo_anim_frame;

static void render_bongo_cat(void) {
    uint32_t    elapsed = timer_elapsed32(last_keypress_timer);
    const char *frame;

    if (elapsed < BONGO_TAP_MS) {
        if (timer_elapsed32(bongo_anim_timer) >= BONGO_TAP_FRAME_MS) {
            bongo_anim_timer = timer_read32();
            bongo_anim_frame ^= 1;
        }
        frame = bongo_tap[bongo_anim_frame & 0x01];
    } else if (elapsed < BONGO_READY_MS) {
        bongo_anim_frame = 0;
        frame = bongo_prep;
    } else {
        if (timer_elapsed32(bongo_anim_timer) >= BONGO_IDLE_FRAME_MS) {
            bongo_anim_timer = timer_read32();
            bongo_anim_frame = (bongo_anim_frame + 1) % BONGO_IDLE_FRAMES;
        }
        frame = bongo_idle[bongo_anim_frame % BONGO_IDLE_FRAMES];
    }

    oled_set_cursor(0, 0);
    oled_write_raw_P(frame, 128 * 4);
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    if (record->event.pressed) {
        if (keycode == OLED_MODE_TOG) {
            oled_mode ^= 1;
            oled_config_save();
            oled_clear();
            oled_string_buf[0] = '\0';
            oled_buf_idx = 0;
            is_idle = 0;
            last_keypress_timer = timer_read32();
            bongo_anim_timer  = last_keypress_timer;
            bongo_anim_frame  = 0;
            return false;
        }

        last_keypress_timer = timer_read32();
        if (is_idle) { oled_clear(); is_idle = 0;}

        if (oled_mode == OLED_MODE_BONGO) {
            return true;
        }

        if (keycode == KC_ENTER || keycode == KC_SPACE) {
            oled_string_buf[0] = '\0';
            oled_buf_idx = 0;
            oled_clear(); 
            return true;
        }

        if (keycode == KC_BSPC) {
            if (oled_buf_idx > 0) {
                oled_buf_idx--;
                oled_string_buf[oled_buf_idx] = '\0';
                oled_clear(); 
            }
            return true;
        }

        if (keycode >= KC_A && keycode <= KC_Z) {
            char pressed_char = 'A' + (keycode - KC_A);

            if (oled_buf_idx < 8) { 
                oled_string_buf[oled_buf_idx] = pressed_char;
                oled_buf_idx++;
                oled_string_buf[oled_buf_idx] = '\0';
            } else {
                for (uint8_t i = 0; i < 7; i++) {
                    oled_string_buf[i] = oled_string_buf[i + 1];
                }
                oled_string_buf[7] = pressed_char;
                oled_string_buf[8] = '\0';
                oled_buf_idx = 8; 
                oled_clear(); 
            }
        }
    }
    return true;
}

bool oled_task_user(void) {
    if (is_booting) {
        is_booting++;
        if (is_booting <= 40) {
            render_lucy_boot();
            return false;
        } else {
            is_booting = 0;
            oled_clear();
        }
    }

    if (is_idle || timer_elapsed32(last_keypress_timer) > 20*1000) {
        if (timer_elapsed32(last_keypress_timer) > 120*1000) {
            oled_clear();
        }
        else render_lucy_text();
        is_idle = 1;
        return false;
    }

    if (oled_mode == OLED_MODE_BONGO) {
        render_bongo_cat();
        return false;
    }

    bool show_cursor = (timer_read() % 1000) < 500;
    char cursor_char = '|'; 
    
    uint8_t target_page = 1; 
    
    uint8_t start_col_pos = 2; 

    if (oled_buf_idx == 0) {
        if (show_cursor) {
            char single_cursor[2] = {cursor_char, '\0'};
            draw_fine_string(single_cursor, start_col_pos, target_page);
        } else {
            draw_fine_string(" ", start_col_pos, target_page);
        }
    } else {
        if (show_cursor) {
            oled_string_buf[oled_buf_idx] = cursor_char;
            oled_string_buf[oled_buf_idx + 1] = '\0';
            
            draw_fine_string(oled_string_buf, start_col_pos, target_page);
            
            oled_string_buf[oled_buf_idx] = '\0'; 
        } else {
            draw_fine_string(oled_string_buf, start_col_pos, target_page);
            draw_fine_string(" ", start_col_pos + (oled_buf_idx * 2), target_page);
        }
    }

    return false;
}

#endif