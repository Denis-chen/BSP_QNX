/*
 * $QNXLicenseC:
 * Copyright 2009 QNX Software Systems. 
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

#include <sim_mmc.h>

#ifdef MMCSD_VENDOR_MCI
 
#include <sim_at91sam9xx.h>

#define TIMEOUT_LOOPS 10000

/******************************
 *  DMAC functions START
 *****************************/
dmac_dev_t * dmac_init (mci_ext_t *mci)
{
	rsrc_request_t req;
	dmac_dev_t     *dmac_dev;

	/* Allocated memory for channel */
	dmac_dev = (dmac_dev_t*)calloc(1, sizeof (dmac_dev_t));
	if (dmac_dev == NULL)
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: DMAC calloc failed\n");
		return NULL;
	}

	/* Map DMAC controller */
	dmac_dev->dmac = (at91sam9xx_dmac_t *)mmap_device_memory(NULL,0x200,
		PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED, (uint32_t)(mci->dbase));

	if (dmac_dev->dmac == MAP_FAILED)
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: DMAC MAP_FAILED\n");
		free (dmac_dev);
		return NULL;
	}

	/* Apply DMAC channel */	
	memset(&req, 0, sizeof(req));
	req.length = 1;
	req.flags = RSRCDBMGR_DMA_CHANNEL | RSRCDBMGR_FLAG_TOPDOWN;

	if (rsrcdbmgr_attach( &req, 1) == -1) 
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: Cannot acquire DMAC channel\n");
		munmap_device_memory((void *)dmac_dev->dmac, sizeof(at91sam9xx_dmac_t));
		free (dmac_dev);
		return NULL;
	}
	else
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: Use DMAC channel %lld\n", req.start);
		dmac_dev->chid = req.start;
	}

	/* Stop channel */
	dmac_dev->dmac->chdr = 1 << (dmac_dev->chid + 8);
	
	/* Reset HW LLI table*/
	memset(&(dmac_dev->dmac->lli[dmac_dev->chid]), 0, sizeof(at91sam9xx_dmac_lli_t));

	dmac_dev->dmac->en = DMAC_ENABLE;

	return dmac_dev;
}


void dmac_fini (dmac_dev_t * dmac_dev)
{
	rsrc_request_t req;
    
	/* Release DMAC channel */	
	memset(&req, 0, sizeof(req));
	req.length = 1;
	req.flags = RSRCDBMGR_DMA_CHANNEL;
	req.start = dmac_dev->chid;
	req.end = dmac_dev->chid;

	/* Return the resource to the database: */
	rsrcdbmgr_detach( &req, 1);
	
	if (dmac_dev != NULL)
	{
		if ( dmac_dev->dmac!= MAP_FAILED)
		{
			munmap_device_memory((void *)dmac_dev->dmac, sizeof(at91sam9xx_dmac_t));
		}
		
		free (dmac_dev);
	}
}


int dmac_channel_attach (dmac_dev_t * dmac_dev, unsigned interface)
{
	/* Setup CFG */
	dmac_dev->dmac->lli[dmac_dev->chid].cfg  = DMAC_SOD_DISABLE 
			| (interface << 0) | (interface << 4);

	return EOK;
}

int dmac_xfer_start (dmac_dev_t * dmac_dev)
{
	if (dmac_dev->dir == DATA_WRITE)
	{
		/* Enable DMA request */
		/* Copy current TCD image into DMA controller */
		dmac_dev->dmac->lli[dmac_dev->chid].saddr = 0;
		dmac_dev->dmac->lli[dmac_dev->chid].daddr = 0;
		dmac_dev->dmac->lli[dmac_dev->chid].ctrlb = 0;
		dmac_dev->dmac->lli[dmac_dev->chid].ctrla = 0;
		dmac_dev->dmac->lli[dmac_dev->chid].dscr = (uint32_t) mphys(&dmac_dev->lli[0]);
	}

	/* Trigger DMA transfer */
	dmac_dev->dmac->cher = (1 << dmac_dev->chid);

	return 0;
}

int dmac_xfer_stop (dmac_dev_t * dmac_dev)
{
	/* Stop DMA transfer */
	dmac_dev->dmac->cher = (1 << (dmac_dev->chid + 8));

	return 0;
}

