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

omapl1xx_gpio_status_t omapl1xx_gpio_configure_pin_as_output(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin)
{    
	uintptr_t base, reg_addr;

	if(gpio_pin >= OMAPL1xx_PER_BANK_GPIO_PINS)
	{
		return OMAPL1xx_GPIO_FAILURE;
	}
	else
	{
		base = startup_io_map(OMAPL1xx_GPIO_SIZE, OMAPL1xx_GPIO_BASE);
		reg_addr = (base + OMAPL1xx_GPIO_DIR01 + (bank_id * 0x28));
        
		if(bank % 2){
			*(volatile uint32_t *)(reg_addr) &= ~(1 << (gpio_pin + OMAPL1xx_PER_BANK_GPIO_PINS));
		}
		else{
			*(volatile uint32_t *)(reg_addr) &= ~(1 << gpio_pin);
		}
		
		startup_io_unmap(base);
		return OMAPL1xx_GPIO_SUCCESS;		
	}
}

omapl1xx_gpio_status_t omapl1xx_gpio_configure_pin_as_input(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin)
{
	uintptr_t base, reg_addr;

	if(gpio_pin >= OMAPL1xx_PER_BANK_GPIO_PINS)
	{
		return OMAPL1xx_GPIO_FAILURE;
	}
	else
	{
		base = startup_io_map(OMAPL1xx_GPIO_SIZE, OMAPL1xx_GPIO_BASE);
		reg_addr = (base + OMAPL1xx_GPIO_DIR01 + (bank_id * 0x28));

		if(bank % 2){
			*(volatile uint32_t *)(reg_addr) |= (1 << (gpio_pin + OMAPL1xx_PER_BANK_GPIO_PINS));
		}
		else{
			*(volatile uint32_t *)(reg_addr) |= (1 << gpio_pin);
		}
        
		startup_io_unmap(base);
		return OMAPL1xx_GPIO_SUCCESS;		
	}
}

omapl1xx_gpio_status_t omapl1xx_gpio_set_bit(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin)
{
	uintptr_t base, reg_addr;

	if(gpio_pin >= OMAPL1xx_PER_BANK_GPIO_PINS)
	{
		return OMAPL1xx_GPIO_FAILURE;
	}
	else
	{
		base = startup_io_map(OMAPL1xx_GPIO_SIZE, OMAPL1xx_GPIO_BASE);
		reg_addr = (base + OMAPL1xx_GPIO_SET_DATA01 + (bank_id * 0x28));

		if(bank % 2){
			*(volatile uint32_t *)(reg_addr) |= (1 << (gpio_pin + OMAPL1xx_PER_BANK_GPIO_PINS));
		}
		else{
			*(volatile uint32_t *)(reg_addr) |= (1 << gpio_pin);
		}
        
		startup_io_unmap(base);
		return OMAPL1xx_GPIO_SUCCESS;		
	}
}

omapl1xx_gpio_status_t omapl1xx_gpio_reset_bit(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin)
{
	uintptr_t base, reg_addr;

	if(gpio_pin >= OMAPL1xx_PER_BANK_GPIO_PINS)
	{
		return OMAPL1xx_GPIO_FAILURE;
	}
	else
	{
		base = startup_io_map(OMAPL1xx_GPIO_SIZE, OMAPL1xx_GPIO_BASE);
		reg_addr = (base + OMAPL1xx_GPIO_CLR_DATA01 + (bank_id * 0x28));

		if(bank % 2){
			*(volatile uint32_t *)(reg_addr) |= (1 << (gpio_pin + OMAPL1xx_PER_BANK_GPIO_PINS));
		}
		else{
			*(volatile uint32_t *)(reg_addr) |= (1 << gpio_pin);
		}
        
		startup_io_unmap(base);
		return OMAPL1xx_GPIO_SUCCESS;		
	}
}

omapl1xx_gpio_bitval_t omapl1xx_gpio_read_pin(omapl1xx_gpio_bank_id_t bank_id, omapl1xx_gpio_bank_t bank, omapl1xx_gpio_id_t gpio_pin)
{
	uintptr_t base, reg_addr;

	if(gpio_pin >= OMAPL1xx_PER_BANK_GPIO_PINS)
	{
		return OMAPL1xx_GPIO_FAULT;
	}
	else
	{
		base = startup_io_map(OMAPL1xx_GPIO_SIZE, OMAPL1xx_GPIO_BASE);
		reg_addr = (base + OMAPL1xx_GPIO_IN_DATA01 + (bank_id * 0x28));

		if(bank % 2){
			if((*(volatile uint32_t *)reg_addr) & (1 << (gpio_pin + OMAPL1xx_PER_BANK_GPIO_PINS))){
				startup_io_unmap(base);
				return OMAPL1xx_GPIO_SET;
			}
			else{
				startup_io_unmap(base);
				return OMAPL1xx_GPIO_RESET;
	        }
		}
		else{
			if((*(volatile uint32_t *)reg_addr) & (1 << gpio_pin)){
				startup_io_unmap(base);
				return OMAPL1xx_GPIO_SET;
			}
			else{
				startup_io_unmap(base);
				return OMAPL1xx_GPIO_RESET;
			}	
		}
	}
}

