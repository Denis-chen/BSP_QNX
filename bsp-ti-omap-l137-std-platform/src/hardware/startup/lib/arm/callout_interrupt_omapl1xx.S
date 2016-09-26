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

#include "callout.ah"
#include <arm/omapl1xx.h>

/*
 * -----------------------------------------------------------------------
 * Routines to patch callout code
 *
 * On entry:
 *	r0 - physical address of syspage
 *	r1 - virtual  address of syspage
 *	r2 - offset from start of syspage to start of the callout routine
 *	r3 - offset from start of syspage to read/write data used by callout
 * -----------------------------------------------------------------------
 */


interrupt_patch:
	stmdb	sp!,{r4,lr}
	add		r4, r0, r2					// address of callout routine

	/*
	 * Map interrupt controller registers
	 */
	mov		r0, #800	// size to map
	ldr		r1, [sp, #8]
	ldr		r1, [r1]
	bl		callout_io_map

	/*
	 * Patch the callout routine
	 */
	CALLOUT_PATCH	r4, r0, r1, r2, ip
	ldmia	sp!,{r4,pc}

/* -----------------------------------------------------------------------
 * Returns interrupt number in r4
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_id_omapl1xx, 0, interrupt_patch)
	/*
	 * Get the interrupt controller base address (patched)
	 */
	mov		ip,     #0x000000ff
	orr		ip, ip, #0x0000ff00
	orr		ip, ip, #0x00ff0000
	orr		ip, ip, #0xff000000

/* Read highest pending interupt vector */
	ldr		r4, [ip, #OMAPL1xx_INTR_GPIR]
	mov		r5,  #0x00
	orr		r5, r5, #0x80000000
/* Is m.s. bit of GPIR set ? == no interrupt */
	and		r1, r4, r5
	teq		r1, #0
/* Yes, spurious if bit is set */
	bne		1f

/* R4 now has correct vector # in it */

	/*
	 * Mask the interrupt / disable it
	 */

	str     r4, [ip, #OMAPL1xx_INTR_EICR]

	/* Clear status too, if you don't do it here and do it in the EOI,
	 * you may miss the new interrrupt which may get generated in the interrupt handler */
	str     r4, [ip, #OMAPL1xx_INTR_SICR]

	b 3f

1:
	mov		r4, #-1

3:
CALLOUT_END(interrupt_id_omapl1xx)


/*
 * -----------------------------------------------------------------------
 * Acknowledge specified interrupt
 *
 * On entry:
 *	r4 contains the interrupt number
 *	r7 contains the interrupt mask count
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_eoi_omapl1xx, 0, interrupt_patch)
	/*
	 * Get the interrupt controller base address (patched)
	 */

	mov		ip,     #0x000000ff
	orr		ip, ip, #0x0000ff00
	orr		ip, ip, #0x00ff0000
	orr		ip, ip, #0xff000000
	/* It is a pulse interrupt so no need to read the SICR again.
	 * Otherwise we will clear the next pending interrupt
	 */ 

	/*
	* Only unmask interrupt if mask count is zero
	*/

	teq     r7, #0
	bne     3f

	/* enable interrupt */
	str     r4, [ip, #OMAPL1xx_INTR_EISR]
3:
CALLOUT_END(interrupt_eoi_omapl1xx)


/*
 * -----------------------------------------------------------------------
 * Mask specified interrupt
 *
 * On entry:
 *	r0 - syspage_ptr
 *	r1 - interrupt number
 *
 * Returns:
 *	r0 - error status
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_mask_omapl1xx, 0, interrupt_patch)
	/*
	 * Get the interrupt controller base address (patched)
	 */
	mov		ip,     #0x000000ff
	orr		ip, ip, #0x0000ff00
	orr		ip, ip, #0x00ff0000
	orr		ip, ip, #0xff000000

	/* disable interrupt */
	/* Clear enable, clear status */

	str     r1, [ip, #OMAPL1xx_INTR_EICR]
	str     r1, [ip, #OMAPL1xx_INTR_SICR]

	mov     r0, #0
	mov     pc, lr
CALLOUT_END(interrupt_mask_omapl1xx)


/*
 * -----------------------------------------------------------------------
 * Unmask specified interrupt
 *
 * On entry:
 *	r0 - syspage_ptr
 *	r1 - interrupt number
 *
 * Returns:
 *	r0 - error status
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_unmask_omapl1xx, 0, interrupt_patch)
	/*
	 * Get the interrupt controller base address (patched)
	 */
	mov		ip,     #0x000000ff
	orr		ip, ip, #0x0000ff00
	orr		ip, ip, #0x00ff0000
	orr		ip, ip, #0xff000000

	/* enable interrupt */
	str     r1, [ip, #OMAPL1xx_INTR_EISR]

	mov		r0, #0
	mov		pc, lr
CALLOUT_END(interrupt_unmask_omapl1xx)

