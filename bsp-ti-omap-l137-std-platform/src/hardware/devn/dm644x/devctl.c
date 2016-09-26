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

static int
dm644x_get_stats(dm644x_dev_t *dm644x, void *data)
{
	emac_regs				*emac  = dm644x->emac;
	nic_stats_t				*stats = data;
	nic_ethernet_stats_t	*estats = &stats->un.estats;

	stats->revision = NIC_STATS_REVISION;

	stats->media = dm644x->cfg.media;

	stats->txed_ok        = emac->TXGOODFRAMES;
	stats->rxed_ok        = emac->RXGOODFRAMES;
	stats->octets_txed_ok = emac->TXOCTETS;
	stats->octets_rxed_ok = emac->RXOCTETS;

	estats->valid_stats =
	    NIC_ETHER_STAT_NO_CARRIER |
	    NIC_ETHER_STAT_TX_DEFERRED |
	    NIC_ETHER_STAT_XCOLL_ABORTED |
	    NIC_ETHER_STAT_LATE_COLLISIONS |
	    NIC_ETHER_STAT_SINGLE_COLLISIONS |
	    NIC_ETHER_STAT_MULTI_COLLISIONS |
	    NIC_ETHER_STAT_TOTAL_COLLISION_FRAMES |
	    NIC_ETHER_STAT_ALIGN_ERRORS |
	    NIC_ETHER_STAT_FCS_ERRORS |
		NIC_ETHER_STAT_JABBER_DETECTED |
		NIC_ETHER_STAT_OVERSIZED_PACKETS |
	    NIC_ETHER_STAT_SHORT_PACKETS;

	estats->no_carrier             = emac->TXCARRIERSENSE;
	estats->tx_deferred            = emac->TXDEFERRED;
	estats->xcoll_aborted          = emac->TXEXCESSIVECOLL;
	estats->late_collisions        = emac->TXLATECOLL;
	estats->single_collisions      = emac->TXSINGLECOLL;
	estats->multi_collisions       = emac->TXMULTICOLL;
	estats->total_collision_frames = emac->TXCOLLISION;

	estats->align_errors           = emac->RXALIGNCODEERRORS;
	estats->fcs_errors             = emac->RXCRCERRORS;
	estats->short_packets          = emac->RXUNDERSIZED;
	estats->oversized_packets      = emac->RXOVERSIZED;
	estats->jabber_detected        = emac->RXJABBER;

	stats->valid_stats =
	    NIC_STAT_TXED_MULTICAST | NIC_STAT_RXED_MULTICAST |
	    NIC_STAT_TXED_BROADCAST | NIC_STAT_RXED_BROADCAST;

	stats->txed_multicast = emac->TXMCASTFRAMES;
	stats->rxed_multicast = emac->RXMCASTFRAMES;
	stats->txed_broadcast = emac->TXBCASTFRAMES;
	stats->rxed_broadcast = emac->RXBCASTFRAMES;

	return EOK;
}


/*==============================================================================
 *
 */
int dm644x_devctl(void *hdl, int dcmd, void *data, size_t size, union _io_net_dcmd_ret_cred *ret) {
	dm644x_dev_t	*dm644x = (dm644x_dev_t *) hdl;
	nic_config_t	*cfg = &dm644x->cfg;
	int				status;
	int				prom;

	status = EOK;

	switch (dcmd) {
	case DCMD_IO_NET_GET_STATS:
		dm644x_get_stats(dm644x, data);
		break;
	case DCMD_IO_NET_GET_CONFIG:
		memcpy(data, cfg, sizeof(*cfg));
		status = EOK;
		break;

	case DCMD_IO_NET_PROMISCUOUS:
		pthread_mutex_lock(&dm644x->mutex);
		/* Get the desired setting */
		prom = *(int *)data;
		if (prom)
			cfg->flags |= NIC_FLAG_PROMISCUOUS;
		else
			cfg->flags &= ~NIC_FLAG_PROMISCUOUS;

		dm644x_set_multicast(dm644x, 0, 0);
		pthread_mutex_unlock(&dm644x->mutex);
		break;

	default:
		status = ENOTSUP;
		break;
	}

	return (status);
}
