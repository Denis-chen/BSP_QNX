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

enum dm644x_spi_options{BASE, IRQ, CLOCK, EDMABASE, EDMAIRQ, EDMACHANNEL, EDMA, C2T, T2C, EN0DEF, EN1DEF, LOOPBACK,
#ifdef	SPI_OMAPL1XX
						ENA0, ENA1, ENA2, ENA3, ENA4, ENA5, ENA6, ENA7, T2T0, T2T1, T2T2, T2T3, T2T4, T2T5, T2T6, T2T7,
						T2E, C2E, PARITYEN0, PARITYEN1, PARITYEN2, PARITYEN3, PARITYEN4, PARITYEN5, PARITYEN6, PARITYEN7, 
						SPICONTROLLER, EN2DEF, EN3DEF, EN4DEF, EN5DEF, EN6DEF, EN7DEF,
#endif
						END
					};

static char *dm644x_opts[] = {
	[BASE]			=	"base",			/* Base address for this SPI controller */
	[IRQ]			=	"irq",			/* IRQ for this SPI intereface */
	[CLOCK]			=	"clock",		/* SPI clock */
	[EDMABASE]		=	"edmabase",		/* Base address for EDMA controller */
	[EDMAIRQ]		=	"edmairq",		/* IRQ for this SPI interface receive event */
	[EDMACHANNEL]	=	"edmachannel",	/* EDMA channel for this SPI interface receive event, Tx channel = rxchannel - 1 */
	[EDMA]			=	"edma",			/* Disable/enable EDMA for SPI */
	[C2T]			=	"c2t",			/* Chip-select-active-to-transmit-start-delay */
	[T2C]			=	"t2c",			/* Transmit-end-to-chip-select-inactive-delay */
	[EN0DEF]		=	"en0def",		/* Chip select 0 default pattern */
	[EN1DEF]		=	"en1def",		/* Chip select 1 default pattern */
	[LOOPBACK]		=	"loopback",		/* Loopback enable, for testing */
#ifdef SPI_OMAPL1XX
	[ENA0]			=	"ena0",          /* Ena signal is used for cs 0 */
	[ENA1]			=	"ena1",          /* Ena signal is used for cs 1 */
	[ENA2]			=	"ena2",          /* Ena signal is used for cs 2 */
	[ENA3]			=	"ena3",          /* Ena signal is used for cs 3 */
	[ENA4]			=	"ena4",          /* Ena signal is used for cs 4 */
	[ENA5]			=	"ena5",          /* Ena signal is used for cs 5 */
	[ENA6]			=	"ena6",          /* Ena signal is used for cs 6 */
	[ENA7]			=	"ena7",          /* Ena signal is used for cs 7 */
	[T2T0]			=	"t2t0",          /* Delay between transmissions for cs 0 */
	[T2T1]			=	"t2t1",          /* Delay between transmissions for cs 1 */
	[T2T2]			=	"t2t2",          /* Delay between transmissions for cs 2 */
	[T2T3]			=	"t2t3",          /* Delay between transmissions for cs 3 */
	[T2T4]			=	"t2t4",          /* Delay between transmissions for cs 4 */
	[T2T5]			=	"t2t5",          /* Delay between transmissions for cs 5 */
	[T2T6]			=	"t2t6",          /* Delay between transmissions for cs 6 */
	[T2T7]			=	"t2t7",          /* Delay between transmissions for cs 7 */
	[T2E]			=	"t2e",          /* Transmit data finished to spi_ena pin inactive timeout */
	[C2E]			=	"c2e",          /* Chip select active to spi_ena active timeout */
	[PARITYEN0]		=	"parena0",       /* Parity 1 => odd parity , 0 => even parity for cs 0 */
	[PARITYEN1]		=	"parena1",       /* Parity 1 => odd parity , 0 => even parity for cs 1 */
	[PARITYEN2]		=	"parena2",       /* Parity 1 => odd parity , 0 => even parity for cs 2 */
	[PARITYEN3]		=	"parena3",       /* Parity 1 => odd parity , 0 => even parity for cs 3 */
	[PARITYEN4]		=	"parena4",       /* Parity 1 => odd parity , 0 => even parity for cs 4 */
	[PARITYEN5]		=	"parena5",       /* Parity 1 => odd parity , 0 => even parity for cs 5 */
	[PARITYEN6]		=	"parena6",       /* Parity 1 => odd parity , 0 => even parity for cs 6 */
	[PARITYEN7]		=	"parena7",       /* Parity 1 => odd parity , 0 => even parity for cs 7 */
	[SPICONTROLLER]	=	"spicntrlr",    /* omapl1xx => OMAPL1xx */
	[EN2DEF]		=	"en2def",		/* Chip select 2 default pattern */
	[EN3DEF]		=	"en3def",		/* Chip select 3 default pattern */
	[EN4DEF]		=	"en4def",		/* Chip select 4 default pattern */
	[EN5DEF]		=	"en5def",		/* Chip select 5 default pattern */
	[EN6DEF]		=	"en6def",		/* Chip select 6 default pattern */
	[EN7DEF]		=	"en7def",		/* Chip select 7 default pattern */
#endif
	[END]			=	NULL
};

