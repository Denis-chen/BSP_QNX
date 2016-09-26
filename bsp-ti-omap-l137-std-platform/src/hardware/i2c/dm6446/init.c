/*
 * $QNXLicenseC:
 * Copyright 2005, 2007-2009, QNX Software Systems.
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

void *
dm6446_init(int argc, char *argv[])
{
	dm6446_dev_t	*dev;
	unsigned		rev, psc;
	uintptr_t		base;

	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
		perror("ThreadCtl");
		return NULL;
	}

	dev = malloc(sizeof(dm6446_dev_t));
	if (!dev)
		return NULL;

	if (-1 == dm6446_options(dev, argc, argv))	
		goto fail;
	
	base = mmap_device_io(dev->reglen, dev->physbase);
	if (base == (uintptr_t)MAP_FAILED) {
		perror("mmap_device_io");
		goto fail;
	}

	/* Check I2C_REV */
	rev = in32(base + DM6446_I2C_ICPID1);
	dev->rev.major = DM6446_I2C_REVMAJOR(rev);
	dev->rev.minor = DM6446_I2C_REVMINOR(rev);
	dev->revid = rev;
	
	/* Reset module */
	out32(base + DM6446_I2C_ICMDR, 0);

	/* Set clock prescaler */
	psc = DM6446_PSC_VALUE; 
	out32(base + DM6446_I2C_ICPSC, psc);	
	
	/*If it is OMAP-L1x, then disable GPIO mode */
	if (dev->revid == OMAPL1xx_I2C_REVID1) 
		out32(base + OMAPL1xx_I2C_ICPFUNC, 0); 
	
	dev->regbase = base;
	if (dm6446_attach_intr(dev))
		goto fail;

	/* Set Own Address */
	out32(base + DM6446_I2C_ICOAR, dev->own_addr);

	/* Take module out of reset */
	out32(base + DM6446_I2C_ICMDR, DM6446_I2C_ICMDR_IRS | DM6446_I2C_ICMDR_MST); 
	
    return dev;

fail:	
    free(dev);
    return NULL;
}
