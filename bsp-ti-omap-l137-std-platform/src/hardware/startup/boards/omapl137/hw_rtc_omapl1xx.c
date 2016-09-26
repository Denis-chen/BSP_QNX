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

#include "startup.h"
#include <arm/omap.h>
#include <arm/omapl1xx.h>

unsigned long
rtc_time_omapl1xx(unsigned base)
{
	struct tm	tm;

	// get the current time from the RTC, and convert it to seconds since epoch

	chip_access(base, 0, 0, OMAPL1xx_RTC_SIZE);

	// unlock the write-protection registers for writing
	chip_write32(OMAPL1xx_RTC_KICK0R, OMAPL1xx_RTC_KICK0R_UNLOCK);
	chip_write32(OMAPL1xx_RTC_KICK1R, OMAPL1xx_RTC_KICK1R_UNLOCK);

	// start the RTC, if it's not already running and enable the SPLITPOWER bit as per the Silicon Errata

	chip_write32(OMAP_RTC_CTRL, 0x80 | 0x01);

    // convert BCD to binary

    tm.tm_sec  = bcd2bin(chip_read32(OMAP_RTC_SECONDS) & 0x7f);    // seconds
    tm.tm_min  = bcd2bin(chip_read32(OMAP_RTC_MINUTES) & 0x7f);    // minutes
    tm.tm_hour = bcd2bin(chip_read32(OMAP_RTC_HOURS) & 0x3f);   // hours
    tm.tm_mday = bcd2bin(chip_read32(OMAP_RTC_DAYS) & 0x3f);    // day
    tm.tm_mon  = bcd2bin(chip_read32(OMAP_RTC_MONTHS) & 0x1f);    // month
    tm.tm_year = (bcd2bin(chip_read32(OMAP_RTC_YEARS) & 0xff))+100;   // year

	// lock registers after writing is done
	chip_write32(OMAPL1xx_RTC_KICK0R, 0);
	chip_write32(OMAPL1xx_RTC_KICK1R, 0);

    chip_done();

    return(calc_time_t(&tm));
}
