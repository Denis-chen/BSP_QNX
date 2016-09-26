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
static int      dm644x_init_ring(dm644x_dev_t * dm644x);
static int      dm644x_init_memory(dm644x_dev_t * dm644x);
static int      dm644x_config(dm644x_dev_t * dm644x);
static int      dm644x_process_interrupt(dm644x_dev_t * dm644x);
static int      dm644x_event_handler(dm644x_dev_t * dm644x);


#define NIC_INTERRUPT_EVENT		0
#define NIC_TIMER_EVENT			1
#define NIC_PRIORITY			21

io_net_dll_entry_t io_net_dll_entry = {
	2,
	dm644x_init,
	NULL
};

io_net_registrant_funcs_t dm644x_funcs = {
	8,
	NULL,
	dm644x_send_packets,
	dm644x_receive_complete,
	dm644x_shutdown1,
	dm644x_shutdown2,
	dm644x_advertise,
	(void *)dm644x_devctl,
	dm644x_flush,
	NULL
};

io_net_registrant_t dm644x_entry = {
	_REG_PRODUCER_UP | _REG_TRACK_MCAST,
	"devn-dm644x",
	"en",
	NULL,
	NULL,
	&dm644x_funcs,
	0
};

int dm644x_init(void *dll_hdl, dispatch_t * dpp, io_net_self_t * ion, char *options) 
{
	int             ret;

	dpp = dpp;
	setbuf(stdout, NULL);

	if (ret = dm644x_detect(dll_hdl, ion, options)) {
		errno = ret;
		return -1;
	}

	return 0;
}

void dm644x_set_multicast(dm644x_dev_t *dm644x, uint32_t crc, uint32_t flag) 
{
}

int dm644x_reset(dm644x_dev_t * dm644x) 
{
	emac_regs	*emac  = dm644x->emac;
	ewrap_regs	*ewrap = dm644x->ewrap;
	int			cnt, clkdiv;

	/* Reset EMAC module and disable interrupts in wrapper */
	emac->SOFTRESET = 1;

	while (emac->SOFTRESET != 0)
		;

	ewrap->EWCTL = 0;

	for (cnt = 0; cnt < 5; cnt++)
		clkdiv = ewrap->EWCTL;

	emac->TXCONTROL = 0x1;
	emac->RXCONTROL = 0x1;

	return (0);
}

static int dm644x_init_ring(dm644x_dev_t *dm644x)
{
	emac_regs	*emac  = dm644x->emac;
	npkt_t		*npkt;
	emac_desc	*desc;
	uint32_t	*addr;
	uint32_t	cnt;

	/* Set TX/RX DMA Head pointers to 0 */
	addr = (uint32_t *)&emac->TX0HDP;
	for (cnt = 0; cnt < 16; cnt++)
		*addr++ = 0;
	addr = (uint32_t *)&emac->RX0HDP;
	for (cnt = 0; cnt < 16; cnt++)
		*addr++ = 0;

	/* Clear Statistics (do this before setting MacControl register) */
	addr = (uint32_t *)&emac->RXGOODFRAMES;
	for (cnt = 0; cnt < EMAC_NUM_STATS; cnt++)
		*addr++ = 0;

	/* No multicast addressing */
	emac->MACHASH1 = 0 ;
	emac->MACHASH2 = 0 ;

	dm644x->rx_desc = (emac_desc *)(dm644x->emac_base + EMAC_WRAPPER_RAM_ADDR - EMAC_BASE_ADDR);
	dm644x->tx_desc = (emac_desc *)(dm644x->emac_base + EMAC_WRAPPER_RAM_ADDR - EMAC_BASE_ADDR + 0x1000);

	desc = dm644x->rx_desc;
	for (cnt = 0; cnt < MAX_RX_BUFFERS; cnt++) {
		dm644x->rx_pktq[cnt] = npkt = dm644x_alloc_npkt(dm644x, MAX_BUF_SIZE);
		dm644x_add_rbuff(dm644x, cnt, npkt);

		desc->next = (unsigned int)(ion_mphys((void *)(desc + 1)));
		++desc;
	}
	--desc;

	/*
	 * Docs says the the chain list must be NULL terminated
	 * but we prefer ring, could cause receive stops when there
	 * is receive overrun, but who knows.
	 */
#if 0
	desc->next = NULL;			/* Must be NULL */
#else
	desc->next = (unsigned int)(ion_mphys((void *)(dm644x->rx_desc)));
#endif

	dm644x->tx_pidx = -1;

	return (0);
}

