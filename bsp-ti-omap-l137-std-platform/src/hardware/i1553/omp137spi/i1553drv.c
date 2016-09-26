/*
 * Halliburton ESG
 * 1553 Driver (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#include <stdio.h>
#include <stdlib.h>

#include "omapl137spi.h"
#include "omapl1xx_gpio.h"
#include "verbose.h"


enum i1553_spi_options{BASE, GPIOIRQPIN, CLOCK, EDMABASE, EDMAIRQ, EDMACHANNEL, EDMA, C2T, T2C,
	                   EN0DEF, EN1DEF, LOOPBACK, RTUADR, CMDGPIO, RXBUFSIZE, TXBUFSIZE,
					   ENA0, ENA1, T2T0, T2T1, T2E, C2E, PARITYEN0, PARITYEN1, SPICONTROLLER,
					   END
					};

extern void  initCircularBuffer(circular_buffer_t *buff, uint16_t *data, size_t size);


static char *i1553_spi_opts[] = {
	[BASE]			=	"base",			/* Base address for this SPI controller */
	[GPIOIRQPIN]	=	"gpioirqpin",			/* IRQ for this SPI intereface */
	[CLOCK]			=	"clock",		/* SPI clock */
	[EDMABASE]		=	"edmabase",		/* Base address for EDMA controller */
	[EDMAIRQ]		=	"edmairq",		/* IRQ for this SPI interface receive event */
	[EDMACHANNEL]	=	"edmachannel",	/* EDMA channel for this SPI interface receive event, Tx channel = rxchannel - 1 */
	[EDMA]			=	"edma",			/* Disable/enable EDMA for SPI */
	[C2T]			=	"c2t",			/* Chip-select-active-to-transmit-start-delay */
	[T2C]			=	"t2c",			/* Transmit-end-to-chip-select-inactive-delay */
	[EN0DEF]		=	"en0def",		/* Chip select 0 default pattern */
	[EN1DEF]		=	"en1def",		/* Chip select 1 default pattern */
	[LOOPBACK]		=	"loopback",		/* Loopback enable, for testing */
	[RTUADR]        =   "rtuaddr",       /* tool rtu address */
	[CMDGPIO]       =   "cmdgpio",       /* gpio line for identify 1553 command or data*/
	[RXBUFSIZE]     =   "rxbufsize"      /* i1553 receiver buffer size */
	[TXBUFSIZE]	    =   "txbufsize"      /* i1553 transmit buffer size */
	[ENA0]			=	"ena0",          /* Ena signal is used for cs 0 */
	[ENA1]			=	"ena1",          /* Ena signal is used for cs 1 */
	[T2T0]			=	"t2t0",          /* Delay between transmissions for cs 0 */
	[T2T1]			=	"t2t1",          /* Delay between transmissions for cs 1 */
	[T2E]			=	"t2e",          /* Transmit data finished to spi_ena pin inactive timeout */
	[C2E]			=	"c2e",          /* Chip select active to spi_ena active timeout */
	[PARITYEN0]		=	"parena0",       /* Parity 1 => odd parity , 0 => even parity for cs 0 */
	[PARITYEN1]		=	"parena1",       /* Parity 1 => odd parity , 0 => even parity for cs 1 */
	[SPICONTROLLER]	=	"spicntrlr",    /* omapl1xx => OMAPL1xx */
	[END]			=	NULL
};

i1553_funcs_t i1553_drv_entry = {
	sizeof(i1553_funcs_t),
	i1553_spi_init,	/* init() */
	i1553_spi_dinit,	/* fini() */
	i1553_drvinfo,	/* drvinfo() */
	i1553_devinfo,	/* devinfo() */
	i1553_setcfg,	/* setcfg() */
	i1553_rxbuf_read,	/* xfer() */
	i1553_txbuf_write	/* dma_xfer() */
};

i1553_devinfo_t devlist[DEVICE_NOS] = {
	{
		0x00,				// Device ID
		"OMAPL1xx-SPI0",		// Description
		{
			16 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			3000000,			// Clock rate 3MHz, EXPII tool setup
			0xffff,          // default rtu address
		},
	},
	{
		0x01,				// Device ID
		"OMAPL1xx-SPI1",		// Description
		{
			16 | SPI_MODE_BODER_MSB,		// data length 8bit, MSB
			3000000,			// Clock rate 6M
			0xffff,          // default rtu address
		},
	},

};

