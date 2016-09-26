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

/*
 * TI OMAP specific GPIO interrupt callouts.
 *
 * interrupt_id_* and interrupt_eoi_* are copied and intermixed with other
 * kernel code during initialisation.
 *
 * They do not follow normal calling conventions, and must fall through
 * to the end, rather than attempting to perform a return instruction.
 *
 * The INTR_GENFLAG_* bits in the intrinfo_entry defines which of the
 * following values can be loaded on entry to these code fragments:
 *
 *	r5 - holds the syspageptr				(INTR_GENFLAG_SYSPAGE  set)
 *	r6 - holds the intrinfo_entry pointer	(INTR_GENFLAG_INTRINFO set)
 *	r7 - holds the interrupt mask count		(INTR_GENFLAG_INTRMASK set)
 *
 * The interrupt_id_* routine returns the (controller-relative) level in r4
 */

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

rw_gpio_intr:    .word   20
#define OFF_BASE			0x00	// GPIO register address
#define OFF_RISING_EDGE		0x04	// Rising edge triggered
#define OFF_FALLING_EDGE	0x08	// Falling edge triggered
#define OFF_SHIFT			0x0C	// Shift
#define OFF_MASK			0x10	// Mask
#define OFF_DISABLE			0x14	// Interrupt disable flag


