/*
 * $QNXLicenseC:
 * Copyright 2005, 2007-2008, QNX Software Systems.
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




#include "proto.h"

int
dm6446_options(dm6446_dev_t *dev, int argc, char *argv[])
{
    int     c;
    int     prev_optind;
    int     done = 0;

    /* defaults */
    dev->intr = DM6446_I2C_IRQ;
    dev->iid = -1;
    dev->physbase = DM6446_I2C_BASE;
    dev->reglen = DM6446_I2C_SIZE;
    dev->own_addr = DM6446_I2C_ADDRESS;
	dev->input_clk = DM6446_I2C_INPUT_CLOCK;   /*as input clock is board specific so added here as an option.*/
    dev->slave_addr = 0x38; /* IO Controller */
    dev->options = 0;
	dev->stop = 1;

    while (!done) {
        prev_optind = optind;
        c = getopt(argc, argv, "a:i:p:f:l:s:v");
        switch (c) {
        case 'a':
            dev->own_addr = strtoul(optarg, &optarg, NULL);
            break;

        case 'i':
            dev->intr = strtol(optarg, &optarg, NULL);
            break;

        case 'p':
            dev->physbase = strtoul(optarg, &optarg, NULL);
            break;

		case 'f':
            dev->input_clk = strtoul(optarg, &optarg, NULL);
	    	break;

		case 'l':
            dev->reglen = strtoul(optarg, &optarg, NULL);
	    	break;
	
        case 's':
            dev->slave_addr = strtoul(optarg, &optarg, NULL);
            break;

        case 'v':
            dev->options |= DM6446_OPT_VERBOSE;
            break;

        case '?':
            if (optopt == '-') {
                ++optind;
                break;
            }
            return -1;

        case -1:
            if (prev_optind < optind) /* -- */
                return -1;

            if (argv[optind] == NULL) {
                done = 1;
                break;
            } 
            if (*argv[optind] != '-') {
                ++optind;
                break;
            }
            return -1;

        case ':':
        default:
            return -1;
        }
    }

    return 0;
}
