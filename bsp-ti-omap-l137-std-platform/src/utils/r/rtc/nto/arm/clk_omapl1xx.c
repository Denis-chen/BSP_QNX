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

#include "rtc.h"
#include <arm/omap.h>
#include <arm/omapl1xx.h>

#define MILTIME			0x00	/* clock in 24 hour format */

/*
 * Clock setup for the RTC on the TI OMAPL1xx (ARM 926 core).
 * Registers are defined in omap.h and omapl1xx.h.
 */

int
RTCFUNC(init,omapl1xx)(struct chip_loc *chip, char *argv[]) {
    if (chip->phys == NIL_PADDR) {
        chip->phys = OMAPL1xx_RTC_REGS;
    }
    if (chip->access_type == NONE) {
        chip->access_type = MEMMAPPED;
    }
    return OMAPL1xx_RTC_SIZE;
}

int
RTCFUNC(get,omapl1xx)(struct tm *tm, int cent_reg) {
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;

	// unlock registers for writing
	chip_write(OMAPL1xx_RTC_KICK0R, OMAPL1xx_RTC_KICK0R_UNLOCK, 32);
	chip_write(OMAPL1xx_RTC_KICK1R, OMAPL1xx_RTC_KICK1R_UNLOCK, 32);

	// convert BCD to binary 
	sec 	= chip_read(OMAP_RTC_SECONDS,32) & 0x7f;
	min 	= chip_read(OMAP_RTC_MINUTES,32) & 0x7f;	
	hour	= chip_read(OMAP_RTC_HOURS,32) & 0x3f;
	mday	= chip_read(OMAP_RTC_DAYS,32) & 0x3f;
	mon		= chip_read(OMAP_RTC_MONTHS,32) & 0x1f;
	year	= chip_read(OMAP_RTC_YEARS,32)& 0xff;

	// Lock registers again
	chip_write(OMAPL1xx_RTC_KICK0R, 0, 32);
	chip_write(OMAPL1xx_RTC_KICK1R, 0, 32);

	tm->tm_sec 	= BCD2BIN(sec);
	tm->tm_min 	= BCD2BIN(min);
	tm->tm_hour	= BCD2BIN(hour);
	tm->tm_mday	= BCD2BIN(mday);
	tm->tm_mon	= BCD2BIN(mon);
	tm->tm_year	= BCD2BIN(year) + 100;

	return(0);
}

int
RTCFUNC(set,omapl1xx)(struct tm *tm, int cent_reg) {
	unsigned	seconds;
	unsigned	minutes;
	unsigned	hours;
	unsigned	day;
	unsigned	month;
	unsigned	year;

	// convert binary to BCD
	seconds	= BIN2BCD(tm->tm_sec);
	minutes	= BIN2BCD(tm->tm_min);
	hours	= BIN2BCD(tm->tm_hour);
	day 	= BIN2BCD(tm->tm_mday);
	month	= BIN2BCD(tm->tm_mon);
	year	= BIN2BCD(tm->tm_year % 100);

	// unlock registers for writing
	chip_write(OMAPL1xx_RTC_KICK0R, OMAPL1xx_RTC_KICK0R_UNLOCK, 32);
	chip_write(OMAPL1xx_RTC_KICK1R, OMAPL1xx_RTC_KICK1R_UNLOCK, 32);

	chip_write(OMAP_RTC_SECONDS, seconds, 32);
	chip_write(OMAP_RTC_MINUTES, minutes, 32);
	chip_write(OMAP_RTC_HOURS, hours | MILTIME, 32);
	chip_write(OMAP_RTC_WEEKS, tm->tm_wday + 1, 32);	// 1 to 7, BCD is same as binary, so no conversion needed
	chip_write(OMAP_RTC_DAYS, day, 32);	// day of month
	chip_write(OMAP_RTC_MONTHS, month, 32);
	chip_write(OMAP_RTC_YEARS, year, 32);

	// Lock registers again
	chip_write(OMAPL1xx_RTC_KICK0R, 0, 32);
	chip_write(OMAPL1xx_RTC_KICK1R, 0, 32);

	return(0);
}