static int dm644x_init_memory(dm644x_dev_t * dm644x) 
{
	nic_config_t	*cfg = &dm644x->cfg;

	dm644x->emac_base = mmap_device_io(cfg->io_window_size[0], cfg->io_window_base[0]);
	dm644x->emac      = (emac_regs *)dm644x->emac_base;
	dm644x->ewrap     = (ewrap_regs *)(dm644x->emac_base + EMAC_WRAPPER_BASE_ADDR - EMAC_BASE_ADDR);
	dm644x->mdio      = (mdio_regs *)(dm644x->emac_base + EMAC_MDIO_BASE_ADDR - EMAC_BASE_ADDR);

	/* allocate rx pkt array */
	if ((dm644x->rx_pktq = calloc(MAX_RX_BUFFERS, sizeof(dm644x->rx_pktq))) == NULL)
		return (1);

	if ((dm644x->tx_pktq = calloc(MAX_TX_BUFFERS, sizeof(dm644x->rx_pktq))) == NULL)
		return (1);

	return (0);
}

static int dm644x_config(dm644x_dev_t * dm644x) 
{
	nic_config_t	*cfg = &dm644x->cfg;
	emac_regs		*emac;
	uint8_t			*ptr_mac;
	const char		zaddr[6] = {0,};

	strcpy((char *)cfg->device_description, "TI DM644x EMAC");
	cfg->mac_length = ETH_MAC_LEN;
	dm644x->num_rx_free = 0;
	dm644x->hashhi = 0;
	dm644x->hashlo = 0;

	if (dm644x_init_memory(dm644x))
		goto fail;

	emac = dm644x->emac;

	/* Override with MAC address from syspage, if available */
	nic_get_syspage_mac((char *)cfg->permanent_address);

	/* Set MAC Addresses & Init multicast Hash to 0 (disable any multicast receive) */
	/* Using channel 0 only - other channels are disabled */
	emac->MACINDEX = 0;

	if (memcmp(cfg->permanent_address, zaddr, 6) == 0) {
		/* No permanent address in syspage, try MAC address register */
		ptr_mac = (uint8_t *)cfg->permanent_address;
#if 0
		ptr_mac[0] = emac->MACADDRHI;
		ptr_mac[1] = emac->MACADDRHI >> 8;
		ptr_mac[2] = emac->MACADDRHI >> 16;
		ptr_mac[3] = emac->MACADDRHI >> 24;
		ptr_mac[4] = emac->MACADDRLO >> 8;
		ptr_mac[5] = emac->MACADDRLO;
#else
		*((uint32_t *)&ptr_mac[0]) = emac->MACADDRHI;
		*((uint16_t *)&ptr_mac[4]) = emac->MACADDRLO;
#endif
	}

	/* check for command line override */
	if (memcmp(cfg->current_address, zaddr, 6) == 0) {
		/*
		 * Don't start the driver if there is no valid MAC address available
		 */
		if (memcmp(cfg->permanent_address, zaddr, 6) == 0) {
			nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: MAC address not specified on cmdline.");
			goto fail;
		}
		memcpy(cfg->current_address, cfg->permanent_address, ETH_MAC_LEN);
	}

	/* Reset MAC */
	dm644x_reset(dm644x);

	/* Set MAC address, clear hash register */
	ptr_mac = (uint8_t *)cfg->current_address;
	emac->MACADDRHI = *((uint32_t *)ptr_mac);
	emac->MACADDRLO = *((uint16_t *)(ptr_mac + 4));

	/* Set source MAC address - REQUIRED */
	emac->MACSRCADDRHI = emac->MACADDRHI;
	emac->MACSRCADDRLO = emac->MACADDRLO;

	/* Clear HASH */
	emac->MACHASH1 = 0;
	emac->MACHASH2 = 0;

	dm644x_set_multicast(dm644x, 0, 0);

	/* Enable TX/RX */
	emac->RXMAXLEN = EMAC_MAX_ETHERNET_PKT_SIZE;
	emac->RXBUFFEROFFSET = 0;

	/* No fancy configs - Use this for promiscous for debug - EMAC_RXMBPENABLE_RXCAFEN_ENABLE */
	emac->RXMBPENABLE = EMAC_RXMBPENABLE_RXBROADEN;

	/* Enable ch 0 only */
	emac->RXUNICASTSET = 0x1;

	dm644x_init_phy(dm644x);

	if (dm644x_init_ring(dm644x))
		goto fail;

	if (cfg->verbose)
        {
                printf(" verbose enabled : \n");
		nic_dump_config(cfg);
        }

	return 0;

fail:
	munmap_device_io(dm644x->emac_base, cfg->io_window_size[0]);

	return 1;
}

