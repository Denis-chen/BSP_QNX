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







#include "dm644x_emac.h"

/*
 * forward decls 
 */
static uint16_t dm644x_mii_read(void *handle, uint8_t phy_id, uint8_t location); 
static void dm644x_mii_write(void *handle, uint8_t phy_id, uint8_t location, uint16_t value);
static void dm644x_mii_callback(void *handle, uchar_t phy, uchar_t newstate); 

static uint16_t dm644x_mii_read(void *handle, uint8_t phy_id, uint8_t location) 
{
	dm644x_dev_t	*dm644x = (dm644x_dev_t *)handle;
	mdio_regs		*mdio = dm644x->mdio;

	mdio->USERACCESS0 = MDIO_USERACCESS0_GO | MDIO_USERACCESS0_WRITE_READ |
								((location & 0x1F) << 21) | ((phy_id & 0x1F) << 16);

	/* Wait for command to complete */
	while ((mdio->USERACCESS0 & MDIO_USERACCESS0_GO) != 0)
		;

	return (mdio->USERACCESS0 & 0xFFFF);
}

void dm644x_mii_write(void *handle, uint8_t phy_id, uint8_t location, uint16_t value) 
{
	dm644x_dev_t	*dm644x = (dm644x_dev_t *)handle;
	mdio_regs		*mdio = dm644x->mdio;

	/* Wait for User access register to be ready */
	while ((mdio->USERACCESS0 & MDIO_USERACCESS0_GO) != 0)
		;

	mdio->USERACCESS0 = MDIO_USERACCESS0_GO | MDIO_USERACCESS0_WRITE_WRITE |
								((location & 0x1F) << 21) | ((phy_id & 0x1F) << 16) | (value & 0xFFFF);
}

static void dm644x_linkup(dm644x_dev_t *dm644x, uint32_t mode)
{
	char			*s;
	nic_config_t	*cfg = &dm644x->cfg;

	cfg->flags &= ~NIC_FLAG_LINK_DOWN;

	cfg->media_rate = -1;   /* Unknown */
	cfg->duplex = -1;       /* Unknown */

	switch (mode) {
		case MDI_10bT:
			s = "10BTHD";
			cfg->media_rate = 10*1000;
			cfg->duplex = 0;
			break;
		case MDI_10bTFD:
			s = "10BTFD";
			cfg->media_rate = 10*1000;
			cfg->duplex = 1;
			break;
		case MDI_100bT3: // Specific to this chip - 100MB half duplex
		case MDI_100bT:
			s = "100BTHD";
			cfg->media_rate = 100*1000;
			cfg->duplex = 0;
			break;
		case MDI_100bTFD:
			s = "100BTFD";
			cfg->media_rate = 100*1000;
			cfg->duplex = 1;
			break;
		case MDI_100bT4:
			s = "100BT4";
			cfg->media_rate = 100*1000;
			cfg->duplex = 0;
			break;
		default:
			s = "Unknown";
	}

	if (cfg->media_rate != -1) {
		if (cfg->verbose)
			nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "Link up (%s)", s);
	}
}

void dm644x_mii_callback(void *handle, uchar_t phy, uchar_t newstate) 
{
	dm644x_dev_t	*dm644x = (dm644x_dev_t *)handle;
	nic_config_t	*cfg = &dm644x->cfg;
	int				i, mode;

	switch (newstate) {
	case MDI_LINK_UP:
		if ((i = MDI_GetActiveMedia(dm644x->mdi, phy, &mode)) != MDI_LINK_UP) {
			if (cfg->verbose)
				nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: callback GetActiveMedia returned %x", i);
			mode = MDI_10bT;
		}

		if (mode != dm644x->phy_mode)
			dm644x->phy_mode = mode;

		dm644x_linkup(dm644x, mode);

		break;

	case MDI_LINK_DOWN:
		cfg->media_rate = cfg->duplex = -1;
		cfg->flags |= NIC_FLAG_LINK_DOWN;
		if (cfg->verbose)
			nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-dm644x: Link down %d", dm644x->cfg.lan);

		if (dm644x->negotiate)
			MDI_AutoNegotiate(dm644x->mdi, phy, NoWait);

		break;

	default:
		break;
	}
}

