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





#include <dm644x_emac.h>

/*
 * forward decls 
 */

	void dump_pkt(npkt_t *npkt)
	{
		int				i, j, len;
		net_buf_t		*buf;
		net_iov_t		*iov;
		unsigned char	*p;
		int				line_cnt;

		len = npkt->framelen;
		line_cnt = 0;
		for (buf = TAILQ_FIRST(&npkt->buffers); buf; buf = TAILQ_NEXT(buf, ptrs)) {
			for (i = 0, iov = buf->net_iov; i < buf->niov; i++, iov++) {
				for (j = 0, p = iov->iov_base; len && j < iov->iov_len; j++, p++, len--) {
					fprintf(stderr, "%02x", *p);
					line_cnt++;
					if ((line_cnt % 16) == 0)
						fprintf(stderr, "\n");
					else
						fprintf(stderr, ":");
				}
			}
		}

		fprintf(stderr, "\n");
	}

int dm644x_add_rbuff(dm644x_dev_t *dm644x, int idx, npkt_t *npkt) 
{
	net_buf_t	*buf;
	net_iov_t	*iov;

	dm644x->rx_pktq[idx] = npkt;
	buf = TAILQ_FIRST(&npkt->buffers);
	iov = buf->net_iov;

	CACHE_INVAL(&dm644x->cachectl, iov->iov_base, iov->iov_phys, iov->iov_len);

	dm644x->rx_desc[idx].buffer       = (unsigned char *)iov->iov_phys;
	dm644x->rx_desc[idx].buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
	dm644x->rx_desc[idx].pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;

	return (0);
}

int dm644x_receive(dm644x_dev_t * dm644x) {
	npkt_t         *npkt;
	npkt_t         *new;
	int             cidx;
	int             pkt_len;
	int             pkt_cnt;
	int             status;
	int             ret;
	net_buf_t      *buf;
	emac_regs	   *emac  = dm644x->emac;
	npkt_t         *tx_Q_head = NULL, *tx_Q_tail = NULL;

	pkt_cnt = 0;
	ret = 1;

	while (1) {
		cidx = dm644x->rx_cidx;
		status = dm644x->rx_desc[cidx].pkt_flag_len;
		if (status & EMAC_CPPI_OWNERSHIP_BIT)
			break;

		pkt_len = status & 0xFFFF;
		npkt = dm644x->rx_pktq[cidx];

		/* advance consumer index for the next loop */
		if (++dm644x->rx_cidx >= MAX_RX_BUFFERS)
			dm644x->rx_cidx = 0;

		if (status & EMAC_CPPI_RX_ERROR_FRAME) {
			dm644x_add_rbuff(dm644x, cidx, npkt);
			emac->RX0CP = (unsigned int)(ion_mphys((void *)&dm644x->rx_desc[cidx]));
			continue;
		}

		pthread_mutex_lock(&dm644x->rx_freeq_mutex);
		if (new = dm644x->rx_free) {
			dm644x->rx_free = new->next;
			new->next = NULL;
			dm644x->num_rx_free--;
			pthread_mutex_unlock(&dm644x->rx_freeq_mutex);
		}
		else {
			pthread_mutex_unlock(&dm644x->rx_freeq_mutex);
			if (!(new = dm644x_alloc_npkt(dm644x, MAX_BUF_SIZE))) {
				dm644x_add_rbuff(dm644x, cidx, npkt);
				emac->RX0CP = (unsigned int)(ion_mphys((void *)&dm644x->rx_desc[cidx]));
				continue;
			}
		}
		new->flags = _NPKT_UP;

		dm644x_add_rbuff(dm644x, cidx, new);
		emac->RX0CP = (unsigned int)(ion_mphys((void *)&dm644x->rx_desc[cidx]));

		npkt->framelen = pkt_len;
		buf = TAILQ_FIRST(&npkt->buffers);
		buf->net_iov->iov_len = npkt->framelen;

		npkt->next = NULL;
//dump_pkt(npkt);

		if (tx_Q_tail)
			tx_Q_tail->next = npkt;
		else
			tx_Q_head = npkt;
		tx_Q_tail = npkt;

		if (++pkt_cnt > 10) {
			ret = 0;
			break;
		}
	}

	if (tx_Q_head) {
		if (dm644x->cfg.flags & NIC_FLAG_PROMISCUOUS)
			tx_Q_head->flags |= _NPKT_PROMISC;
		
		dm644x->rx_traffic = 1;

		if (tx_Q_head = dm644x->ion->tx_up_start(dm644x->reg_hdl, tx_Q_head, 0, 0, dm644x->cell, dm644x->cfg.lan, 0, dm644x)) {
			while (npkt = tx_Q_head) {
				tx_Q_head = npkt->next;
				dm644x_receive_complete(npkt, dm644x, dm644x);
			}
		}
	}

	return (ret);
}

int dm644x_receive_complete(npkt_t * npkt, void *hdl, void *func_hdl) 
{
	dm644x_dev_t   *dm644x = (dm644x_dev_t *) hdl;
	
	if (dm644x->num_rx_free < 16) {
		net_buf_t	*buf = TAILQ_FIRST(&npkt->buffers);
		net_iov_t	*iov = buf->net_iov;

		npkt->ref_cnt = 1;
		npkt->req_complete = 0;
		npkt->flags = _NPKT_UP;
		npkt->tot_iov = 1;

//		iov->iov_base = (void *)(MAC_ALIGNED_BUF_ADDR(npkt->org_data));
//		iov->iov_phys = ion_mphys(iov->iov_base);
		iov->iov_len = MAX_BUF_SIZE;
		buf->niov = 1;

		pthread_mutex_lock(&dm644x->rx_freeq_mutex);
		npkt->next = dm644x->rx_free;
		dm644x->rx_free = npkt;
		dm644x->num_rx_free++;
		pthread_mutex_unlock(&dm644x->rx_freeq_mutex);

		return (0);
	}

	ion_free(npkt->org_data);
	ion_free(npkt);

	return (0);
}
