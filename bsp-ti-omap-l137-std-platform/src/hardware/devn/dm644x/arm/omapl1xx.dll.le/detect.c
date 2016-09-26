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

#include <drvr/hwinfo.h>
#include <hw/hwinfo_omapl1xx.h>

int dm644x_detect(void *dll_hdl, io_net_self_t * ion, char *optstring) {
	char           *value, *freeptr, *options;
	int             opt;
    unsigned        cnt = 0;
	nic_config_t   *cfg;
	dm644x_dev_t   *dm644x;
	static char    *dm644x_opts[] = {
		"multicast",
		"noirq",
		NULL
	};

	if ((dm644x = calloc(sizeof(*dm644x), 1)) == NULL)
		return -1;

	cfg = &dm644x->cfg;

	/*
	 * set some defaults for the command line options 
	 * prime with -1 so we know if these options are 
	 * given on the command line 
	 */
	dm644x->cfg.mtu = -1;
	dm644x->cfg.mru = -1;
	cfg->flags = NIC_FLAG_MULTICAST;
	cfg->priority = NIC_PRIORITY;
	cfg->iftype = IFT_ETHER;
	cfg->revision = NIC_CONFIG_REVISION;
	dm644x->cfg.revision = NIC_STATS_REVISION;
	cfg->media_rate = -1;
	cfg->lan = -1;
	cfg->media = -1;
	cfg->duplex = -1;
        cfg->phy_addr = -1;  // Expected to get this from user as command line input, else do a search
	strcpy((char *)cfg->uptype, "en");

        /*
	 * getsubopt() is destructive
	 */
	if (optstring && *optstring != '\0')
		freeptr = strdup(optstring);
	else
		freeptr = NULL;

	options = freeptr;

	while (options && *options != '\0') {
		if ((opt = getsubopt(&options, dm644x_opts, &value)) == -1) {
			if (nic_parse_options(cfg, value) != EOK)
				continue;
		}

		switch (opt) {
		case 0:
			dm644x->multicast_all = 1;
			break;
		case 1:
			dm644x->noirq = 1;
			break;
                }
	}

	if (freeptr)
		free(freeptr);

	/* If media not spcified on cmdline, set to someting reasonable */
	dm644x->cfg.media = NIC_MEDIA_802_3;

	/*
	 *  See if the mtu or mru option were specified
	 */
	if (cfg->mtu == -1 || cfg->mtu > ETH_MAX_PKT_LEN)
		/* TODO Add oversized packet support */
		cfg->mtu = ETH_MAX_PKT_LEN;
	else if ( cfg->mtu < ETH_MIN_PKT_LEN)
		cfg->mtu = ETH_MIN_PKT_LEN;

	if (cfg->mru == -1 || cfg->mru > ETH_MAX_PKT_LEN)
		/* TODO Add oversized packet support */
		cfg->mru = ETH_MAX_PKT_LEN;
	else if ( cfg->mru < ETH_MIN_PKT_LEN)
		cfg->mru = ETH_MIN_PKT_LEN;

      /* figure out the number of IRQ tags */
      while(hwitag_find_ivec(hwi_find_device(OMAPL1xx_HWI_ENET, 0), &cnt) != HWI_ILLEGAL_VECTOR);
      cfg->num_irqs = cnt;

      if (cfg->num_irqs == 0) {
          /* Unable to find the irq info from the syspage, so hardcode it */
          cfg->num_irqs = 2;
          cfg->irq[0] = 34; // OMAPL1XX - RX int
          cfg->irq[1] = 35; // OMAPL1XX - TX int
          if (cfg->verbose){
              nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: Hardcode num_irqs : %d",cfg->num_irqs);
              nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: RX irq : %d ; TX irq : %d",cfg->irq[0],cfg->irq[1]);
          }
      }
      else {
          if (cfg->verbose)
              nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: num_irqs : %d",cfg->num_irqs);
          cnt = 0;
          while(cnt < cfg->num_irqs){
              cfg->irq[cnt] = hwitag_find_ivec(hwi_find_device(OMAPL1xx_HWI_ENET, 0), &cnt);
              if (cfg->verbose)
                  nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: Got %s irq : %d @ index : %d",(cnt==1)?"RX":"TX",cfg->irq[cnt-1],(cnt-1));
          }
      }

	if (cfg->num_io_windows == 0) {
		cfg->num_io_windows = 1;
		cfg->io_window_base[0] = EMAC_BASE_ADDR;
	}
	if (cfg->io_window_size[0] == 0)
		cfg->io_window_size[0] = 0x5000;

	if (dm644x_register_device(dm644x, ion, dll_hdl) == -1) {
		free(dm644x);
		return (ENODEV);
	}
	else {
            dm644x_advertise(dm644x->reg_hdl, dm644x);
        }

	return (EOK);
}
