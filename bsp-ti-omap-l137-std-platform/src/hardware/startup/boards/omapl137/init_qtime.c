/*
 * $QNXLicenseC:
 * Copyright 2010, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/*
 * TI OMAP L137 specific timer support
 */

#include "startup.h"
#include <arm/omapl1xx.h>
#include <hw/hwinfo_omapl1xx.h>
#include <drvr/hwinfo.h>

#define	OMAPL137_CLOCK_SCALE	-15 /* timer_rate in nanoseconds */

static uintptr_t timer_base;

extern struct callout_rtn timer_load_omapl1xx;
extern struct callout_rtn timer_value_omapl1xx;
extern struct callout_rtn timer_reload_omapl1xx;

static const struct callout_slot timer_callouts[] = {
	{CALLOUT_SLOT(timer_load, _omapl1xx)},
	{CALLOUT_SLOT(timer_value, _omapl1xx)},
	{CALLOUT_SLOT(timer_reload, _omapl1xx)},
};

static unsigned
timer_start_omapl1xx()
{
	return in32(timer_base + OMAPL1xx_TMR_TIM12);
}

static unsigned
timer_diff_omapl1xx(unsigned start)
{
	unsigned now = in32(timer_base + OMAPL1xx_TMR_TIM12);

	return (start - now);
}

void
init_qtime()
{
	unsigned timer_index, clock_index, ivec_index;

	// First clock and First interrupt of the Timer 1 register ( Interrupt is T64P0_TINT12)
	timer_index = clock_index = ivec_index = 0;

	struct qtime_entry *qtime = alloc_qtime();

	// Map the timer 1 registers
	
	timer_base = startup_io_map(OMAPL1xx_TIMER_SIZE, OMAPL1xx_TMR_1_REGS);

	/*
	 * Setup GPIO ctrl register to ensure timer is in correct mode
	 * Set all timer pins to timer mode
	 */

	out32 (timer_base + OMAPL1xx_TMR_GPINTGPEN, 0);

	// clear the interrupt status register
	
	out32 (timer_base + OMAPL1xx_TMR_INTCTLSTAT, OMAPL1xx_PRDINTSTAT12);

	/*
	 * write an initial value to the timer - the kernel will later calculate the proper
	 * load value for a 1ms tick, and load it via the timer_load callout.
	 */

	out32 (timer_base + OMAPL1xx_TMR_PRD12, 0xffffffff);

	/*
	 * set up timer 0 control register / global control register,
	 * but don't start the timer (timer_load will do that).
	 */

	out32 (timer_base + OMAPL1xx_TMR_TGCR, OMAPL1xx_TGCR_TIMMODE | OMAPL1xx_TGCR_TIM12RS); // Select dual 32bit Unchained mode and bring Timer1:2 out of reset
	out32 (timer_base + OMAPL1xx_TMR_TCR, 0); // Do not enable the timer here
	out32 (timer_base + OMAPL1xx_TMR_TIM12, 0);

	timer_start = timer_start_omapl1xx;
	timer_diff = timer_diff_omapl1xx;

	qtime->intr = hwitag_find_ivec(hwi_find_device(OMAPL1xx_HWI_TIMER, 0), &ivec_index);

	qtime->timer_scale = OMAPL137_CLOCK_SCALE; /* timer_rate in nanoseconds */
	qtime->cycles_per_sec = (uint64_t) hwitag_find_clkfreq(hwi_find_device(OMAPL1xx_HWI_TIMER, timer_index), &clock_index); // AuxClk
	qtime->timer_rate = ((uint64_t)1000000000000000ULL)/((uint64_t)qtime->cycles_per_sec); /* (10 ^ -(qtime->timer_scale))/qtime->cycles_per_sec */

	if(debug_flag > 1){
		kprintf("Qtime->Interrupt: %d\n",		qtime->intr);
		kprintf("Qtime->Timer_scale: -%d\n",	-(qtime->timer_scale));
		kprintf("Qtime->Cycles_Per_Sec: %d\n",	qtime->cycles_per_sec);
		kprintf("Qtime->Timer_Rate: %d\n",		qtime->timer_rate);
	}

	add_callout_array(timer_callouts, sizeof(timer_callouts));

	startup_io_unmap(timer_base);
}
