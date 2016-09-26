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


static void dm644x_edma_setbit(uintptr_t base, int reg, int bit)
{
	if (bit > 31)
		reg += 4, bit -= 32;

	out32(base + reg, (1 << bit));
}

#ifdef SPI_OMAPL1XX
static int omapl1xx_edma_init(dm644x_spi_t *dm644x)
{
	static struct sigevent event;
	unsigned channel;
	dma_attach_flags flags = DMA_ATTACH_EVENT_ON_COMPLETE | DMA_ATTACH_CASCADE;
	/* We are using only channel_controller=0 so variant=l137 works for both l137 & l138 spi */
	if(dm644x->dmafuncs.init("variant=l137")) {
		sdbg("unable to initialize dma lib");
		return -1;
	}
	SIGEV_INTR_INIT(&event);
	event.sigev_notify   = SIGEV_PULSE;
	event.sigev_coid     = dm644x->coid;
	event.sigev_code     = DM644X_DMA_EVENT;
	event.sigev_priority = DM644X_SPI_PRIORITY;

	channel = dm644x->ch_rxedma;
	/* get the receive dma handle */
	dm644x->dma_rx_handle = dm644x->dmafuncs.channel_attach("edma,channel_controller=0,async", &event, &channel, OMAPL1XX_SPI_DMA_PRIORITY, flags);
	if (dm644x->dma_rx_handle == NULL) {
		sdbg("unable to attach channel %d for rx", channel);
		goto fail1;
	}
	channel = dm644x->ch_txedma;
	/* get the transmit dma handle */
	dm644x->dma_tx_handle = dm644x->dmafuncs.channel_attach("edma,channel_controller=0,async", &event, &channel, OMAPL1XX_SPI_DMA_PRIORITY, flags);
	if (dm644x->dma_tx_handle == NULL) {
		sdbg("unable to attach channel %d for tx", channel);
		goto fail2;
	}
	return 0;

	dm644x->dmafuncs.channel_release(dm644x->dma_tx_handle);
fail2:
	dm644x->dmafuncs.channel_release(dm644x->dma_rx_handle);
fail1:
	dm644x->dmafuncs.fini();
	return -1;
}

static int omapl1xx_setup_edma(dm644x_spi_t *dm644x, int device, spi_dma_paddr_t *paddr, int len)
{
	dma_addr_t dma_src_addr = {0}, dma_dst_addr = {0};
	/* initialize transfer info struct with terms common for tx & rx */
	dma_transfer_t tinfo = {
		.src_fragments = 0,		/* no scatter-gather/segments used */
		.dst_fragments = 0,		/* no scatter-gather/segments used */
		.xfer_unit_size = 1,	/* transfer 1 byte @ a time */
		.xfer_bytes = len,		/* transfer length */
		.mode_flags = 0,
		.src_addrs = &dma_src_addr,
		.dst_addrs = &dma_dst_addr,
	};
	/* setup transfer info for spi TX channel */
	if(paddr->wpaddr) {
		dma_src_addr.paddr = paddr->wpaddr;
		dma_src_addr.vaddr = mmap_device_memory(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, paddr->wpaddr);
		if(dma_src_addr.vaddr == MAP_FAILED) {
			sdbg("mmap_device_memory failed : %s", strerror(errno));
			return -1;
		}
	} else {
		dma_src_addr.paddr = dm644x->pdmabuf;
		dma_src_addr.vaddr = dm644x->dmabuf;
	}
		
	/* dst is spi tx reg */
	dma_dst_addr.paddr = dm644x->spipbase + OMAPL1xx_SPI_DAT1;
	dma_dst_addr.vaddr = (void*) (dm644x->spivbase + OMAPL1xx_SPI_DAT1);
	tinfo.src_flags = 0;
	tinfo.dst_flags = DMA_ADDR_FLAG_NO_INCREMENT;
	if (dm644x->dmafuncs.setup_xfer(dm644x->dma_tx_handle, &tinfo)) {
		sdbg("setting up dma channel for tx failed");
		return -1;
	}

	/* setup transfer info for spi RX channel */
	/* src is spi rx reg */
	dma_src_addr.paddr = dm644x->spipbase + OMAPL1xx_SPI_BUF;
	dma_src_addr.vaddr = (void*) (dm644x->spivbase + OMAPL1xx_SPI_BUF);
	tinfo.src_flags = DMA_ADDR_FLAG_NO_INCREMENT;
	tinfo.dst_flags = 0;
	if(paddr->rpaddr) {
		dma_dst_addr.paddr = paddr->rpaddr;
		dma_dst_addr.vaddr = mmap_device_memory(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, paddr->rpaddr);
		if(dma_dst_addr.vaddr == MAP_FAILED) {
			sdbg("mmap_device_memory failed : %s", strerror(errno));
			return -1;
		}
	} else {
		dma_dst_addr.paddr = dm644x->pdmabuf;
		dma_dst_addr.vaddr = dm644x->dmabuf;
	}
	if (dm644x->dmafuncs.setup_xfer(dm644x->dma_rx_handle, &tinfo)) {
		sdbg("setting up dma channel for spi rx channel failed");
		return -1;
	}

	if (dm644x->dmafuncs.xfer_start(dm644x->dma_tx_handle)) {
		sdbg("unable to start dma xfer for tx channel");
		return -1;
	}
	if (dm644x->dmafuncs.xfer_start(dm644x->dma_rx_handle)) {
		sdbg("unable to start dma xfer for rx handle");
		return -1;
	}

	out32(dm644x->spivbase + DM6446_SPI_GCR1, dm644x->gcr1 | DM6446_SPIGCR1_SPIENA);
	out32(dm644x->spivbase + DM6446_SPI_INT, DM6446_SPIINT_DMAREQEN);

	return 0;
}