static int i1553_options(i1553_spi_t *dev, char *optstring)
{
	int		opt, rc = 0, err = EOK;
	char	*options, *freeptr, *c, *value;

	if (optstring == NULL)
		return 0;

	freeptr = options = strdup(optstring);

	while (options && *options != '\0') {
		c = options;
		if ((opt = getsubopt(&options, i1553_spi_opts, &value)) == -1)
			goto error;

		switch (opt) {
			case  BASE:
				dev->spipbase  = strtoul(value, 0, 0);
				continue;
			case  GPIOIRQPIN:
				dev->i1553irq = 0xC120 + strtoul(value, 0, 0);      // *map to logic gpio interrupt #
				continue;
			case  CLOCK:
				dev->clk       = strtoul(value, 0, 0);
				continue;
			case  EDMABASE:
				dev->edmapbase = strtoul(value, 0, 0);
				continue;
			case  EDMAIRQ:
				dev->edmairq   = strtoul(value, 0, 0);
				continue;
			case  EDMACHANNEL:
				dev->ch_rxedma = strtoul(value, 0, 0);
				continue;
			case  EDMA:
				dev->edma      = strtol(value, 0, 0);
				continue;
			case  C2T:
				dev->spidelay |= strtoul(value, 0, 0) << 24;
				continue;
			case  T2C:
				dev->spidelay |= strtoul(value, 0, 0) << 16;
				continue;
			case  EN0DEF:
				dev->spidef   |= (strtoul(value, 0, 0) & 1);
 				dev->spidef   |= (1 << 24);
				continue;
			case EN1DEF:
				dev->spidef   |= ((strtoul(value, 0, 0) & 1) << 1);
 				dev->spidef   |= (1 << 25);
				continue;
			case LOOPBACK:
				dev->gcr1     |= DM6446_SPIGCR1_LOOPBACK;
				continue;
			case RTUADR:
				dev->rtu_addr = strtoul(value, 0, 0);                // Rtu address
				continue;
			case CMDGPIO:
				dev->io_number = strtoul(value, 0, 0);               // io pin for identify 1553 command or data
		        continue;
			case RXBUFSIZE:
				dev->rxbufsize = strtoul(value, 0, 0);
				continue;
			case TXBUFSIZE:
				dev->txbufsize = strtoul(value, 0, 0);
				continue;
			case ENA0:
				dev->spifmt[0] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case ENA1:
				dev->spifmt[1] |= (strtoul(value, 0, 0) << 21 );
				continue;
			case T2T0:
				dev->spifmt[0] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2T1:
				dev->spifmt[1] |= (strtoul(value, 0, 0) << 24 );
				continue;
			case T2E:
				dev->spidelay |= strtoul(value, 0, 0) << 8;
				continue;
			case C2E:
				dev->spidelay |= strtoul(value, 0, 0) << 8;
				continue;
			case PARITYEN0:
				dev->spifmt[0] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case PARITYEN1:
				dev->spifmt[1] |= (strtoul(value, 0, 0) << 23 ) | (1 << 22);
				continue;
			case SPICONTROLLER:
				if(!strncmp(value, "omapl1xx", strlen("omapl1xx")))
					dev->spicntrlr = OMAPL1xx_SPI_CONTROLLER;
				continue;

		}
error:
		fprintf(stderr, "i1553: unknown option %s", c);
		err = EINVAL;
		rc = -1;
	}

	free(freeptr);

	return rc;
}

