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
 * TI OMAPL137 interrupt controller support.
 */

#include "startup.h"
#include <arm/omapl1xx.h>

extern struct callout_rtn interrupt_id_omapl1xx;
extern struct callout_rtn interrupt_eoi_omapl1xx;
extern struct callout_rtn interrupt_mask_omapl1xx;
extern struct callout_rtn interrupt_unmask_omapl1xx;

extern struct callout_rtn interrupt_id_dma_ints;
extern struct callout_rtn interrupt_eoi_dma_ints;
extern struct callout_rtn interrupt_mask_dma_ints;
extern struct callout_rtn interrupt_unmask_dma_ints;

extern struct callout_rtn interrupt_id_omapl1xx_gpio;
extern struct callout_rtn interrupt_eoi_omapl1xx_gpio;
extern struct callout_rtn interrupt_mask_omapl1xx_gpio;
extern struct callout_rtn interrupt_unmask_omapl1xx_gpio;

static paddr_t irq_base = OMAPL1xx_INTR_BASE;

static paddr_t dma0_intr_base_addr = (OMAPL1xx_EDMA0_CC_BASE + OMAPL1xx_EDMA_GLOBAL + OMAPL1xx_EDMA_IER);

static unsigned omapl1xx_gpio32_intr[6] =
		{
			0x01E26038,
			(1 << 30),	/* GPIO3_14 Rising edge detect enable */
			(0 << 30),  /* GPIO3_14 Falling edge detect enable */
			0,          /* shift */
			0xFFFF0000,  /* Interrupt Status Register(INSTAT23) */
			0           /*disable interrupt*/
		};
const static struct startup_intrinfo intrs[] = {
	{	_NTO_INTR_CLASS_EXTERNAL,	/* vector base */
		91,			/* number of vectors */
		_NTO_INTR_SPARE,	/* cascade vector */
		0,			/* CPU vector base */
		0,			/* CPU vector stride */
		0,			/* flags */
		{INTR_GENFLAG_LOAD_SYSPAGE, 0, &interrupt_id_omapl1xx},
		{INTR_GENFLAG_LOAD_SYSPAGE | INTR_GENFLAG_LOAD_INTRMASK, 0, &interrupt_eoi_omapl1xx},
		&interrupt_mask_omapl1xx,	/* mask   callout */
		&interrupt_unmask_omapl1xx,	/* unmask callout */
		0,			/* config callout */
		&irq_base
	},
	{	0xC100,			/* vector base */
		32,			/* number of vectors */
		0xB,			/* cascade vector */
		0,			/* CPU vector base */
		0,			/* CPU vector stride */
		0,			/* flags */
		{INTR_GENFLAG_LOAD_SYSPAGE, 0, &interrupt_id_dma_ints},
		{INTR_GENFLAG_LOAD_SYSPAGE | INTR_GENFLAG_LOAD_INTRMASK, 0, &interrupt_eoi_dma_ints},
		&interrupt_mask_dma_ints,	/* mask   callout */
		&interrupt_unmask_dma_ints,	/* unmask callout */
		0,			/* config callout */
		&dma0_intr_base_addr
	},
	{	0xC120,				/* vector base */
		16,			/* number of vectors */
		45,			/* cascade vector */            /*interrupt # for GPIO bank 3 of omapl137*/
		0,			/* CPU vector base */
		0,			/* CPU vector stride */
		0,			/* flags */
		{INTR_GENFLAG_LOAD_SYSPAGE, 0, &interrupt_id_omapl1xx_gpio},
		{INTR_GENFLAG_LOAD_SYSPAGE | INTR_GENFLAG_LOAD_INTRMASK, 0, &interrupt_eoi_omapl1xx_gpio},
		&interrupt_mask_omapl1xx_gpio,	/* mask   callout */
		&interrupt_unmask_omapl1xx_gpio,	/* unmask callout */
		0,			/* config callout */
		omapl1xx_gpio32_intr
	},
};

void
init_intrinfo()
{
	int     i = 0;

	/* Host interrupt disable */
	out32(irq_base + OMAPL1xx_INTR_HIER, 0x0);

	/* Ensure all interrupts are masked initially */
	out32(irq_base + OMAPL1xx_INTR_GER, 0); // disable all interrupts

	// disable all GPIO interrupts
	out32 (OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_BINTEN, 0);

	/* Disable every interrupt  */
	out32(irq_base + OMAPL1xx_INTR_ECR1, 0xffffffff); // disable interrupts 0..31
	out32(irq_base + OMAPL1xx_INTR_ECR2, 0xffffffff); // disable interrupts 32..63
	out32(irq_base + OMAPL1xx_INTR_ECR3, 0x07ffffff); // disable interrupts 64..90

	out32(irq_base + OMAPL1xx_INTR_CR, 0); // set mode - no nesting for now

	out32(irq_base + OMAPL1xx_INTR_SECR1, 0xffffffff); // clear status regs
	out32(irq_base + OMAPL1xx_INTR_SECR2, 0xffffffff); // clear status regs
	out32(irq_base + OMAPL1xx_INTR_SECR3, 0x07ffffff); // clear status regs

	/* Set priorities so all interrupts are IRQ not FIQ */
	out32(irq_base + OMAPL1xx_INTR_HIER, 0x2); // enable IRQ, disable FIQ

	out32(irq_base + OMAPL1xx_INTR_HIEISR, 0x1); // enable IRQ, disable FIQ

	// Map all the 91 interrupts to channel 4 first

	for (i=0; i < 91; i+=4)
		out32(irq_base + OMAPL1xx_INTR_CMR0 + i,  0x04040404);

	// Remap the GPIO bank 3 interrupt to higher channel for highest priority of 1553

    out32(irq_base + OMAPL1xx_INTR_CMR0 + 4 * 11, 0x04040304 );

	/* Configure GPIO interrupts. GPIO3_Pin14 configured as input and rising edge trigger*/
	out32((OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_DIR23), in32(OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_DIR23) | (1 << 30));

	/* Enable rising edge interrupt */
	out32((OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_SET_RIS_TRIG23), in32(OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_SET_RIS_TRIG23) | (1 << 30));

	/* Disable falling edge interrupt */
	out32((OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_CLR_FAL_TRIG23), in32(OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_CLR_FAL_TRIG23) | (1 << 30));

	/* GPIO Bank 6 interrupt enable */
	out32 ((OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_BINTEN), in32(OMAPL1xx_GPIO_BASE + OMAPL1xx_GPIO_BINTEN) | (1 << 3));

	add_interrupt_array(intrs, sizeof(intrs));
	out32(irq_base + OMAPL1xx_INTR_GER, 1); // enable global interrupt
}

