/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems.
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

/*
 * These macros are for the PINMUX register values. As these are board specific
 * they should not be in the omapl1xx.h header file which is SOC specific. 
 */

#define OMAPL137_SYSCFG_RESET_VALUE		0x00000000

#define OMAPL137_SYSCFG_PINMUX8_VALUE   0x20022022
#define OMAPL137_SYSCFG_PINMUX9_VALUE   0x00000882
#define OMAPL137_SYSCFG_PINMUX10_VALUE  0x22222220
#define OMAPL137_SYSCFG_PINMUX11_VALUE  0x00800022
#define OMAPL137_SYSCFG_PINMUX13_VALUE  0x22000000
#define OMAPL137_SYSCFG_PINMUX14_VALUE  0x11222222
#define OMAPL137_SYSCFG_PINMUX15_VALUE  0x21111111
#define OMAPL137_SYSCFG_PINMUX16_VALUE  0x11111112
#define OMAPL137_SYSCFG_PINMUX17_VALUE  0x21811111
#define OMAPL137_SYSCFG_PINMUX18_VALUE  0x11111112
#define OMAPL137_SYSCFG_PINMUX19_VALUE  0x00000001

#define OMAPL137_SYSCFG_PINMUX_OUT(r)	out32(OMAPL1xx_SYSCFG_BASE + (OMAPL1xx_SYSCFG_PINMUX##r),OMAPL137_SYSCFG_PINMUX##r##_VALUE)

#define OMAPL137_SYSCFG_SUSPSRC_RESET_VALUE     0xE38079DF

#define OMAPL137_SYSCFG_CFGCHIP0_VALUE  OMAPL137_SYSCFG_RESET_VALUE
#define OMAPL137_SYSCFG_CFGCHIP1_VALUE  OMAPL137_SYSCFG_RESET_VALUE
#define OMAPL137_SYSCFG_CFGCHIP2_VALUE  0x0000EF02
#define OMAPL137_SYSCFG_CFGCHIP3_VALUE  0x0000FF05
#define OMAPL137_SYSCFG_CFGCHIP4_VALUE  0x0000FF00
#define OMAPL137_SYSCFG_USB2_0_RESET	0x00008000
#define OMAPL137_SYSCFG_CFGCHIP_OUT(r)	out32(OMAPL1xx_SYSCFG_BASE + (OMAPL1xx_SYSCFG_CFGCHIP##r),OMAPL137_SYSCFG_CFGCHIP##r##_VALUE)

/*
 * Initialize hwinfo structure in the system page.
 * This code is hardware dependant.
 */

// I2C I/O Macros
#define I2C_OUT(reg,data)  	out32(OMAPL1xx_I2C0_BASE + reg, data)
#define I2C_IN(reg)    		in32(OMAPL1xx_I2C0_BASE + reg)

// Bit Manipulation Macros
#define SETBIT(dest,mask)				(dest |= mask)
#define CLRBIT(dest,mask)				(dest &= ~mask)
#define CHKBIT(dest,mask)				(dest & mask)

extern void hwi_omapl137(); /* hardware/startup/lib/arm/hwi_omapl137.c */

unsigned char mac_addr[6];

static void omapl137_i2c_init()
{
	int i;

	I2C_OUT (OMAPL1xx_I2C_ICMDR, 0); // reset I2C
	for (i = 0; i < 10000; i++)
            ;

    // set up clocking speed - hardcoded for the OMAPL137 reference board
	I2C_OUT (OMAPL1xx_I2C_ICPSC, 0x17);
	I2C_OUT (OMAPL1xx_I2C_ICCLKL, 0x14); // This might need to change on a differently clocked board
	I2C_OUT (OMAPL1xx_I2C_ICCLKH, 0x14);// This might need to change on a differently clocked board

	I2C_OUT (OMAPL1xx_I2C_ICOAR, 0xa); // set own slave address
	I2C_OUT (OMAPL1xx_I2C_ICCNT, 0x0);

	I2C_OUT (OMAPL1xx_I2C_ICIMR, 0x3e); // enable interrupts or it won't work

	I2C_OUT (OMAPL1xx_I2C_ICMDR, OMAPL1xx_I2C_ICMDR_IRS); // enable I2C

    for (i = 0; i < 1000; i++)
            ;
}

