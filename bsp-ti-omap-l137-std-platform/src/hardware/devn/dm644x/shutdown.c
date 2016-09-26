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


/*==============================================================================
 *
 */
int dm644x_shutdown1(int reg_hdl, void *hdl) {
	dm644x_dev_t   *dm644x = (dm644x_dev_t *) hdl;

	dm644x_reset(dm644x);

	MDI_DeRegister(&dm644x->mdi);

	InterruptDetach(dm644x->iid);
	ConnectDetach(dm644x->coid);
	ChannelDestroy(dm644x->chid);

	return (0);
}

int dm644x_shutdown2(int reg_hdl, void *hdl) {
	int				cnt;
	npkt_t			*npkt;
	dm644x_dev_t	*dm644x = (dm644x_dev_t *) hdl;
	nic_config_t	*cfg = &dm644x->cfg;

	pthread_cancel(dm644x->tid);
	pthread_join(dm644x->tid, NULL);

	/* check for used rx buffers and free them */
	for (cnt = 0; cnt < MAX_RX_BUFFERS; cnt++) {
		if (npkt = dm644x->rx_pktq[cnt]) {
			ion_free(npkt->org_data);
			ion_free(npkt);
			dm644x->rx_pktq[cnt] = NULL;
		}
	}

	for (; npkt = dm644x->rx_free;) {
		dm644x->rx_free = npkt->next;
		ion_free(npkt->org_data);
		ion_free(npkt);
	}


	dm644x_flush(dm644x->reg_hdl, dm644x);

	pthread_mutex_destroy(&dm644x->mutex);
	pthread_mutex_destroy(&dm644x->rx_freeq_mutex);

	munmap_device_io(dm644x->emac_base, cfg->io_window_size[0]);

	free(dm644x->rx_pktq);
	free(dm644x->tx_pktq);
	free(dm644x);

	return (0);
}