void dmac_channel_release (dmac_dev_t * dmac_dev)
{
	int timeout = 0;

	/* Disable channel */	
	dmac_dev->dmac->chdr |= (1 << dmac_dev->chid);
	
	while (dmac_dev->dmac->chsr & (1 <<dmac_dev->chid))
	{
		delay (1);
		if (timeout++ > TIMEOUT_LOOPS)
		{
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "%s: release DMAC channel time out\n", __func__);
			break;
		}
	}
		
	/* release DMAC mmap resources */
	if (dmac_dev->blknum > 0) 
	{
		munmap (dmac_dev->lli, (dmac_dev->blknum) * sizeof(at91sam9xx_dmac_bd_t) );
	}

	dmac_dev->blknum = 0;
}

int dmac_setup_xfer (dmac_dev_t * dmac_dev, paddr_t paddr, int len, int dir)
{
	/* Stop Channel */
	dmac_dev->dmac->chdr = 1 << (dmac_dev->chid + 8);

	dmac_dev->blknum = 0;

	if (dir == DATA_READ)		/* For read, single block is enough */
	{
		/* Set last lli descriptor to 0 to mark NULL */
		dmac_dev->dmac->lli[dmac_dev->chid].dscr = 0;
		/* Set Source and address transfer size and modulo operation */
		dmac_dev->dmac->lli[dmac_dev->chid].ctrla = ((len/4) & 0xffff) 
			| SRC_WIDTH(2) | DST_WIDTH(2);
		dmac_dev->dmac->lli[dmac_dev->chid].ctrlb = 0x0;
		dmac_dev->dmac->lli[dmac_dev->chid].cfg &= ~(DMAC_DST_H2SEL_HW | DMAC_SRC_H2SEL_HW);
		dmac_dev->dmac->lli[dmac_dev->chid].ctrla |= SCSIZE;
		dmac_dev->dmac->lli[dmac_dev->chid].ctrlb |= (SET_DSCR | DMAC_FC_PER2MEM);
		dmac_dev->dmac->lli[dmac_dev->chid].ctrlb |= FLAG_SRC_INCREMENT; 
		dmac_dev->dmac->lli[dmac_dev->chid].ctrlb |= FLAG_DST_INCREMENT;
		dmac_dev->dmac->lli[dmac_dev->chid].daddr = paddr;
		dmac_dev->dmac->lli[dmac_dev->chid].saddr = dmac_dev->io_addr;
		dmac_dev->dmac->lli[dmac_dev->chid].cfg |= DMAC_DST_H2SEL_SW | DMAC_SRC_H2SEL_HW;
	} 
	else if (dir == DATA_WRITE)	/* For write, use multiple block for performance */
	{
		int i, blknum = len/dmac_dev->blksz;

		dmac_dev->blknum = blknum;

		dmac_dev->dmac->lli[dmac_dev->chid].cfg &= ~(DMAC_DST_H2SEL_HW | DMAC_SRC_H2SEL_HW);
		dmac_dev->dmac->lli[dmac_dev->chid].cfg |= DMAC_DST_H2SEL_HW | DMAC_SRC_H2SEL_SW;

		/* setup buffer link list */
		dmac_dev->lli = mmap (0, (blknum) * sizeof(at91sam9xx_dmac_bd_t), 
			PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED, NOFD, 0);

		if (dmac_dev->lli == MAP_FAILED)
		{
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: DMA setup_xfer map failed\n");
			return (-1);
		}

		memset(dmac_dev->lli, 0, blknum * sizeof(at91sam9xx_dmac_bd_t));

		for (i = 0; i < blknum; i++)
		{
			/* Set Source and address transfer size and modulo operation */
			dmac_dev->lli[i].ctrla = ((dmac_dev->blksz/4) & 0xffff) 
				| SRC_WIDTH(2) | DST_WIDTH(2) | DCSIZE;
			dmac_dev->lli[i].ctrlb = DMAC_FC_MEM2PER | FLAG_SRC_INCREMENT 
				| FLAG_DST_INCREMENT;
			dmac_dev->lli[i].saddr = paddr + dmac_dev->blksz * i;
			dmac_dev->lli[i].daddr = dmac_dev->io_addr;

			if (i == blknum - 1)
			{
				/* Set last lli descriptor to 0 to mark NULL */
				dmac_dev->lli[i].dscr = 0;
			}
			else
			{
				/* Set next lli descriptor */
				dmac_dev->lli[i].dscr = mphys(&dmac_dev->lli[i + 1]);
			}
		}
	}
	else
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: DMA setup_xfer neither READ or WRITE\n");
		return -1;
	}

	return 0;
}