spi_funcs_t spi_drv_entry = {
	sizeof(spi_funcs_t),
	dm644x_init,	/* init() */
	dm644x_dinit,	/* fini() */
	dm644x_drvinfo,	/* drvinfo() */
	dm644x_devinfo,	/* devinfo() */
	dm644x_setcfg,	/* setcfg() */
	dm644x_xfer,	/* xfer() */
	dm644x_dmaxfer	/* dma_xfer() */
};

spi_devinfo_t devlist[DEVICE_NOS] = {
	{
		0x00,				// Device ID
		"DM644x-SPI0",		// Description
		{ 
			8 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			5000000			// Clock rate 5M
		},
	},
	{
		0x01,				// Device ID
		"DM644x-SPI1",		// Description
		{ 
			8 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			6000000			// Clock rate 6M
		},
	},
	{
		0x02,				// Device ID
		"DM644x-SPI2",		// Description
		{ 
			8 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			7000000			// Clock rate 7M
		},
	},
	{
		0x03,				// Device ID
		"DM644x-SPI3",		// Description
		{ 
			8 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			8000000			// Clock rate 8M
		},
	},
#ifdef SPI_OMAPL1XX
	{
		0x04,				// Device ID
		"DM644x-SPI4",		// Description
		{ 
			8 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			8000000			// Clock rate 8M
		},
	},
	{
		0x05,				// Device ID
		"DM644x-SPI5",		// Description
		{ 
			8 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			8000000			// Clock rate 8M
		},
	},
	{
		0x06,				// Device ID
		"DM644x-SPI6",		// Description
		{ 
			8 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			8000000			// Clock rate 8M
		},
	},
	{
		0x07,				// Device ID
		"DM644x-SPI7",		// Description
		{ 
			8 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			8000000			// Clock rate 8M
		},
	},
#endif 
};

