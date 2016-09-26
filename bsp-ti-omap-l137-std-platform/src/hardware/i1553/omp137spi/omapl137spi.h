/*
 * Halliburton ESG
 * 1553 Driver (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */


#ifndef _OMAPL137SPI_H_INCLUDED
#define _OMAPL137SPI_H_INCLUDED

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <hw/spi-master.h>
#include <arm/omapl1xx.h>
#ifdef SPI_OMAPL1XX
#include <hw/dma.h>

#define OMAPL1XX_SPI_DMA_PRIORITY 0
#define sdbg(...)	do { fprintf(stderr, __VA_ARGS__ ); fprintf(stderr, "\n");} while(0)
#endif

#define OMAPL1xx_SPI_CONTROLLER (1 << 7)

#define	I1553_RX_PRIORITY		             63               //QNX thread can have priority 1 to 255(highest), Unprivileged threads can have a priority 1 to 63, configured highest for 1553.
#define I1553_HANDLER_THREAD_PRIORITY        60

#define	I1553_RX_EVENT		                 1
#define I1553_RX_POLLING                     3
#define I1553_RX_MODE_CMD_NO_DATA            4
#define I1553_RX_CMD_W_DATA                  5
#define I1553_RX_INVALID_CMD                 255
#define I1553_RX_INVALID_DATA                256
#define	OMAPL1xx_DMA_EVENT		             2

#define	OMAPL1xx_SPI_MAXDMALEN	(1024 * 8)

/* SPI Register Definitions */

#define OMAPL137_SPI0_BASE		0x01C41000
#define OMAPL137_SPI0_INT			20
#define OMAPL1xx_SPI1_DMA			36

#define OMAPL137_SPI_GCR0			0x00		/* SPI Global Control Register 0 */
	#define OMAPL137_SPIGCR0_RESET	(1 << 0)
#define OMAPL137_SPI_GCR1			0x04		/* SPI Global Control Register 1 */
	#define OMAPL137_SPIGCR1_MASTER	(1 << 0)
	#define OMAPL137_SPIGCR1_CLKMOD	(1 << 1)
	#define OMAPL137_SPIGCR1_LOOPBACK	(1 << 16)
	#define OMAPL137_SPIGCR1_SPIENA	(1 << 24)
#define OMAPL137_SPI_INT			0x08		/* SPI Interrupt Register */
	#define OMAPL137_SPIINT_BITERRENA	(1 << 4)
	#define OMAPL137_SPIINT_OVRNINTEN	(1 << 6)
	#define OMAPL137_SPIINT_RXINTEN	(1 << 8)
	#define OMAPL137_SPIINT_DMAREQEN	(1 << 16)
#define OMAPL137_SPI_LVL			0x0C		/* SPI Interrupt Level Register */
#define OMAPL137_SPI_FLG			0x10		/* SPI Flag Register */
#define OMAPL137_SPI_PC0			0x14		/* SPI Pin Control Register 0 */
#define OMAPL137_SPI_PC2			0x1C		/* SPI Pin Control Register 2 */
	#define OMAPL137_SPIPC_EN0	(1 << 0)
	#define OMAPL137_SPIPC_EN1	(1 << 1)
	#define OMAPL137_SPIPC_CLK	(1 << 9)
	#define OMAPL137_SPIPC_DO		(1 << 10)
	#define OMAPL137_SPIPC_DI		(1 << 11)
#define OMAPL137_SPI_DAT1			0x3C		/* SPI Shift Register */
	#define	OMAPL137_SPIDAT1_CSHOLD	(1 << 28)
	#define	OMAPL137_SPIDAT1_CSNR(c)	((c) << 16)
	#define	OMAPL137_SPIDAT1_DFSEL(d)	((d) << 24)
#define OMAPL137_SPI_BUF			0x40		/* SPI Buffer Register */
	#define	OMAPL137_SPIBUF_RXEMPTY	(1 << 31)
	#define	OMAPL137_SPIBUF_RXOVR		(1 << 30)
	#define	OMAPL137_SPIBUF_TXFULL	(1 << 29)
	#define	OMAPL137_SPIBUF_BITERR	(1 << 28)
#define OMAPL137_SPI_EMU			0x44		/* SPI Emulation Register */
#define OMAPL137_SPI_DELAY		0x48		/* SPI Delay Register */
	#define	OMAPL137_SPIDELAY_C2TDELAY(d)	((d) << 24)
	#define	OMAPL137_SPIDELAY_T2CDELAY(d)	((d) << 16)
#define OMAPL137_SPI_DEF			0x4C		/* SPI Default Chip Select Register */
	#define	OMAPL137_SPIDEF_EN1DEF	(1 << 1)
	#define	OMAPL137_SPIDEF_EN0DEF	(1 << 0)