/******************************
 *  DMAC functions END
 *****************************/

static int stopdma (SIM_HBA *hba)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	if (mci->ctrler == PDC) 
	{
		WRITE32(MCI_PTCR, MCI_TXTDIS | MCI_RXTDIS);
		WRITE32(MCI_RPR, 0);
		WRITE32(MCI_RCR, 0);
		WRITE32(MCI_RNPR, 0);
		WRITE32(MCI_RNCR, 0);
		WRITE32(MCI_TPR, 0);
		WRITE32(MCI_TCR, 0);
		WRITE32(MCI_TNPR, 0);
		WRITE32(MCI_TNCR, 0);
	}

	if (mci->ctrler == DMAC)
	{
		dmac_xfer_stop(mci->dmac_dev);
	}

	return 0;
}

static int startdma (SIM_HBA *hba, int dir)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	if (mci->ctrler == PDC) 
	{
		if (dir == DATA_READ)
		{
			WRITE32(MCI_PTCR, MCI_RXTEN);
		}
		else if (dir == DATA_WRITE)
		{
			WRITE32 (MCI_PTCR, MCI_TXTEN);
		}
		else
		{
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: DMA transfer neither READ or WRITE\n");
			return -1;
		}
	}
	else if (mci->ctrler == DMAC)
	{
		dmac_xfer_start(mci->dmac_dev);
	}
	else
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: DMA controller neither PDC or DMAC\n");
		return -1;
	}
	
	return 0;
}


static int _mci_detect (SIM_HBA *hba)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	/* card detection pin is not available */
	
	return (MMC_SUCCESS);
}

