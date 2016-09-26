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

/*
 * Choose 12Mhz as the targeted I2C Module clock frequency after the prescaler.
 */
#define OMAPL137_I2C_MODULE_CLOCK	    12000000UL

int
dm6446_set_bus_speed(void *hdl, unsigned int speed, unsigned int *ospeed)
{
	dm6446_dev_t	*dev = hdl;
	unsigned	iclk, psc, scll, div = 0;
	
	if (speed > 850000) {
		errno = EINVAL;
		return -1;
	}else if ((speed > 400000) && (dev->revid != OMAPL1xx_I2C_REVID1)) {
		errno = EINVAL;
		return -1;
	}		

	/*
	 * Save some cycles if the speed is the same
	 */
	if (speed == dev->expected_speed) {
		if (ospeed)
			*ospeed = dev->bus_speed;
		return 0;
	}
		
	/*
	 * Don't change speed if stop condition is not generated
	 */
	if (!dev->stop) {
		errno = EBUSY;
		return -1;
	}

	/*
	 * Disable/Reset I2C
	 */
	out32(dev->regbase + DM6446_I2C_ICMDR, 0);
	
	/* Find clock prescaler */
	psc = in32(dev->regbase + DM6446_I2C_ICPSC);
		
	if (dev->revid == OMAPL1xx_I2C_REVID1) {
		/*Calculate PSC value and Clock divider value in order to set i2c prescaled Module freq as 12MHz*/
		psc = ((dev->input_clk + (OMAPL137_I2C_MODULE_CLOCK - 1)) / OMAPL137_I2C_MODULE_CLOCK) - 1;
		out32(dev->regbase + DM6446_I2C_ICPSC, psc);  
				
		if (psc == 0)
			div = 7;
		else if (psc == 1)
			div = 6;
		else
			div = 5;

		/* Find the module clock freq */
		iclk = dev->input_clk / (psc + 1);  
      
		/* Find the clock divider */
		scll = (((iclk / speed) - (2 * div)) / 2); 
	}
	else {
		iclk = DM6446_I2C_INPUT_CLOCK / (psc + 1);

		scll = ((((iclk / speed) / 3) - 10) / 2);
	}
	
	out32(dev->regbase + DM6446_I2C_ICCLKL, scll);
	out32(dev->regbase + DM6446_I2C_ICCLKH, scll);

	/*
	 * Take I2C out of reset
	 */
	out32(dev->regbase + DM6446_I2C_ICMDR, DM6446_I2C_ICMDR_IRS | DM6446_I2C_ICMDR_MST);
	
	if (dev->revid == OMAPL1xx_I2C_REVID1) 
		dev->bus_speed = iclk / (2 * (scll + div));
	else
		dev->bus_speed = iclk / (3 * ((2 * scll) + 10));
			
	if (ospeed)
		*ospeed = dev->bus_speed;

	return 0;
}