#define OMAPL137_SPI_FMT0			0x50		/* SPI Data Format Register 0 */
#define OMAPL137_SPI_FMT1			0x54		/* SPI Data Format Register 1 */
#define OMAPL137_SPI_FMT2			0x58		/* SPI Data Format Register 2 */
#define OMAPL137_SPI_FMT3			0x5C		/* SPI Data Format Register 3 */
	#define	OMAPL137_SPIFMT_CLEN(l)		((l) & 0x1F)
	#define	OMAPL137_SPIFMT_PRESCALE(p)	(((p) & 0xFF) << 8)
	#define	OMAPL137_SPIFMT_PHASE1		(1 << 16)
	#define	OMAPL137_SPIFMT_POLARITY1		(1 << 17)
	#define	OMAPL137_SPIFMT_SHIFTLSB		(1 << 20)


#ifdef SPI_OMAPL1XX
#define DEVICE_NOS	8
#else
#define DEVICE_NOS	2
#endif

typedef struct {
	volatile uint32_t   opt;
	volatile uint32_t   src;
	volatile uint32_t   abcnt;
	volatile uint32_t   dst;
	volatile uint32_t   srcdstbidx;
	volatile uint32_t   linkbcntrld;
	volatile uint32_t   srcdstcidx;
	volatile uint32_t   ccnt;
} omaplxx_edma_param;

typedef struct {
	volatile uint16_t * base;
	volatile uint16_t * rdptr;
	volatile uint16_t * wrptr;
	volatile uint16_t maxSize;
	volatile uint16_t fillLevel;
	volatile uint16_t overflow;
	volatile uint16_t underflow;
	volatile uint32_t  end;


} circular_buffer_t;

typedef struct {
	I1553DEV	i1553_spi;		/* This has to be the first element */

	uint32_t	spipbase;
	uint32_t	edmapbase;
	int			i1553irq;
	int			edmairq;

	uint32_t	clk;
	int			ch_rxedma;
	int			ch_txedma;

	uintptr_t	spivbase;
	uintptr_t	edmavbase;
	uintptr_t   iovbase;
	int			rx_iid;
	int			edmaiid;

	//Message Channel
	int			chid, coid;

	//Thread ID
	int         thread_i1553_handler_id;
	uint16_t    i1553_thread_priority;

	int			lastdev;
	int			edma;

	uint32_t	spidelay;
	uint32_t	spidef;
	uint32_t	gcr1;

	circular_buffer_t    *rxbuf;       /* 16bit word for 1553 receiver*/
	circular_buffer_t    *txbuf;
	size_t    rxbufsize;
	size_t    txbufsize;
	uint16_t  i1553_cmd_rcv_cnt;

	int			txlen, txdone;
	int			rxlen;		/* Datalength */
	int			dtime;		/* usec per data, for time out use */

	uint16_t    rtu_addr;
	uint16_t    io_number;

	uint16_t    rtu_status;


	struct sigevent	i1553_rxevent;
#ifdef SPI_OMAPL1XX
	uint32_t	spifmt[DEVICE_NOS];		/* used for omapl1xx alone for storing format register details */
	uint8_t		spicntrlr;	/* used to specify that we are using omapl1xx spi controller */
	dma_functions_t dmafuncs; /* dma functions structure */
	void *dma_rx_handle;	/* dma handle for receive channel */
	void *dma_tx_handle;	/* dma handle for transmit channel */
#endif
} i1553_spi_t;


#define SPI_SEND_ONE_WORD(x)  while(spibuf & OMAPL137_SPIBUF_TXFULL)\
    {\
    	spibuf = in32(base + OMAPL137_SPI_BUF);\
    }\
    out16(base + OMAPL137_SPI_DAT0, x)


extern void *i1553_spi_init(void *hdl, char *options);
extern void i1553_spi_dinit(void *hdl);
extern void *i1553_rxbuf_read(void *hdl, uint8_t *buf, int *len);
extern void i1553_edma_disable(i1553_spi_t *i1553spi);
extern int i1553_txbuf_write(void *hdl, uint8_t *buf, int len);
extern int i1553_init_edma(i1553_spi_t *i1553spi);
extern int i1553_setcfg(void *hdl, uint16_t device, i1553_cfg_t *cfg);

extern int i1553_devinfo(void *hdl, uint32_t device, i1553_devinfo_t *info);
extern int i1553_drvinfo(void *hdl, spi_drvinfo_t *info);
extern int i1553_cfg(i1553_spi_t *i1553spi, int chip, i1553_cfg_t *cfg);

extern int i1553_spi_xchange(i1553_spi_t *i1553spi, uint8_t *buf, int len);
extern int i1553_read(i1553_spi_t *i1553spi, uint8_t *buf, int len);
extern int i1553_write(i1553_spi_t *i1553spi, uint8_t *buf, int len);

extern int i1553_hwinit(i1553_spi_t *i1553spi);
extern int i1553_attach_intr(i1553_spi_t *i1553spi);
extern int i1553_spi_cfg(i1553_spi_t *i1553spi, int device, i1553_cfg_t *cfg);
extern int i1553_wait(i1553_spi_t *dev, int len);
extern int i1553_setup_edma(i1553_spi_t *dm644x, int device, i1553_dma_paddr_t *paddr, int len);
extern int i1553_select_format(i1553_spi_t *dm644x, int chip, i1553_cfg_t *cfg);
#ifdef SPI_OMAPL1XX
void omapl1xx_edma_disable(dm644x_spi_t*);
#endif
extern paddr_t mphys(void *);

#endif
