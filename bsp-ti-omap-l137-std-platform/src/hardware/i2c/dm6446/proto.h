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




#ifndef __PROTO_H_INCLUDED
#define __PROTO_H_INCLUDED

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include <hw/i2c.h>
#include <arm/dm6446.h>
#include <arm/omapl1xx.h>

typedef struct _dm6446_dev {
    unsigned            reglen;
    uintptr_t           regbase;
    unsigned            physbase;
    unsigned            input_clk;

    int                 intr;
    int                 iid;
    struct sigevent     intrevent;

    unsigned            own_addr;
    unsigned            slave_addr;
	unsigned			addr_fmt;
    unsigned            options;
	unsigned			expected_speed;
	unsigned			bus_speed;

	unsigned			txrx;
#define	JACINTO_I2C_IDLE	0
#define	JACINTO_I2C_TX		1
#define	JACINTO_I2C_RX		2
	unsigned			tot_len, cur_len;
	uint8_t				*buf;
	unsigned			stop;
	unsigned			revid;

    struct {
        unsigned char   major;
        unsigned char   minor;
    } rev;
} dm6446_dev_t;

#define DM6446_OPT_VERBOSE        0x00000002

#define DM6446_I2C_IRQ            39
#define DM6446_I2C_ADDRESS        1
#define DM6446_I2C_INPUT_CLOCK	 27000000
#define DM6446_PSC_VALUE			 2


#define DM6446_I2C_REVMAJOR(rev)      (((rev) >> 8) & 0xff)
#define DM6446_I2C_REVMINOR(rev)      ((rev) & 0xff)

void *dm6446_init(int argc, char *argv[]);
void dm6446_fini(void *hdl);
int dm6446_options(dm6446_dev_t *dev, int argc, char *argv[]);

int dm6446_set_slave_addr(void *hdl, unsigned int addr, i2c_addrfmt_t fmt);
int dm6446_set_bus_speed(void *hdl, unsigned int speed, unsigned int *ospeed);
int dm6446_version_info(i2c_libversion_t *version);
int dm6446_driver_info(void *hdl, i2c_driver_info_t *info);
int dm6446_devctl(void *hdl, int cmd, void *msg, int msglen,
        int *nbytes, int *info);
i2c_status_t dm6446_recv(void *hdl, void *buf, unsigned int len, unsigned int stop);
i2c_status_t dm6446_send(void *hdl, void *buf, unsigned int len, unsigned int stop);

int dm6446_wait_bus_not_busy(dm6446_dev_t *dev);
uint32_t dm6446_wait_complete(dm6446_dev_t *dev);
int	dm6446_attach_intr(dm6446_dev_t *dev);

#endif