// write is not yet tested
void omapl137_i2c_write(unsigned short addr, unsigned char *pbuf, unsigned short len)
{
        int i;

        I2C_OUT (OMAPL1xx_I2C_ICCNT, len);
        I2C_OUT (OMAPL1xx_I2C_ICSAR, addr);
        I2C_OUT (OMAPL1xx_I2C_ICMDR, OMAPL1xx_I2C_ICMDR_STT |
               OMAPL1xx_I2C_ICMDR_TRX | OMAPL1xx_I2C_ICMDR_MST | OMAPL1xx_I2C_ICMDR_IRS | OMAPL1xx_I2C_ICMDR_FREE);

        for (i = 0; i < 10; i++)
                ;

        for (i = 0 ; i < len ; i++ ) {
        	I2C_OUT (OMAPL1xx_I2C_ICDXR, pbuf[i]);

                while (!(I2C_IN(OMAPL1xx_I2C_ICSTR) & OMAPL1xx_I2C_ICSTR_ICXRDY))
                        ;
        }

        I2C_OUT (OMAPL1xx_I2C_ICMDR, I2C_IN(OMAPL1xx_I2C_ICMDR) | OMAPL1xx_I2C_ICMDR_STP);             // Generate STOP
}

void omapl137_i2c_read(unsigned short addr, unsigned short offset, unsigned char *pbuf, unsigned short len)
{
        int i;

        I2C_OUT (OMAPL1xx_I2C_ICCNT, 2); // length of offset address in bytes
        I2C_OUT (OMAPL1xx_I2C_ICSAR, addr); // I2C device address

        I2C_OUT (OMAPL1xx_I2C_ICMDR, OMAPL1xx_I2C_ICMDR_STT | OMAPL1xx_I2C_ICMDR_TRX |
                OMAPL1xx_I2C_ICMDR_MST | OMAPL1xx_I2C_ICMDR_IRS | OMAPL1xx_I2C_ICMDR_FREE);

        for (i = 0; i < 100; i++)
                ;

        // wait for xmit rdy
        while (!(I2C_IN(OMAPL1xx_I2C_ICSTR) & OMAPL1xx_I2C_ICSTR_ICXRDY))
               ;

        // Write address ms byte
        I2C_OUT (OMAPL1xx_I2C_ICDXR, (offset >> 8) & 0xff);

        while (!(I2C_IN(OMAPL1xx_I2C_ICSTR) & OMAPL1xx_I2C_ICSTR_ICXRDY))
               ;

        // Write address ls byte
        I2C_OUT (OMAPL1xx_I2C_ICDXR, (offset  & 0xff));

        while (!(I2C_IN(OMAPL1xx_I2C_ICSTR) & OMAPL1xx_I2C_ICSTR_ICXRDY))
               ;

		// Address phase done

		I2C_OUT (OMAPL1xx_I2C_ICCNT, len & 0xffff);
		I2C_OUT (OMAPL1xx_I2C_ICSAR, addr);
		I2C_OUT (OMAPL1xx_I2C_ICMDR, OMAPL1xx_I2C_ICMDR_STT |
				OMAPL1xx_I2C_ICMDR_MST | OMAPL1xx_I2C_ICMDR_IRS | OMAPL1xx_I2C_ICMDR_STP | OMAPL1xx_I2C_ICMDR_FREE);

		// read data from the device
		for (i = 0 ; i < len ; i++ ) {
			while (!(I2C_IN(OMAPL1xx_I2C_ICSTR) & OMAPL1xx_I2C_ICSTR_ICRRDY))
					;

			pbuf[i] = I2C_IN(OMAPL1xx_I2C_ICDRR) & 0xFF;
		}
		I2C_OUT (OMAPL1xx_I2C_ICMDR, I2C_IN(OMAPL1xx_I2C_ICMDR) | OMAPL1xx_I2C_ICMDR_STP);             // Generate STOP
}

