#include "quantum.h"
#include "ws2812.h"
#include <ch.h>
#include <hal.h>

/* Adapted from https://github.com/bigjosh/SimpleNeoPixelDemo/ */

/* Keyboard-local override of platforms/chibios/drivers/ws2812.c (kept
 * upstream untouched). Placed in the keyboard directory so VPATH resolves
 * this file instead of the shared bitbang driver (see build_keyboard.mk
 * VPATH order). Adds run-time dual-clock bit timing: the board boots at
 * 96 MHz HSE when the crystal is fitted, 48 MHz HSI when it is not. */

#ifndef NOP_FUDGE
#    if defined(STM32F0XX) || defined(STM32F1XX) || defined(GD32VF103) || defined(STM32F3XX) || defined(STM32F4XX) || defined(STM32L0XX) || defined(WB32F3G71xx) || defined(WB32FQ95xx)
#        define NOP_FUDGE 0.4
#    else
#        error("NOP_FUDGE configuration required")
#        define NOP_FUDGE 1 // this just pleases the compile so the above error is easier to spot
#    endif
#endif

// Push Pull or Open Drain Configuration
// Default Push Pull
#ifndef WS2812_EXTERNAL_PULLUP
#    define WS2812_OUTPUT_MODE PAL_MODE_OUTPUT_PUSHPULL
#else
#    define WS2812_OUTPUT_MODE PAL_MODE_OUTPUT_OPENDRAIN
#endif

// The reset gap can be 6000 ns, but depending on the LED strip it may have to be increased
// to values like 600000 ns. If it is too small, the pixels will show nothing most of the time.
#ifndef WS2812_RES
#    define WS2812_RES (1000 * WS2812_TRST_US) // Width of the low gap between bits to cause a frame to latch
#endif

#define NUMBER_NOPS 6
#define CYCLES_PER_SEC (CPU_CLOCK / NUMBER_NOPS * NOP_FUDGE)
#define NS_PER_SEC (1000000000L) // Note that this has to be SIGNED since we want to be able to check for negative values of derivatives
#define NS_PER_CYCLE (NS_PER_SEC / CYCLES_PER_SEC)
#define NS_TO_CYCLES(n) ((n) / NS_PER_CYCLE)

#define wait_ns(x)                                  \
    do {                                            \
        for (int i = 0; i < NS_TO_CYCLES(x); i++) { \
            __asm__ volatile("nop\n\t"              \
                             "nop\n\t"              \
                             "nop\n\t"              \
                             "nop\n\t"              \
                             "nop\n\t"              \
                             "nop\n\t");            \
        }                                           \
    } while (0)

/*
 * Above 72 MHz the NOP loop above mistimes the bit cell: the cost of its
 * back-branch is swallowed by the integer NS_TO_CYCLES rounding, so at e.g.
 * 96 MHz the whole cell shrinks to ~1.0 us (below the WS2812 window) and the
 * T1H margin is lost. For such clocks expand every half-bit wait into a
 * straight, branch-free run of NOPs. With the Flash prefetch buffer enabled
 * the Cortex-M3 retires sequential NOPs at exactly one per cycle, so the
 * timing depends solely on CPU_CLOCK. The wait is inherently bounded and no
 * hardware counter is read, so it can never spin forever (a DWT CYCCNT loop
 * does exactly that on silicon where CYCCNT does not count).
 *
 * The values subtracted from the NOP counts cover the fixed instructions
 * surrounding each wait in sendByte (the BSRR/BRR store and the bit-loop
 * tail). If sendByte is changed, recount with objdump -d ws2812.o.
 */
#if defined(CPU_CLOCK) && (CPU_CLOCK > 72000000)
/* ns rounded to nearest CPU cycle */
#    define WS2812_NS_TO_CYC(ns) \
        ((uint32_t)(((uint64_t)(ns) * (uint64_t)(CPU_CLOCK) + 500000000ULL) / 1000000000ULL))
/*
 * Straight, branch-free NOP run. "fixed" is the number of non-NOP cycles
 * between the surrounding GPIO stores for this particular call site, counted
 * from `arm-none-eabi-objdump -d ws2812.o` (bit-loop layout):
 *   1: not-taken test branch before the run (1-bit high time)
 *   3: taken test branch reaching the run (0-bit high time)
 *   7: subs + taken loop branch + lsl + tst + next BSRR store (1-bit low time)
 *   9: extra unconditional branch plus the 7 above (0-bit low time)
 * so each output half-bit is exactly WS2812_NS_TO_CYC(ns) cycles and both bit
 * cells are WS2812_NS_TO_CYC(WS2812_TIMING) cycles.
 */
#    define ws2812_delay_cyc(ns, fixed)                                          \
        __asm__ volatile(".rept %c0\n\t"                                        \
                         "nop\n\t"                                              \
                         ".endr"                                                \
                         :                                                      \
                         : "i"((int)(WS2812_NS_TO_CYC(ns) - (uint32_t)(fixed))) \
                         : "memory")

#    if defined(WS2812_HSI_FALLBACK)
/*
 * Run-time clock fallback support: the firmware normally runs at the
 * compile-time CPU_CLOCK (>72 MHz), but can boot on the internal HSI at
 * exactly 48 MHz when no HSE crystal is fitted (see
 * boards/.../board.c stm32_clock_init_user). CPU_CLOCK above is then wrong
 * for the actual hardware, so a second sendByte is compiled with the
 * original NOP-loop math pinned to 48 MHz - the same timing path that was
 * proven on 48 MHz HSI builds - and the active one is selected per frame.
 */
#        define WS2812_FALLBACK_CLOCK 48000000U
#        define CYCLES_PER_SEC_FB \
            (WS2812_FALLBACK_CLOCK / NUMBER_NOPS * NOP_FUDGE)