static int dm644x_options(dm644x_spi_t *dev, char *optstring)
{
	int		opt, rc = 0, err = EOK;
	char	*options, *freeptr, *c, *value;

	if (optstring == NULL)
		return 0;

	freeptr = options = strdup(optstring);

	while (options && *options != '\0') {
		c = options;
		if ((opt = getsubopt(&options, dm644x_opts, &value)) == -1)
			goto error;

		switch (opt) {
			case  BASE:
				dev->spipbase  = strtoul(value, 0, 0); 
				continue;
			case  IRQ:
				dev->spiirq    = strtoul(value, 0, 0);
				continue;
			case  CLOCK: 
				dev->clk       = strtoul(value, 0, 0);
				continue;
			case  EDMABASE:
				dev->edmapbase = strtoul(value, 0, 0);
				continue;
			case  EDMAIRQ:
				dev->edmairq   = strtoul(value, 0, 0);
				continue;
			case  EDMACHANNEL:
				dev->ch_rxedma = strtoul(value, 0, 0);
				continue;
			case  EDMA:
				dev->edma      = strtol(value, 0, 0);
				continue;
			case  C2T:
				dev->spidelay |= strtoul(value, 0, 0) << 24;
				continue;
			case  T2C:
				dev->spidelay |= strtoul(value, 0, 0) << 16;
				continue;
			case  EN0DEF:
				dev->spidef   |= (strtoul(value, 0, 0) & 1);
#ifdef SPI_OMAPL1XX
 				dev->spidef   |= (1 << 24);
#endif
				continue;
			case EN1DEF:
				dev->spidef   |= ((strtoul(value, 0, 0) & 1) << 1);
#ifdef SPI_OMAPL1XX
 				dev->spidef   |= (1 << 25);
#endif
				continue;
			case LOOPBACK:
				dev->gcr1     |= DM6446_SPIGCR1_LOOPBACK;
				continue;
#ifdef SPI_OMAPL1XX
			case ENA0:
				dev->spifmt[0] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case ENA1:
				dev->spifmt[1] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case ENA2:
				dev->spifmt[2] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case ENA3:
				dev->spifmt[3] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case ENA4:
				dev->spifmt[4] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case ENA5:
				dev->spifmt[5] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case ENA6:
				dev->spifmt[6] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case ENA7:
				dev->spifmt[7] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case T2T0:
				dev->spifmt[0] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2T1:
				dev->spifmt[1] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2T2:
				dev->spifmt[2] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2T3:
				dev->spifmt[3] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2T4:
				dev->spifmt[4] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2T5:
				dev->spifmt[5] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2T6:
				dev->spifmt[6] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2T7:
				dev->spifmt[7] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2E:
				dev->spidelay |= strtoul(value, 0, 0) << 8;
				continue;
			case C2E:
				dev->spidelay |= strtoul(value, 0, 0) << 8;
				continue;
			case PARITYEN0:
				dev->spifmt[0] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case PARITYEN1:
				dev->spifmt[1] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case PARITYEN2:
				dev->spifmt[2] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case PARITYEN3:
				dev->spifmt[3] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case PARITYEN4:
				dev->spifmt[4] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case PARITYEN5:
				dev->spifmt[5] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case PARITYEN6:
				dev->spifmt[6] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case PARITYEN7:
				dev->spifmt[7] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case SPICONTROLLER:
				if(!strncmp(value, "omapl1xx", strlen("omapl1xx")))
					dev->spicntrlr = OMAPL1xx_SPI_CONTROLLER;
				continue;
			case EN2DEF:
				dev->spidef   |= ((strtoul(value, 0, 0) & 1) << 2) | (1 << 26);
				continue;
			case EN3DEF:
				dev->spidef   |= ((strtoul(value, 0, 0) & 1) << 3) | (1 << 27);
				continue;
			case EN4DEF:
				dev->spidef   |= ((strtoul(value, 0, 0) & 1) << 4) | (1 << 28);
				continue;
			case EN5DEF:
				dev->spidef   |= ((strtoul(value, 0, 0) & 1) << 5) | (1 << 29);
				continue;
			case EN6DEF:
				dev->spidef   |= ((strtoul(value, 0, 0) & 1) << 6) | (1 << 30);
				continue;
			case EN7DEF:
				dev->spidef   |= ((strtoul(value, 0, 0) & 1) << 7) | (1 << 31);
				continue;		
#endif
		}
error:
		fprintf(stderr, "spi-dm644x: unknown option %s", c);
		err = EINVAL;
		rc = -1;
	}

	free(freeptr);

	return rc;
}