void omapl1xx_edma_disable(dm644x_spi_t *dm644x)
{
	dm644x->dmafuncs.channel_release(dm644x->dma_tx_handle);
	dm644x->dmafuncs.channel_release(dm644x->dma_rx_handle);
	dm644x->dmafuncs.fini();
	return;
}

#endif /* #ifdef SPI_OMAPL1XX */

void dm6446_edma_disablespi(dm644x_spi_t *dm644x)
{
#ifdef SPI_OMAPL1XX
	if (dm644x->spicntrlr & OMAPL1xx_SPI_CONTROLLER && dm644x->edma)
		return omapl1xx_edma_disable(dm644x);
#endif
	uintptr_t	edmabase = dm644x->edmavbase;
	uintptr_t	region0base = edmabase + DM6446_EDMA_REGION0;

	dm644x_edma_setbit(region0base, DM6446_EDMA_EECR, dm644x->ch_rxedma);
	dm644x_edma_setbit(region0base, DM6446_EDMA_EECR, dm644x->ch_txedma);
	dm644x_edma_setbit(region0base, DM6446_EDMA_ECR,  dm644x->ch_rxedma);
	dm644x_edma_setbit(region0base, DM6446_EDMA_ECR,  dm644x->ch_txedma);
	dm644x_edma_setbit(region0base, DM6446_EDMA_SECR, dm644x->ch_rxedma);
	dm644x_edma_setbit(region0base, DM6446_EDMA_SECR, dm644x->ch_txedma);
	dm644x_edma_setbit(edmabase,    DM6446_EDMA_EMCR, dm644x->ch_rxedma);
	dm644x_edma_setbit(edmabase,    DM6446_EDMA_EMCR, dm644x->ch_txedma);
}

int dm644x_setup_edma(dm644x_spi_t *dm644x, int device, spi_dma_paddr_t *paddr, int len)
{
#ifdef SPI_OMAPL1XX
	if (dm644x->spicntrlr & OMAPL1xx_SPI_CONTROLLER && dm644x->edma)
		return omapl1xx_setup_edma(dm644x, device, paddr, len);
#endif
	uint32_t			acnt, bcnt, ccnt;
	dm6446_edma_param	*param;

	acnt = 1;
	bcnt = len & 0x7FFF;		/* We use 32KB chunk */
	ccnt = (len + 0x7FFF) >> 15;

	if (bcnt == 0)
		bcnt = 0x8000;

	if (dm644x->dlen > 8) {
		acnt <<= 1;
		bcnt >>= 1;
	}

	dm6446_edma_disablespi(dm644x);

	/********************
	 * Setup Tx channel *
	 ********************/
	param = (dm6446_edma_param *)(dm644x->edmavbase + DM6446_EDMA_PARAM_BASE + (0x20 * dm644x->ch_txedma));

	if (paddr->wpaddr)
		param->src     = (uint32_t)paddr->wpaddr;
	else
		param->src     = (uint32_t)paddr->rpaddr;
	param->opt         = (dm644x->ch_txedma << 12);	/* ACNT only */
	param->abcnt       = (bcnt << 16) | acnt;
	param->dst         = dm644x->spipbase + 0x3C;	/* Destination address */
	param->srcdstbidx  = (0 << 16) | acnt;
	param->linkbcntrld = 0xFFFF | (0x8000 << 16);
	param->srcdstcidx  = (0 << 16) | acnt;
	param->ccnt        = ccnt;

	/********************
	 * Setup Rx channel *
	 ********************/
	param = (dm6446_edma_param *)(dm644x->edmavbase + DM6446_EDMA_PARAM_BASE + (0x20 * dm644x->ch_rxedma));

	if (paddr->rpaddr) {
		param->dst        = (uint32_t)paddr->rpaddr;
		param->srcdstbidx = (acnt << 16) | 0;
		param->srcdstcidx = (acnt << 16) | 0;
	}
	else {
		param->dst        = dm644x->pdmabuf;
		param->srcdstbidx = (0 << 16) | 0;
		param->srcdstcidx = (0 << 16) | 0;
	}
	param->src            = dm644x->spipbase + 0x40;	/* Source address */
	param->abcnt          = (bcnt << 16) | acnt;
	param->linkbcntrld    = 0xFFFF | (0x8000 << 16);
	param->ccnt           = ccnt;
	param->opt            = (dm644x->ch_rxedma << 12) | (1 << 20);	/* ACNT only, interrupt enable */

	/* Enable EDMA event */
	dm644x_edma_setbit(dm644x->edmavbase, DM6446_EDMA_REGION0 + DM6446_EDMA_EESR, dm644x->ch_rxedma);
	dm644x_edma_setbit(dm644x->edmavbase, DM6446_EDMA_REGION0 + DM6446_EDMA_EESR, dm644x->ch_txedma);

	/*
	 * There is a chip bug in DM6446, the SPI has to be enabled before
	 * enabling EDMA request. The bug should be fixed in Jacinto.
	 */
	/*
	 * Enable SPI master
	 */
	out32(dm644x->spivbase + DM6446_SPI_GCR1, dm644x->gcr1 | DM6446_SPIGCR1_SPIENA);

	/*
	 * Enable EDMA request
	 */
	out32(dm644x->spivbase + DM6446_SPI_INT, DM6446_SPIINT_DMAREQEN);

	return 0;
}

