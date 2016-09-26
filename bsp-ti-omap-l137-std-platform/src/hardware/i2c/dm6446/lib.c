/*
 * $QNXLicenseC:
 * Copyright 2005-2008, QNX Software Systems.
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
i2c_master_getfuncs(i2c_master_funcs_t *funcs, int tabsize)
{
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, 
            init, dm6446_init, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs,
            fini, dm6446_fini, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs,
            send, dm6446_send, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs,
            recv, dm6446_recv, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs,
            set_slave_addr, dm6446_set_slave_addr, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs,
            set_bus_speed, dm6446_set_bus_speed, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs,
            version_info, dm6446_version_info, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs,
            driver_info, dm6446_driver_info, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs,
            ctl, dm6446_devctl, tabsize);
    return 0;
}