void *dm644x_init(void *hdl, char *options)
{
	dm644x_spi_t	*dev;
	uintptr_t		base;
	uint32_t        tmp = 0;
	dev = calloc(1, sizeof(dm644x_spi_t));

	if (dev == NULL)
		return NULL;
	dev->spipbase  = DRA446_SPI1_BASE;
	dev->spiirq    = DRA446_SPI1_INT;			/* We use SPI0 interrupt */
	dev->edmapbase = DM6446_EDMA_BASE;
	dev->edmairq   = 0xC100 + 36;	/* We use interrupt of receive channel */
	dev->clk       = 90000000;		/* 90MHz SPI clock */
	dev->ch_rxedma = 36;
	dev->edma      = -1;
	dev->gcr1      = DM6446_SPIGCR1_MASTER | DM6446_SPIGCR1_CLKMOD;

	if (dm644x_options(dev, options))
		goto fail0;

	/*
	 * Map in SPI and EDMA register
	 */
	if ((base = mmap_device_io(
#ifdef SPI_OMAPL1XX
			(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER) ? OMAPL1xx_SPI0_SIZE :
#endif
			DM6446_SPI_SIZE,
			dev->spipbase)) == (uintptr_t)MAP_FAILED)
		goto fail0;

	dev->spivbase = base;

	/*
	 * Put SPI module in reset mode
	 */
	out32(base + DM6446_SPI_GCR0, 0);
	out32(base + DM6446_SPI_GCR0, 1);

	/*
	 * Enable SPI master and clock
	 */
	out32(base + DM6446_SPI_GCR1, DM6446_SPIGCR1_MASTER | DM6446_SPIGCR1_CLKMOD);

	/*
	 * Configure SPI function pins
	 */

#ifdef SPI_OMAPL1XX
	/* For OMAPL1XX we can use ENA pin if needed 
	 * If any of device is using ENA pin then enable the pin
	 */
	int j;
	if(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER)
		for(j = 0; j < DEVICE_NOS; j++)
			if(dev->spifmt[j] & OMAPL1xx_SPIFMT_WAITENA) {
				tmp = OMAPL1xx_SPIPC_ENA;
				break;
			}
			/* top 8 bits of dev->spidef specifiy which all chip selects are used */
			tmp |= dev->spidef >> 24;
#else
	tmp = DM6446_SPIPC_EN0 | DM6446_SPIPC_EN1;
#endif
	out32(base + DM6446_SPI_PC0, DM6446_SPIPC_CLK | DM6446_SPIPC_DO | DM6446_SPIPC_DI | tmp );
	/*
	 * Set default data format 
	 */
	{
		int				i;
		spi_devinfo_t	*info;

		for (i = 0, info = &devlist[0]; i < DEVICE_NOS; i++, info++) {
			dm644x_cfg(dev, i, &info->cfg);
		}
	}

	/*
	 * Chip select default pattern : logic 0 when no transaction is in progress
	 */
	out32(base + DM6446_SPI_DEF, dev->spidef);

	/*
	 * C2T/D2C delay & For omapl1xx C2E & T2E 
	 */
	out32(base + DM6446_SPI_DELAY, dev->spidelay);

	/*
	 * Attach SPI interrupt or DMA interrupt
	 */
	if (dm644x_attach_intr(dev))
		goto fail1;

	if (dev->edma) {
#ifdef SPI_OMAPL1XX
		if(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER) {
		/* For OMAPL1XX Tx channel = Rx channel + 1 */
			dev->ch_txedma = dev->ch_rxedma + 1;
		/* initialize the dma_functions_t stucture */
			if(get_dmafuncs(&dev->dmafuncs, sizeof(dev->dmafuncs)))
				goto fail2;
		} else
#endif
		/*
		 * We assume the EDMA channel for Tx is Rx channel - 1
		 */
		dev->ch_txedma = dev->ch_rxedma - 1;

		if (dm644x_init_edma(dev))
			goto fail2;
	}

	dev->spi.hdl = hdl;
	dev->lastdev = -1;

	return dev;

fail2:
	InterruptDetach(dev->spiiid);
	ConnectDetach(dev->coid);
	ChannelDestroy(dev->chid);
fail1:
	out32(base + DM6446_SPI_GCR1, 0);
	out32(base + DM6446_SPI_GCR0, 0);
	munmap_device_io(dev->spivbase, DM6446_SPI_SIZE);
fail0:
	free(dev);
	return NULL;
}