int dm644x_register_device(dm644x_dev_t * dm644x, io_net_self_t * ion, void *dll_hdl) 
{
	nic_config_t	*cfg = &dm644x->cfg;
	pthread_attr_t  pattr;
	struct sched_param param;
	struct sigevent event;
	uint16_t        lan;

	dm644x->ion = ion;
	dm644x->dll_hdl = dll_hdl;

	if ((dm644x->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1)
		return (1);

	if ((dm644x->coid = ConnectAttach(0, 0, dm644x->chid, _NTO_SIDE_CHANNEL, 0)) == -1)
		goto fail0;

	if (pthread_mutex_init(&dm644x->mutex, NULL) == -1)
		goto fail1;

	if (pthread_mutex_init(&dm644x->rx_freeq_mutex, NULL) == -1)
		goto fail2;

	dm644x->cachectl.fd = NOFD;

	if (cache_init(0, &dm644x->cachectl, NULL) == -1)
		goto fail3;

	pthread_attr_init(&pattr);
	pthread_attr_setschedpolicy(&pattr, SCHED_RR);
	param.sched_priority = cfg->priority;
	pthread_attr_setschedparam(&pattr, &param);
	pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);

	if (dm644x_config(dm644x))
		goto fail3;

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = dm644x->coid;
	event.sigev_code = NIC_INTERRUPT_EVENT;
	event.sigev_priority = cfg->priority;
	if (dm644x->noirq) {
		int                 timerid;
		struct itimerspec   value;

		if (timer_create(CLOCK_REALTIME, &event, &timerid) == -1)
			goto fail4;

		value.it_value.tv_sec = value.it_interval.tv_sec = 0;
		value.it_value.tv_nsec = value.it_interval.tv_nsec = 1000000UL;

		if (timer_settime(timerid, 0, &value, NULL) == -1)
			goto fail4;
	}
	else if ((dm644x->iid = InterruptAttachEvent(cfg->irq[0], &event, _NTO_INTR_FLAGS_TRK_MSK)) == -1) {
		nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: InterruptAttachEvent failed");
		goto fail4;
	}

	/*
	 * create the interface thread 
	 */
	if (pthread_create(&dm644x->tid, &pattr, (void *)dm644x_event_handler, dm644x)) {
		nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x:  Unable to create driver thread");
		goto fail5;
	}

	dm644x_entry.func_hdl = (void *)dm644x;
	dm644x_entry.top_type = (char *)cfg->uptype;

	if (cfg->lan != -1) {
		dm644x_entry.flags |= _REG_ENDPOINT_ARG;
		lan = cfg->lan;
	}

	if (ion_register(dll_hdl, &dm644x_entry, &dm644x->reg_hdl, &dm644x->cell, &lan) < 0)
		goto fail6;

	cfg->lan = lan;

	if (!dm644x->max_pkts)
		dm644x->max_pkts = 100;
	dm644x->ion->devctl(dm644x->reg_hdl, DCMD_IO_NET_MAX_QUEUE, &dm644x->max_pkts, sizeof(dm644x->max_pkts), NULL);

	/* Start receive process */
	dm644x->emac->RX0HDP = (unsigned int)(ion_mphys((void *)dm644x->rx_desc));
	dm644x->ewrap->EWCTL = 1;
	dm644x->ewrap->EWINTTCNT = 1500;
	dm644x->emac->RXINTMASKCLEAR = 0xFF;
	if (!dm644x->noirq) {
		dm644x->emac->RXINTMASKSET = 0x01;
		dm644x->emac->TXINTMASKSET = 0x01;
	}

	return (0);

fail6:
	pthread_cancel(dm644x->tid);
	pthread_join(dm644x->tid, NULL);
fail5:
	InterruptDetach(dm644x->iid);
fail4:
	dm644x_reset(dm644x);
fail3:
	pthread_mutex_destroy(&dm644x->rx_freeq_mutex);
fail2:
	pthread_mutex_destroy(&dm644x->mutex);
fail1:
	ConnectDetach(dm644x->coid);
fail0:
	ChannelDestroy(dm644x->chid);

	return (1);
}

