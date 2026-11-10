#ifndef SWITCH_BOARD_H
#define SWITCH_BOARD_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define DS_PL_HI()      (PORTB |=  (1<<3))
#define DS_PL_LO()      (PORTB &= ~(1<<3))


#define CLOCK_PULSE() \
    do { \
        PORTB |= (1<<1); \
        PORTB &= ~(1<<1); \
    } while(0)

#endif