#        define NS_PER_CYCLE_FB (NS_PER_SEC / CYCLES_PER_SEC_FB)
#        define NS_TO_CYCLES_FB(n) ((n) / NS_PER_CYCLE_FB)
#        define wait_ns_fb(x)                              \
            do {                                          \
                for (int i = 0; i < NS_TO_CYCLES_FB(x); i++) { \
                    __asm__ volatile("nop\n\t"             \
                                     "nop\n\t"             \
                                     "nop\n\t"             \
                                     "nop\n\t"             \
                                     "nop\n\t"             \
                                     "nop\n\t");           \
                }                                          \
            } while (0)
extern bool clock_hse_ok;
#    endif
#else
#    define ws2812_delay_cyc(ns, fixed) wait_ns(ns)
#endif

#if defined(WS2812_HSI_FALLBACK) && defined(CPU_CLOCK) && (CPU_CLOCK > 72000000)
#    define WS2812_DUAL_CLOCK
#endif

#if defined(WS2812_HSI_FALLBACK) && !defined(WS2812_DUAL_CLOCK)
#    error "WS2812_HSI_FALLBACK requires CPU_CLOCK above 72 MHz"
#endif

#if defined(WS2812_DUAL_CLOCK)
static void sendByte_96(uint8_t byte)
#else
void sendByte(uint8_t byte)
#endif
{
    // WS2812 protocol wants most significant bits first
    for (unsigned char bit = 0; bit < 8; bit++) {
        bool is_one = byte & (1 << (7 - bit));
        // using something like wait_ns(is_one ? T1L : T0L) here throws off timings
        if (is_one) {
            // 1
            writePinHigh(RGB_DI_PIN);
            ws2812_delay_cyc(WS2812_T1H, 1);
            writePinLow(RGB_DI_PIN);
            ws2812_delay_cyc(WS2812_T1L, 7);
        } else {
            // 0
            writePinHigh(RGB_DI_PIN);
            ws2812_delay_cyc(WS2812_T0H, 3);
            writePinLow(RGB_DI_PIN);
            ws2812_delay_cyc(WS2812_T0L, 9);
        }
    }
}

#if defined(WS2812_DUAL_CLOCK)
/* 48 MHz HSI fallback, uses the classic NOP-loop timing math (see
 * wait_ns_fb above) instead of the 96 MHz branch-free NOP runs. */
static void sendByte_48(uint8_t byte) {
    // WS2812 protocol wants most significant bits first
    for (unsigned char bit = 0; bit < 8; bit++) {
        bool is_one = byte & (1 << (7 - bit));
        if (is_one) {
            // 1
            writePinHigh(RGB_DI_PIN);
            wait_ns_fb(WS2812_T1H);
            writePinLow(RGB_DI_PIN);
            wait_ns_fb(WS2812_T1L);
        } else {
            // 0
            writePinHigh(RGB_DI_PIN);
            wait_ns_fb(WS2812_T0H);
            writePinLow(RGB_DI_PIN);
            wait_ns_fb(WS2812_T0L);
        }
    }
}
#endif

void ws2812_init(void) {
    writePinLow(RGB_DI_PIN);     // fix first led being green when booting.
    palSetLineMode(RGB_DI_PIN, WS2812_OUTPUT_MODE);
}

// Setleds for standard RGB
void ws2812_setleds(LED_TYPE *ledarray, uint16_t leds) {
    static bool s_init = false;
    if (!s_init) {
        ws2812_init();
        s_init = true;
    }

    // this code is very time dependent, so we need to disable interrupts
    chSysLock();

#if defined(WS2812_DUAL_CLOCK)
    // Select the timing for the clock tree actually running; the indirect
    // call only adds a few cycles *between* bytes, never inside a bit cell.
    void (*send_byte_fn)(uint8_t) = clock_hse_ok ? sendByte_96 : sendByte_48;
#endif

    for (uint8_t i = 0; i < leds; i++) {
        // WS2812 protocol dictates grb order
#if (WS2812_BYTE_ORDER == WS2812_BYTE_ORDER_GRB)
#if defined(WS2812_DUAL_CLOCK)
        send_byte_fn(ledarray[i].g);
        send_byte_fn(ledarray[i].r);
        send_byte_fn(ledarray[i].b);
#else
        sendByte(ledarray[i].g);
        sendByte(ledarray[i].r);
        sendByte(ledarray[i].b);
#endif
#elif (WS2812_BYTE_ORDER == WS2812_BYTE_ORDER_RGB)
#if defined(WS2812_DUAL_CLOCK)
        send_byte_fn(ledarray[i].r);
        send_byte_fn(ledarray[i].g);
        send_byte_fn(ledarray[i].b);
#else
        sendByte(ledarray[i].r);
        sendByte(ledarray[i].g);
        sendByte(ledarray[i].b);
#endif
#elif (WS2812_BYTE_ORDER == WS2812_BYTE_ORDER_BGR)
#if defined(WS2812_DUAL_CLOCK)
        send_byte_fn(ledarray[i].b);
        send_byte_fn(ledarray[i].g);
        send_byte_fn(ledarray[i].r);
#else
        sendByte(ledarray[i].b);
        sendByte(ledarray[i].g);
        sendByte(ledarray[i].r);
#endif
#endif

#ifdef RGBW
#    if defined(WS2812_DUAL_CLOCK)
        send_byte_fn(ledarray[i].w);
#    else
        sendByte(ledarray[i].w);
#    endif
#endif
    }

#if defined(WS2812_DUAL_CLOCK)
    if (clock_hse_ok) {
        wait_ns(WS2812_RES);
    } else {
        wait_ns_fb(WS2812_RES);
    }
#else
    wait_ns(WS2812_RES);
#endif

    chSysUnlock();
}
