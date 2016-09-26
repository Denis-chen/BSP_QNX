/*
 * Halliburton ESG
 * 1553 Driver (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#include "omapl137spi.h"

int i1553_spi_cfg(i1553_spi_t *i1553spi, int device, i1553_cfg_t *cfg)
{
	int32_t		fmt, prescale;

	fmt = cfg->mode & SPI_MODE_CHAR_LEN_MASK;

	/*
	 * Only support data length from 2 to 16
	 */
	if (fmt > 16 || fmt < 2)
		return -1;

	prescale = (i1553spi->clk / cfg->clock_rate) - 1;


	/*
	 * Check if the bit rate is too high or too low
	 */
	if (prescale > 255 || prescale < 0)
		return -1;
#ifdef SPI_OMAPL1XX
	/* reset fmt */
	fmt &= ~(DM6446_SPIFMT_PHASE1 | DM6446_SPIFMT_SHIFTLSB | DM6446_SPIFMT_PRESCALE(0xff));
#endif
	fmt |= DM6446_SPIFMT_PRESCALE(prescale);

	if (cfg->mode & SPI_MODE_CKPHASE_HALF)
		fmt |= DM6446_SPIFMT_PHASE1;

	if (!(cfg->mode & SPI_MODE_BODER_MSB))
		fmt |= DM6446_SPIFMT_SHIFTLSB;
#ifdef SPI_OMAPL1XX
	/* if omapl1xx , set if ENA & T2T  Delay if required */
	if(dm644x->spicntrlr & OMAPL1xx_SPI_CONTROLLER)
		fmt |= dm644x->spifmt[device];
		/* if cs is >= 3 we do not update registers now but we save the configuration
		 * & update the format registers just before the transmission really starts
		 */
	if(device > 2)
		dm644x->spifmt[device] = fmt;
	else
#endif
	/*
	 * Program the data format register
	 * Or we can always use format 0, and re-program FMT0 register
	 * every time the format has to be changed.
	 */

	out32(i1553spi->spivbase + DM6446_SPI_FMT0 + (device * 4), fmt);
	/*
	 * Programming control field alone cannot trigger a spi transfer
	 */
#ifdef SPI_OMAPL1XX
	/* touch dat field only if its current device */
	if(dm644x->spicntrlr & OMAPL1xx_SPI_CONTROLLER && dm644x->lastdev == device) {
		/* accessing DM6446_SPI_DAT1 as byte works only for little endian architecture */
		uint8_t dat1_ctrl_field = in8(dm644x->spivbase + DM6446_SPI_DAT1 + 3);
		if (!(cfg->mode & SPI_MODE_CSHOLD_HIGH)) {
			out8(dm644x->spivbase + DM6446_SPI_DAT1 + 3, dat1_ctrl_field & ~(1 << 4));
		} else
			out8(dm644x->spivbase + DM6446_SPI_DAT1 + 3, dat1_ctrl_field | (1 << 4));

	}
#endif
	return 0;
}
