/* Copyright 2023 YANG
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#define CH_CFG_ST_TIMEDELTA 0

#define CH_CFG_USE_CONDVARS_TIMEOUT FALSE

#include_next <chconf.h>

/*
 * The clock tree is selected at run time in stm32_clock_init_user():
 * HSE 96 MHz when the external crystal starts, HSI 48 MHz fallback when it
 * does not. st_lld_init() (from halInit()) programs the periodic SysTick
 * with the compile-time 96 MHz HCLK. Retune it here if the HSE failed; this
 * hook runs inside chSysInit(), after halInit() and before interrupts are
 * globally enabled. The implementation lives in the board override.
 */
#undef CH_CFG_SYSTEM_INIT_HOOK
#define CH_CFG_SYSTEM_INIT_HOOK()                  \
    do {                                           \
        void clock_systick_adjust(void);           \
        clock_systick_adjust();                     \
    } while (0)
