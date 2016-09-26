/*
 * Halliburton ESG
 * 1553 interrupt (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#include <sys/siginfo.h>
#include "omapl137spi.h"
#include "i1553protocal.h"

/*
 * 1553 receive interrupt handler
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */
static const struct sigevent *i1553_rx_intr(void *area, int id)
{
    i1553_spi_t  *dev = area;

    /*read the received 1553 data through spi from FPGA */
    uintptr_t    base = dev->spivbase;
    uint32_t     spibuf = in32(base + OMAPL137_SPI_BUF);
    uint32_t     cmd_flag = 0;
    uint16_t     io_number = 0;
    uint16_t     rx_word = 0;

    /*read the cmd_word line status */
    base = dev->iovbase;
    io_number = dev->io_number;
    cmd_flag = (in32(base) >> io_number) & 0x00000001;

    SPI_SEND_ONE_WORD(dev->rtu_status);                 /* this is dummy send for SPI read, FPGA won't transmit this one */

    while(spibuf & OMAPL137_SPIBUF_RXEMPTY)             /* check if rx ready, may need adding timeout here later */
    {
    	spibuf = in32(base + OMAPL137_SPI_BUF);
    }
    rx_word = (uint32_t)spibuf;

    /*check if it is a command */
    if(cmd_flag)
    {
      if(rx_word > 0x8000)                                             /* is it polling command */
      {
    	  SPI_SEND_ONE_WORD(dev->rtu_status);                          /* send the rtu status response right away to keep the tool connection, the response time should be less than 10 us */
          dev->txlen = (rx_word & 0x001F)?(rx_word & 0x001F):32;       /* check how many words the bcu poll */

          dev->i1553_rxevent.sigev_code = I1553_RX_POLLING;            /* tell the client that polling command received and data need to be send */
          return(&dev->i1553_rxevent);
      }
      switch (rx_word & RX_MODE_CMD_MASK)
      {
      case RX_MODE_CMD_NO_DATA:
    	 SPI_SEND_ONE_WORD(dev->rtu_status);                          /* mode command with no data, reply response immediately */
    	 dev->i1553_rxevent.sigev_code = I1553_RX_MODE_CMD_NO_DATA;
    	 break;
      case RX_MODE_CMD_ONE_WORD_DATA:
    	 dev->rxlen = 1;
    	 dev->i1553_rxevent.sigev_code = I1553_RX_EVENT;            /* need to wait for next data word to send the response, tell client just normal rx event to ignore */
    	 break;
      case RX_BLIND_CMD_W_DATA1:
      case RX_BLIND_CMD_W_DATA2:
    	 dev->rxlen = (rx_word & 0x001F)?(rx_word & 0x001F):32;     /* need to wait for received all the data word to send the response, tell client just normal rx event to ignore */
    	 dev->i1553_rxevent.sigev_code = I1553_RX_EVENT;
    	 break;
      default:
    	 dev->i1553_rxevent.sigev_code = I1553_RX_INVALID_CMD;
    	 break;
      }
    }
    else     /* it's a data word */
    {
      if(dev->rxlen != 0)
      {
    	  dev->rxlen--;
    	  if(!dev->rxlen)
    	  {
    		  SPI_SEND_ONE_WORD(dev->rtu_status);                  /* all data word received and need to reply right away */
              dev->i1553_rxevent.sigev_code = I1553_RX_CMD_W_DATA;
    	  }
      }
      else
      {
    	  dev->i1553_rxevent.sigev_code = I1553_RX_INVALID_DATA;
    	  return(&dev->i1553_rxevent);
      }
    }

    if(dev->rxbuf->fillLevel < dev->rxbuf->maxSize)
    {
    	*(dev->rxbuf->wrptr++) = rx_word;                                 /*copy data the circular buffer */
    	dev->rxbuf->fillLevel++;
    	if((uint32_t)(dev->rxbuf->wrptr) > dev->rxbuf->end)               /* check to see if reach the buffer end */
    	{
    		dev->rxbuf->wrptr = dev->rxbuf->base;
    	}
    }
    else
    {
         dev->rxbuf->overflow = 1;
    }
     return(&dev->i1553_rxevent);
}

/*
 * Attach 1553 receive interrupt
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */

int i1553_attach_intr(i1553_spi_t *i1553spi)
{
	if((i1553spi->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1)
		return -1;

	if((i1553spi->coid = ConnectAttach(0, 0, i1553spi->chid, _NTO_SIDE_CHANNEL, 0)) == -1)
		goto fail0;

    //i1553spi->i1553_rxevent.sigev_notify  = SIGEV_PULSE;
    //i1553spi->i1553_rxevent.sigev_coid    = i1553spi->coid;
    //i1553spi->i1553_rxevent.sigev_code    = I1553_RX_EVENT;
    //i1553spi->i1553_rxevent.sigev_priority = I1553_RX_PRIORITY;

    SIGEV_PULSE_INIT(&(i1553spi->i1553_rxevent), i1553spi->coid, I1553_RX_PRIORITY, I1553_RX_EVENT, 1);

    /*
     * Attach the interrupt
     */

    i1553spi->rx_iid = InterruptAttach(i1553spi->i1553irq, i1553_rx_intr, i1553spi, 0, _NTO_INTR_FLAGS_TRK_MSK);

    if(i1553spi->rx_iid != -1)
    	return 0;

    ConnectDetach(i1553spi->coid);
 fail0:
    ChannelDestroy(i1553spi->chid);
}

