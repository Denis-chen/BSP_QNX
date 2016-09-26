/*
 * Copyright (C) 2005 Texas Instruments
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. See the  
 * License for the specific language governing permissions and limitations  
 * under the License. 
 */

/* 
 * Modifications:
 * ver. 1.0: Sep 2005, TI PSP Team - Created EMAC version for uBoot.
 *
 */
 
#ifndef _DM644X_EMAC_H_
#define _DM644X_EMAC_H_

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <atomic.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/siginfo.h>
#include <sys/syspage.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <sys/io-net.h>
#include <sys/dcmd_io-net.h>
#include <sys/slogcodes.h>
#include <sys/cache.h>

#include <drvr/mdi.h>
#include <drvr/eth.h>

#include <drvr/nicsupport.h>



#define NIC_PRIORITY        21
#define NIC_MDI_PRIORITY    10

/***********************************************
 ********** Configurable items *****************
 ***********************************************/

// OMAP L1XX values
#define EMAC_BASE_ADDR             (0x01e20000)
#define EMAC_WRAPPER_BASE_ADDR     (0x01e22000)
#define EMAC_WRAPPER_RAM_ADDR      (0x01e20000)
#define EMAC_MDIO_BASE_ADDR        (0x01e24000)

#define EMAC_CNTRL_REGS_BASE       (0x01e23000)





/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ          75000000    /* PLL/4 - 75 MHz for OMAPL137 */
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ        2000000     /* 2.0 MHz */

/* PHY mask - set only those phy number bits where phy is/can be connected */
#define EMAC_MDIO_PHY_MASK          0xFFFFFFFF


// PHY specific mode - 100MBit half duplex
#define MDI_100bT3                  (1<<7)

/* Ethernet Min/Max packet size */
#define EMAC_MIN_ETHERNET_PKT_SIZE  60
#define EMAC_MAX_ETHERNET_PKT_SIZE  1518
#define EMAC_PKT_ALIGN              18  /* 1518 + 18 = 1536 (packet aligned on 32 byte boundry) */

/* Number of RX packet buffers
 * NOTE: Only 1 buffer supported as of now 
 */
//#define EMAC_MAX_RX_BUFFERS         10

/***********************************************
 ******** Internally used macros ***************
 ***********************************************/

#define EMAC_CH_TX                  1 
#define EMAC_CH_RX                  0

/* Each descriptor occupies 4, lets start RX desc's at 0 and 
 * reserve space for 64 descriptors max
 */
#define EMAC_RX_DESC_BASE           0x0
#define EMAC_TX_DESC_BASE           0x1000

/* EMAC Teardown value */
#define EMAC_TEARDOWN_VALUE         0xFFFFFFFC

/* MII Status Register */
#define MII_STATUS_REG              1

/* Intel LXT971 Digtal Config Register */
#define MII_DIGITAL_CONFIG_REG      26

/* Number of statistics registers */
#define EMAC_NUM_STATS              36

/* EMAC Descriptor */
typedef volatile struct _emac_desc 
{
  unsigned int      next;           /* Pointer to next descriptor in chain */
  unsigned char     *buffer;        /* Pointer to data buffer              */
  unsigned int      buff_off_len;   /* Buffer Offset(MSW) and Length(LSW)  */
  unsigned int      pkt_flag_len;   /* Packet Flags(MSW) and Length(LSW)   */
} emac_desc;

/* CPPI bit positions */
#define EMAC_CPPI_SOP_BIT               (0x80000000)  /*(1 << 31)*/
#define EMAC_CPPI_EOP_BIT               (0x40000000)  /*(1 << 30*/
#define EMAC_CPPI_OWNERSHIP_BIT         (0x20000000)  /*(1 << 29)*/
#define EMAC_CPPI_EOQ_BIT               (0x10000000)  /*(1 << 28)*/
#define EMAC_CPPI_TEARDOWN_COMPLETE_BIT (0x08000000)  /*(1 << 27)*/
#define EMAC_CPPI_PASS_CRC_BIT          (0x04000000)  /*(1 << 26)*/

