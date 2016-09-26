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




#include "startup.h"

static const struct arm_cache_config	arm720_cache_config = {
	CACHE_FLAG_UNIFIED|CACHE_FLAG_VIRTUAL,	// flags
	16,										// line_size
	512										// num_lines
};

const struct armv_cache	armv_cache_720 = {
	&arm720_cache_config,	// no cache type register - use static values
	&cache_720,
	0,						// no Icache - use unified cache config
	0						// no Icache - use unified cache callouts
};

__SRCVERSION("armv_cache_720.c $Rev: 392398 $");