static void
dm644x_setup_phy(dm644x_dev_t *dm644x)
{
	int	an_capable, force_advertise = -1;
	nic_config_t	*cfg = &dm644x->cfg;

        if (cfg->duplex != -1) {
		/* User forced the duplex, in this case, we disable autoneg. */
		force_advertise = 0;
		if (cfg->media_rate == -1) /* Force speed to a default */
			cfg->media_rate = 100*1000;
	}
	else if (cfg->media_rate != -1) {
		if (cfg->media_rate == 100*1000)
			force_advertise = MDI_100bTFD | MDI_100bT;
		else
			force_advertise = MDI_10bTFD | MDI_10bT;
	}

	an_capable = dm644x_mii_read(dm644x, cfg->phy_addr, MDI_BMSR);

	if (force_advertise != -1 || !an_capable) {
		uint16_t reg = dm644x_mii_read(dm644x, cfg->phy_addr, MDI_BMCR);

		reg &= ~(BMCR_RESTART_AN | BMCR_SPEED_100 | BMCR_FULL_DUPLEX);

		if (an_capable && force_advertise != 0) {
			/*
			 * If we force the speed, but the link partner
			 * is autonegotiating, there is a greater chance
			 * that everything will work if we advertise with
			 * the speed that we are forcing to.
			 */
			MDI_SetAdvert(dm644x->mdi, cfg->phy_addr, force_advertise);

			reg |= BMCR_RESTART_AN | BMCR_AN_ENABLE;

			if (cfg->verbose)
				nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-dm644x: "
					"restricted autonegotiate (%dMbps only)",
					cfg->media_rate / 1000);
		}
		else {
			reg &= ~BMCR_AN_ENABLE;

			if (cfg->verbose)
				nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-dm644x: forcing the link");
		}

		if (cfg->duplex > 0)
			reg |= BMCR_FULL_DUPLEX;
		if (cfg->media_rate == 100*1000)
			reg |= BMCR_SPEED_100;

		dm644x_mii_write(dm644x, cfg->phy_addr, MDI_BMCR, reg);

		if (reg & BMCR_AN_ENABLE)
			MDI_EnableMonitor(dm644x->mdi, 1);
	}
	else {
		int status;

		/* Not forcing the link */
		dm644x->negotiate = 1;
		cfg->flags |= NIC_FLAG_LINK_DOWN;
		MDI_AutoNegotiate(dm644x->mdi, cfg->phy_addr, NoWait);
		status = MDI_EnableMonitor(dm644x->mdi, 1);

		if (status != MDI_SUCCESS)
			nic_slogf(_SLOGC_NETWORK, _SLOG_INFO,
				"devn-dm644x: MDI_EnableMonitor returned %x", status);
	}
}


int
dm644x_init_phy(dm644x_dev_t *dm644x)
{
	struct sigevent mdi_event;
	nic_config_t    *cfg = &dm644x->cfg;
	emac_regs		*emac = dm644x->emac;
	mdio_regs		*mdio = dm644x->mdio;
	uint32_t		clkdiv;


	/* Init MDIO & get link state */
	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;
	mdio->CONTROL = ((clkdiv & 0xFF) | MDIO_CONTROL_ENABLE | MDIO_CONTROL_FAULT);

	/* Enable MII interface and Full duplex mode */
	emac->MACCONTROL = (EMAC_MACCONTROL_MIIEN_ENABLE | EMAC_MACCONTROL_FULLDUPLEX_ENABLE | EMAC_MACCONTROL_RMIISPEED_100);
	
        mdi_event.sigev_coid = dm644x->coid;
	mdi_event.sigev_code = 0;
	MDI_Register_Extended(dm644x, dm644x_mii_write, dm644x_mii_read, 
		dm644x_mii_callback, (mdi_t **)&dm644x->mdi, &mdi_event,
		NIC_MDI_PRIORITY, 3); /* 3 second interval */

        if (dm644x->cfg.phy_addr == -1){
	    for (dm644x->cfg.phy_addr = 0; dm644x->cfg.phy_addr < 32; dm644x->cfg.phy_addr++) {
		    if (MDI_FindPhy(dm644x->mdi, dm644x->cfg.phy_addr) == MDI_SUCCESS &&
			    MDI_InitPhy(dm644x->mdi, dm644x->cfg.phy_addr) == MDI_SUCCESS) {
			    if (cfg->verbose) {
				    nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-dm644x: MII transceiver found at address %d.", dm644x->cfg.phy_addr);
				    nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-dm644x: Phy Type %s", dm644x->mdi->PhyData [dm644x->cfg.phy_addr]->Desc);
		  	    }

			    cfg->connector = NIC_CONNECTOR_MII;
			    dm644x_setup_phy(dm644x);

			    return 0; // success
	        }
            }
	}
        else {  // User has specified the phyaddr on command line 
	    if (MDI_FindPhy(dm644x->mdi, dm644x->cfg.phy_addr) == MDI_SUCCESS &&
			MDI_InitPhy(dm644x->mdi, dm644x->cfg.phy_addr) == MDI_SUCCESS) {
			if (cfg->verbose) {
				nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-dm644x: MII transceiver(user CLI) found at address %d.", dm644x->cfg.phy_addr);
				nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-dm644x: Phy Type %s", dm644x->mdi->PhyData [dm644x->cfg.phy_addr]->Desc);
			}

			cfg->connector = NIC_CONNECTOR_MII;
			dm644x_setup_phy(dm644x);

			return 0; // success
             }
             else {
                 nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: PHY not found at address %d ", dm644x->cfg.phy_addr);
                 return -1;  // failure
             }

        }

	if (cfg->verbose)
		nic_slogf (_SLOGC_NETWORK, _SLOG_ERROR, "devn-dm644x: Can not locate MII transceiver");

	cfg->phy_addr = -1;
	cfg->connector = NIC_CONNECTOR_UTP;
    return -1; // failure
}