#define EMAC_CPPI_RX_ERROR_FRAME        (0x03FC0000)


#define EMAC_MACCONTROL_RMIISPEED_100   (1 << 15)

#define EMAC_MACCONTROL_MIIEN_ENABLE    (0x20)
#define EMAC_MACCONTROL_FULLDUPLEX_ENABLE   (0x1)

#define EMAC_RXMBPENABLE_RXCAFEN_ENABLE (0x200000)
#define EMAC_RXMBPENABLE_RXBROADEN      (0x2000)

// Values for MACEIOVECTOR register

#define ACK_CORXTHRESH_INT              (0x0)
#define ACK_CORX_INT                    (0x1)
#define ACK_COTX_INT                    (0x2)
#define ACK_COMISC_INT                  (0x3)


#define EMAC_MACADDRLO_MATCHFILT   (1 << 19)
#define EMAC_MACADDRLO_VALID       (1 << 20)


// Values for MACEOIVECTOR

#define MACINVECTOR_RX0PEND         0x00000001
#define MACINVECTOR_TX0PEND         0x00010000
#define MACINVECTOR_USERPEND        0x01000000
#define MACINVECTOR_LINKPEND        0x02000000
#define MACINVECTOR_HOSTPEND        0x04000000
#define MACINVECTOR_STATPEND        0x08000000

#define MDIO_CONTROL_ENABLE         (0x40000000)
#define MDIO_CONTROL_FAULT          (0x80000)
#define MDIO_USERACCESS0_GO         (0x80000000)
#define MDIO_USERACCESS0_WRITE_READ (0x0)
#define MDIO_USERACCESS0_WRITE_WRITE  (0x40000000)

#define MAC_BUFFER_ALIGN             32
#define MAX_BUF_SIZE                 (2048 - 32)

#define MAC_ALIGNED_BUF_ADDR(addr) \
        ((((uint32_t)(addr) + MAC_BUFFER_ALIGN - 1) & ~(MAC_BUFFER_ALIGN - 1)))

#define MAX_RX_BUFFERS                128
#define MAX_TX_BUFFERS                64

#define DM644x_DEFRAG_PACKET          0x80000000

/* EMAC Control Register  Overlay Structure */

typedef volatile struct  {
    volatile unsigned int REVID;
    volatile unsigned int SOFTRESET;
    volatile unsigned char RSVD0[4];
    volatile unsigned int INTCONTROL;
    volatile unsigned int C0RXTHRESHEN;
    volatile unsigned int C0RXEN;
    volatile unsigned int C0TXEN;
    volatile unsigned int C0MISCEN;
    volatile unsigned int C1RXTHRESHEN;
    volatile unsigned int C1RXEN;
    volatile unsigned int C1TXEN;
    volatile unsigned int C1MISCEN;
    volatile unsigned int C2RXTHRESHEN;
    volatile unsigned int C2RXEN;
    volatile unsigned int C2TXEN;
    volatile unsigned int C2MISCEN;
    volatile unsigned int C0RXTHRESHSTAT;
    volatile unsigned int C0RXSTAT;
    volatile unsigned int C0TXSTAT;
    volatile unsigned int C0MISCSTAT;
    volatile unsigned int C1RXTHRESHSTAT;
    volatile unsigned int C1RXSTAT;
    volatile unsigned int C1TXSTAT;
    volatile unsigned int C1MISCSTAT;
    volatile unsigned int C2RXTHRESHSTAT;
    volatile unsigned int C2RXSTAT;
    volatile unsigned int C2TXSTAT;
    volatile unsigned int C2MISCSTAT;
    volatile unsigned int C0RXIMAX;
    volatile unsigned int C0TXIMAX;
    volatile unsigned int C1RXIMAX;
    volatile unsigned int C1TXIMAX;
    volatile unsigned int C2RXIMAX;
    volatile unsigned int C2TXIMAX;
} emac_ctrl_regs;


