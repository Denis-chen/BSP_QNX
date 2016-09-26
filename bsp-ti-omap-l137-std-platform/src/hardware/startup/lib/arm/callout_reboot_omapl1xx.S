#
# Copyright 2010, QNX Software Systems.
#
# Licensed under the Apache License, Version 2.0 (the "License"). You
# may not reproduce, modify or distribute this software except in
# compliance with the License. You may obtain a copy of the License
# at: http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTIES OF ANY KIND, either express or implied.
#
# This file may contain contributions from others, either as
# contributors under the License or as licensors under other terms.
# Please review this entire file for other proprietary rights or license
# notices, as well as the QNX Development Suite License Guide at
# http://licensing.qnx.com/license-guide/ for other information.
#
/*
 * TI OMAP L1xx system reboot
 * Use watchdog timer to reset the board
 */

#include "callout.ah"
#include <arm/omapl1xx.h>

/*
 * -----------------------------------------------------------------------
 * Routine to patch callout code
 *
 * On entry:
 *	r0 - physical address of syspage
 *	r1 - virtual  address of syspage
 *	r2 - offset from start of syspage to start of the callout routine
 *	r3 - offset from start of syspage to read/write data used by callout
 * -----------------------------------------------------------------------
 */
patch_reboot:
	stmdb	sp!,{r4,lr}
	add		r4, r0, r2					// address of callout routine

	/*
	 * Map timer registers
	 */
	mov		r0, #128			// size to map
	ldr		r1, Lreset
	bl		callout_io_map
	CALLOUT_PATCH	r4, r0, r1, r2, ip

	ldmia	sp!,{r4,pc}

// The address to patch in
Lreset:	.word	OMAPL1xx_TMR_2_REGS

CALLOUT_START(reboot_omapl1xx, 0, patch_reboot)
	mov		ip,     #0x000000ff		// timer register base (patched)
	orr		ip, ip, #0x0000ff00
	orr		ip, ip, #0x00ff0000
	orr		ip, ip, #0xff000000

    /*
     * Get the base address of Clock, Reset, and Power Mode Control registers (patched)
     */

    // setup the timers to watchdog mode, set it going, then wait...

	mov	r1, $0x08
	str	r1, [ip, #OMAPL1xx_TMR_TGCR]
	ldr	r1, [ip, #OMAPL1xx_TMR_TGCR]
	orr	r1, r1, $0x03
	str	r1, [ip, #OMAPL1xx_TMR_TGCR]
	mov	r1, $0
	str	r1, [ip, #OMAPL1xx_TMR_TIM12]
	str	r1, [ip, #OMAPL1xx_TMR_TIM34]
	str	r1, [ip, #OMAPL1xx_TMR_PRD12]
	ldr	r0, WDT_PRD34
	str	r1, [ip, #OMAPL1xx_TMR_PRD34]
	ldr	r1, [ip, #OMAPL1xx_TMR_TCR]
	orr	r1, r1, $0x40
	str	r1, [ip, #OMAPL1xx_TMR_WDTCR]
	ldr	r1, [ip, #OMAPL1xx_TMR_WDTCR]
	orr	r1, r1, $0x4000
	str	r1, [ip, #OMAPL1xx_TMR_WDTCR]
	ldr	r1, WDTCR_VAL1
	str	r1, [ip, #OMAPL1xx_TMR_WDTCR]
	ldr	r1, WDTCR_VAL2
	str	r1, [ip, #OMAPL1xx_TMR_WDTCR]
	nop
	nop
	nop
	nop
reset_cpu_loop:
	b	reset_cpu_loop

WDT_TGCR:
	.word	0x01c21c24
WDT_TIM12:
	.word	0x01c21c10
WDT_TIM34:
	.word	0x01c21c14
WDT_PRD12:
	.word	0x01c21c18
WDT_PRD34:
	.word	0x01c21c1c
WDT_TCR:
	.word	0x01c21c20
WDT_WDTCR:
	.word	0x01c21c28
WDTCR_VAL1:
	.word	0xa5c64000
WDTCR_VAL2:
	.word	0xda7e4000
0:	b	0b

CALLOUT_END(reboot_omapl1xx)
