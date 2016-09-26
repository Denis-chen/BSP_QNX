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

// Module Description:  board specific interface
#include <stdint.h>
#include <sys/mman.h>
#include <sim_mmc.h>
#include <sim_dra446.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <hw/sysinfo.h>
#include <arm/omapl1xx.h>
#include <drvr/hwinfo.h>
#include <hw/hwinfo_omapl1xx.h>
#include <signal.h>


enum {GPIO_BASE, WP_GPIO_BANK, WP_GPIO_NO, INS_GPIO_BANK, INS_GPIO_NO, INS_GPIO_IRQ, END};
static char *const bs_options[] = {
	[GPIO_BASE]		= "gpio_base",
	[WP_GPIO_BANK]	= "wp_gpio_bank",
	[WP_GPIO_NO]	= "wp_gpio",
	[INS_GPIO_BANK]	= "ins_gpio_bank",
	[INS_GPIO_NO]	= "ins_gpio",
	[INS_GPIO_IRQ] 	= "ins_gpio_irq",
	[END]			= NULL,
};

static int parse_options(char *bsopt, struct bs_opt *bsopts)
{
	int opt;
	char *valuep;

	if(bsopt) {
		valuep = bsopt;
		while(*valuep++ != '\0')
			if(*valuep == ' ')
					*valuep = ',';
		while((opt = getsubopt(&bsopt, bs_options, &valuep)) != -1) {
			switch(opt) {
			case GPIO_BASE:
				bsopts->gpio_vbase = strtol(valuep, NULL, 0);
			case WP_GPIO_BANK:
				bsopts->wp_gpio_vbase = strtol(valuep, NULL, 0);
				continue;
			case WP_GPIO_NO:
				bsopts->wp_gpio = strtol(valuep, NULL, 0);
				continue;
			case INS_GPIO_BANK:
				bsopts->ins_bank = strtol(valuep, NULL, 0);
				continue;
			case INS_GPIO_NO:
				bsopts->ins_gpio = strtol(valuep, NULL, 0);
				continue;
			case INS_GPIO_IRQ:
				bsopts->ins_irq = strtol(valuep, NULL, 0);
				continue;
			default:
				cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, 1, 3,"unknown option %s\n", valuep);
				continue;
			}
		}
	}

	if(bsopts->wp_gpio_vbase & 0x1)
		bsopts->wp_gpio += 16;
	
	if(bsopts->ins_bank & 0x1)
		bsopts->ins_gpio += 16;

	bsopts->wp_gpio = 1 << bsopts->wp_gpio;
	bsopts->ins_gpio = 1 << bsopts->ins_gpio;

	bsopts->gpio_vbase = mmap_device_io(OMAPL1xx_GPIO_SIZE, bsopts->gpio_vbase);
	if(bsopts->gpio_vbase == MAP_DEVICE_FAILED)
		return -1;
	bsopts->wp_gpio_vbase = bsopts->gpio_vbase + GPIO_BANK_REG_START_OFF + 
							GPIO_BANK_REG_SET_SIZ * (bsopts->wp_gpio_vbase >> 1);
	bsopts->ins_gpio_vbase = bsopts->gpio_vbase + GPIO_BANK_REG_START_OFF + 
							GPIO_BANK_REG_SET_SIZ * (bsopts->ins_bank >> 1);
	return 0;
}


static unsigned omapl1xx_get_syspage_clk(void)
{
	unsigned	item, offset, clock = 0;
	char		*name;
	hwi_tag		*tag;

	item = hwi_find_item(HWI_NULL_OFF, HWI_ITEM_DEVCLASS_DISK, OMAPL1xx_HWI_MMCSD, NULL);
	if (item == HWI_NULL_OFF)
		return 0;

	offset = item;

	while ((offset = hwi_next_tag(offset, 1)) != HWI_NULL_OFF) {
		tag = hwi_off2tag(offset);
		name = __hwi_find_string(((hwi_tag *)tag)->prefix.name);

		if (strcmp(name, HWI_TAG_NAME_inputclk) == 0)
			clock = ((struct hwi_inputclk *)tag)->clk / ((struct hwi_inputclk *)tag)->div;
	}

	return clock;
}

static int mmc_omapl1xx_detect(SIM_HBA *hba)
{
	SIM_MMC_EXT		*ext;
	dra446_ext_t	*dra446;
	int				sts;
	CONFIG_INFO		*cfg;
	struct bs_opt *bsopts;

	ext    = (SIM_MMC_EXT *)hba->ext;
	dra446 = (dra446_ext_t *)ext->handle;
	cfg    = (CONFIG_INFO *)&hba->cfg;
	bsopts = &dra446->bsopts;

	sts = in32(bsopts->ins_gpio_vbase + OMAPL1xx_GPIO_IN_DATA); 
	cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, hba->verbosity, 3,"SD0_STATUS: GPIO4_01(wp,cd) = %x\n", sts & 0x3);

	if(sts & bsopts->ins_gpio)		/* No card */
		return MMC_FAILURE;

	if (sts & bsopts->wp_gpio)		/* read only(Write protected) */
		ext->eflags |= MMC_EFLAG_WP;
	else
		ext->eflags &= ~MMC_EFLAG_WP;

	return MMC_SUCCESS;
}



