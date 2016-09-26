/*
 * $QNXLicenseC: 
 * Copyright 2007, QNX Software Systems.  
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





#include "dm644xspi.h"



int dm644x_cfg(dm644x_spi_t *dm644x, int device, spi_cfg_t *cfg)
{
	int32_t		fmt, prescale;

	fmt = cfg->mode & SPI_MODE_CHAR_LEN_MASK;

	/*
	 * Only support data length from 2 to 16
	 */
	if (fmt > 16 || fmt < 2)
		return -1;

	prescale = (dm644x->clk / cfg->clock_rate) - 1;

	/*
	 * Check if the bit rate is too high or too low
	 */
	if (prescale > 255 || prescale < 0)
		return -1;
#ifdef SPI_OMAPL1XX
	/* reset fmt */
	fmt &= ~(DM6446_SPIFMT_PHASE1 | DM6446_SPIFMT_SHIFTLSB | DM6446_SPIFMT_PRESCALE(0xff));
#endif
	fmt |= DM6446_SPIFMT_PRESCALE(prescale);

	if (cfg->mode & SPI_MODE_CKPHASE_HALF)
		fmt |= DM6446_SPIFMT_PHASE1;

	if (!(cfg->mode & SPI_MODE_BODER_MSB))
		fmt |= DM6446_SPIFMT_SHIFTLSB;
#ifdef SPI_OMAPL1XX
	/* if omapl1xx , set if ENA & T2T  Delay if required */
	if(dm644x->spicntrlr & OMAPL1xx_SPI_CONTROLLER)
		fmt |= dm644x->spifmt[device];
		/* if cs is >= 3 we do not update registers now but we save the configuration
		 * & update the format registers just before the transmission really starts
		 */
	if(device > 2)
		dm644x->spifmt[device] = fmt;
	else
#endif
	/*
	 * Program the data format register
	 * Or we can always use format 0, and re-program FMT0 register
	 * every time the format has to be changed.
	 */

	out32(dm644x->spivbase + DM6446_SPI_FMT0 + (device * 4), fmt);
	/* 
	 * Programming control field alone cannot trigger a spi transfer
	 */
#ifdef SPI_OMAPL1XX
	/* touch dat field only if its current device */
	if(dm644x->spicntrlr & OMAPL1xx_SPI_CONTROLLER && dm644x->lastdev == device) {
		/* accessing DM6446_SPI_DAT1 as byte works only for little endian architecture */
		uint8_t dat1_ctrl_field = in8(dm644x->spivbase + DM6446_SPI_DAT1 + 3);
		if (!(cfg->mode & SPI_MODE_CSHOLD_HIGH)) {
			out8(dm644x->spivbase + DM6446_SPI_DAT1 + 3, dat1_ctrl_field & ~(1 << 4));
		} else
			out8(dm644x->spivbase + DM6446_SPI_DAT1 + 3, dat1_ctrl_field | (1 << 4));

	}
#endif
	return 0;
}