static int _mci_interrupt (SIM_HBA *hba, int irq, int resp_type, uint32_t *resp)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;
	uint32_t	status, status2;
	int		intr;
	volatile int	sts;
	int		timeout = 0;
	
	intr = MMC_INTR_NONE;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	status2 = READ32(MCI_SR);
	status = status2 & READ32(MCI_IMR);

	/* Disable incoming interrupt */
	WRITE32 (MCI_IDR, status);

	/*
	 * If there is error status and it is not a CRC error for R3, 
	 * since R3 command doesn't require CRC check.
	 */
	if ( (status & MCI_ERRORS) 
	&& (! ((resp_type == MMC_RSP_R3) && ((status & MCI_ERRORS) == RCRCE))))
	{
		intr |= MMC_INTR_ERROR;

		if (status & RINDE ) {
			intr |= MMC_ERR_CMD_IDX;
		}

		if (status & RCRCE ) {
			intr |= MMC_ERR_CMD_CRC;
		}

		if (status & RENDE ) {
			intr |= MMC_ERR_CMD_END;
		}

		if (status & RTOE ) {
			intr |= MMC_ERR_CMD_TO;
		}

		if (status & DCRCE ) {
			intr |= MMC_ERR_DATA_CRC;
		}

		if (status & DTOE ) {
			intr |= MMC_ERR_DATA_TO;
		}
		
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: interrupt status 0x%x", status);
	}

	else if (status & CMDRDY) {			/* Send Command done */
		intr |= MMC_INTR_COMMAND;

		/* Handle a normal command that has been completed */
		if (resp_type & MMC_RSP_PRESENT) {
			if (resp_type & MMC_RSP_136)	/* 136 bit response */
			{
				resp[3] = READ32(MCI_RSPR0);
				resp[2] = READ32(MCI_RSPR1);
				resp[1] = READ32(MCI_RSPR2);
				resp[0] = READ32(MCI_RSPR3);
			} else							/* 48 bit response */
			{
				resp[0] = READ32(MCI_RSPR0);
			}
		}

		if ((mci->cmd == MMC_WRITE_BLOCK) || (mci->cmd == MMC_WRITE_MULTIPLE_BLOCK))
		{
			if (mci->ctrler == DMAC)	/* If use DMAC */
			{
				/* Clean up pending BTC flags */
				sts = mci->dmac_dev->dmac->ebcisr;
			}

			WRITE32(MCI_MR, ((READ32(MCI_MR) | WRPROOF| mci->blksz << 16)));

			if (mci->cmd == MMC_WRITE_BLOCK)	/* For single block write */
			{
				WRITE32(MCI_IER, MCI_ERRORS | XFRDONE);
				WRITE32(MCI_BLKR, (mci->blksz << 16) | 1);	/* block number is 1 for single block */
				WRITE32(MCI_DMA, DMAEN);
				startdma(hba, DATA_WRITE);
			}
			else							/* For multiple block write */
			{
				WRITE32(MCI_BLKR, (mci->blksz << 16) | mci->blknum);
				WRITE32(MCI_DMA, DMAEN);
				startdma(hba, DATA_WRITE);
				if (mci->ctrler == DMAC)	/* If use DMAC */
				{
					/* Wait for DMA done */
					sts = mci->dmac_dev->dmac->ebcisr;

					/* 
					 * Check both BTC bit (b0 - b7) and CBTC bit (b8 - b15) of the specified 
					 * channel (chid) to make sure they are both set, which indicate buffer/Chained 
					 * buffer transferring have terminated and LLI Fetch operation has been disabled.
					 */
					while ( !( ( (sts>>(mci->dmac_dev->chid + 8)) & 1) & ( (sts>>(mci->dmac_dev->chid)) & 1) ) )
					{
						delay (1);
						sts = mci->dmac_dev->dmac->ebcisr;
						if (timeout++ > TIMEOUT_LOOPS)
						{
							slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "%s: wait for DMAC done time out\n", __func__);
							break;
						}
					}

					/* DMA done, wait for FIFO empty */
					WRITE32(MCI_IER, MCI_ERRORS | FIFOEMPTY);
				}
				else					/* If use PDC */
				{
					/* Wait for DMA done */
					sts = READ32(MCI_SR);
					while ( ! (sts&BLKE))
					{
						delay (1);
						sts = READ32(MCI_SR);
						if (timeout++ > TIMEOUT_LOOPS)
						{
							slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "%s: wait for PDC done time out\n", __func__);
							break;
						}
					}

					intr |= MMC_INTR_DATA;
				}
			}
		}
	} 
	else if ( ((status & ENDRX) || (status & NOTBUSY)) 
			&& (mci->ctrler == PDC)) 
	{
		/* Read/Write data done for PDC */
		if (mci->cmd == MMC_STOP_TRANSMISSION)	/* For multiblock */
		{
			intr |= MMC_INTR_COMMAND;
			resp[0] = READ32(MCI_RSPR0);
		}
		else									/* For other case */
		{
			intr |= MMC_INTR_DATA;
		}
	}
	else if ( (status & FIFOEMPTY) && (mci->ctrler == DMAC))
	{
		/* Write done for DMAC */
		intr |= MMC_INTR_DATA;
	}
	else if ( ((status & XFRDONE) || (status & NOTBUSY))
			&& (mci->ctrler == DMAC))
	{
		/* Read/Write data done for DMAC */
		if (mci->cmd == MMC_STOP_TRANSMISSION)	/* For multiblock stop */
		{
			intr |= MMC_INTR_COMMAND;
			resp[0] = READ32(MCI_RSPR0);
		}
		else									/* For other cases */
		{
			intr |= MMC_INTR_DATA;
		}
	}

	return intr;
}

/*
 * setup DMA transfer
 */
