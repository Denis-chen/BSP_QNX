/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems.
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

// Module Description:  board specific header file

#ifndef _BS_H_INCLUDED
#define _BS_H_INCLUDED


// add new chipset externs here
#define MMCSD_VENDOR_TI_DRA446

#define OMAPL1XX_SD_IRQ		16
#define EDMA_EVENT_SD_RX	16
#define EDMA_EVENT_SD_TX	17
#define GPIO_IRQ			44
#define GPIO_BANK_REG_START_OFF	0x10
#define GPIO_BANK_REG_SET_SIZ	0x28

#define OMAPL1xx_GPIO_IN_DATA 		0x10
#define OMAPL1xx_GPIO_INTSTAT		0x24
#define OMAPL1xx_GPIO_SET_RIS_TRIG	0x14
#define OMAPL1xx_GPIO_SET_FAL_TRIG	0x1C
#define OMAPL1xx_GPIO_CLR_RIS_TRIG	0x18
#define OMAPL1xx_GPIO_CLR_FAL_TRIG	0x20

#include <sys/hwinfo.h>

struct gpio_t {
	unsigned int bank;
	unsigned int pin;
};

struct hwi_sdmmc_gpio_info {
	struct hwi_prefix	prefix;
	struct sdmmc_gpio_info_t {
		struct gpio_t wp;
		struct gpio_t ins;
		uintptr_t gpio_base;
		unsigned int ins_irq;
	} gpio_info;
};

int bs_init(SIM_HBA *hba);
int bs_dinit(SIM_HBA *hba);

__SRCVERSION("$URL$ $Rev$");
#endif
