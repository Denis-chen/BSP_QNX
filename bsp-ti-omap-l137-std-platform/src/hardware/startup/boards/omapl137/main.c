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
#include <arm/omapl1xx.h>

extern struct callout_rtn reboot_omapl1xx;
extern void allocate_dsplink_memory(paddr_t, size_t); /* hardware/startup/boards/omapl137/init_hwinfo.c */
extern void allocate_dsp_memory(paddr_t*, size_t*, int);  /* hardware/startup/boards/omapl137/init_hwinfo.c */
extern unsigned long omapl137_cpu_clock(void);  /* hardware/startup/boards/omapl137/init_hwinfo.c */
extern void init_gpio(); /* hardware/startup/boards/omapl137/init_gpio.c */

const struct callout_slot callouts[] = {
	{CALLOUT_SLOT(reboot, _omapl1xx)},
};

const struct debug_device debug_devices[] = {
	{"8250",
		{"0x01d0d000^2.115200.150000000.16",	/* UART 2 on EVM : use default baud rate */
		},
		init_8250,
		put_8250,
		{&display_char_8250,
			&poll_key_8250,
			&break_detect_8250,
		}
	},
};

/*
 * main()
 *	Startup program executing out of RAM
 *
 * 1. It gathers information about the system and places it in a structure
 *    called the system page. The kernel references this structure to
 *    determine everything it needs to know about the system. This structure
 *    is also available to user programs (read only if protection is on)
 *    via _syspage->.
 *
 * 2. It (optionally) turns on the MMU and starts the next program
 *    in the image file system.
 */
int
main(int argc, char **argv, char **envv)
{
	int     opt;
	char   *p;
	paddr_t resmem_addr[32];
	size_t  resmem_size[32];
	int     dsp_mem_count = 0;
	paddr_t linkmem_addr = 0;
	size_t  linkmem_size = 0;
	int     link_present = 0;

	add_callout_array(callouts, sizeof(callouts));

	while ((opt = getopt(argc, argv, COMMON_OPTIONS_STRING "x:L:E")) != -1)
	{
		switch (opt)
		{
		case 'x':
			resmem_addr[dsp_mem_count] = getsize(optarg, &p);
			if (*p == ',')
			{
				resmem_size[dsp_mem_count] = getsize(p + 1, &p);
			}
			dsp_mem_count++;
			break;
		case 'L':
			linkmem_addr = getsize(optarg, &p);
			if (*p == ',')
			{
				linkmem_size = getsize(p + 1, &p);
			}
			link_present = 1;
			break;
		default:
			handle_common_option(opt);
			break;
		}
	}

	/*
	 * Initialise debugging output
	 * UART uses 1/2 sysclk rate as clock ~150MHz
	*/
	select_debug(debug_devices, sizeof(debug_devices));

	/*
	 * Collect information on all free RAM in the system
	 */
	init_raminfo();

	/* These allocations MUST be completed in this order */
	if (link_present)
		allocate_dsplink_memory(linkmem_addr, linkmem_size);

	if (dsp_mem_count > 0)
		allocate_dsp_memory(resmem_addr, resmem_size, dsp_mem_count);
	/* End allocations */

	/*
	 * calculate CPU frequency
	 */
	cpu_freq = omapl137_cpu_clock();
	kprintf("CPU Frequency is %d Hz\n", cpu_freq);

	/* Remove RAM used by modules in the image */
	alloc_ram(shdr->ram_paddr, shdr->ram_size, 1);
	if (shdr->flags1 & STARTUP_HDR_FLAGS1_VIRTUAL)
		init_mmu();

	/* Initialize the Interrupts related Information */
	init_intrinfo();

	/* Initialize the Cache related information */
	init_cacheattr();

	/* Initialize the CPU related information */
	init_cpuinfo();

	/* Initialize the Hwinfo section of the Syspage */
	init_hwinfo();

	/* Initialize the Timer related information */
	init_qtime();

	/* Initialize the Gpio pins which are used in the board */
	init_gpio();

	add_typed_string(_CS_MACHINE, "TI_OMAP_L137");

	/*
	 * Load bootstrap executables in the image file system and Initialise
	 * various syspage pointers. This must be the _last_ initialisation done
	 * before transferring control to the next program.
	 */
	init_system_private();

	/*
	 * This is handy for debugging a new version of the startup program.
	 * Commenting this line out will save a great deal of code.
	 */
	print_syspage();

	return 0;
}