static int _mci_setup_dma (SIM_HBA *hba, paddr_t paddr, int len, int dir)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;
	uint32_t	xlen = 0;
	uint32_t	mr;
	uint16_t	blkcnt;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	if (mci->ctrler == PDC)
	{
		
		if (dir == DATA_READ)
		{

			if (len > mci->blksz) 
			{
				xlen = mci->blksz ;
			}
			else
			{
				xlen = len;
			}

			mr = READ32(MCI_MR) & 0xffff;
			WRITE32(MCI_MR, mr | (mci->blksz << 16));
			WRITE32(MCI_BLKR, 1 | (mci->blksz << 16));	/* block number is 1 for single block */
			WRITE32(MCI_RPR, paddr);
			WRITE32(MCI_RCR, xlen / 4);

		} 
		else if (dir == DATA_WRITE)
		{

			blkcnt = len / mci->blksz;
			xlen = mci->blksz * blkcnt;

			if (blkcnt == 0)
			{
				return 0;
			}

			mr = READ32(MCI_MR) & 0xffff;
			WRITE32(MCI_MR, mr | (mci->blksz << 16));
			WRITE32(MCI_BLKR, blkcnt | (mci->blksz << 16));

			mci->blknum = blkcnt;
			WRITE32(MCI_TPR, paddr);
			WRITE32(MCI_TCR, xlen / 4);
		}
	}
	else if (mci->ctrler == DMAC)
	{
		mci->dmac_dev->dir = dir;

		if (dir == DATA_READ)
		{
			if (len > mci->blksz) 
			{
				xlen = mci->blksz ;
			}
			else
			{
				xlen = len;
			}

			mr = READ32(MCI_MR) & 0xffff;
			WRITE32(MCI_MR, mr | (xlen << 16));
			dmac_channel_attach(mci->dmac_dev, mci->dintf);

			if (dmac_setup_xfer (mci->dmac_dev, paddr, xlen, dir) == -1)
			{
				return -1;
			}
		}
		else
		{
			blkcnt = len / mci->blksz;
			xlen = mci->blksz * blkcnt;

			if (blkcnt == 0)
			{
				return 0;
			}

			mci->blknum = blkcnt;

			mr = READ32(MCI_MR) & 0xffff;
			WRITE32(MCI_MR, mr | (mci->blksz << 16));
			WRITE32(MCI_BLKR, (mci->blksz << 16) | blkcnt);
			dmac_channel_attach(mci->dmac_dev, mci->dintf);

			if (dmac_setup_xfer (mci->dmac_dev, paddr, xlen, dir) == -1)
			{
				return -1;
			}
		}
	}

	return (xlen);
}

static int _mci_dma_done (SIM_HBA *hba, int dir)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	/* Disable DMA */
	stopdma (hba);
	
	if (mci->ctrler == DMAC)
	{
		dmac_channel_release(mci->dmac_dev);
	}
	
	/* Disable all incoming interrupt */
	WRITE32 (MCI_IDR, 0xffffffff);

	return MMC_SUCCESS;
}

static int _mci_command (SIM_HBA *hba, mmc_cmd_t *cmd)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;
	uint32_t	command;
	uint32_t	mask = 0;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	command = cmd->opcode;
	mci->cmd = cmd->opcode;

	if (cmd->resp & MMC_RSP_PRESENT) {	/* If need response */
		command |= MAXLAT;				/* Wait maximum time */
		if (cmd->resp & MMC_RSP_BUSY)
		{
			command |= RSPTYPE_R1B;
		}
		else if (cmd->resp & MMC_RSP_136) 
		{
			command |= RSPTYPE_136;
		}
		else 
		{
			command |= RSPTYPE_48;
		}
	} else 
	{
		command |= RSPTYPE_NONE;
	}
	if (cmd->opcode == MMC_STOP_TRANSMISSION)
	{
		command |= (TRCMD_STOP | TRTYP_MB);
	}
	else if (cmd->eflags & MMC_CMD_DATA) 
	{
		if (mci->ctrler == DMAC)
		{
			WRITE32(MCI_DMA, (READ32(MCI_DMA) | DMAEN));
			WRITE32(MCI_MR, ((READ32(MCI_MR) | WRPROOF| RDPROOF | BLK_LENGTH << 16)));
		}

		if (cmd->eflags & MMC_CMD_DATA_IN) 
		{
			command |= (TRDIR | TRCMD_START);       
		}
		else 
		{
			command |= TRCMD_START;
		}

		if(cmd->eflags & MMC_CMD_DATA_MULTI) 
		{
			command |= TRTYP_MB; 
		}
		else {
			command |= TRTYP_SB;
		}
	} 
        
	/* Set the arguments and send the command */
	if (cmd->resp & MMC_RSP_BUSY) 
	{
		mask |= NOTBUSY;
	}
	else if (!(cmd->eflags & MMC_CMD_DATA)) 
	{
		mask |= CMDRDY; 
	} else 
	{
		if (command & TRCMD_START) 
		{
			if (mci->ctrler == PDC)
			{
				if (command & TRDIR) 
				{
					mask |= ENDRX;
				}
				else 
				{
					mask |= CMDRDY | NOTBUSY;
				}
			}
			else 
			{
				if (command & TRDIR) 
				{
					mask |= XFRDONE;
				}
				else
				{
					mask |= CMDRDY;
				}
				
			}
		}
	}

	/* If command is READ DATA then enable the DMA */
	if (command & TRCMD_START) 
	{
		if (command & TRDIR) 
		{
			startdma (hba, DATA_READ);
		}
	}

	/* Send the command */
	WRITE32(MCI_ARGR, cmd->argument);
	WRITE32(MCI_CMDR, command);

	/* Enable selected interrupts */
	WRITE32(MCI_IER, MCI_ERRORS | mask);

	return (MMC_SUCCESS);
}