/* EMAC Register overlay */

/* Ethernet MAC Register Overlay Structure */
typedef volatile struct  {
    volatile unsigned int TXREVID;
    volatile unsigned int TXCONTROL;
    volatile unsigned int TXTEARDOWN;
    volatile unsigned char RSVD0[4];
    volatile unsigned int RXREVID;
    volatile unsigned int RXCONTROL;
    volatile unsigned int RXTEARDOWN;
    volatile unsigned char RSVD1[100];
    volatile unsigned int TXINTSTATRAW;
    volatile unsigned int TXINTSTATMASKED;
    volatile unsigned int TXINTMASKSET;
    volatile unsigned int TXINTMASKCLEAR;
    volatile unsigned int MACINVECTOR;
    volatile unsigned int MACEOIVECTOR;
    volatile unsigned char RSVD2[8];
    volatile unsigned int RXINTSTATRAW;
    volatile unsigned int RXINTSTATMASKED;
    volatile unsigned int RXINTMASKSET;
    volatile unsigned int RXINTMASKCLEAR;
    volatile unsigned int MACINTSTATRAW;
    volatile unsigned int MACINTSTATMASKED;
    volatile unsigned int MACINTMASKSET;
    volatile unsigned int MACINTMASKCLEAR;
    volatile unsigned char RSVD3[64];
    volatile unsigned int RXMBPENABLE;
    volatile unsigned int RXUNICASTSET;
    volatile unsigned int RXUNICASTCLEAR;
    volatile unsigned int RXMAXLEN;
    volatile unsigned int RXBUFFEROFFSET;
    volatile unsigned int RXFILTERLOWTHRESH;
    volatile unsigned char RSVD4[8];
    volatile unsigned int RX0FLOWTHRESH;
    volatile unsigned int RX1FLOWTHRESH;
    volatile unsigned int RX2FLOWTHRESH;
    volatile unsigned int RX3FLOWTHRESH;
    volatile unsigned int RX4FLOWTHRESH;
    volatile unsigned int RX5FLOWTHRESH;
    volatile unsigned int RX6FLOWTHRESH;
    volatile unsigned int RX7FLOWTHRESH;
    volatile unsigned int RX0FREEBUFFER;
    volatile unsigned int RX1FREEBUFFER;
    volatile unsigned int RX2FREEBUFFER;
    volatile unsigned int RX3FREEBUFFER;
    volatile unsigned int RX4FREEBUFFER;
    volatile unsigned int RX5FREEBUFFER;
    volatile unsigned int RX6FREEBUFFER;
    volatile unsigned int RX7FREEBUFFER;
    volatile unsigned int MACCONTROL;
    volatile unsigned int MACSTATUS;
    volatile unsigned int EMCONTROL;
    volatile unsigned int FIFOCONTROL;
    volatile unsigned int MACCONFIG;
    volatile unsigned int SOFTRESET;
    volatile unsigned char RSVD5[88];
    volatile unsigned int MACSRCADDRLO;
    volatile unsigned int MACSRCADDRHI;
    volatile unsigned int MACHASH1;
    volatile unsigned int MACHASH2;
    volatile unsigned int BOFFTEST;
    volatile unsigned int TPACETEST;
    volatile unsigned int RXPAUSE;
    volatile unsigned int TXPAUSE;
    volatile unsigned char RSVD6[16];
    volatile unsigned int RXGOODFRAMES;
    volatile unsigned int RXBCASTFRAMES;
    volatile unsigned int RXMCASTFRAMES;
    volatile unsigned int RXPAUSEFRAMES;
    volatile unsigned int RXCRCERRORS;
    volatile unsigned int RXALIGNCODEERRORS;
    volatile unsigned int RXOVERSIZED;
    volatile unsigned int RXJABBER;
    volatile unsigned int RXUNDERSIZED;
    volatile unsigned int RXFRAGMENTS;
    volatile unsigned int RXFILTERED;
    volatile unsigned int RXQOSFILTERED;
    volatile unsigned int RXOCTETS;
    volatile unsigned int TXGOODFRAMES;
    volatile unsigned int TXBCASTFRAMES;
    volatile unsigned int TXMCASTFRAMES;
    volatile unsigned int TXPAUSEFRAMES;
    volatile unsigned int TXDEFERRED;
    volatile unsigned int TXCOLLISION;
    volatile unsigned int TXSINGLECOLL;
    volatile unsigned int TXMULTICOLL;
    volatile unsigned int TXEXCESSIVECOLL;
    volatile unsigned int TXLATECOLL;
    volatile unsigned int TXUNDERRUN;
    volatile unsigned int TXCARRIERSENSE;
    volatile unsigned int TXOCTETS;
    volatile unsigned int FRAME64;
    volatile unsigned int FRAME65T127;
    volatile unsigned int FRAME128T255;
    volatile unsigned int FRAME256T511;
    volatile unsigned int FRAME512T1023;
    volatile unsigned int FRAME1024TUP;
    volatile unsigned int NETOCTETS;
    volatile unsigned int RXSOFOVERRUNS;
    volatile unsigned int RXMOFOVERRUNS;
    volatile unsigned int RXDMAOVERRUNS;
    volatile unsigned char RSVD7[624];
    volatile unsigned int MACADDRLO;
    volatile unsigned int MACADDRHI;
    volatile unsigned int MACINDEX;
    volatile unsigned char RSVD8[244];
    volatile unsigned int TX0HDP;
    volatile unsigned int TX1HDP;
    volatile unsigned int TX2HDP;
    volatile unsigned int TX3HDP;
    volatile unsigned int TX4HDP;
    volatile unsigned int TX5HDP;
    volatile unsigned int TX6HDP;
    volatile unsigned int TX7HDP;
    volatile unsigned int RX0HDP;
    volatile unsigned int RX1HDP;
    volatile unsigned int RX2HDP;
    volatile unsigned int RX3HDP;
    volatile unsigned int RX4HDP;
    volatile unsigned int RX5HDP;
    volatile unsigned int RX6HDP;
    volatile unsigned int RX7HDP;
    volatile unsigned int TX0CP;
    volatile unsigned int TX1CP;
    volatile unsigned int TX2CP;
    volatile unsigned int TX3CP;
    volatile unsigned int TX4CP;
    volatile unsigned int TX5CP;
    volatile unsigned int TX6CP;
    volatile unsigned int TX7CP;
    volatile unsigned int RX0CP;
    volatile unsigned int RX1CP;
    volatile unsigned int RX2CP;
    volatile unsigned int RX3CP;
    volatile unsigned int RX4CP;
    volatile unsigned int RX5CP;
    volatile unsigned int RX6CP;
    volatile unsigned int RX7CP;
} emac_regs;

