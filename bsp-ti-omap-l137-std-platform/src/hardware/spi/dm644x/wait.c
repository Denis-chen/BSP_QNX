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


int dm644x_wait(dm644x_spi_t *dev, int len)
{
	struct _pulse		pulse;
	dm6446_edma_param	*param;

	while (1) {
		if (len) {
			uint64_t	to = dev->dtime;
			to *= len * 1000 * 10;	/* 10 times for time out */
			TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_RECEIVE, NULL, &to, NULL);
		}

		if (MsgReceivePulse(dev->chid, &pulse, sizeof(pulse), NULL) == -1) {
			out32(dev->spivbase + DM6446_SPI_INT, 0);
			return -1;
		}

		switch (pulse.code) {
			case DM644X_SPI_EVENT:
				/* Disable Interrupt */
				out32(dev->spivbase + DM6446_SPI_INT, 0);
				return 0;

			case DM644X_DMA_EVENT:
#ifdef SPI_OMAPL1XX
				if(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER) {
					dev->dmafuncs.xfer_complete(dev->dma_rx_handle);
					dev->dmafuncs.xfer_complete(dev->dma_tx_handle);
					return 0;
				}
#endif
				param = (dm6446_edma_param *)(dev->edmavbase + DM6446_EDMA_PARAM_BASE + (0x20 * dev->ch_rxedma));
				/* Disable DMA request */
				out32(dev->spivbase + DM6446_SPI_INT, 0);
				/* Unmask the Interrupt */
				InterruptUnmask(dev->edmairq, dev->edmaiid);
				if (param->abcnt != 0 || param->ccnt != 0)
					continue;
				return 0;
		}
	}

	return 0;
}