interrupt_gpio_patch_id:
	stmdb	sp!, {r4, lr}
	add		r2, r0, r2          // address of callout routine
	add		r4, r0, r3          // address of rw data

	/*
	 * Patch the callout with the rw offset
	 */
	and		ip, r3, #0xff
	ldr		r1, [r2]
	bic		r1, r1, #0xff
	orr		r1, r1, ip
	str		r1, [r2], #4
	mov		ip, r3, lsr #8
	and		ip, ip, #0xff
	ldr		r1, [r2]
	bic		r1, r1, #0xff
	orr		r1, r1, ip
	str		r1, [r2]

	ldr		r2, [sp, #8]

	/*
	 * Map GPIO_BASE and store in RW data
	 */
	mov		r0, #0x100
	ldr		r1, [r2]		/* Offset of register */
	and		r1, r1, #0xffffff00
	bl		callout_io_map

	ldr		r2, [sp, #8]
	ldr		r1, [r2]
	and		r1, r1, #0xff
	orr		r0, r0, r1
	str		r0, [r4, #OFF_BASE]

	ldr		r1, [r2, #0x04]	/* Rising edge */
	str		r1, [r4, #OFF_RISING_EDGE]

	ldr		r1, [r2, #0x08]	/* Falling edge */
	str		r1, [r4, #OFF_FALLING_EDGE]

	ldr		r1, [r2, #0x0C]	/* Shift */
	str		r1, [r4, #OFF_SHIFT]

	ldr		r1, [r2, #0x10]	/* Mask */
	str		r1, [r4, #OFF_MASK]

	ldr		r1, [r2, #0x14]	/* Disable */
	str		r1, [r4, #OFF_DISABLE]

	ldmia	sp!, {r4, pc}


/*
 * Patch other callouts
  */
patch_other:
	stmdb   sp!, {r4, lr}
	add     r4, r0, r2          // address of callout routine

	/*
	 * Patch callout with rw offset
	 */
	and		ip, r3, #0xff
	ldr		r1, [r4]
	bic		r1, r1, #0xff
	orr		r1, r1, ip
	str		r1, [r4], #4
	mov		ip, r3, lsr #8
	ldr		r1, [r4]
	bic		r1, r1, #0xff
	orr		r1, r1, ip
	str		r1, [r4]

	ldmia	sp!, {r4, pc}

/* -----------------------------------------------------------------------
 * Returns interrupt number in r4
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_id_omapl1xx_gpio, rw_gpio_intr, interrupt_gpio_patch_id)
	mov		ip,     #0x000000ff     // RW offset (patched)
	orr		ip, ip, #0x0000ff00
	add		ip, r5, ip              // INTR_GENFLAG_LOAD_SYSPAGE specified

	ldr		r0, [ip, #OFF_BASE]		// Get GPIO base address

	ldr		r1, [r0, #0x24]			// Get interrupt status
	ldr		r2, [ip, #OFF_MASK]

	ands	r1, r2, r1
	moveq	r4, #-1
	beq		1f

	ldr		r3, [ip, #OFF_SHIFT]
	mov		r2, #1
	mov		r2, r2, lsl r3

	mov		r4, #16
0:	subs	r4, r4, #1
	tst		r1, r2, lsl r4
	beq		0b

	mov		r2, r2, lsl r4

	/*
     * Do we want to disable the interrupt ?
     */
	ldr		r1, [ip, #OFF_DISABLE]
	tst		r1, r2
	bne		0f

	/*
     * Disable the interrupt
     */

	ldr		r1, [ip, #OFF_RISING_EDGE]
	ands	r1, r1, r2
	strne	r1, [r0, #0x18]

	ldr		r1, [ip, #OFF_FALLING_EDGE]
	ands	r1, r1, r2
	strne	r1, [r0, #0x20]

0:
	/*
	 * clear the interrupt
	 */
	str 	r2, [r0, #0x24]
1:
CALLOUT_END(interrupt_id_omapl1xx_gpio)


/*
 * -----------------------------------------------------------------------
 * Acknowledge specified interrupt
 *
 * On entry:
 *	r4 contains the interrupt number
 *	r7 contains the interrupt mask count
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_eoi_omapl1xx_gpio, rw_gpio_intr, patch_other)
	mov     ip,     #0x000000ff     // RW data offset (patched)
	orr     ip, ip, #0x0000ff00
	add     ip, r5, ip              // INTR_GENFLAG_LOAD_SYSPAGE specified

    /*
     * Only enable interrupt if mask count is zero
     */
    teq     r7, #0
    bne     1f

	/*
     * Enable the interrupt
     */
	ldr		r3, [ip, #OFF_SHIFT]
	mov		r2, #1
	add		r3, r3, r4
	mov		r2, r2, lsl r3

	/*
     * re-enable the interrupt
     */
	ldr		r0, [ip, #OFF_BASE]		// Get GPIO base address

	ldr		r1, [ip, #OFF_RISING_EDGE]
	ands	r1, r1, r2
	strne	r1, [r0, #0x14]

	ldr		r1, [ip, #OFF_FALLING_EDGE]
	ands	r1, r1, r2
	strne	r1, [r0, #0x1C]
1:
CALLOUT_END(interrupt_eoi_omapl1xx_gpio)


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
CALLOUT_START(interrupt_mask_omapl1xx_gpio, rw_gpio_intr, patch_other)
	mov     ip,     #0x000000ff     // RW data offset (patched)
	orr     ip, ip, #0x0000ff00
	add     ip, r0, ip              // INTR_GENFLAG_LOAD_SYSPAGE specified

	/*
     * Do we want to disable the interrupt ?
     */
	ldr		r3, [ip, #OFF_SHIFT]
	mov		r2, #1
	add		r3, r3, r1
	mov		r2, r2, lsl r3

	ldr		r3, [ip, #OFF_DISABLE]
	tst		r3, r2
	bne		1f

	/*
     * Disable the interrupt
     */
	ldr		r0, [ip, #OFF_BASE]		// Get GPIO base address

	ldr		r3, [ip, #OFF_RISING_EDGE]
	ands	r3, r3, r2
	strne	r3, [r0, #0x18]

	ldr		r3, [ip, #OFF_FALLING_EDGE]
	ands	r3, r3, r2
	strne	r3, [r0, #0x20]

1:
    mov     r0, #0
    mov     pc, lr
CALLOUT_END(interrupt_mask_omapl1xx_gpio)


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
CALLOUT_START(interrupt_unmask_omapl1xx_gpio, rw_gpio_intr, patch_other)
	mov     ip,     #0x000000ff     // RW data offset (patched)
	orr     ip, ip, #0x0000ff00
	add     ip, r0, ip              // INTR_GENFLAG_LOAD_SYSPAGE specified

	ldr		r0, [ip, #OFF_BASE]		// Get GPIO base address

	/*
     * Enable the interrupt
     */
	ldr		r3, [ip, #OFF_SHIFT]
	mov		r2, #1
	add		r3, r3, r1
	mov		r2, r2, lsl r3

	ldr		r3, [ip, #OFF_RISING_EDGE]
	ands	r3, r3, r2
	strne	r3, [r0, #0x14]

	ldr		r3, [ip, #OFF_FALLING_EDGE]
	ands	r3, r3, r2
	strne	r3, [r0, #0x1C]

	mov		r0, #0
	mov		pc, lr
CALLOUT_END(interrupt_unmask_omapl1xx_gpio)