/* EMAC Wrapper Register Overlay */
typedef volatile struct  {
//    volatile unsigned char RSVD0[4100];
    volatile unsigned char RSVD0[4];
    volatile unsigned int EWCTL;
    volatile unsigned int EWINTTCNT;
} ewrap_regs;


/* EMAC MDIO Register Overlay */
typedef volatile struct  {
    volatile unsigned int VERSION;
    volatile unsigned int CONTROL;
    volatile unsigned int ALIVE;
    volatile unsigned int LINK;
    volatile unsigned int LINKINTRAW;
    volatile unsigned int LINKINTMASKED;
    volatile unsigned char RSVD0[8];
    volatile unsigned int USERINTRAW;
    volatile unsigned int USERINTMASKED;
    volatile unsigned int USERINTMASKSET;
    volatile unsigned int USERINTMASKCLEAR;
    volatile unsigned char RSVD1[80];
    volatile unsigned int USERACCESS0;
    volatile unsigned int USERPHYSEL0;
    volatile unsigned int USERACCESS1;
    volatile unsigned int USERPHYSEL1;
} mdio_regs;

typedef struct _nic_dm644x_ext {
        nic_config_t    cfg;                /* New libdrvr config structure */
        int             chid;               /* channel id */
        int		coid;				/* connection id */
	int		tid;				/* thread id */
	int		iid;				/* interrupt id */
        int             reg_hdl;
        void            *dll_hdl;
        uint16_t        cell;
	unsigned	max_pkts;
	pthread_mutex_t	mutex;
	mdi_t		*mdi;
	int		phy_mode;
	int		negotiate;

	int		noirq;

        uintptr_t	emac_base;
	emac_regs	*emac;
	emac_ctrl_regs  *emac_ctrl; // L137 EMAC Control Registers
        ewrap_regs	*ewrap;
	mdio_regs	*mdio;

	emac_desc	*rx_desc;
	emac_desc	*tx_desc;

	npkt_t		**rx_pktq;
	npkt_t		**tx_pktq;

	int		rx_cidx;			/* last rx buffer consumed */
	int		tx_cidx;			/* last command comsumed */
	int		tx_pidx;			/* last command produced */

	struct cache_ctrl	cachectl;

	npkt_t		*nhead;
	npkt_t		*ntail;
	npkt_t		*rx_free;
	npkt_t		*tx_free;
	int		num_rx_free;		/* free buffers for receiver */
	int		rx_traffic;			/* Traffic indicator */
	pthread_mutex_t	rx_freeq_mutex;
        io_net_self_t   *ion;
	uint32_t	multicast_all;
	uint8_t		hash_cnt[64];
	uint32_t	hashhi;
	uint32_t	hashlo;
} dm644x_dev_t;