int mmc_omapl1xx_cd_proc_irq(void *area, int irq)
{
	SIM_HBA *hba = area;
	struct bs_opt *bsopts = &((dra446_ext_t*)((SIM_MMC_EXT *)hba->ext)->handle)->bsopts;

	if(bsopts->ins_irq != irq)
		return 0;
	uint32_t intstat = in32(bsopts->ins_gpio_vbase + OMAPL1xx_GPIO_INTSTAT);

	cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, hba->verbosity, 2, "MMCSD cd intstat: 0x%x",intstat);

	/* No gpio interrupt */
	if(!(intstat & bsopts->ins_gpio))
		return 0;

	/* clear the interrupt */
	out32(bsopts->ins_gpio_vbase + OMAPL1xx_GPIO_INTSTAT, bsopts->ins_gpio);

	return 1;
}



static int mmc_omapl1xx_cd_init_irq(SIM_HBA *hba)
{
	CONFIG_INFO		*cfg;
	cfg = (CONFIG_INFO *)&hba->cfg;
	struct bs_opt *bsopts = &((dra446_ext_t*)((SIM_MMC_EXT *)hba->ext)->handle)->bsopts;
	/* disable gpio interrupt */
	uint32_t binten = in32(bsopts->gpio_vbase + OMAPL1xx_GPIO_BINTEN);
	out32(bsopts->gpio_vbase + OMAPL1xx_GPIO_BINTEN,binten & ~(1 << bsopts->ins_bank));

	/* set gpio Upper and lower edge interrupt detection */
	out32(bsopts->ins_gpio_vbase + OMAPL1xx_GPIO_SET_RIS_TRIG, bsopts->ins_gpio);
	out32(bsopts->ins_gpio_vbase + OMAPL1xx_GPIO_SET_FAL_TRIG, bsopts->ins_gpio);
	
	/* Clear Any Pending Interrupt */
	out32(bsopts->ins_gpio_vbase + OMAPL1xx_GPIO_INTSTAT, bsopts->ins_gpio);

	/* Enable Interrupt */
	binten=in32(bsopts->gpio_vbase + OMAPL1xx_GPIO_BINTEN);
	out32(bsopts->gpio_vbase + OMAPL1xx_GPIO_BINTEN,binten | (1 << bsopts->ins_bank));

	cfg->IRQRegisters[cfg->NumIRQs] = bsopts->ins_irq;
	cfg->NumIRQs ++;

	cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, hba->verbosity, 2, "mmcsd init cd check,to use int %d", bsopts->ins_irq);
	return 1;
}

static void mmc_omapl1xx_cd_fini_irq(SIM_HBA *hba)
{
	struct bs_opt *bsopts = &((dra446_ext_t*)((SIM_MMC_EXT *)hba->ext)->handle)->bsopts;

	out32(bsopts->ins_gpio_vbase + OMAPL1xx_GPIO_CLR_RIS_TRIG, bsopts->ins_gpio);
	out32(bsopts->ins_gpio_vbase + OMAPL1xx_GPIO_CLR_FAL_TRIG, bsopts->ins_gpio);

	uint32_t binten = in32(bsopts->gpio_vbase + OMAPL1xx_GPIO_BINTEN);
	out32(bsopts->gpio_vbase + OMAPL1xx_GPIO_BINTEN, binten & ~(1 << bsopts->ins_bank));
}

static void omapl1xx_config(SIM_HBA *hba)
{
	CONFIG_INFO		*cfg;

	cfg = (CONFIG_INFO *)&hba->cfg;

	if (!cfg->NumIOPorts) {
		cfg->IOPort_Base[0]   = OMAPL1xx_MMCSD0_BASE;
		cfg->IOPort_Length[0] = OMAPL1xx_MMCSD_SIZE;
		cfg->IOPort_Base[1]   = OMAPL1xx_EDMA0_CC_BASE;
		cfg->IOPort_Length[1] = OMAPL1xx_EDMA_CC_SIZE;
		cfg->NumIOPorts = 2;
	} else if (cfg->NumIOPorts == 1) {
		cfg->IOPort_Base[1] = OMAPL1xx_EDMA0_CC_BASE;
		cfg->IOPort_Length[1] = OMAPL1xx_EDMA_CC_SIZE;
		cfg->NumIOPorts = 2;
	}

	if (!cfg->NumIRQs) {
		cfg->IRQRegisters[0] = OMAPL1XX_SD_IRQ;
		cfg->NumIRQs = 1;
	}

	if (!cfg->NumDMAs) {
		cfg->DMALst[0] = EDMA_EVENT_SD_RX;
		cfg->DMALst[1] = EDMA_EVENT_SD_TX;
		cfg->NumDMAs = 2;
	} else if (cfg->NumDMAs == 1) {
		cfg->DMALst[1] = cfg->DMALst[0] + 1;
		cfg->NumDMAs = 2;
	}
}

