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
#include "hwinfo_private.h"
#include "arm/omapl1xx.h"
#include <drvr/hwinfo.h>		// for hwi support routines in libdrvr
#include <hw/hwinfo_omapl1xx.h>

#define	USB_1_1_GPIO_BANK	2
#define	USB_1_1_GPIO_PIN	4

/*
 * Add L138 specific devices to the hardware info section of the syspage
*/

extern unsigned char mac_addr[6];
extern unsigned long cpu_freq;

struct hwi_usb_gpio	{
	struct hwi_prefix	prefix;
	union gpio_info_t{
		_Uint32t	gpio_bank;
		_Uint32t	gpio_pin;
	}gpio_info;
};

struct gpio_t {
	unsigned int bank;
	unsigned int pin;
};

struct hwi_sdmmc_gpio_info {
	struct hwi_prefix	prefix;
	struct sdmmc_gpio_info_t {
		struct gpio_t wp;
		struct gpio_t ins;
		uintptr_t gpio_base;
		unsigned int ins_irq;
	} gpio_info;
};

void hwi_omapl138()
{
	/* add an Ethernet device */
	{
		unsigned hwi_off;
		hwiattr_enet_t enet_attr = HWIATTR_ENET_T_INITIALIZER;

		/* if BSP wants to provide a MAC address, add in the tag */
		HWIATTR_ENET_SET_MAC(&enet_attr, mac_addr);

		HWIATTR_ENET_SET_NUM_IRQ(&enet_attr, 2);
		HWIATTR_ENET_SET_NUM_PHY(&enet_attr, 1);
		HWIATTR_ENET_SET_DLL(&enet_attr, "devn-dm644x-omapl1xx");
		
		/* create device and set the PHY address */
		HWIATTR_ENET_SET_LOCATION(&enet_attr, OMAPL1xx_ETH_BASE, OMAPL1xx_ETH_SIZE, 0, hwi_find_as(OMAPL1xx_ETH_BASE, 1));
		hwi_off = hwidev_add_enet(OMAPL1xx_HWI_ENET, &enet_attr, HWI_NULL_OFF);
		ASSERT(hwi_off != HWI_NULL_OFF);
		/* By default we assume PHY's are connected in numeric order. If this is not
		 * the case, then correct it in the board specific init_hwinfo() */
		hwitag_set_phyaddr(hwi_off, 0, 0);

		/* Assign interrupt vectors to the Ethernet device */
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_ENET, 0), 0, 34);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_ENET, 0), 1, 35);		
	}

	/* add 3 UART's */
	{
		unsigned hwi_off;
		hwiattr_uart_t uart_attr = HWIATTR_UART_T_INITIALIZER;
		/* The UART0 has a clock of PLL0_SYSCLK2, UART 1/2 have the ASYNC3 clock which by default is PLL0_SYSCLK2 only */
		struct hwi_inputclk clksrc = {.clk = cpu_freq, .div = 2};
		
		/* each UART has an interrupt */
		HWIATTR_UART_SET_NUM_IRQ(&uart_attr, 1);
		HWIATTR_UART_SET_NUM_CLK(&uart_attr, 1);
		
		/* create uart0 and set the clock source */
		HWIATTR_UART_SET_LOCATION(&uart_attr, OMAPL1xx_UART0_BASE, OMAPL1xx_UART_SIZE, 0, hwi_find_as(OMAPL1xx_UART0_BASE, 1));
		hwi_off = hwidev_add_uart(OMAPL1xx_HWI_UART, &uart_attr, HWI_NULL_OFF);
		ASSERT(hwi_find_unit(hwi_off) == 0);
		hwitag_set_inputclk(hwi_off, 0, &clksrc);

		/* create uart1 and set the clock source */
		HWIATTR_UART_SET_LOCATION(&uart_attr, OMAPL1xx_UART1_BASE, OMAPL1xx_UART_SIZE, 0, hwi_find_as(OMAPL1xx_UART1_BASE, 1));
		hwi_off = hwidev_add_uart(OMAPL1xx_HWI_UART, &uart_attr, HWI_NULL_OFF);
		ASSERT(hwi_find_unit(hwi_off) == 1);
		hwitag_set_inputclk(hwi_off, 0, &clksrc);

		/* create uart2 and set the clock source */
		HWIATTR_UART_SET_LOCATION(&uart_attr, OMAPL1xx_UART2_BASE, OMAPL1xx_UART_SIZE, 0, hwi_find_as(OMAPL1xx_UART2_BASE, 1));
		hwi_off = hwidev_add_uart(OMAPL1xx_HWI_UART, &uart_attr, HWI_NULL_OFF);
		ASSERT(hwi_find_unit(hwi_off) == 2);
		hwitag_set_inputclk(hwi_off, 0, &clksrc);

		/* Assign interrupt vectors to the 3 UART devices */
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_UART, 0), -1, 25);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_UART, 1), -1, 53);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_UART, 2), -1, 61);	
	}

	/* add 2 SPI's */
	{
		unsigned hwi_off;
		hwiattr_spi_t spi_attr = HWIATTR_SPI_T_INITIALIZER;
		/* The SPI0 has a clock of PLL0_SYSCLK2, SPI1 has the ASYNC3 clock which by default is PLL0_SYSCLK2 only */
		struct hwi_inputclk clksrc = {.clk = cpu_freq, .div = 2};
		
		/* each SPI has an interrupt */
		HWIATTR_SPI_SET_NUM_IRQ(&spi_attr, 1);
		HWIATTR_SPI_SET_NUM_CLK(&spi_attr, 1);
		
		/* create spi0 and set the clock source */
		HWIATTR_SPI_SET_LOCATION(&spi_attr, OMAPL1xx_SPI0_BASE, OMAPL1xx_SPI0_SIZE, 0, hwi_find_as(OMAPL1xx_SPI0_BASE, 1));
		hwi_off = hwibus_add_spi(HWI_NULL_OFF, &spi_attr);
		ASSERT(hwi_find_unit(hwi_off) == 0);
		hwitag_set_inputclk(hwi_off, 0, &clksrc);

		/* create spi1 and set the clock source */
		HWIATTR_SPI_SET_LOCATION(&spi_attr, OMAPL1xx_SPI1_BASE, OMAPL1xx_SPI1_SIZE, 0, hwi_find_as(OMAPL1xx_SPI1_BASE, 1));
		hwi_off = hwibus_add_spi(HWI_NULL_OFF, &spi_attr);
		ASSERT(hwi_find_unit(hwi_off) == 1);
		hwitag_set_inputclk(hwi_off, 0, &clksrc);

		/* Assign interrupt vectors to the 2 SPI buses */
		hwitag_set_avail_ivec(hwi_find_bus(OMAPL1xx_HWI_SPI, 0), -1, 20);
		hwitag_set_avail_ivec(hwi_find_bus(OMAPL1xx_HWI_SPI, 1), -1, 56);	
	}

	/* add 2 I2C's */
	{
		unsigned hwi_off;
		hwiattr_i2c_t i2c_attr = HWIATTR_I2C_T_INITIALIZER;
		/* The I2C0 has a clock of AUXCLK, I2C1 has the PLL0_SYSCLK4 */
		struct hwi_inputclk clksrc0 = {.clk = OMAPL1xx_PLL_INPUT_CLK, .div = 1};
		struct hwi_inputclk clksrc1 = {.clk = cpu_freq, .div = 4};
		
		/* each I2C has an interrupt */
		HWIATTR_I2C_SET_NUM_IRQ(&i2c_attr, 1);
		HWIATTR_I2C_SET_NUM_CLK(&i2c_attr, 1);
		
		/* create i2c0 and set the clock source */
		HWIATTR_I2C_SET_LOCATION(&i2c_attr, OMAPL1xx_I2C0_BASE, OMAPL1xx_I2C0_SIZE, 0, hwi_find_as(OMAPL1xx_I2C0_BASE, 1));
		hwi_off = hwibus_add_i2c(HWI_NULL_OFF, &i2c_attr);
		ASSERT(hwi_find_unit(hwi_off) == 0);
		hwitag_set_inputclk(hwi_off, 0, &clksrc0);

		/* create i2c1 and set the clock source */
		HWIATTR_I2C_SET_LOCATION(&i2c_attr, OMAPL1xx_I2C1_BASE, OMAPL1xx_I2C1_SIZE, 0, hwi_find_as(OMAPL1xx_I2C1_BASE, 1));
		hwi_off = hwibus_add_i2c(HWI_NULL_OFF, &i2c_attr);
		ASSERT(hwi_find_unit(hwi_off) == 1);
		hwitag_set_inputclk(hwi_off, 0, &clksrc1);

		/* Assign interrupt vectors to the 2 I2C buses */
		hwitag_set_avail_ivec(hwi_find_bus(OMAPL1xx_HWI_I2C, 0), -1, 15);
		hwitag_set_avail_ivec(hwi_find_bus(OMAPL1xx_HWI_I2C, 1), -1, 51);	
	}

	/* add 2 USB controllers */
	{
		unsigned hwi_off;
		struct hwi_usb_gpio	*tag;

		hwiattr_usb_t usb_attr0 = HWIATTR_USB_T_INITIALIZER;
		hwiattr_usb_t usb_attr1 = HWIATTR_USB_T_INITIALIZER;

		HWIATTR_USB_SET_NUM_IRQ(&usb_attr0, 1);
		HWIATTR_USB_SET_NUM_IRQ(&usb_attr1, 2);
		
		/* create usb0 */
		HWIATTR_USB_SET_LOCATION(&usb_attr0, OMAPL1xx_USB0_BASE, OMAPL1xx_USB0_SIZE, 0, hwi_find_as(OMAPL1xx_USB0_BASE, 1));
		hwi_off = hwibus_add_usb(HWI_NULL_OFF, &usb_attr0);
		ASSERT(hwi_find_unit(hwi_off) == 0);
		/* The USB0 (USB 2.0) has a clock of AUXCLK */
		hwitag_add_inputclk(hwi_off, OMAPL1xx_PLL_INPUT_CLK, 1);

		/* create usb1 */
		HWIATTR_USB_SET_LOCATION(&usb_attr1, OMAPL1xx_USB1_BASE, OMAPL1xx_USB1_SIZE, 0, hwi_find_as(OMAPL1xx_USB1_BASE, 1));
		hwi_off = hwibus_add_usb(HWI_NULL_OFF, &usb_attr1);
		ASSERT(hwi_find_unit(hwi_off) == 1);
		/* The USB1 (USB 1.1) has a clock derived from USB 2.0 PHY which is 48MHz */
		hwitag_add_inputclk(hwi_off, (OMAPL1xx_PLL_INPUT_CLK * 2), 1);
		tag = hwi_alloc_tag("gpio_bank", sizeof(struct hwi_usb_gpio), 4);
		tag->gpio_info.gpio_bank = USB_1_1_GPIO_BANK;
		tag = hwi_alloc_tag("gpio_pin", sizeof(struct hwi_usb_gpio), 4);
		tag->gpio_info.gpio_pin = USB_1_1_GPIO_PIN;

		/* Assign interrupt vectors to the 2 USB buses */
		hwitag_set_avail_ivec(hwi_find_bus(OMAPL1xx_HWI_USB, 0), -1, 58);
		
		hwitag_set_avail_ivec(hwi_find_bus(OMAPL1xx_HWI_USB, 1), 0, 59);
		hwitag_set_avail_ivec(hwi_find_bus(OMAPL1xx_HWI_USB, 1), 1, 60);		
	}

	/* add the RTC device */
	{
		unsigned hwi_off;
		hwiattr_rtc_t rtc_attr = HWIATTR_RTC_T_INITIALIZER;

		/* the rtc has 1 interrupt */
		HWIATTR_RTC_SET_NUM_IRQ(&rtc_attr, 1);

		HWIATTR_RTC_SET_LOCATION(&rtc_attr, OMAPL1xx_RTC_REGS, OMAPL1xx_RTC_SIZE, 0, hwi_find_as(OMAPL1xx_RTC_REGS, 1));
		hwi_off = hwidev_add_rtc(OMAPL1xx_HWI_RTC, &rtc_attr, HWI_NULL_OFF);
		ASSERT(hwi_find_unit(hwi_off) == 0);
		/* The RTC operates off of a dedicated 32 KHz crystal oscillator */
		hwitag_add_inputclk(hwi_off, OMAPL1xx_RTC_CLOCK, 1);

		/* Assign interrupt vectors to the RTC device */
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_RTC, 0), -1, 19);
	}

	/* add the GPIO module */
	{
		unsigned hwi_off;
		hwiattr_common_t attr = HWIATTR_COMMON_INITIALIZER;

		HWIATTR_SET_NUM_IRQ(&attr, 9);
		HWIATTR_SET_LOCATION(&attr, OMAPL1xx_GPIO_BASE, OMAPL1xx_GPIO_SIZE, 0, hwi_find_as(OMAPL1xx_GPIO_BASE, 1));

 		hwi_off = hwidev_add(OMAPL1xx_HWI_GPIO, 0, HWI_NULL_OFF);
		ASSERT(hwi_off != HWI_NULL_OFF);
		hwitag_add_common(hwi_off, &attr);
		/* The GPIO operates with a clock of PLL0_SYSCLK4 which is 300MHz / 4 = 75MHz */
		hwitag_add_inputclk(hwi_off, cpu_freq, 4);

		/* Assign interrupt vectors to the GPIO device */
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 0, 42);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 1, 43);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 2, 44);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 3, 45);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 4, 46);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 5, 47);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 6, 48);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 7, 49);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_GPIO, 0), 8, 50);
	}

	/* add the Timers module */
	{
		unsigned hwi_off;
		hwiattr_timer_t timer_attr0 = HWIATTR_TIMER_T_INITIALIZER;
		hwiattr_timer_t timer_attr2_3 = HWIATTR_TIMER_T_INITIALIZER;
		/* Timer 0 is clocked by the AUXCLK */
		struct hwi_inputclk clksrc_timer0 = {.clk = OMAPL1xx_PLL_INPUT_CLK, .div = 1};
		/* Timers 2 and 3 are clocked by the ASYNC3 clock which is PLL0_SYSCLK2 by default */
		struct hwi_inputclk clksrc_timers_2_3 = {.clk = cpu_freq, .div = 2};

		HWIATTR_TIMER_SET_NUM_IRQ(&timer_attr0, 2);
		HWIATTR_TIMER_SET_NUM_CLK(&timer_attr0, 1);

		HWIATTR_TIMER_SET_NUM_IRQ(&timer_attr2_3, 1);
		HWIATTR_TIMER_SET_NUM_CLK(&timer_attr2_3, 1);

		/* timer 0 */
		HWIATTR_TIMER_SET_LOCATION(&timer_attr0, OMAPL1xx_TMR_1_REGS, OMAPL1xx_TIMER_SIZE, 0, hwi_find_as(OMAPL1xx_TMR_1_REGS, 1));
		hwi_off = hwidev_add_timer(OMAPL1xx_HWI_TIMER, &timer_attr0, HWI_NULL_OFF);
		ASSERT(hwi_find_unit(hwi_off) == 0);
		hwitag_set_inputclk(hwi_off, 0, &clksrc_timer0);

		/* timer 1 is watchdog, which is added seperately in the hwinfo section */

		/* timer 2 */
		HWIATTR_TIMER_SET_LOCATION(&timer_attr2_3, OMAPL1xx_TMR_3_REGS, OMAPL1xx_TIMER_SIZE, 0, hwi_find_as(OMAPL1xx_TMR_3_REGS, 1));
		hwi_off = hwidev_add_timer(OMAPL1xx_HWI_TIMER, &timer_attr2_3, HWI_NULL_OFF);
		ASSERT(hwi_find_unit(hwi_off) == 1);
		hwitag_set_inputclk(hwi_off, 0, &clksrc_timers_2_3);

		/* timer 3 */
		HWIATTR_TIMER_SET_LOCATION(&timer_attr2_3, OMAPL1xx_TMR_4_REGS, OMAPL1xx_TIMER_SIZE, 0, hwi_find_as(OMAPL1xx_TMR_4_REGS, 1));
		hwi_off = hwidev_add_timer(OMAPL1xx_HWI_TIMER, &timer_attr2_3, HWI_NULL_OFF);
		ASSERT(hwi_find_unit(hwi_off) == 2);
		hwitag_set_inputclk(hwi_off, 0, &clksrc_timers_2_3);

		/* Assign interrupt vectors to the Timers 1, 3, 4 */
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_TIMER, 0), 0, 21);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_TIMER, 0), 1, 22);

		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_TIMER, 1), -1, 68);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_TIMER, 2), -1, 96);	
	}

	/* add the WATCHDOG device */
	{
		unsigned hwi_off;
		hwiattr_common_t attr = HWIATTR_COMMON_INITIALIZER;

		HWIATTR_SET_LOCATION(&attr, OMAPL1xx_TMR_2_REGS, OMAPL1xx_TIMER_SIZE, 0, hwi_find_as(OMAPL1xx_TMR_2_REGS, 1));
		HWIATTR_SET_NUM_IRQ(&attr, 2);

		hwi_off = hwidev_add(OMAPL1xx_HWI_WDOG, hwi_devclass_TIMER, HWI_NULL_OFF);
		ASSERT(hwi_off != HWI_NULL_OFF);

		hwitag_add_common(hwi_off, &attr);
		/* Timer 1 (Watchdog) is clocked by the AUXCLK */
		hwitag_add_inputclk(hwi_off, OMAPL1xx_PLL_INPUT_CLK, 1);

		/* Assign interrupt vectors to the Watchdog timer (Timer 2) */
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_WDOG, 0), 0, 23);
		hwitag_set_avail_ivec(hwi_find_device(OMAPL1xx_HWI_WDOG, 0), 1, 24);	
	}

	/* add 2 dma controllers (32 channels each). The EDMA is clocked by PLL0_SYSCLK2.
	 * The EDMA0 controller has 1 channel controller and 2 transfer controllers
	 * The EDMA1 controller has 1 channel controller and 1 transfer controller */
	{
		/* Add EDMA0 Channel Controller */
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DMA, OMAPL1xx_HWI_EDMA0_CC, 0);
		hwi_add_location(OMAPL1xx_EDMA0_CC_BASE, OMAPL1xx_EDMA_CC_SIZE, 0, hwi_find_as(OMAPL1xx_EDMA0_CC_BASE, 1));
		hwi_add_inputclk(cpu_freq, 2);
		hwi_add_irq(11);
		hwi_add_irq(12);

		/* Add EDMA0 Transfer Controller 0 */
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DMA, OMAPL1xx_HWI_EDMA0_TC0, 0);
		hwi_add_location(OMAPL1xx_EDMA0_TC0_BASE, OMAPL1xx_EDMA_TC_SIZE, 0, hwi_find_as(OMAPL1xx_EDMA0_TC0_BASE, 1));
		hwi_add_inputclk(cpu_freq, 2);
		hwi_add_irq(13);

		/* Add EDMA0 Transfer Controller 1 */
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DMA, OMAPL1xx_HWI_EDMA0_TC1, 0);
		hwi_add_location(OMAPL1xx_EDMA0_TC1_BASE, OMAPL1xx_EDMA_TC_SIZE, 0, hwi_find_as(OMAPL1xx_EDMA0_TC1_BASE, 1));
		hwi_add_inputclk(cpu_freq, 2);
		hwi_add_irq(32);

		/* Add EDMA1 Channel Controller */
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DMA, OMAPL1xx_HWI_EDMA1_CC, 0);
		hwi_add_location(OMAPL1xx_EDMA1_CC_BASE, OMAPL1xx_EDMA_CC_SIZE, 0, hwi_find_as(OMAPL1xx_EDMA1_CC_BASE, 1));
		hwi_add_inputclk(cpu_freq, 2);
		hwi_add_irq(93);
		hwi_add_irq(94);

		/* Add EDMA1 Transfer Controller 0 */
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DMA, OMAPL1xx_HWI_EDMA1_TC0, 0);
		hwi_add_location(OMAPL1xx_EDMA1_TC0_BASE, OMAPL1xx_EDMA_TC_SIZE, 0, hwi_find_as(OMAPL1xx_EDMA1_TC0_BASE, 1));
		hwi_add_inputclk(cpu_freq, 2);
		hwi_add_irq(95);
	}

	/* Add the MMCSD module. The MMCSD is clocked by PLL0_SYSCLK2. */
	{
		struct hwi_sdmmc_gpio_info *tag;
		/* Add MMCSD0 Controller */
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DISK, OMAPL1xx_HWI_MMCSD, 0);
		hwi_add_location(OMAPL1xx_MMCSD0_BASE, OMAPL1xx_MMCSD_SIZE, 0, hwi_find_as(OMAPL1xx_MMCSD0_BASE, 1));
		hwi_add_inputclk(cpu_freq, 2);
		hwi_add_irq(16);
		hwi_add_irq(17);

		tag = hwi_alloc_tag("sdmmc_gpio_info", sizeof(struct hwi_sdmmc_gpio_info), 4);
		tag->gpio_info.gpio_base = OMAPL1xx_GPIO_BASE;
		tag->gpio_info.wp.bank = 4;
		tag->gpio_info.wp.pin = 1;
		tag->gpio_info.ins.bank = 4;
		tag->gpio_info.ins.pin = 0;
		tag->gpio_info.ins_irq = 46;

		/* Add MMCSD1 Controller */
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DISK, OMAPL1xx_HWI_MMCSD, 0);
		hwi_add_location(OMAPL1xx_MMCSD1_BASE, OMAPL1xx_MMCSD_SIZE, 0, hwi_find_as(OMAPL1xx_MMCSD1_BASE, 1));
		hwi_add_inputclk(cpu_freq, 2);
		hwi_add_irq(72);
		hwi_add_irq(73);
	}

	/* Add the SATA module. The SATA is clocked by peripheral serial clock which is PLL0_SYSCLK2. */
	{
		/* Add the SATA Controller */
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DISK, OMAPL1xx_HWI_SATA, 0);
		hwi_add_location(OMAPL1xx_SATA_BASE, OMAPL1xx_SATA_SIZE, 0, hwi_find_as(OMAPL1xx_SATA_BASE, 1));
		hwi_add_inputclk(cpu_freq, 2);
		hwi_add_irq(67);
	}
}

__SRCVERSION("$URL$ $Rev$");