static int _mci_cfg_bus (SIM_HBA *hba, int width, int mmc)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;
	uint32_t	value;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;
	
	value = READ32 (MCI_SDCR) & 0xFFFFFF3F;
	
	if (width == 8) 
	{
		WRITE32 (MCI_SDCR, value | SDCBUS8B);
	}
	else if (width == 4) 
	{
		WRITE32 (MCI_SDCR, value | SDCBUS4B);
	}
	else
	{
		WRITE32 (MCI_SDCR, value);
	}
	
	return (MMC_SUCCESS);
}

static int _mci_clock (SIM_HBA *hba, int *clock, int high_speed)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;
	uint32_t	div;
	uint32_t	value;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;
	
	if (*clock > mci->clock ) 
	{
		*clock = mci->clock;
	}
	
	if (*clock < (mci->clock/512))
	{
		*clock = mci->clock / 512;
	}
	
	div = mci->clock/ *clock;

	if (div > 255) 
	{
		div = 255;
	}


	*clock = mci->clock/div;

	value = READ32 (MCI_MR) & 0xffffff00;	
	WRITE32 ( MCI_MR, value | div);

	return (MMC_SUCCESS);
}

static int _mci_block_size (SIM_HBA *hba, int blksz)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;
	uint32_t	value;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	if (blksz > BLK_LENGTH)
	{
		return (MMC_FAILURE);
	}
		
	mci->blksz = blksz;
	if (mci->ctrler == DMAC)
	{
		mci->dmac_dev->blksz = blksz;
	}
	value = READ32 (MCI_MR) & 0xffff;
	WRITE32 (MCI_MR, value | blksz);
	
	return (MMC_SUCCESS);
}

/*
 * Reset host controller and card
 * The clock should be enabled and set to minimum (<400KHz)
 */
static int _mci_powerup (SIM_HBA *hba)
{
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;

	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	return (MMC_SUCCESS);
}

static int _mci_powerdown (SIM_HBA *hba)
{
	CONFIG_INFO	*cfg;
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;

	ext = (SIM_MMC_EXT *)hba->ext;
	cfg = (CONFIG_INFO *)&hba->cfg;
	mci = (mci_ext_t *)ext->handle;

	return (MMC_SUCCESS);
}

static int _mci_shutdown (SIM_HBA *hba)
{
	CONFIG_INFO	*cfg;
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci;
	uintptr_t	base;

	ext = (SIM_MMC_EXT *)hba->ext;
	cfg = (CONFIG_INFO *)&hba->cfg;
	mci = (mci_ext_t *)ext->handle;
	base = mci->base;

	/* Reset and disable controller */
	WRITE32 (MCI_CR, SWRST|PWSDIS);
	delay (100);
	WRITE32 (MCI_CR, MCIDIS | PWSDIS);

	if (mci->ctrler == DMAC) 
	{
		/* Indicate we are done with DMA lib */
		dmac_fini(mci->dmac_dev);
	}

	munmap_device_memory ((void *)mci->base, cfg->MemLength[0]);

	free(mci);

	return (MMC_SUCCESS);
}

