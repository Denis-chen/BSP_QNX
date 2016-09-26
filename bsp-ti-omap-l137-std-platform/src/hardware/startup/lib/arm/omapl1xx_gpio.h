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

#ifndef __OMAPL1XX_GPIO_H_INCLUDED
#define __OMAPL1XX_GPIO_H_INCLUDED

#include <arm/omapl1xx.h>

#define OMAPL1xx_PER_BANK_GPIO_PINS		16

typedef enum
{
	OMAPL1xx_GPIO_0,
	OMAPL1xx_GPIO_1,
	OMAPL1xx_GPIO_2,
	OMAPL1xx_GPIO_3,
	OMAPL1xx_GPIO_4,
	OMAPL1xx_GPIO_5,
	OMAPL1xx_GPIO_6,
	OMAPL1xx_GPIO_7,
	OMAPL1xx_GPIO_8,
	OMAPL1xx_GPIO_9,
	OMAPL1xx_GPIO_10,
	OMAPL1xx_GPIO_11,
	OMAPL1xx_GPIO_12,
	OMAPL1xx_GPIO_13,
	OMAPL1xx_GPIO_14,
	OMAPL1xx_GPIO_15
	
} omapl1xx_gpio_id_t;

typedef enum
{
	OMAPL1xx_GPIO_BANK_0_1,
	OMAPL1xx_GPIO_BANK_2_3,
	OMAPL1xx_GPIO_BANK_4_5,
	OMAPL1xx_GPIO_BANK_6_7,
	OMAPL1xx_GPIO_BANK_8_9

} omapl1xx_gpio_bank_id_t;

typedef enum
{
	OMAPL1xx_GPIO_BANK_EVEN,
	OMAPL1xx_GPIO_BANK_ODD

} omapl1xx_gpio_bank_t;

typedef enum
{
	OMAPL1xx_GPIO_SUCCESS,
	OMAPL1xx_GPIO_FAILURE

} omapl1xx_gpio_status_t;

typedef enum
{
	OMAPL1xx_GPIO_RESET,
	OMAPL1xx_GPIO_SET,
	OMAPL1xx_GPIO_FAULT

} omapl1xx_gpio_bitval_t;

typedef enum
{
	OMAPL1xx_GPIO_DIR,
	OMAPL1xx_GPIO_OUT_DATA,
	OMAPL1xx_GPIO_SETDATA,
	OMAPL1xx_GPIO_CLRDATA,
	OMAPL1xx_GPIO_INDATA,
	OMAPL1xx_GPIO_SETRISTRIG,
	OMAPL1xx_GPIO_CLRRISTRIG,
	OMAPL1xx_GPIO_SETFALTRIG,
	OMAPL1xx_GPIO__CLRFALTRIG
}ompal1xx_gpio_reg_id_t;

omapl1xx_gpio_status_t omapl1xx_gpio_configure_pin_as_output(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin);
omapl1xx_gpio_status_t omapl1xx_gpio_set_bit(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin);
omapl1xx_gpio_status_t omapl1xx_gpio_reset_bit(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin);

omapl1xx_gpio_status_t omapl1xx_gpio_configure_pin_as_input(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin);
omapl1xx_gpio_bitval_t omapl1xx_gpio_read_pin(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin);

#endif	/* __OMAPL1XX_GPIO_H_INCLUDED */

__SRCVERSION( "$URL$ $REV$" );
