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


int dm644x_select_format(dm644x_spi_t *dm644x, int chip, spi_cfg_t *cfg)
{
	uint32_t	data = 0;

	if (chip > DEVICE_NOS - 1)
		return -1;

	if (chip != dm644x->lastdev) {
		/*
		 * Disable SPI first
		 */
		out32(dm644x->spivbase + DM6446_SPI_GCR1, DM6446_SPIGCR1_MASTER | DM6446_SPIGCR1_CLKMOD);

		if (cfg->mode & SPI_MODE_CSHOLD_HIGH)
			data |= 1 << 28;

#ifdef SPI_OMAPL1XX
		/* For OMAPL1XX if required enable spi delay between transfers */
		if((dm644x->spicntrlr & OMAPL1xx_SPI_CONTROLLER) && (dm644x->spifmt[chip] & (0x3F << 24)))
			data |= OMAPL1xx_SPIDAT1_WDEL;
		/* setup the chip selects */
		data |= ((1 << chip) ^ (dm644x->spidef & 0xFF)) << 16;
		/* for all chipselect values >= 3, we use format register 3 itself */
		if(chip > 2) {
			data |= 3 << 24;
			out32(dm644x->spivbase + DM6446_SPI_FMT3, dm644x->spifmt[chip]); 
		}
		else
			data |= chip << 24;
#else
		data |= (chip << 24) | (chip << 16);
#endif
		/*
		 * Program the control field
		 */
		out32(dm644x->spivbase + DM6446_SPI_DAT1, data);
		dm644x->lastdev = chip;

		dm644x->dlen = cfg->mode & SPI_MODE_CHAR_LEN_MASK;

		/*
		 * For timeout
		 */
		dm644x->dtime = dm644x->dlen * 1000 * 1000 / cfg->clock_rate;
		if (dm644x->dlen > 8)
			dm644x->dtime >>= 1;
		if (dm644x->dtime == 0)
			dm644x->dtime = 1;
	}
	return 0;
}
