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
extern void     dump_packet(npkt_t * npkt);
static npkt_t  *dm644x_defrag(dm644x_dev_t * dm644x, npkt_t * npkt);
static int      dm644x_send(dm644x_dev_t * dm644x);


/*
 * Defragment routine
 */
static npkt_t  *dm644x_defrag(dm644x_dev_t * dm644x, npkt_t * npkt) {
	npkt_t         *dpkt = NULL;
	char           *dst;
	net_iov_t      *iov;
	net_buf_t      *dov, *buf;
	int				i;

	/*
	 * Defrag all the packets - too BAD!!!
	 */
	if (npkt->framelen <= 1514) {
		/* Grab a free npkt packet */
		dpkt = dm644x_alloc_npkt(dm644x, MAX_BUF_SIZE);

		dov = TAILQ_FIRST(&dpkt->buffers);

		/* copy each small data fragment into the big buffer */
		dst = dov->net_iov->iov_base;
		for (buf = TAILQ_FIRST(&npkt->buffers); buf != NULL; buf = TAILQ_NEXT(buf, ptrs)) {
			for (i = 0, iov = buf->net_iov; i < buf->niov; i++, iov++) {
				memcpy(dst, iov->iov_base, iov->iov_len);
				dst += iov->iov_len;
			}
		}
		dpkt->framelen = npkt->framelen;
	}

	pthread_mutex_unlock(&dm644x->mutex);
	ion_tx_complete(dm644x->reg_hdl, npkt);	/* tell io-net we are done */
											/* with those */
											/* small packet buffers */
	pthread_mutex_lock(&dm644x->mutex);

	return (dpkt);
}



static int dm644x_send(dm644x_dev_t * dm644x) {
	int             pidx = 0;
	int             framelen;
	net_buf_t      *buf;
	net_iov_t      *iov;
	npkt_t         *npkt;
	emac_desc		*desc;
	emac_regs		*emac  = dm644x->emac;

	if (dm644x->tx_pidx != -1)
		return 0;
	if (dm644x->nhead == NULL)
		return 0;

	dm644x->tx_cidx = 0;

	emac->TX0HDP = 0;
	desc = dm644x->tx_desc;

	while (npkt = dm644x->nhead) {
		dm644x->nhead = npkt->next;
		npkt->next = NULL;
		if (npkt == dm644x->ntail)
			dm644x->ntail = NULL;

		if ((npkt = dm644x_defrag(dm644x, npkt)) == NULL)
			continue;

		dm644x->tx_pktq[pidx++] = npkt;
		buf = TAILQ_FIRST(&npkt->buffers);
		iov = buf->net_iov;

		if ((framelen = npkt->framelen) < EMAC_MIN_ETHERNET_PKT_SIZE)
			framelen = EMAC_MIN_ETHERNET_PKT_SIZE;

		CACHE_FLUSH(&dm644x->cachectl, iov->iov_base, iov->iov_phys, iov->iov_len);

		desc->next         = (unsigned int)(ion_mphys((void *)(desc + 1)));
		desc->buffer       = (unsigned char *)iov->iov_phys;
		desc->buff_off_len = (framelen & 0xFFFF);
		desc->pkt_flag_len = ((framelen & 0xFFFF) | EMAC_CPPI_SOP_BIT | EMAC_CPPI_OWNERSHIP_BIT | EMAC_CPPI_EOP_BIT);

		desc++;
		if (pidx >= MAX_TX_BUFFERS)
			break;
	}

	--desc;
	desc->next = 0;
	dm644x->tx_pidx = pidx - 1;

	/* Send the packet */
	emac->TX0HDP = (unsigned int)(ion_mphys((void *)(dm644x->tx_desc)));

	return (0);
}


/*
 * entry from redirector 
 */
int dm644x_send_packets(npkt_t * npkt, void *hdl) {
	dm644x_dev_t   *dm644x = (dm644x_dev_t *) hdl;
	net_buf_t      *buf;
	io_net_msg_join_mcast_t *msg;

	int retVal = TX_DOWN_OK;

	if (npkt->flags & _NPKT_MSG) {
		buf = TAILQ_FIRST(&npkt->buffers);
		if (buf != NULL) {
			/*
			 * If selective mutlicasting worked on this card this is where
			 * it would happen 
			 */
			msg = (io_net_msg_join_mcast_t *) buf->net_iov->iov_base;
			if (msg) {
				uint32_t        crc = nic_calc_crc_be((uint8_t *)(LLADDR(&msg->mc_min.addr_dl)), 6);

				pthread_mutex_lock(&dm644x->mutex);
				switch (msg->type) {
				case _IO_NET_JOIN_MCAST:
				case _IO_NET_REMOVE_MCAST:
					dm644x_set_multicast(dm644x, crc >> 26, msg->type);
					break;

				default:
					if (dm644x->cfg.verbose)
						nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: Message type not supported.");
					errno = ENOTSUP;
					retVal = TX_DOWN_FAILED;
					break;
				}
				pthread_mutex_unlock(&dm644x->mutex);
			}
		}

		ion_tx_complete(dm644x->reg_hdl, npkt);
	}
	else {
		pthread_mutex_lock(&dm644x->mutex);

		if (dm644x->ntail)
			dm644x->ntail->next = npkt;
		else 
			dm644x->nhead = npkt;

		while (npkt->next)
			npkt = npkt->next;

		dm644x->ntail = npkt;

		dm644x_send(dm644x);

		pthread_mutex_unlock(&dm644x->mutex);
	}

	return (retVal);
}

int dm644x_transmit(dm644x_dev_t * dm644x) {
	int             cidx;
	npkt_t         *npkt;

	if (dm644x->tx_pidx == -1)
		return 0;

	while (1) {
		cidx = dm644x->tx_cidx;

		if (dm644x->tx_desc[cidx].pkt_flag_len & EMAC_CPPI_OWNERSHIP_BIT)
			break;

		dm644x->emac->TX0CP = (unsigned int)(ion_mphys((void *)&dm644x->tx_desc[cidx]));

		npkt = dm644x->tx_pktq[cidx];
		dm644x->tx_pktq[cidx] = NULL;

		if (!npkt)
			break;

		ion_free(npkt->org_data);
		ion_free(npkt);

		if (dm644x->tx_desc[cidx].pkt_flag_len & EMAC_CPPI_EOQ_BIT) {		/* End of queue */
			dm644x->tx_pidx = -1;
			break;
		}
		dm644x->tx_cidx = cidx + 1;
	}

//	if (dm644x->tx_pidx == -1)
		dm644x_send(dm644x);

	return (0);
}
