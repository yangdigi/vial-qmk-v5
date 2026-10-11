# project specific files
SRC ?=	matrix.c ec_matrix.c led.c rawhid_user.c

# MCU name
MCU = STM32F103
BOARD = DC32F103

# Bootloader selection
MCU_LDSCRIPT = STM32F103CBT6_uf2_bootloader
BOOTLOADER = custom

# Build Options
#   change yes to no to disable
#
CUSTOM_MATRIX           = yes # Custom matrix file
UNICODE_ENABLE          = yes # Unicode
BOOTMAGIC_ENABLE = no       # Enable Bootmagic Lite
MOUSEKEY_ENABLE = yes       # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE ?= yes       # Console for debug
COMMAND_ENABLE = yes        # Commands for debug and configuration
NKRO_ENABLE = yes           # Enable N-Key Rollover
BACKLIGHT_ENABLE = no      # Enable keyboard backlight functionality
RGBLIGHT_ENABLE = yes       # Enable keyboard RGB underglow
AUDIO_ENABLE = no           # Audio output
SLEEP_LED_ENABLE = no


# Enter lower-power sleep mode when on the ChibiOS idle thread
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE
OPT_DEFS += -DCORTEX_VTOR_INIT=0x4000

# WS2812 uses the bitbang driver, but the keyboard-local ws2812.c (same
# directory) shadows the shared platforms/chibios/drivers/ws2812.c via VPATH
# search order, so the shared driver stays untouched.
WS2812_DRIVER = bitbang

# Select WS2812 bit timing at run time: 96 MHz HSE normally, 48 MHz HSI
# fallback when the external crystal is not fitted (see board.c clock init).
OPT_DEFS += -DWS2812_HSI_FALLBACK