void dm644x_dinit(void *hdl)
{
	dm644x_spi_t	*dev = hdl;

	/*
	 * unmap the register, detach the interrupt
	 */
	InterruptDetach(dev->spiiid);
	if (dev->edma)
#ifdef SPI_OMAPL1XX
		if (!(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER))
#endif
		InterruptDetach(dev->edmaiid);

	ConnectDetach(dev->coid);
	ChannelDestroy(dev->chid);

	/*
	 * Disable SPI
	 */
	out32(dev->spivbase + DM6446_SPI_GCR1, 0);
	out32(dev->spivbase + DM6446_SPI_GCR0, 0);
	munmap_device_io(dev->spivbase, DM6446_SPI_SIZE);

	/*
	 * Disable EDMA
	 */
	if (dev->edma) {
		dm6446_edma_disablespi(dev);
#ifdef SPI_OMAPL1XX
		if(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER)
			omapl1xx_edma_disable(dev);
		else
#endif
			munmap_device_io(dev->edmavbase, DM6446_EDMA_SIZE);			
		if (dev->edma > 0)
			munmap(dev->dmabuf, DM644X_SPI_MAXDMALEN);
	}

	free(hdl);
}

int dm644x_drvinfo(void *hdl, spi_drvinfo_t *info)
{
	info->version = (SPI_VERSION_MAJOR << SPI_VERMAJOR_SHIFT) | (SPI_VERSION_MINOR << SPI_VERMINOR_SHIFT) | (SPI_REVISION << SPI_VERREV_SHIFT);
#ifdef SPI_OMAPL1XX
	dm644x_spi_t *dev = hdl;
	if(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER) {
		strcpy(info->name, "OMAP L1xx SPI");
	} else
#endif
		strcpy(info->name, "TI DM644x SPI");
	info->feature = SPI_FEATURE_DMA;	/* DMA supported, buffer address alignemnt is 1 */

	return (EOK);
}

int dm644x_setcfg(void *hdl, uint16_t device, spi_cfg_t *cfg)
{
#ifdef SPI_OMAPL1XX
	dm644x_spi_t *dev = hdl;
	if(device >= DEVICE_NOS
			/* only allow transfer for devices that are actually present */
			|| (dev->spidef & (1 << (device + 24))) == 0 
			){
		return EINVAL;
	}
#else
	if (device <= 3) {
#endif
		memcpy(&devlist[device].cfg, cfg, sizeof(spi_cfg_t));

		if (dm644x_cfg(hdl, device, cfg) == 0)
			return (EOK);
#ifndef SPI_OMAPL1XX
	}
#endif
	return (EINVAL);
}

int dm644x_devinfo(void *hdl, uint32_t device, spi_devinfo_t *info)
{
	int		dev = device & SPI_DEV_ID_MASK;

#ifdef SPI_OMAPL1XX
	dm644x_spi_t	*dm644x = hdl;
#endif
	if (device & SPI_DEV_DEFAULT) {
		/*
		 * Info of this device
		 */
		if (dev >= 0 && dev <= DEVICE_NOS
#ifdef SPI_OMAPL1XX
		&& (dm644x->spidef & (1 << (dev + 24))) 
#endif
		)
			memcpy(info, &devlist[dev], sizeof(spi_devinfo_t));
		else
			return (EINVAL);
	}
	else {
		/*
		 * Info of next device 
		 */
		if (dev == SPI_DEV_ID_NONE)
			dev = -1;
		if (dev < DEVICE_NOS
#ifdef SPI_OMAPL1XX
			&& (dm644x->spidef & (1 << (dev + 24))) 
#endif
		)
			memcpy(info, &devlist[dev + 1], sizeof(spi_devinfo_t));
		else
			return (EINVAL);
	}

	return (EOK);
}