void *i1553_spi_init(void *hdl, char *options)
{
	i1553_spi_t	   *dev;
	i1553_dev_t    *handler = hdl;
	uintptr_t		base;
	uint32_t        tmp = 0;
	int             devnum = 0;
	uint16_t       *buf;

	pthread_attr_t      pattr;
	struct sched_param  param;



	devnum = handler->devnum;
	if(devnum > 1 || devnum < 0)   // only has two spi bus
	{
		return NULL;
	}

	dev = calloc(1, sizeof(i1553_spi_t));

	if (dev == NULL)
		return NULL;
	dev->spipbase  = OMAPL137_SPI0_BASE;
	dev->spiirq    = OMAPL137_SPI0_INT;			/* We use SPI0 interrupt */
	dev->edmapbase = DM6446_EDMA_BASE;
	dev->edmairq   = 0xC100 + 1;	/* We use interrupt of transmitter channel of SPI 0 for 1553 rtu, probably we won't use edma at all since the polling words changes */
	dev->clk       = 150000000;		/* 90MHz SPI clock */
	dev->ch_txedma = 1;
	dev->edma      = 0;             /* default no use edma */
	dev->gcr1      = DM6446_SPIGCR1_MASTER | DM6446_SPIGCR1_CLKMOD;
	dev->spidef    = 0;             // default
	dev->spidelay = ( 8 << 24) | (8 << 16);   // default based on EPXII tool

    dev->i1553_thread_priority = I1553_HANDLER_THREAD_PRIORITY;


	if (i1553_options(dev, options))
		goto fail0;

	/* initialize rtu status */

    dev->rtu_status = dev->rtu_addr << 8;


    /* initialize the 1553 receiver circular buffer */
    buf = calloc(dev->rxbufsize, sizeof(uint16_t));

    initCircularBuffer(dev->rxbuf, buf, dev->rxbufsize);

    /* initialize the 1553 transmit circular buffer */

    buf = calloc(dev->txbufsize, sizeof(uint16_t));

    initCircularBuffer(dev->txbuf, buf, dev->txbufsize);

	/*
	 * Map in SPI and EDMA register
	 */
	if ((base = mmap_device_io(OMAPL1xx_SPI0_SIZE, dev->spipbase)) == (uintptr_t)MAP_FAILED)
		goto fail0;

	dev->spivbase = base;

	/*
	 * Put SPI module in reset mode
	 */
	out32(base + OMAPL137_SPI_GCR0, 0);
	out32(base + OMAPL137_SPI_GCR0, 1);

	/*
	 * Enable SPI master and clock
	 */
	out32(base + OMAPL137_SPI_GCR1, OMAPL137_SPIGCR1_MASTER | OMAPL137_SPIGCR1_CLKMOD);

	/*
	 * Configure SPI function pins
	 */

#ifdef SPI_OMAPL1XX
	/* For OMAPL1XX we can use ENA pin if needed
	 * If any of device is using ENA pin then enable the pin
	 */
	int j;
	if(dev->spicntrlr & OMAPL1xx_SPI_CONTROLLER)
		for(j = 0; j < DEVICE_NOS; j++)
			if(dev->spifmt[j] & OMAPL1xx_SPIFMT_WAITENA) {
				tmp = OMAPL1xx_SPIPC_ENA;
				break;
			}
			/* top 8 bits of dev->spidef specifiy which all chip selects are used */
			tmp |= dev->spidef >> 24;
#else
	tmp = OMAPL137_SPIPC_EN0 | OMAPL137_SPIPC_EN1;
#endif
	out32(base + OMAPL137_SPI_PC0, OMAPL137_SPIPC_CLK | OMAPL137_SPIPC_DO | OMAPL137_SPIPC_DI | tmp );
	/*
	 * Set default data format
	 */
		i1553_devinfo_t	*info;

	    info = devnum ? &devlist[1] : &devlist[0];                 // two set up is same, just in case later we have different configure

		i1553_spi_cfg(dev, devnum, &info->cfg);


	/*
	 * Chip select default pattern : logic 0 when no transaction is in progress
	 */
	out32(base + OMAPL137_SPI_DEF, dev->spidef);

	/*
	 * C2T/D2C delay & For omapl1xx C2E & T2E
	 */
	out32(base + OMAPL137_SPI_DELAY, dev->spidelay);


	/*
	 * Attach the GPIO pin for identify the 1553 command and data.
	 */

	if ((base = mmap_device_io(OMAPL1xx_GPIO_SIZE, dev->spipbase)) == (uintptr_t)MAP_FAILED)
			goto fail1;

	dev->iovbase = base + OMAPL1xx_GPIO_DIR01 + dev->io_number/32 * 0x28 + 0x10;   /* virtual address for IN_DATA reg */

	dev->io_number = (dev->io_number % 32);


	if (i1553_attach_intr(dev))
		goto fail2;

	/* create the i1553 handler thread */

	//Setup i1553 handler thread attributes

	pthread_attr_init(&pattr);
	pthread_attr_setshcedpolicy(&pattr, SCHED_RR);                //round-robin scheduling
	param.sched_priority = dev->i1553_thread_priority;
	pthread_attr_setschedparam(&pattr, &param);
	pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);

	/* Create I1553 thread handler  */

	if(pthread_create(&dev->thread_i1553_handler_id, &pattr, (void *)i1553LinkHandler, dev))
	{
		goto fail3;
	}

	dev->i1553_spi.hdl = hdl;
	dev->lastdev = -1;

	return dev;

fail3:
	InterruptDetach(dev->spiiid);
	ConnectDetach(dev->coid);
	ChannelDestroy(dev->chid);
