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
#include "omapl1xx_gpio.h"

/* Initialize the GPIO pins which need to be configured for the OMAPL137 Board */

void init_gpio()
{
	/* Configure GPIO pin 15 of Bank 4 as output and drive it high. This pin is connected to USB, to drive VBUS high */
	omapl1xx_gpio_configure_pin_as_output(OMAPL1xx_GPIO_BANK_4_5, OMAPL1xx_GPIO_BANK_EVEN, OMAPL1xx_GPIO_15);
	omapl1xx_gpio_set_bit(OMAPL1xx_GPIO_BANK_4_5, OMAPL1xx_GPIO_BANK_EVEN, OMAPL1xx_GPIO_15);

	/* Configure GPIO pin 15 of Bank 1 as output and drive it high. This pin is connected to USB, to drive ON_BD_USB_DRV high */
	omapl1xx_gpio_configure_pin_as_output(OMAPL1xx_GPIO_BANK_0_1, OMAPL1xx_GPIO_BANK_ODD, OMAPL1xx_GPIO_15);
	omapl1xx_gpio_set_bit(OMAPL1xx_GPIO_BANK_0_1, OMAPL1xx_GPIO_BANK_ODD, OMAPL1xx_GPIO_15);
	
	/* Configure GPIO pins 1 & 2 of Bank 2 as input (SD_WP & SD_INS) */
	omapl1xx_gpio_configure_pin_as_input(OMAPL1xx_GPIO_BANK_2_3, OMAPL1xx_GPIO_BANK_EVEN, OMAPL1xx_GPIO_1);
	omapl1xx_gpio_configure_pin_as_input(OMAPL1xx_GPIO_BANK_2_3, OMAPL1xx_GPIO_BANK_EVEN, OMAPL1xx_GPIO_2);
}