int 	dm644x_transmit(dm644x_dev_t *dm644x);
int 	dm644x_init(void *, dispatch_t *dpp, io_net_self_t *, char *options);
int 	dm644x_send_packets(npkt_t *npkt, void *);
int 	dm644x_receive(dm644x_dev_t *dm644x);
int 	dm644x_receive_complete(npkt_t *npkt, void *, void *);
int 	dm644x_devctl(void *hdl, int dcmd, void *data, size_t size, union _io_net_dcmd_ret_cred *ret);
int 	dm644x_shutdown1(int, void *);
int 	dm644x_shutdown2(int, void *);
int 	dm644x_advertise(int reg_hdl, void *func_hdl);
int 	dm644x_flush(int, void *hdl);
int 	dm644x_detect(void *, io_net_self_t *, char *options);
int 	dm644x_register_device(dm644x_dev_t *dm644x, io_net_self_t *, void *);
int	dm644x_add_rbuff(dm644x_dev_t *dm644x, int idx, npkt_t * npkt);
int 	dm644x_reset(dm644x_dev_t *dm644x);
int	dm644x_init_phy(dm644x_dev_t *dm644x);
void	dm644x_set_multicast(dm644x_dev_t *dm644x, uint32_t crc, uint32_t flag);
void	dm644x_dinit_memory(dm644x_dev_t * dm644x, int step);
npkt_t *dm644x_alloc_npkt(dm644x_dev_t *dm644x, size_t size);

extern paddr_t mphys(void *);

void 	nsec_delay(int nsec);


#define ion_rx_packets          dm644x->ion->tx_up
#define ion_register            dm644x->ion->reg
#define ion_deregister          dm644x->ion->dereg
#define ion_alloc               dm644x->ion->alloc
#define ion_free                dm644x->ion->free
#define ion_alloc_npkt          dm644x->ion->alloc_up_npkt
#define ion_mphys               dm644x->ion->mphys
#define ion_tx_complete         dm644x->ion->tx_done
#define ion_add_done            dm644x->ion->reg_tx_done

#endif  /* _DM644X_EMAC_H_ */
