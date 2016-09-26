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



/*
 * We use the same buffer for transmit and receive
 * For exchange, that's exactly what we wanted
 * For Read, it doesn't matter what we write to SPI, so we are OK.
 * For transmit, the receive data is put at the buffer we just transmitted, we are still OK.
 */
static const struct sigevent *spi_intr(void *area, int id)
{
	dm644x_spi_t	*dev = area;
	uintptr_t		base = dev->spivbase;
	uint32_t		spibuf = in32(base + DM6446_SPI_BUF);

	/*
	 * More to exchange ?
	 */
	if (dev->xdone <= dev->xlen) {

		/*
		 * Error, stop tranction
		 */
		if (spibuf & (DM6446_SPIBUF_RXOVR | DM6446_SPIBUF_BITERR | DM6446_SPIBUF_RXEMPTY)) {
			dev->xdone = -1;
			return (&dev->spievent);
		}

		if (dev->dlen <= 8) {
			/*
			 * Read data first
			 */
			dev->pbuf[dev->xdone] = spibuf;
			dev->xdone++;

			/*
			 * More to transmit?
			 */
			if (dev->xdone < dev->xlen)
				out16(base + DM6446_SPI_DAT1, dev->pbuf[dev->xdone]);
		}
		else {
			/*
			 * Read data first
			 */
			*(uint16_t *)(&dev->pbuf[dev->xdone]) = spibuf;
			dev->xdone += 2;

			/*
			 * More to transmit?
			 */
			if (dev->xdone < dev->xlen)
				out16(base + DM6446_SPI_DAT1, *(uint16_t *)(&dev->pbuf[dev->xdone]));
		}

		if (dev->xdone == dev->xlen)
			return (&dev->spievent);
	}
#if 0
	else {
		/*
		 * Should not happen
		 */
		out32(base + DM6446_SPI_INT, 0);
	}
#endif

	return 0;
}

int dm644x_attach_intr(dm644x_spi_t *dm644x)
{
	if ((dm644x->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1)
		return -1;

	if ((dm644x->coid = ConnectAttach(0, 0, dm644x->chid, _NTO_SIDE_CHANNEL, 0)) == -1) 
		goto fail0;

	dm644x->spievent.sigev_notify   = SIGEV_PULSE;
	dm644x->spievent.sigev_coid     = dm644x->coid;
	dm644x->spievent.sigev_code     = DM644X_SPI_EVENT;
	dm644x->spievent.sigev_priority = DM644X_SPI_PRIORITY;

	/*
	 * Attach SPI interrupt
	 */
	dm644x->spiiid = InterruptAttach(dm644x->spiirq, spi_intr, dm644x, 0, _NTO_INTR_FLAGS_TRK_MSK);

	if (dm644x->spiiid != -1)
		return 0;

	ConnectDetach(dm644x->coid);
fail0:
	ChannelDestroy(dm644x->chid);

	return -1;
}
