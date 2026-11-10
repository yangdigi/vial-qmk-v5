/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    Modified for unicore_f1e (APM32F103CB):
    - __early_init() calls stm32_clock_init_user() below, a copy of the
      ChibiOS STM32F10X_MD stm32_clock_init() with one change: FLASH->ACR
      is programmed with 3 wait states (0x13, APM32F103 errata 7.2)
      whenever STM32_HCLK is above 72 MHz. This happens while the CPU is
      still running on the 8 MHz HSI, i.e. BEFORE the switch to the PLL.
      The stock hal_lld.c routine hard-codes STM32_FLASHBITS to 0x00000012
      (2 wait states) above 48 MHz and re-defines it unconditionally, so it
      cannot be fixed from project files; mcuconf.h therefore sets
      STM32_NO_INIT TRUE, which turns the stock stm32_clock_init() into a
      no-op, and this board provides the working clock init instead.
    - The HSE start-up is waited on with a bounded timeout instead of
      forever. If the 12 MHz crystal is not fitted (HSERDY never asserts),
      the firmware falls back to the internal HSI: PLL input = HSI/2
      (4 MHz), PLL multiplier x12 -> SYSCLK/HCLK 48 MHz, APB1 24 MHz,
      APB2 48 MHz, ADC 8 MHz, USBCLK 48 MHz (USBPRE /1), 1 flash wait
      state. clock_hse_ok records which tree is running so other code can
      adapt (SysTick retune in chconf.h, WS2812 bit timing).
*/

#include "hal.h"

/*
 * TRUE  - HSE started, running on the 96 MHz HSE PLL.
 * FALSE - HSE failed to start (no external crystal), running on the
 *         48 MHz HSI PLL. Written once in __early_init, before any other
 *         initialization, so it needs no special synchronization.
 */
bool clock_hse_ok = false;

/* PLL multiplier field for the 48 MHz HSI fallback: HSI/2 (4 MHz) x 12. */
#define STM32_FB_PLLMUL                 (((12U - 2U) << 18))
#define STM32_FB_HCLK                   48000000U

/*
 * HSE start-up timeout in busy-loop iterations. The loop executes while the
 * CPU is still on the 8 MHz HSI (~6 cycles/iteration), so 20000 iterations
 * is roughly 15 ms - comfortably above a 12 MHz crystal's typical 1..2 ms
 * start-up time, yet short when no crystal is fitted.
 */
#define STM32_FB_HSE_TIMEOUT_ITERS      20000U

/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
#if HAL_USE_PAL || defined(__DOXYGEN__)
const PALConfig pal_default_config =
{
  {VAL_GPIOAODR, VAL_GPIOACRL, VAL_GPIOACRH},
  {VAL_GPIOBODR, VAL_GPIOBCRL, VAL_GPIOBCRH},
  {VAL_GPIOCODR, VAL_GPIOCCRL, VAL_GPIOCCRH},
  {VAL_GPIODODR, VAL_GPIODCRL, VAL_GPIODCRH},
  {VAL_GPIOEODR, VAL_GPIOECRL, VAL_GPIOECRH},
};
#endif

/*
 * Clocks initialization for the MD sub-family.
 * Copy of ChibiOS stm32_clock_init() with the 96 MHz flash fix, see the
 * file header. Must stay in sync with hal_lld.c if the clock tree in
 * mcuconf.h changes.
 */