int dm644x_init_edma(dm644x_spi_t *dm644x)
{
	struct sigevent	event;

	/*
	 * Map in DMA buffer
	 */
	if (dm644x->edma != 0) {
		if (dm644x->edma > 0) {
			dm644x->dmabuf = mmap(0, DM644X_SPI_MAXDMALEN, PROT_READ|PROT_WRITE|PROT_NOCACHE, 
							MAP_ANON|MAP_PHYS|MAP_PRIVATE, NOFD, 0);
			if (dm644x->dmabuf != MAP_FAILED) {
				dm644x->pdmabuf = mphys(dm644x->dmabuf);
				if (dm644x->pdmabuf == -1)
					dm644x->edma = 0;
			}
		}
		else {
			dm644x->pdmabuf = mphys(&dm644x->dmabuf);
			if (dm644x->pdmabuf == -1)
				dm644x->edma = 0;
		}
	}

#ifdef SPI_OMAPL1XX
	if (dm644x->spicntrlr & OMAPL1xx_SPI_CONTROLLER && dm644x->edma)
		return omapl1xx_edma_init(dm644x);
#endif
	/*
	 * Map in EDMA register
	 */
	if ((dm644x->edmavbase = mmap_device_io(DM6446_EDMA_SIZE, dm644x->edmapbase)) == (uintptr_t)MAP_FAILED)
		goto fail0;

	/*
	 * Enable SPI EDMA channel for region 0, should be done in startup
	 */
	InterruptDisable();
	if (dm644x->ch_rxedma > 31)
		out32(dm644x->edmavbase + DM6446_EDMA_DRAEH(0), in32(dm644x->edmavbase + DM6446_EDMA_DRAEH(0)) | (1 << (dm644x->ch_rxedma - 32)));
	else
		out32(dm644x->edmavbase + DM6446_EDMA_DRAE(0), in32(dm644x->edmavbase + DM6446_EDMA_DRAE(0)) | (1 << dm644x->ch_rxedma));
	if (dm644x->ch_txedma > 31)
		out32(dm644x->edmavbase + DM6446_EDMA_DRAEH(0), in32(dm644x->edmavbase + DM6446_EDMA_DRAEH(0)) | (1 << (dm644x->ch_txedma - 32)));
	else
		out32(dm644x->edmavbase + DM6446_EDMA_DRAE(0), in32(dm644x->edmavbase + DM6446_EDMA_DRAE(0)) | (1 << dm644x->ch_txedma));
	InterruptEnable();

	dm6446_edma_disablespi(dm644x);

	/*
	 * Attach DMA interrupt
	 * We only attach Rx interrupt
	 */
	SIGEV_INTR_INIT(&event);
	event.sigev_notify   = SIGEV_PULSE;
	event.sigev_coid     = dm644x->coid;
	event.sigev_code     = DM644X_DMA_EVENT;
	event.sigev_priority = DM644X_SPI_PRIORITY;

	dm644x->edmaiid = InterruptAttachEvent(dm644x->edmairq, &event, _NTO_INTR_FLAGS_TRK_MSK);

	if (dm644x->edmaiid != -1)
		return 0;

	munmap_device_io(dm644x->edmavbase, DM6446_EDMA_SIZE);
fail0:
	if (dm644x->edma > 0)
		munmap(dm644x->dmabuf, DM644X_SPI_MAXDMALEN);

	return -1;
}
