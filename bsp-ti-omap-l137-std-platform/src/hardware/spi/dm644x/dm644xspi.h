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







#ifndef _DM644XSPI_H_INCLUDED
#define _DM644XSPI_H_INCLUDED

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <hw/spi-master.h>
#include <arm/dm6446.h>
#include <arm/omapl1xx.h>
#ifdef SPI_OMAPL1XX
#include <hw/dma.h>

#define OMAPL1XX_SPI_DMA_PRIORITY 0
#define sdbg(...)	do { fprintf(stderr, __VA_ARGS__ ); fprintf(stderr, "\n");} while(0)
#endif

#define OMAPL1xx_SPI_CONTROLLER (1 << 7)

#define	DM644X_SPI_PRIORITY		21
#define	DM644X_SPI_EVENT		1
#define	DM644X_DMA_EVENT		2

#define	DM644X_SPI_MAXDMALEN	(1024 * 8)

/* JACINTO Definitions */

#define DRA446_SPI1_BASE		0x01C24400
#define DRA446_SPI1_INT			33
#define DRA446_SPI1_DMA			36

#ifdef SPI_OMAPL1XX
#define DEVICE_NOS	8
#else
#define DEVICE_NOS	4
#endif

typedef struct {
	volatile uint32_t   opt;
	volatile uint32_t   src;
	volatile uint32_t   abcnt;
	volatile uint32_t   dst;
	volatile uint32_t   srcdstbidx;
	volatile uint32_t   linkbcntrld;
	volatile uint32_t   srcdstcidx;
	volatile uint32_t   ccnt;
} dm6446_edma_param;

typedef struct {
	SPIDEV		spi;		/* This has to be the first element */

	uint32_t	spipbase;
	uint32_t	edmapbase;
	int			spiirq;
	int			edmairq;

	uint32_t	clk;
	int			ch_rxedma;
	int			ch_txedma;

	uintptr_t	spivbase;
	uintptr_t	edmavbase;
	int			spiiid;
	int			edmaiid;

	int			chid, coid;

	int			lastdev;
	int			edma;

	uint32_t	spidelay;
	uint32_t	spidef;
	uint32_t	gcr1;

	uint8_t		*pbuf;
	int			xlen, xdone;
	int			dlen;		/* Datalength */
	int			dtime;		/* usec per data, for time out use */

	void		*dmabuf;	/* Our DMA buffer */
	uint32_t	pdmabuf;	/* Our DMA buffer physical address */

	struct sigevent	spievent;
#ifdef SPI_OMAPL1XX
	uint32_t	spifmt[DEVICE_NOS];		/* used for omapl1xx alone for storing format register details */
	uint8_t		spicntrlr;	/* used to specify that we are using omapl1xx spi controller */
	dma_functions_t dmafuncs; /* dma functions structure */
	void *dma_rx_handle;	/* dma handle for receive channel */
	void *dma_tx_handle;	/* dma handle for transmit channel */
#endif
} dm644x_spi_t;

extern void *dm644x_init(void *hdl, char *options);
extern void dm644x_dinit(void *hdl);
extern void *dm644x_xfer(void *hdl, uint32_t device, uint8_t *buf, int *len);
extern void dm6446_edma_disablespi(dm644x_spi_t *dm644x);
extern int dm644x_dmaxfer(void *hdl, uint32_t device, spi_dma_paddr_t *paddr, int len);
extern int dm644x_init_edma(dm644x_spi_t *dm644x);
extern int dm644x_setcfg(void *hdl, uint16_t device, spi_cfg_t *cfg);

extern int dm644x_devinfo(void *hdl, uint32_t device, spi_devinfo_t *info);
extern int dm644x_drvinfo(void *hdl, spi_drvinfo_t *info);
extern int dm644x_cfg(dm644x_spi_t *dm644x, int chip, spi_cfg_t *cfg);

extern int dm644x_xchange(dm644x_spi_t *dm644x, uint8_t *buf, int len);
extern int dm644x_read(dm644x_spi_t *dm644x, uint8_t *buf, int len);
extern int dm644x_write(dm644x_spi_t *dm644x, uint8_t *buf, int len);

extern int dm644x_hwinit(dm644x_spi_t *dm644x);
extern int dm644x_attach_intr(dm644x_spi_t *dm644x);
extern int dm644x_cfg(dm644x_spi_t *dm644x, int device, spi_cfg_t *cfg);
extern int dm644x_wait(dm644x_spi_t *dev, int len);
extern int dm644x_setup_edma(dm644x_spi_t *dm644x, int device, spi_dma_paddr_t *paddr, int len);
extern int dm644x_select_format(dm644x_spi_t *dm644x, int chip, spi_cfg_t *cfg);
#ifdef SPI_OMAPL1XX
void omapl1xx_edma_disable(dm644x_spi_t*);
#endif
extern paddr_t mphys(void *);

#endif
