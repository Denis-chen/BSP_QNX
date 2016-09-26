/*
 * $QNXLicenseC:
 * Copyright 2005, 2007-2008, QNX Software Systems.
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




#include "proto.h"


int
dm6446_wait_bus_not_busy(dm6446_dev_t *dev)
{
	uintptr_t	base = dev->regbase;

	if (dev->stop ) {
		if (in32(base + DM6446_I2C_ICSTR) & DM6446_I2C_ICSTR_BB) {
			/*
			 * Reset I2C module if bus is busy (something is wrong)
			 */
			out32(base + DM6446_I2C_ICMDR, 0);
			nanospin_ns(1024);
			if (in32(base + DM6446_I2C_ICSTR) & DM6446_I2C_ICSTR_BB)
				return -1;
		}
	}

	return 0;
}


static i2c_status_t
dm6446_check_error(dm6446_dev_t *dev, uint32_t status)
{
	uintptr_t	base = dev->regbase;

	if (status == 0)
		return I2C_STATUS_ERROR;

	if (status & ( DM6446_I2C_ICSTR_AAS | DM6446_I2C_ICSTR_AD0)) {
		out32(base + DM6446_I2C_ICSTR, status);
		fprintf(stderr, "aad0 / aas\n");
		return I2C_STATUS_ERROR;
	}
 
	if (status & DM6446_I2C_ICSTR_AL) {
		out32(base + DM6446_I2C_ICSTR, status);
		fprintf(stderr, "al\n");
		return I2C_STATUS_ARBL;
	}

	if (status & DM6446_I2C_ICSTR_NACK) {
		out32(base + DM6446_I2C_ICSTR, status);
		out32(base + DM6446_I2C_ICMDR, in32(base + DM6446_I2C_ICMDR) | DM6446_I2C_ICMDR_STP);
		fprintf(stderr, "nack\n");
		return I2C_STATUS_NACK;
	}

	return I2C_STATUS_DONE;
}

uint32_t
dm6446_wait_complete(dm6446_dev_t *dev)
{
	i2c_status_t	sts = I2C_STATUS_ERROR;
	uint64_t		to = dev->tot_len;
	
	to *= 256 * 1024 * 1024;

	TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_INTR, NULL, &to, NULL);
	if (InterruptWait(0, NULL) == -1) {
		perror("InterruptWait");
		dev->stop = 1;
		return 0;
	}

	if (dev->cur_len == dev->tot_len)
		sts = dm6446_check_error(dev, 
					in32(dev->regbase + DM6446_I2C_ICSTR));

	if (sts != I2C_STATUS_DONE)
		dev->stop = 1;

	return sts;
}

static const struct sigevent *dm6446_intr(void *area, int id)
{
	dm6446_dev_t	*dev = (dm6446_dev_t *)area;
	uintptr_t		base = dev->regbase;
	unsigned		ivr;

	/*
	 * Unexpected interrupt, mask all interrupts
	 */
	if (dev->txrx == JACINTO_I2C_IDLE) {
		out32(base + DM6446_I2C_ICIMR, 0);
		return NULL;
	}

	/*
	 * Get interrupt vector 
	 */
	ivr = in32(base + DM6446_I2C_ICIVR);

	if (ivr == DM6446_I2C_ICIVR_ICRRDY) {
		if (dev->cur_len < dev->tot_len)
			dev->buf[dev->cur_len++] = in32(base + DM6446_I2C_ICDRR);
	} else if (ivr == DM6446_I2C_ICIVR_ICXRDY) {
		if (dev->cur_len < dev->tot_len)
			out32(base + DM6446_I2C_ICDXR, dev->buf[dev->cur_len++]);
	} else if (ivr != 0) {
		/*
		 * Complete, DM6446_I2C_ICIVR_ARDY, DM6446_I2C_ICIVR_SCD
		 * Errors,   DM6446_I2C_ICIVR_AL, DM6446_I2C_ICIVR_NACK, DM6446_I2C_ICIVR_AAS
		 * Status will be checked later.
		 */
		out32(base + DM6446_I2C_ICIMR, 0);
		dev->txrx = JACINTO_I2C_IDLE;

		return (&dev->intrevent);
	}

	return NULL;
}

int	dm6446_attach_intr(dm6446_dev_t *dev)
{
	/* Mask all interrupts */
	out32(dev->regbase + DM6446_I2C_ICIMR, 0);

	/* Initialize interrupt handler */
	SIGEV_INTR_INIT(&dev->intrevent);

	dev->iid = InterruptAttach(dev->intr, dm6446_intr, dev, 0, _NTO_INTR_FLAGS_TRK_MSK);
	if (dev->iid == -1) {
		perror("InterruptAttach");
		return -1;
	}

	return 0;
}