/* MCI args */
static char *opts[] = 
{
	"slot",		// selection of mci slot: 0 or 1
	"dmac",		// DMA controller is DMAC
	"pdc",		// DMA controller is PDC
	"dbase",	// base address of DMAC controller
	"dintf",	// HW interface number of DMAC controller
	NULL
};

static int mci_args (SIM_HBA *hba, char *options)
{
	SIM_MMC_EXT *ext;
	mci_ext_t   *mci;
	char        *value;
	int         opt;
	int         val;
	int         idx;
	
	ext = (SIM_MMC_EXT *)hba->ext;
	mci = (mci_ext_t *)ext->handle;

	for (idx = 0; idx < strlen(options); idx++)
	{
		if (':'==options[idx]) 
		{
			options[idx] = ',';
		}
	}

	strlwr(options);

	if (*options == '\0') {	
		return (0);
	}

	while (*options != '\0') {
		if ((opt = getsubopt(&options, opts, &value)) == -1) {
			continue;
		}
		switch (opt) {
			case 0:
				val = strtoull(value, 0, 0);
				if ((val > 1) || (val < 0)) {
					slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, 
						"MMC: wrong MCI slot option %d", val);
					slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, 
						"MMC: MCI slot must be 0 (A) or 1 (B)");
				}
				else {
					mci->slot = val;
				}
				break;
			case 1:
				if (mci->ctrler == PDC)
				{
					slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, 
						"MMC: only one DMA controller (DMAC or PDC) can be used");
					return (-1);
				}
				mci->ctrler = DMAC;
				break;
			case 2: 
				if (mci->ctrler == DMAC)
				{
					slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, 
						"MMC: only one DMA controller (DMAC or PDC) can be used");
					return (-1);
				}
				mci->ctrler = PDC;
				break;
			case 3: 
				mci->dbase = strtoull(value, 0, 0);
				break;
			case 4: 
				mci->dintf = strtoull(value, 0, 0);
				break;
			default:
				slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, 
					"MMC: Unrecognized options %s\n", value);
				return (-1);
		}
	}

	return (0);
}

