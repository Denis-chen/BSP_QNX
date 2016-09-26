/*
 * $QNXLicenseC:
 * Copyright 2006-2008, QNX Software Systems.
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


#define DCMD_I2C_READ_SER_CDROM     __DIOTF(_DCMD_I2C, 20, i2c_recv_t) 
#define DCMD_I2C_WRTE_SER_CDROM     __DIOT (_DCMD_I2C, 21, i2c_send_t) 


int devctl_read_serial_cdrom(void *hdl, void *msg, int len);
int devctl_write_serial_cdrom(void *hdl, void *msg, int len);


/**
 * hb_i2c_ctl is the devctl()-callout of the QNX library for every function, which
 * is not implemented inside the I2C-library function-table.
 * 
 * @param *hdl handle to the i2c device-tavle
 * @param cmd command indentifier of the device-control
 * @param *msg  message passed to devctl()
 * @param  msglen  length of message
 * @param  *nbytes return length
 * @param  *info  additional information that could be passed back
 * @return Errorcode (0 = ok, else error)
 */
int
dm6446_devctl (void *hdl, int cmd, void *msg, int msglen, int *nbytes, int *info)
{
    int  retval = -1;
    dm6446_dev_t  *dev = hdl;

    if (NULL != dev) {
        switch (cmd) {
        case (int)DCMD_I2C_READ_SER_CDROM:
            retval = devctl_read_serial_cdrom (hdl, msg, msglen);
            break;
        
        case (int)DCMD_I2C_WRTE_SER_CDROM:
            retval = devctl_write_serial_cdrom (hdl, msg, msglen);
            break;
        
        default:
            if (dev->options & DM6446_OPT_VERBOSE) {
                fprintf(stderr, "devctl: unknown command \n");
            }
            retval = EINVAL;
            break;
        }
    }
    return retval;
}

/**
 * This devctl() implements the command used for reading from a serial cdrom-drive
 * 
 * @param *hdl handle to the i2c device-tavle
 * @param *msg  message passed to devctl()
 * @param  msglen  length of message
 * 
 * @return Errorcode (0 = ok, else error)
 */
int
devctl_read_serial_cdrom (void *hdl, void *msg, int len)
{
    dm6446_dev_t  *dev = hdl;
    i2c_recv_t    *hdr = msg;
    uint8_t       *data;
    uint32_t      status;
    uint32_t      first_byte = 1;

    /* Parse message header:
     *   1 Set address format and store addr in dev struct 
     *   2 Skip the message header and get data pointer
     *   3 Get data length
     */
    dm6446_set_slave_addr(hdl, hdr->slave.addr, hdr->slave.fmt);
    data = msg + sizeof(i2c_recv_t);
    len -= sizeof(i2c_recv_t);
    
    if (len <= 0) 
        return EOK;

    if (-1 == dm6446_wait_bus_not_busy(dev))
        return EIO;

    /* set slave address */
    out32(dev->regbase + DM6446_I2C_ICSAR, dev->slave_addr);

    /* set data count */
    out32(dev->regbase + DM6446_I2C_ICCNT, len);

    /* set start condition */
    out32(dev->regbase + DM6446_I2C_ICMDR, 
          DM6446_I2C_ICMDR_IRS |
          DM6446_I2C_ICMDR_MST |
          DM6446_I2C_ICMDR_STT);

    while (len != 0) {
        status = in32(dev->regbase + DM6446_I2C_ICSTR);
        while(!(status & DM6446_I2C_ICSTR_ICRRDY)) {
            nanospin_ns(1000);
            status = in32(dev->regbase + DM6446_I2C_ICSTR);
        }
        
        *data = in32(dev->regbase + DM6446_I2C_ICDRR) & 0xFF;
        if(first_byte) {
            /* Get the lenght of message sent by CD drive */
            hdr->len = len = *data + 1;
            first_byte = 0; 
        }
            
        data++;
        len--;
    }
    
    out32(dev->regbase + DM6446_I2C_ICMDR, DM6446_I2C_ICMDR_STP);
   
    return EOK;
}

int
devctl_write_serial_cdrom(void *hdl, void *msg, int len)
{
    dm6446_dev_t  *dev = hdl;
    i2c_recv_t    *hdr = msg;
    uint8_t       *data;
    uint32_t      status;

    /* Parse message header:
     *   1 Set address format and store addr in dev struct 
     *   2 Skip the message header and get data pointer
     *   3 Get data length
     */
    dm6446_set_slave_addr(hdl, hdr->slave.addr, hdr->slave.fmt);
    data = msg + sizeof(i2c_send_t);
    len -= sizeof(i2c_send_t);

    if (len <= 0)
        return EOK;

    /* set slave address */
    out32(dev->regbase + DM6446_I2C_ICSAR, dev->slave_addr);

    /* set data count */
    out32(dev->regbase + DM6446_I2C_ICCNT, len);

    if (-1 == dm6446_wait_bus_not_busy(dev)){
        return EIO;
	}

    /* set start condition */
    out32(dev->regbase + DM6446_I2C_ICMDR, 
            DM6446_I2C_ICMDR_IRS | 
            DM6446_I2C_ICMDR_MST |
            DM6446_I2C_ICMDR_TRX | 
            DM6446_I2C_ICMDR_STT |
		    DM6446_I2C_ICMDR_STP);
	
    while (len != 0) {

        status = in32(dev->regbase + DM6446_I2C_ICSTR);
        while(!(status & DM6446_I2C_ICSTR_ICXRDY)) {
            nanospin_ns(1000);
            status = in32(dev->regbase + DM6446_I2C_ICSTR);
        }

        if (dev->options & DM6446_OPT_VERBOSE) {
            fprintf(stderr, "send: status = %xh\n", status);
        }

        //if (ret = dm6446_check_error(dev, status)) 
        //    break;

		out32(dev->regbase + DM6446_I2C_ICDXR, *data);
		++data;
		len--;
    }

    return EOK;
}