// Set OMAPL137 MUX pins correctly for EMIFA, SPI, UART, I2C, Ethernet, USB operations
void init_mux_pins(void)
{
	unsigned int cfgchip_val, timeout;

	// unlock registers for writing

	out32 (OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_KICK0R, OMAPL1xx_SYSCFG_KICK0R_UNLOCK);
	out32 (OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_KICK1R, OMAPL1xx_SYSCFG_KICK1R_UNLOCK);

	/* UART2_RXD, I2C0_SCL, I2C0_SDA, I2C1_SDA, I2C1_SCL are selected */	
	OMAPL137_SYSCFG_PINMUX_OUT(8);

	/* GP4[15], UART2_TXD are selected  */	
	OMAPL137_SYSCFG_PINMUX_OUT(9);

	/* RMII_RXER[0], RMII_RXD[1], RMII_RXD[0], RMII_CRS_DV, RMII_TXEN, RMII_TXD[1], RMII_TXD[0] are selected  */	
	OMAPL137_SYSCFG_PINMUX_OUT(10);

	/* MDIO_D, MDIO_CLK are selected  */		
	OMAPL137_SYSCFG_PINMUX_OUT(11);

	/* EMA_D[1], EMA_D[0] are selected  */
	OMAPL137_SYSCFG_PINMUX_OUT(13);

	/* EMA_D[9], EMA_D[8], EMA_D[7], EMA_D[6], EMA_D[5], EMA_D[4], EMA_D[3], EMA_D[2] are selected  */	
	OMAPL137_SYSCFG_PINMUX_OUT(14);

	/* EMA_A[1], EMA_A[0], EMA_D[15], EMA_D[14], EMA_D[13], EMA_D[12], EMA_D[11], EMA_D[10] are selected  */		
	OMAPL137_SYSCFG_PINMUX_OUT(15);

	/* EMA_A[9], EMA_A[8], EMA_A[7], EMA_A[6], EMA_A[5], EMA_A[4], EMA_A[3], EMA_A[2] are selected  */	
	OMAPL137_SYSCFG_PINMUX_OUT(16);

	/* EMA_CAS, EMA_SDCKE, GP1[15], EMA_BA[0], EMA_BA[1], EMA_A[12], EMA_A[11], EMA_A[10] are selected  */	
	OMAPL137_SYSCFG_PINMUX_OUT(17);

	/* EMA_WE_DQM[0], EMA_WE_DQM[1], EMA_OE, EMA_CS[3], EMA_CS[2], EMA_CS[0], EMA_WE, EMA_RAS are selected  */	
	OMAPL137_SYSCFG_PINMUX_OUT(18);

	/* EMA_WAIT[0] is selected  */	
	OMAPL137_SYSCFG_PINMUX_OUT(19);

	//Configure the Miscellaneous SYSCFG registers

	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_SUSPSRC), OMAPL137_SYSCFG_SUSPSRC_RESET_VALUE);
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CHIPSIG), OMAPL137_SYSCFG_RESET_VALUE);
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CHIPSIG_CLR), OMAPL137_SYSCFG_RESET_VALUE);

	// Configure the CFGCHIP registers

	/* Reset Value  */	
	OMAPL137_SYSCFG_CFGCHIP_OUT(0);
	/* Reset Value  */	
	OMAPL137_SYSCFG_CFGCHIP_OUT(1);

	/* USB1.1 OHCI and USB2.0 OTG PHY initializations  */	
	OMAPL137_SYSCFG_CFGCHIP_OUT(2);
	cfgchip_val = in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2);
	CLRBIT(cfgchip_val, OMAPL137_SYSCFG_USB2_0_RESET);
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), cfgchip_val);

	/* configure phy */	
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) & (~USB0PHYPWDN)));    //Enable USB2.0 Phy for normal operation	
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) & (~USB0OTGPWRDN)));   //Enable OTG Subsystem for normal operation
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) & (~USB1PHYCLKMUX)));  //USB1.1 PHY ref clk is sourced by USB2.0 PHY
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) | USB1SUSPENDM));      //Enable USB1.1 Phy	   
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) | USB0DATPOL));        //Do not alter differential data polarity	
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) | USB0SESNDEN));       //USB2.0 Session End Comparator Enabled	 
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) | USB0VBDTCTEN));      //All VBUS line comparators are Enabled	   
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) | USB0REF_FREQ_24M));  //input clock as 24MHz	  
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) & (~USB0PHYCLKMUX)));  //Mask USB0PHYCLKMUX	   
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) | USB0PHYCLKMUX));     //ref clk is from AUXCLK generated from PLL.	   
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) & (~USB0OTGMODE)));    //USB otg mode, no override, [14,13]=0,0	  
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) | USB0OTGHOSTMODE));   //USB Host opertion [14,13]=0,1	   
	out32((OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2), (in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) | USB0PHY_PLLON));     //USB0PHY_PLLON is set.  	
	
	/* wait until the usb phy pll locks */
	timeout = 0xf00000;
	do {
		timeout--;
		if (timeout == 0)
		{
			kprintf("USB PHY PLL lock Timed Out\n");
			break;
		}
	}
	while((in32(OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_CFGCHIP2) & USB0PHYCLKGD) == 0);


	/* EMIFA is clocked by PLLC_SYSCLK3, EMIFB is clocked by DIV4.5 PLL output, Divide by 4.5 is enabled */
	OMAPL137_SYSCFG_CFGCHIP_OUT(3);

	/* Reset Value  */
	OMAPL137_SYSCFG_CFGCHIP_OUT(4);

	// Lock registers again
	out32 (OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_KICK0R, 0);
	out32 (OMAPL1xx_SYSCFG_BASE + OMAPL1xx_SYSCFG_KICK1R, 0);
}