void stm32_clock_init_user(void) {

  /* HSI setup, it enforces the reset situation in order to handle possible
     problems with JTAG probes and re-initializations.*/
  RCC->CR |= RCC_CR_HSION;                  /* Make sure HSI is ON.         */
  while (!(RCC->CR & RCC_CR_HSIRDY))
    ;                                       /* Wait until HSI is stable.    */
  RCC->CR &= RCC_CR_HSITRIM | RCC_CR_HSION; /* CR Reset value.              */
  RCC->CFGR = 0;                            /* CFGR reset value.            */
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
    ;                                       /* Waits until HSI is selected. */

#if STM32_HSE_ENABLED
  /* HSE activation with a bounded timeout: if the external crystal is not
     fitted HSERDY never asserts, fall back to the internal HSI instead of
     hanging here forever (which is what the stock routine does).*/
  RCC->CR |= RCC_CR_HSEON;
  {
    uint32_t hse_timeout = STM32_FB_HSE_TIMEOUT_ITERS;
    clock_hse_ok = false;
    do {
      if ((RCC->CR & RCC_CR_HSERDY) != 0U) {
        clock_hse_ok = true;
        break;
      }
    } while (--hse_timeout != 0U);
  }
  if (!clock_hse_ok) {
    RCC->CR &= ~RCC_CR_HSEON;          /* Stop the failed oscillator.      */
  }
#endif

#if STM32_LSI_ENABLED
  /* LSI activation.*/
  RCC->CSR |= RCC_CSR_LSION;
  while ((RCC->CSR & RCC_CSR_LSIRDY) == 0)
    ;                                       /* Waits until LSI is stable.   */
#endif

#if STM32_ACTIVATE_PLL
  /* PLL activation. The PLL source/multiplier can only be changed while the
     PLL is disabled, which it still is at this point.*/
  if (clock_hse_ok) {
    /* HSE path: configured tree from mcuconf.h (12 MHz x8 = 96 MHz).*/
    RCC->CFGR |= STM32_PLLMUL | STM32_PLLXTPRE | STM32_PLLSRC;
  }
#if STM32_HSE_ENABLED
  else {
    /* HSI fallback path: HSI/2 (4 MHz) x12 = 48 MHz. PLLSRC and PLLXTPRE
       are both zero here (CFGR was reset above).*/
    RCC->CFGR |= STM32_FB_PLLMUL;
  }
#endif
  RCC->CR   |= RCC_CR_PLLON;
  while (!(RCC->CR & RCC_CR_PLLRDY))
    ;                                       /* Waits until PLL is stable.   */
#endif

  /* Clock settings.*/
  if (clock_hse_ok) {
#if STM32_HAS_USB
    RCC->CFGR = STM32_MCOSEL | STM32_USBPRE | STM32_PLLMUL | STM32_PLLXTPRE |
                STM32_PLLSRC | STM32_ADCPRE | STM32_PPRE2  | STM32_PPRE1    |
                STM32_HPRE;
#else
    RCC->CFGR = STM32_MCOSEL |                STM32_PLLMUL | STM32_PLLXTPRE |
                STM32_PLLSRC | STM32_ADCPRE | STM32_PPRE2  | STM32_PPRE1    |
                STM32_HPRE;
#endif
  }
#if STM32_HSE_ENABLED
  else {
    /* 48 MHz HSI fallback: HCLK 48 MHz, PCLK1 24 MHz, PCLK2 48 MHz,
       ADCCLK 8 MHz, USBCLK = PLL/1 = 48 MHz.*/
#if STM32_HAS_USB
    RCC->CFGR = STM32_MCOSEL_NOCLOCK | STM32_USBPRE_DIV1    | STM32_FB_PLLMUL |
                STM32_PLLXTPRE_DIV1  | STM32_PLLSRC_HSI      | STM32_ADCPRE_DIV6 |
                STM32_PPRE2_DIV1     | STM32_PPRE1_DIV2      | STM32_HPRE_DIV1;
#else
    RCC->CFGR = STM32_MCOSEL_NOCLOCK                          | STM32_FB_PLLMUL |
                STM32_PLLXTPRE_DIV1  | STM32_PLLSRC_HSI      | STM32_ADCPRE_DIV6 |
                STM32_PPRE2_DIV1     | STM32_PPRE1_DIV2      | STM32_HPRE_DIV1;
#endif
  }
#endif

  /* Flash setup and final clock selection. This is still executed on the
     8 MHz HSI, before the switch to the PLL.*/
  if (clock_hse_ok) {
#if STM32_HCLK > 72000000
    /* 3 wait states + prefetch: APM32F103 errata 7.2 (72~96 MHz). */
    FLASH->ACR = 0x00000013U;
#else
    FLASH->ACR = STM32_FLASHBITS;
#endif
  }
#if STM32_HSE_ENABLED
  else {
    /* 1 wait state + prefetch buffer for 24 < HCLK <= 48 MHz. */
    FLASH->ACR = 0x00000011U;
  }
#endif

  /* Switching to the configured clock source if it is different from HSI.*/
#if (STM32_SW != STM32_SW_HSI)
  /* Switches clock source. The fallback tree uses the PLL as well, so the
     same value applies to both paths.*/
  RCC->CFGR |= STM32_SW;
  while ((RCC->CFGR & RCC_CFGR_SWS) != (STM32_SW << 2))
    ;                                       /* Waits selection complete.    */
#endif

  if (clock_hse_ok) {
#if !STM32_HSI_ENABLED
    RCC->CR &= ~RCC_CR_HSION;
#endif
  }
  /* In the HSI fallback the HSI must stay enabled: it is the PLL source. */
}

/*
 * SysTick retuning for the 48 MHz HSI fallback.
 * halInit() -> st_lld_init() programs the periodic SysTick using the
 * compile-time STM32_HCLK from mcuconf.h (96 MHz). Called from
 * CH_CFG_SYSTEM_INIT_HOOK in chconf.h, i.e. after halInit() and just before
 * chSysInit() enables interrupts, so reprogram LOAD for the actual HCLK if
 * the HSE failed. The single tick that may have accumulated is discarded.
 */
void clock_systick_adjust(void) {
  /* clock_hse_ok was set in __early_init (stm32_clock_init_user) but that
     runs *before* the C runtime zeroes .bss, so the variable may have been
     cleared back to false. Re-evaluate from the hardware instead. */
  bool hse_running = ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) &&
                     ((RCC->CFGR & RCC_CFGR_PLLSRC) != 0U); /* PLLSRC set = HSE, clear = HSI/2 */
  clock_hse_ok = hse_running;

  if (!hse_running) {
    SysTick->LOAD = (STM32_FB_HCLK / OSAL_ST_FREQUENCY) - 1U;
    SysTick->VAL  = 0U;
    (void)SysTick->CTRL;
    SCB->ICSR     = SCB_ICSR_PENDSTCLR_Msk;
  }
}

/*
 * Early initialization code.
 * This initialization must be performed just after stack setup and before
 * any other initialization.
 */
void __early_init(void) {

  stm32_clock_init_user();
}

/*
 * Board-specific initialization code.
 */
void boardInit(void) {
}