static inline void __attribute__((always_inline))FILLUP_BSOPT(struct bs_opt *bsopts, uintptr_t gpio_base,
							int ins_bank, int ins_no, int ins_irq, int wp_bank, int wp_no,
							int (*check_ins_irq)(void *area, int irq))
{
		bsopts->gpio_vbase = gpio_base;
		bsopts->ins_gpio_vbase = ins_bank;
		bsopts->ins_gpio = ins_no;
		bsopts->ins_irq = ins_irq;
		bsopts->wp_gpio_vbase = wp_bank;
		bsopts->wp_gpio = wp_no;
		bsopts->ins_bank = ins_bank;
		bsopts->check_ins_irq = mmc_omapl1xx_cd_proc_irq;
}

static void get_hwinfo_gpio(struct bs_opt *bsopts)
{
	struct hwi_sdmmc_gpio_info *tag;
	unsigned tag_off, mydev = hwi_find_item(HWI_NULL_OFF, OMAPL1xx_HWI_MMCSD, NULL);
	if(mydev == HWI_NULL_OFF) {
		cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, 2, 2, "unable to find mmcsd hwinfo");
		fprintf(stderr, "hwi_find_tag returned null\n");
		return ;
	}
	tag_off = hwi_find_tag(mydev, 1, "sdmmc_gpio_info");
	if(tag_off == HWI_NULL_OFF) {
		cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, 2, 2, "unable to find mmcsd_gpio hwinfo");
		return ;
	}
	tag = (struct hwi_sdmmc_gpio_info *)hwi_off2tag(tag_off);
	bsopts->gpio_vbase = tag->gpio_info.gpio_base;
	bsopts->wp_gpio_vbase = tag->gpio_info.wp.bank;
	bsopts->wp_gpio = tag->gpio_info.wp.pin;
	bsopts->ins_bank = bsopts->ins_gpio_vbase = tag->gpio_info.ins.bank;
	bsopts->ins_gpio = tag->gpio_info.ins.pin;
	bsopts->ins_irq = tag->gpio_info.ins_irq;
	return ;
}

int bs_init(SIM_HBA *hba)
{
	SIM_MMC_EXT		*ext;
	dra446_ext_t	*dra446;
	CONFIG_INFO		*cfg;
	struct bs_opt  *bsopts;

	cfg = (CONFIG_INFO *)&hba->cfg;
	hba->verbosity = 2;

	omapl1xx_config(hba);
	cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, hba->verbosity, 2, "bs_init: SDC_PORT_BASE = %x", cfg->IOPort_Base[0]);

	if (dra446_attach(hba) != MMC_SUCCESS)
		return MMC_FAILURE;

	ext    = (SIM_MMC_EXT *)hba->ext;
	dra446 = (dra446_ext_t *)ext->handle;
	bsopts = &dra446->bsopts;
	/*fill up with default values for omapl137evm */
	FILLUP_BSOPT(bsopts, OMAPL1xx_GPIO_BASE, 4, 0, 46, 4, 1, mmc_omapl1xx_cd_proc_irq);
	get_hwinfo_gpio(bsopts);
	/*
	 * Comment out the next line if the MMC clock is already in syspage.
	 *  Example of how to put MMC clock into syspage in startup:
	 *    hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DISK, "mmc", 0);
	 *    hwi_add_inputclk(552000000, 6);
	 */
	dra446->mmc_clock = omapl1xx_get_syspage_clk();
	if(!dra446->mmc_clock) {
		dra446->mmc_clock = 300000000 / 2;
	}
	ext->hccap |= MMC_HCCAP_HS;

	if(parse_options(ext->opts, bsopts) == 0) {
		ext->detect = mmc_omapl1xx_detect;
		mmc_omapl1xx_cd_init_irq(hba);
		ext->hccap |= MMC_HCCAP_CD_INTR;
	}

	cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, hba->verbosity, 2, "bs_init: mmc_clock = %d", dra446->mmc_clock);

	return MMC_SUCCESS;
}

int bs_dinit(SIM_HBA *hba)
{
	SIM_MMC_EXT		*ext;
	dra446_ext_t	*dra446;
	struct bs_opt *bsopts;

	ext    = (SIM_MMC_EXT *)hba->ext;
	dra446 = (dra446_ext_t *)ext->handle;
	bsopts = &dra446->bsopts;

	mmc_omapl1xx_cd_fini_irq(hba);

	if (bsopts->gpio_vbase > 0)
		munmap_device_io(bsopts->gpio_vbase, OMAPL1xx_GPIO_SIZE);

	cam_slogf( _SLOGC_CAM_XPT, _SLOG_INFO, hba->verbosity, 2, "bs_dinit: munmap GPIO memeory %d", bsopts->gpio_vbase);

	return (CAM_SUCCESS);
}

__SRCVERSION("$URL$ $Rev$");