void omapl137_psc_enable (int module)
{
	unsigned int base;

	if (module <= 15)
		base = OMAPL1xx_PSC0_BASE;
	else
	{
		base = OMAPL1xx_PSC1_BASE;
		module -= 16; // normalize PD1 module counts
	}

	while ( in32(base + OMAPL1xx_PSC_PTSTAT) & 1)
		; // wait for any previous transition to complete

	out32(base + OMAPL1xx_PSC_MDCTL0 + (4*module), 3); // enable module via its MDCTL register
	out32(base + OMAPL1xx_PSC_PTCMD, 1); // start transition
	while ( in32(base + OMAPL1xx_PSC_PTSTAT) & 1)
		; // wait for transition to complete
}

void omapl137_psc_init(void)
{
	// Enable the resources we need - can disable unused things here to save power

	omapl137_psc_enable (PSC_MODULE_UART2);
	omapl137_psc_enable (PSC_MODULE_EDMA0);
	omapl137_psc_enable (PSC_MODULE_EDMA_TC0);
	omapl137_psc_enable (PSC_MODULE_EDMA_TC1);
	omapl137_psc_enable (PSC_MODULE_MMCSD0);
	omapl137_psc_enable (PSC_MODULE_AINTC);
	omapl137_psc_enable (PSC_MODULE_I2C1);
	omapl137_psc_enable (PSC_MODULE_SPI0);	
	omapl137_psc_enable (PSC_MODULE_SPI1);
	omapl137_psc_enable (PSC_MODULE_SHRAM);
	omapl137_psc_enable (PSC_MODULE_GPIO);
	omapl137_psc_enable (PSC_MODULE_EMAC);
	omapl137_psc_enable (PSC_MODULE_USB0);
	omapl137_psc_enable (PSC_MODULE_USB1);
}

/*
 * We assume the DPLL is in lock mode
 */
unsigned long
omapl137_cpu_clock(void)
{

	unsigned dpll_div, dpll_mult;
	unsigned dpll_pre, dpll_post;
	unsigned ck_gen1;

	// SYSCLK6 is used to drive the ARM, so that's what we look at here
	// See the TI document SPRUG84b for full details

	dpll_div  = (in32(OMAPL1xx_PLL_BASE + OMAPL1xx_PLL_PLLDIV6) & 0x1F) + 1;
	dpll_mult = (in32(OMAPL1xx_PLL_BASE + OMAPL1xx_PLL_PLLM) & 0x1F) + 1;

	dpll_pre = in32(OMAPL1xx_PLL_BASE + OMAPL1xx_PLL_PREDIV);

	if (dpll_pre & 0x8000) // enabled ?
		dpll_pre = (dpll_pre & 0x1f) + 1;
	else
		dpll_pre = 1;

	dpll_post = in32(OMAPL1xx_PLL_BASE + OMAPL1xx_PLL_POSTDIV);

	if (dpll_post & 0x8000) // enabled ?
		dpll_post = (dpll_post & 0x1f) + 1;
	else
		dpll_post = 1;

	ck_gen1 = OMAPL1xx_PLL_INPUT_CLK * dpll_mult;
	ck_gen1 /=  (dpll_pre * dpll_post * dpll_div);

	return (ck_gen1);
}

void
allocate_dsp_memory(paddr_t * resmem_addr, size_t * resmem_size, int count)
{
	int     i;

	for (i = 0; i < count; i++)
	{
		alloc_ram(resmem_addr[i], resmem_size[i], 1);
		hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_MISC, "misc", 0);
		hwi_add_location(resmem_addr[i], resmem_size[i], 0, hwi_find_as(resmem_addr[i], 0));
	}
}

void
allocate_dsplink_memory(paddr_t linkmem_addr, size_t linkmem_size)
{
	alloc_ram(linkmem_addr, linkmem_size, 1);
	hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_DMA, "misc", 0);
	hwi_add_location(linkmem_addr, linkmem_size, 0, hwi_find_as(linkmem_addr, 0));
}

void
init_hwinfo()
{
	set_syspage_section(&lsp.cpu.arm_boxinfo, sizeof(*lsp.cpu.arm_boxinfo.p));

	/* Enable the PSC */
	omapl137_psc_init();

	/* Initialize the Pin Muxing */
	init_mux_pins();

    omapl137_i2c_init();

    // This is a very specific read for the OMAP EVM board.
    omapl137_i2c_read(0x50, 0x7f00, (unsigned char *)&mac_addr, 6); // Get MAC address from I2C0 eeprom at address 0x50, offset 0x7f00

	/* Initialize the Hwinfo section of the Syspage */
	hwi_omapl137();
}