fail2:
    munmap_device_io(dev->iovbase, OMAPL1xx_GPIO_SIZE);
fail1:
	out32(base + OMAPL137_SPI_GCR1, 0);
	out32(base + OMAPL137_SPI_GCR0, 0);
	munmap_device_io(dev->spivbase, OMAPL1xx_SPI0_SIZE);
fail0:
	free(dev);
	return NULL;
}




/*****************************************************************************************
* Function name		: int i1553LinkHandler(i1553_spi_t *dev)
* 	 returns		: bytes read
* 	 arg1			: resmgr context_t
* 	 arg2			: io message
* 	 arg3			: ocb
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: 1553 read operation
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
******************************************************************************************/

int i1553LinkHandler(i1553_spi_t *dev)
{

	struct _pulse    pulse;
	iov_t            iov;
	int              rcvid;
	int              i;
	uint16_t         xlen,xlen1,xlen2;
	uint16_t         paddinglen;
	uint16_t         txdata;


	/* Enable IO privilege */
	if(ThreadCtl(_NTO_TCTL_ID, 0) == -1)
	{
	    LOG_INFO("ThreadCtl failed\n");
		return EXIT_FAILURE;
	}

	LOG_INFO("i1553Linkhandler: 1553 link handler thread started with priority %d\n", getprio(0));

	SETIOV(&iov, &pulse, sizeof(pulse));

	while(1)
	{
		LOG_DETAILS("i1553LinkHandler: Waiting for message\n");
		rcvid = MsgReceive(dev->coid, &iov, 1, NULL);

		if(rcvid == -1)
		{
			if(errno == ESRCH)
			{
				LOG_INFO("i1553LinkHandler receive channel does not exit. End !\n");
				// No such channel exists
				pthread_exist(NULL);
			}
			continue;
		}

		else if(rcvid == 0)                      //it's a pulse
		{
			switch (pulse.code)
			{
			/*  Polling command received */
			case I1553_RX_POLLING:
			  if(dev->txbuf->fillLevel >= dev->txlen)
			  {
				  xlen = dev->txlen;
				  paddinglen = 0;
			   }
			  else
			  {
				  xlen = dev->txbuf->fillLevel;
				  paddinglen = dev->txlen - dev->txbuf->fillLevel;
				  dev->txbuf->underflow = 1;
			  }
              if(((uint32_t)(dev->txbuf->rdptr) + xlen * sizeof(uint16_t)) <= dev->txbuf->end)         /*check if over the circular buffer boundary*/
              {
            	  for(i=0; i<xlen; i++)
            	  {
            		  tx_data = *dev->txbuf->rdptr++;
            		  SPI_SEND_ONE_WORD(tx_data);
            	  }
              }
              else
              {
            	  xlen1 = (dev->txbuf->end - (uint32_t)(dev->txbuf->rdptr))/sizeof(uint16_t) + 1;
            	  xlen2 = xlen - xlen1;
            	  for(i=0; i<xlen1; i++)
            	  {
            		  tx_data = *dev->txbuf->rdptr++;
            		  SPI_SEND_ONE_WORD(tx_data);
            	  }
            	  dev->txbuf->rdptr = dev->txbuf->base;                /* reset the pointer to the start of circular buffer */
                  for(i=0; i<xlen2; i++)
                  {
                	  tx_data = *dev->txbuf->rdptr++;
                	  SPI_SEND_ONE_WORD(tx_data);
                  }
              }
              dev->txbuf->fillLevel -= xlen;                          /* update the fill level here after the data been send out */
              for(i=0; i<paddinglen; i++)
              {
            	  SPI_SEND_ONE_WORD(0);                               /* Padding 0s if underflow to keep the tool connected */
              }
              /* check if the buffer is already half empty, then need to notify the client that data need to be delivered */
              if(dev->txbuf->fillLevel <= dev->txbuf->maxSize/2)             // we don't use the IOFUNC_NOTIFY_OUTPUT_CHECK macro here, instead check by our selves to make sure the trigger function should be called */
              {
               iofunc_notify_trigger(dev->i1553_spi.notify, 1, IOFUNC_NOTIFY_OUTPUT);
              }

              break;

			case I1553_RX_MODE_CMD_NO_DATA:
			case I1553_RX_CMD_W_DATA:
				dev->i1553_cmd_rcv_cnt++;
				if(IOFUNC_NOTIFY_INPUT_CHECK(dev->i1553_spi.notify, dev->i1553_cmd_rcv_cnt, 0))                     /* the count value is default 1, there is no meaning check here just put here as holder that for later if we wanna notify the client when multi items avaialable */
                 iofunc_notify_tigger(dev->i1553_spi.notify, dev->i1553_cmd_rcv_cnt, IOFUNC_NOTIFY_INPUT);          /* Notify the client new command is available */
				break;
			case I1553_RX_EVENT:
				break;
			default:
				break;
			}
		}
		else
		{
			// If non-pulse message then reply error
			MsgReplyv(rcvid, ENOTSUP, *iov, 1);
		}
	}

   LOG_INFO("i1553LinkHandler Terminated...\n");

   return(0);
}

