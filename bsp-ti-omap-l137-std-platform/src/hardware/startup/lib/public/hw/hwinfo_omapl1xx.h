/*
 * $QNXLicenseC:
 * Copyright 2010, QNX Software Systems. 
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


/*
* TI OMAPL1xx SOC devices
*
* This file contains names for all of the devices that may exists on any OMAPL1xx
* processor variant. Specifically, these are the internal devices that are part
* of the OMAPL1xx SOC and also board specific devices. 
* The names are added to the hwinfo section of the syspage (at the discretion of
* the startup writer) so that drivers can use hwi_find_xxx() to determine
* the existence of SOC bus/device instances in a portable fashion and optionally
* retrieve device specific information like interrupt vectors, etc.
* 
* Not all of the devices are on every processor so hwi_find_xxx() may return
* HWI_NULL_OFF if a bus/device, or bus/device instance (ie. unit) is not found.
*
*/

#ifndef __HWINFO_OMAPL1xx_H_INCLUDED
#define __HWINFO_OMAPL1xx_H_INCLUDED

#include <hw/sysinfo.h>

/*
 * =============================================================================
 * 
 *                                B U S E S
 * 
 * =============================================================================
*/

/* USB */
#define OMAPL1xx_HWI_USB		HWI_ITEM_BUS_USB

/* SPI */
#define OMAPL1xx_HWI_SPI		HWI_ITEM_BUS_SPI

/* I2C */
#define OMAPL1xx_HWI_I2C		HWI_ITEM_BUS_I2C

/*
 * =============================================================================
 * 
 *                              D E V I C E S
 * 
 * =============================================================================
*/

/* SDRAM */
#define OMAPL1xx_HWI_SDRAM		"sdram"

/* SRAM */
#define OMAPL1xx_HWI_SRAM		"sram"

/* DMA */
#define OMAPL1xx_HWI_DMA		"dma"
#define OMAPL1xx_HWI_EDMA0_CC	"edma0_cc"
#define OMAPL1xx_HWI_EDMA0_TC0	"edma0_tc0"
#define OMAPL1xx_HWI_EDMA0_TC1	"edma0_tc1"
#define OMAPL1xx_HWI_EDMA1_CC	"edma1_cc"
#define OMAPL1xx_HWI_EDMA1_TC0	"edma1_tc0"

/* Ethernet Controller */
#define OMAPL1xx_HWI_ENET		"network"

/* IDE Controller */
#define OMAPL1xx_HWI_IDE		"ide"

/* UART - 3 devices */
#define	OMAPL1xx_HWI_UART		"uart"

/* GPIO */
#define OMAPL1xx_HWI_GPIO		"gpio"

/* RTC */
#define	OMAPL1xx_HWI_RTC		"rtc"

/* Timers */
#define	OMAPL1xx_HWI_TIMER		"timer"

/* Watchdog */
#define	OMAPL1xx_HWI_WDOG		"wdog"

/* MMCSD */
#define	OMAPL1xx_HWI_MMCSD		"mmcsd"

/* SATA */
#define	OMAPL1xx_HWI_SATA		"sata"

/* NOR flash */
#define	OMAPL1xx_HWI_NOR		"nor_flash"

/* NAND flash */
#define	OMAPL1xx_HWI_NAND		"nand_flash"

/* 2D graphics */
#define	OMAPL1xx_HWI_GRAPHICS	"2d-graphics"

#endif	/* __HWINFO_OMAPL1xx_H_INCLUDED */

__SRCVERSION("hwinfo_omapl1xx.h $Rev: $");

