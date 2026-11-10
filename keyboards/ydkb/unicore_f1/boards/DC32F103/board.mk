# List of all the board related files.
BOARDSRC = $(KEYBOARD_PATH_1)/boards/DC32F103/board.c

# Required include directories: our board.c plus the stock board.h location.
BOARDINC = $(KEYBOARD_PATH_1)/boards/DC32F103

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