/*****************************************************************************************
* Function name		: void *i1553_rxbuf_read(void *hdl, uint8_t *buf, int *len)
* 	 returns		: bytes read
* 	 arg1			: driver handle
* 	 arg2			: read buf pointer
* 	 arg3			: read data length
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: 1553 read operation
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
******************************************************************************************/
void *i1553_rxbuf_read(void *hdl, uint8_t *buf, int *len)
{
  i1553_spi_t  *dev = hdl;
  int rlen;
  int nbytes,nbytes_1, nbytes_2;
  int nword = *len/sizeof(uint16_t);
  uint8_t  *dstbuf;

  rlen = *len;                     /*read the target read length */

  nbytes = (nword <= dev->rxbuf->fillLevel) ? rlen : dev->rxbuf->fillLevel*sizeof(uint16_t);

  /* check if over the circular buffer boundary */
  if(((uint32_t)(dev->rxbuf->rdptr) + nbytes) < dev->rxbuf->end)
  	 {
  		 memcpy(buf, (uint8_t *)(dev->rxbuf->rdptr), nbytes);
  	 }
  else
  {
	  dstbuf = buf;
	  nbytes_1 = dev->rxbuf->end - ((uint32_t)dev->rxbuf->rdptr) + 1;
	  nbytes_1 *= sizeof(uint16_t);
      nbytes_2 = nbytes - nbytes_1;

      memcpy(dstbuf, (uint8_t *)(dev->rxbuf->rdptr), nbytes_1);
      dstbuf += nbytes_1;
      dev->rxbuf->rdptr = dev->rxbuf->base;             /* reset the read pointer to beginning */
      memcpy(dstbuf, (uint8_t *)(dev->rxbuf->rdptr), nbytes_2);
  }

  *len = nbytes;                                           /* updated how many bytes really been read out */
  dev->rxbuf->fillLevel -= (nbytes * sizeof(uint16_t));    /* update driver circular buffer count value */
  return buf;

}
/*****************************************************************************************
* Function name		: int i1553_txbuf_write(void *hdl, uint8_t *buf, int *len)
* 	 returns		: bytes read
* 	 arg1			: driver handle
* 	 arg2			: write buf source pointer
* 	 arg3			: write data length
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: 1553 read operation
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
******************************************************************************************/
int i1553_txbuf_write(void *hdl, uint8_t *buf, int *len)
{
  i1553_spi_t  *dev = hdl;
  int wlen;
  int nbytes, nbytes_1, nbytes_2;
  int nword = *len/sizeof(uint16_t);
  uint8_t  *srcbuf;

  wlen = *len;                     /*read the target read length */

  nbytes = (nword <= dev->txbuf->fillLevel) ? wlen : dev->txbuf->fillLevel*sizeof(uint16_t);

  /* check if over the circular buffer boundary */
  if(((uint32_t)(dev->txbuf->wrptr) + nbytes) < dev->txbuf->end)
  	 {
  		 memcpy((uint8_t *)(dev->txbuf->wrptr), buf, nbytes);         // copy directly
  	 }
  else
  {
	  srcbuf = buf;
	  nbytes_1 = dev->txbuf->end - ((uint32_t)dev->txbuf->wrptr) + 1;
	  nbytes_1 *= sizeof(uint16_t);
      nbytes_2 = nbytes - nbytes_1;

      memcpy((uint8_t*)dev->txbuf->wrptr, srcbuf, nbytes_1);
      srcbuf += nbytes_1;
      dev->txbuf->wrptr = dev->txbuf->base;             /* reset the read pointer to beginning */
      memcpy((uint8_t *)(dev->txbuf->wrptr),srcbuf, nbytes_2);
  }

  *len = nbytes;                                           /* updated how many bytes really been read out */
  dev->txbuf->fillLevel -= (nbytes * sizeof(uint16_t));    /* update driver circular buffer count value */
  return nbytes;

}


