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

i2c_status_t
dm6446_recv(void *hdl, void *buf, unsigned int len, unsigned int stop)
{
	dm6446_dev_t	*dev = hdl;
	uintptr_t		base = dev->regbase;
	unsigned stat;

	if (len <= 0) 
		return I2C_STATUS_DONE;

	if (-1 == dm6446_wait_bus_not_busy(dev))
		return I2C_STATUS_BUSY;
	
	/* Clear any pending interrupts by reading the IVR */
	if (dev->revid == OMAPL1xx_I2C_REVID1)
		stat = in32(base + DM6446_I2C_ICIVR);

	/* Enable I2C master mode */
	out32(base + DM6446_I2C_ICMDR, DM6446_I2C_ICMDR_IRS | DM6446_I2C_ICMDR_MST);

	/* set slave address */
	out32(base + DM6446_I2C_ICSAR, dev->slave_addr);

	/* set data count */
	out32(base + DM6446_I2C_ICCNT, len);

        /* ** start condition previously set here ** */

	dev->tot_len = len;
	dev->cur_len = 0;
	dev->buf     = buf;
	dev->txrx    = JACINTO_I2C_RX;
	dev->stop    = stop;

	/* Enable interrupts */
	out32(base + DM6446_I2C_ICIMR,
			DM6446_I2C_ICIMR_AAS | DM6446_I2C_ICIMR_ICRRDY |
			(stop? DM6446_I2C_ICIMR_SCD: DM6446_I2C_ICIMR_ARDY));

	/* set start condition */
	out32(base + DM6446_I2C_ICMDR, 
			DM6446_I2C_ICMDR_IRS |
			DM6446_I2C_ICMDR_MST |
			DM6446_I2C_ICMDR_STT |
			(stop? DM6446_I2C_ICMDR_STP : 0) |
			((dev->addr_fmt != I2C_ADDRFMT_7BIT) ? DM6446_I2C_ICMDR_XA : 0));

	return dm6446_wait_complete(dev);
}