int mci_init (SIM_HBA *hba)
{
	CONFIG_INFO	*cfg;
	SIM_MMC_EXT	*ext;
	mci_ext_t	*mci = NULL;
	uintptr_t	base;

	ext = (SIM_MMC_EXT *)hba->ext;
	cfg = (CONFIG_INFO *)&hba->cfg;
	hba->verbosity = 4;

	if (!ext->opts) 
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: missing board-specific options\n");
		goto ARGSERR;
	}
	
	if ((mci = calloc(1, sizeof(mci_ext_t))) == NULL)
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: alloc memory failed\n");
		goto ERR;
	}

	cfg->MemLength[0] = 0x1000;
	cfg->NumMemWindows = 1;
	cfg->MemBase[0] = cfg->IOPort_Base[0];

	base = (uintptr_t)mmap_device_memory(NULL, cfg->MemLength[0],
		PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED, cfg->MemBase[0]);

	if (base == (uintptr_t)MAP_FAILED) 
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: mmap_device_memory failed\n");
		goto ERR;
	}

	mci->clock     = 100000000;
	mci->base      = base;
	mci->hba       = hba;
	ext->handle    = mci;
	ext->clock     = mci->clock;
	ext->detect    = _mci_detect;
	ext->powerup   = _mci_powerup;
	ext->powerdown = _mci_powerdown;
	ext->cfg_bus   = _mci_cfg_bus;
	ext->set_clock = _mci_clock;
	ext->set_blksz = _mci_block_size;
	ext->interrupt = _mci_interrupt;
	ext->command   = _mci_command;
	ext->setup_dma = _mci_setup_dma;
	ext->dma_done  = _mci_dma_done;
	ext->setup_pio = NULL;
	ext->pio_done  = NULL;
	ext->shutdown  = _mci_shutdown;

	/* Parse options */
	mci->slot = -1;
	mci->ctrler = -1;
	mci->dbase = 0;
	mci->dintf = -1;
	mci->blksz = BLK_LENGTH;

	if (mci_args(hba, ext->opts) == -1)
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: improper options used");
		goto ARGSERR;
	}

	if (mci->slot == -1 ) 
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: no slot specified");
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: select slot 0 (A)");
		mci->slot = 0;
	}
	else 
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: select slot %d", mci->slot);
	}

	if (mci->ctrler == -1) 
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: no DMA controller specified (DMAC or PDC)");
		goto ARGSERR;
	}

	if (mci->ctrler == DMAC)
	{
		if (mci->dbase == 0)
		{
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: no DMAC base address specified");
			goto ARGSERR;
		}
		
		if (mci->dintf == -1)
		{
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: no DMAC HW interface specified");
			goto ARGSERR;
		}
	}

	/* Configure capacity of controller */
	ext->hccap |= MMC_HCCAP_BW1 | MMC_HCCAP_BW4 | MMC_HCCAP_DMA;
	if (mci->ctrler == DMAC) 
	{
		ext->hccap |= MMC_HCCAP_BW8;
	}

	/* Disable the controller */
	WRITE32(MCI_CR, SWRST|PWSDIS);
	delay (100);
	WRITE32(MCI_CR, MCIDIS | PWSDIS); 

	/* Enable the controller */
	WRITE32(MCI_CR, MCIEN | PWSDIS);
	WRITE32(MCI_IDR, 0xffffffff);

	/* Set Timeout to Max */
	WRITE32(MCI_DTOR, 0x7f);

	/* Use the lowest baudrate */
	if (mci->ctrler == PDC)
	{
		WRITE32 ( MCI_MR, 0xff | PWSDIV(3) | PDCMODE);
	}
	else if (mci->ctrler == DMAC)
	{
		WRITE32 (MCI_MR, 0xff | WRPROOF| RDPROOF);
		
		mci->dmac_dev = dmac_init(mci);

		if (mci->dmac_dev == NULL)
		{
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: dmafuncs init FAILED\n");
			goto ERR;
		}

		mci->dmac_dev->io_addr = cfg->MemBase[0] + MCI_FIFO;
		mci->dmac_dev->blksz = mci->blksz;
	}
	else
	{
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: wrong DMA controller");
		goto ERR;
	}

	/* Select slot, set bus to 1 bit */
	WRITE32 (MCI_SDCR, mci->slot);

	if (!cfg->Description[0])
		strncpy(cfg->Description, "Atmel MCI/HSMCI ", sizeof(cfg->Description));

	return (MMC_SUCCESS);

ARGSERR:
	printf("\nImproper board-specific options used. Accepting args: \n");
	printf("    slot=#       The slot been used (0 or 1)\n");
	printf("    pdc          The SoC use Peripheral DMA Controller\n");
	printf("    dmac         The SoC use DMAC controller\n");
	printf("    dbase=#      The base address of DMAC controller if DMAC is used\n");
	printf("    dintf=#      The hardware interface # of MCI DMA request if DMAC is used\n");
	printf("NOTE:\n");
	printf("    1. pdc and dmac cannot be used the the same time\n");
	printf("    2. The args are seperated by colon ':'\n");
	printf("Example:\n");
	printf("at91sam9263 port 1: devb-mmcsd-at91sam9xx mmcsd ioport=0xFFF80000,irq=10,bs=pdc\n");
	printf("at91sam9263 port 2: devb-mmcsd-at91sam9xx mmcsd ioport=0xFFF84000,irq=11,bs=pdc\n");
	printf("at91sam9g45 port 1: devb-mmcsd-at91sam9xx mmcsd ioport=0xFFF80000,irq=11,bs=dmac:dbase=0xffffec00:dintf=0\n");
	printf("at91sam9g45 port 2: devb-mmcsd-at91sam9xx mmcsd ioport=0xFFFD0000,irq=29,bs=dmac:dbase=0xffffec00:dintf=13\n");

ERR:
	if (mci)
	{
		munmap_device_memory ((void *)mci->base, (uint32_t)cfg->MemLength[0]);
		free (mci);
	}
	return (MMC_FAILURE);
}

#endif