void *dm644x_xfer(void *hdl, uint32_t device, uint8_t *buf, int *len)
{
	dm644x_spi_t	*dev = hdl;
	uintptr_t		base = dev->spivbase;
	int				chip = device & SPI_DEV_ID_MASK;

#ifdef SPI_OMAPL1XX
	if( chip < 0 || chip >= DEVICE_NOS
			/* only allow transfer for devices that are actually present */
			|| (dev->spidef & (1 << (chip + 24))) == 0 
			)
	{
		*len = 0;
		return NULL;
	}
#endif

	dev->xlen = *len;

	/*
	 * Use EDMA ?
	 */
	if (dev->edma > 0 && dev->xlen >= dev->edma && dev->xlen <= DM644X_SPI_MAXDMALEN) {
		spi_dma_paddr_t	paddr;
		/*
		 * Unfortunately we have to copy the buffer for write and exchange
		 */
		if (((device >> SPI_DEV_XFER_SHIFT) & SPI_DEV_XFER_MASK) != SPI_DEV_READ)
			memcpy(dev->dmabuf, buf, dev->xlen);

		paddr.wpaddr = paddr.rpaddr = dev->pdmabuf;
		*len = dm644x_dmaxfer(hdl, device, &paddr, dev->xlen);

		return dev->dmabuf;
	}

	if (dm644x_select_format(dev, chip, &devlist[chip].cfg)) {
		*len = 0;
		return buf;
	}

	dev->pbuf  = buf;
	dev->xdone = 0;
	/*
	 * Enable receive interrupt
	 * TODO : Error interrupt 
	 */
#ifdef SPI_OMAPL1XX
	if(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER)
		/* level setting is for omapl1XX */
		out32(base + DM6446_SPI_LVL, DM6446_SPIINT_RXINTEN | DM6446_SPIINT_BITERRENA | DM6446_SPIINT_OVRNINTEN);

#endif
	out32(base + DM6446_SPI_INT, DM6446_SPIINT_RXINTEN | DM6446_SPIINT_BITERRENA | DM6446_SPIINT_OVRNINTEN);

	out32(base + DM6446_SPI_GCR1, dev->gcr1 | DM6446_SPIGCR1_SPIENA);

	/*
	 * Write the first data to shift register
	 */
	if (dev->dlen <= 8)
		out16(base + DM6446_SPI_DAT1, *buf);
	else
		out16(base + DM6446_SPI_DAT1, *(uint16_t *)buf);

	/*
	 * Wait for exchange to finish with timeout
	 */
	if (dm644x_wait(dev, dev->xlen * 10))
		dev->xdone = -1;

	/*
	 * Disable SPI function
	 */
	out32(base + DM6446_SPI_GCR1, DM6446_SPIGCR1_MASTER | DM6446_SPIGCR1_CLKMOD);

	*len = dev->xdone;

	return buf;
}

int dm644x_dmaxfer(void *hdl, uint32_t device, spi_dma_paddr_t *paddr, int len)
{
	dm644x_spi_t	*dev = hdl;
	int				chip = device & SPI_DEV_ID_MASK;

#ifdef SPI_OMAPL1XX
	if( chip < 0 || chip >= DEVICE_NOS
			/* only allow transfer for devices that are actually present */
			|| (dev->spidef & (1 << (chip + 24))) == 0 ){
		return -1;
	}
#endif

	/*
	 * Is the EDMA disabled?
	 */
	if (dev->edma == 0)
		return -1;

	/*
	 * FIXME
	 * Looks like we need to re-pregram DAT1 register to generate a SPI event
	 */
	dev->lastdev = -1;

	if (dm644x_select_format(hdl, chip, &devlist[chip].cfg))
		return -1;

	if (dm644x_setup_edma(hdl, chip, paddr, len))
		return -1;

	/*
	 * Wait for EDMA to finish with timeout
	 */
	if (dm644x_wait(dev, len))
		len = -1;

	/*
	 * Disable EDMA request and SPI function
	 */
	out32(dev->spivbase + DM6446_SPI_INT, 0);
	out32(dev->spivbase + DM6446_SPI_GCR1, DM6446_SPIGCR1_MASTER | DM6446_SPIGCR1_CLKMOD);

#ifdef SPI_OMAPL1XX
	if(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER) {
		dev->dmafuncs.xfer_abort(dev->dma_tx_handle);
		dev->dmafuncs.xfer_abort(dev->dma_rx_handle);		
	}
#endif

	return len;
}