static int dm644x_process_interrupt(dm644x_dev_t * dm644x) 
{
	emac_regs	*emac = dm644x->emac;

	pthread_mutex_lock(&dm644x->mutex);
	dm644x->ewrap->EWCTL = 0;
	if (emac->RXINTSTATRAW)
		dm644x_receive(dm644x);
	if (emac->TXINTSTATRAW)
		dm644x_transmit(dm644x);
	dm644x->ewrap->EWCTL = 1;
	pthread_mutex_unlock(&dm644x->mutex);

	if (!dm644x->noirq)
		InterruptUnmask(dm644x->cfg.irq[0], dm644x->iid);

	return (0);
}

static int dm644x_event_handler(dm644x_dev_t * dm644x) 
{
	struct _pulse   pulse;
	iov_t           iov;
	int             rcvid;

	SETIOV(&iov, &pulse, sizeof(pulse));

	while (1) {
		if ((rcvid = MsgReceivev(dm644x->chid, &iov, 1, NULL)) == -1) {
			if (errno == ESRCH)
				pthread_exit(NULL);
			continue;
		}

		switch (pulse.code) {
		case NIC_TIMER_EVENT:
		case NIC_INTERRUPT_EVENT:
			dm644x_process_interrupt(dm644x);
			break;

		case MDI_TIMER:
			if (!dm644x->rx_traffic ||
				(dm644x->cfg.flags & NIC_FLAG_LINK_DOWN)) {
				/*
				 * Only monitor the link if the
				 * link state is unknown or there's
				 * no traffic.
				 */
				MDI_MonitorPhy(dm644x->mdi);
			}
			dm644x->rx_traffic = 0;
			break;

		default:
			if (rcvid)
				MsgReplyv(rcvid, ENOTSUP, &iov, 1);
			break;
		}
	}

	return (0);
}

