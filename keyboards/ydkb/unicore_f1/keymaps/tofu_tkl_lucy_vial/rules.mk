VIA_ENABLE = yes
VIAL_ENABLE = yes
VIAL_INSECURE = yes
QMK_SETTINGS = yes
TAP_DANCE_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
CONSOLE_ENABLE = no

OLED_ENABLE = yes
#OLED_DRIVER = ssd1306  # or 'custom'
OLED_TRANSPORT = i2c   # or 'spi' or 'custom'
I2C_DRIVER_REQUIRED = yes

# project specific files
SRC +=	oled_mono.c