int dm644x_advertise(int reg_hdl, void *func_hdl) 
{
	npkt_t			*npkt;
	net_buf_t		*nb;
	net_iov_t		*iov;
	io_net_msg_dl_advert_t *ap;
	dm644x_dev_t	*dm644x = (dm644x_dev_t *) func_hdl;
	nic_config_t	*cfg = &dm644x->cfg;

	dm644x = (dm644x_dev_t *) func_hdl;

	if ((npkt = dm644x_alloc_npkt(dm644x, MAX_BUF_SIZE)) == NULL)
		return 0;

	nb = npkt->buffers.tqh_first;
	iov = nb->net_iov;

	ap = iov->iov_base;
	iov->iov_len = sizeof *ap;

	memset(ap, 0x00, sizeof *ap);
	ap->type = _IO_NET_MSG_DL_ADVERT;

	ap->iflags = (IFF_SIMPLEX | IFF_BROADCAST | IFF_RUNNING);

	if (cfg->flags & NIC_FLAG_MULTICAST)
		ap->iflags |= IFF_MULTICAST;

	ap->mtu_min = 0;
	ap->mtu_max = cfg->mtu;
	ap->mtu_preferred = cfg->mtu;
	strcpy(ap->up_type, (char *)cfg->uptype);
	itoa(cfg->lan, ap->up_type + 2, 10);

	strcpy(ap->dl.sdl_data, ap->up_type);

	ap->dl.sdl_len = sizeof(struct sockaddr_dl);
	ap->dl.sdl_family = AF_LINK;
	ap->dl.sdl_index = cfg->lan;
	ap->dl.sdl_type = IFT_ETHER;
	ap->dl.sdl_nlen = strlen(ap->dl.sdl_data);	/* not null terminated */
	ap->dl.sdl_alen = 6;
	memcpy(ap->dl.sdl_data + ap->dl.sdl_nlen, cfg->current_address, 6);

	npkt->flags |= _NPKT_MSG;
	npkt->iface = 0;
	npkt->framelen = sizeof *ap;

	if (ion_add_done(dm644x->reg_hdl, npkt, dm644x) == -1) {
		ion_free(npkt->org_data);
		ion_free(npkt);
		return (0);
	}

	if (ion_rx_packets(dm644x->reg_hdl, npkt, 0, 0, dm644x->cell, cfg->lan, 0) == 0)
		ion_tx_complete(dm644x->reg_hdl, npkt);

	return (0);
}

int dm644x_flush(int reg_hdl, void *hdl) 
{
	dm644x_dev_t   *dm644x = (dm644x_dev_t *) hdl;
	npkt_t         *npkt;
	npkt_t         *tmp;

	pthread_mutex_lock(&dm644x->mutex);

	npkt = dm644x->nhead;
	dm644x->nhead = dm644x->ntail = NULL;

	pthread_mutex_unlock(&dm644x->mutex);

	for (; npkt; npkt = tmp) {
		tmp = npkt->next;
		ion_tx_complete(dm644x->reg_hdl, npkt);
	}

	return (0);
}


npkt_t *dm644x_alloc_npkt(dm644x_dev_t * dm644x, size_t size) 
{
	npkt_t         *npkt;
	net_buf_t      *nb;
	net_iov_t      *iov;
	uint32_t		offset, paddr, vaddr;

	if ((npkt = ion_alloc_npkt(sizeof(net_buf_t) + sizeof(net_iov_t), (void **)&nb)) == NULL)
		return (NULL);

	if ((vaddr = (uint32_t)ion_alloc(size + MAC_BUFFER_ALIGN, 0)) == NULL) {
		ion_free(npkt);
		return (NULL);
	}

	TAILQ_INSERT_HEAD(&npkt->buffers, nb, ptrs);

	iov = (net_iov_t *) (nb + 1);

	nb->niov = 1;
	nb->net_iov = iov;

	paddr = (paddr_t)(ion_mphys((void *)vaddr));
	offset = MAC_ALIGNED_BUF_ADDR(paddr) - paddr;

	iov->iov_base = (void *)(vaddr + offset);
	iov->iov_len = size;
	iov->iov_phys = paddr + offset;

	npkt->org_data = (void *)vaddr;
	npkt->next = NULL;
	npkt->tot_iov = 1;
	npkt->flags = _NPKT_UP;
	npkt->ref_cnt = 1;
	npkt->req_complete = 0;

	return (npkt);
}
