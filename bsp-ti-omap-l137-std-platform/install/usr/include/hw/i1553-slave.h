#ifndef _I1553_SLAVE_LIB_H_INCLUDED
#define _I1553_SLAVE_LIB_H_INCLUDED

#include <sys/iofunc.h>
#include <sys/dispatch.h>

#ifndef	__TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#ifndef	_INTTYPES_H_INCLUDED
#include <inttypes.h>
#endif

#include <sys/iomsg.h>

#define	I1553_VERSION_MAJOR	1
#define	I1553_VERMAJOR_SHIFT	16
#define	I1553_VERSION_MINOR	0
#define	I1553_VERMINOR_SHIFT	8
#define I1553_REVISION		0
#define	I1553_VERREV_SHIFT	0


/*
 * SPI driver information
 */
typedef struct {
	uint32_t	version;
	char		name[16];	/* Driver name */
	uint32_t	feature;
#define	SPI_FEATURE_DMA			(1 << 31)
#define	SPI_FEATURE_DMA_ALIGN	0xFF		/* DMA buffer alignment mask, alignment = 2^alignment */
} i1553_drvinfo_t;


typedef struct {
	uint32_t	mode;
#define	SPI_MODE_CHAR_LEN_MASK	(0xFF)		/* Charactor length */
#define	SPI_MODE_CKPOL_HIGH		(1 <<  8)
#define	SPI_MODE_CKPHASE_HALF	(1 <<  9)
#define	SPI_MODE_BODER_MSB		(1 << 10)
#define	SPI_MODE_CSPOL_MASK		(1 << 11)	/* Chip select polarity */
#define	SPI_MODE_CSPOL_HIGH		(1 << 11)
#define	SPI_MODE_CSSTAT_HIGH	(1 << 12)
#define	SPI_MODE_CSHOLD_HIGH	(1 << 13)
#define	SPI_MODE_RDY_MASK		(3 << 14)	/* Ready signal control */
#define	SPI_MODE_RDY_NONE		(0 << 14)
#define	SPI_MODE_RDY_EDGE		(1 << 14)	/* Falling edge signal */
#define	SPI_MODE_RDY_LEVEL		(2 << 14)	/* Low level signal */
#define	SPI_MODE_IDLE_INSERT	(1 << 16)

#define	SPI_MODE_LOCKED			(1 << 31)	/* The device is locked by another client */

	uint32_t	clock_rate;
	uint16_t    rtu_addr;
	uint16_t    gpio_bank;
	uint16_t    gpio_pin;
} i1553_cfg_t;


#define	I1553_DEV_ID_MASK			0xFFFF
#define	I1553_DEV_ID_NONE			I1553_DEV_ID_MASK

/* For SPI API interface */
#define	I1553_DEV_DEFAULT			(1 << 31)	/* Default device, use by spi_setcfg()/spi_getdevinfo() call */
#define	I1553_DEV_LOCK			     (1 << 30)	/* Lock device, for spi_read()/spi_write()/spi_exchange() */
#define	I1553_DEV_UNLOCK		 	(1 << 29)	/* Unlock device, for spi_read()/spi_write()/spi_exchange() */

/* For SPI driver interface */
#define SPI_DEV_XFER_MASK		3
#define SPI_DEV_XFER_SHIFT		16
#define SPI_DEV_EXCHANGE		3
#define SPI_DEV_READ			1
#define SPI_DEV_WRITE			2

typedef struct {
	uint32_t	device;		/* Device ID */
	char		name[16];	/* Device description */
	i1553_cfg_t	cfg;		/* Device configuration */
} i1553_devinfo_t;

/*
 * Resource Manager Interface
 */
//#define	_IOMGR_SPI	(_IOMGR_PRIVATE_BASE + 0x01)

#define _IOMSG_I1553  (_IOMGR_PRIVATE_BASE + 0x02)

typedef struct {
	io_msg_t	msg_hdr;
#define	_I1553_IOMSG_RTU_ADDR		0x0001
#define	_I1553_IOMSG_RTU_STATUS		0x0002
#define	_I1553_IOMSG_RX_BUFF_SIZE	0x0003
#define	_I1553_IOMSG_TX_BUFF_SIZE	0x0004
#define	_I1553_IOMSG_DMAXCHANGE	    0x0005
	uint32_t	device;
	int32_t		xlen;
} i1553_msg_t;

/*
 * DMA buffer address : physical address
 */
typedef struct {
	uint64_t	rpaddr;
	uint64_t	wpaddr;
} i1553_dma_paddr_t;


/*
 * The following devctls are used by a client application
 * to control the SPI interface.
 */

#include <devctl.h>
#define _DCMD_I1553				_DCMD_MISC

#define DCMD_I1553_SET_CONFIG	__DIOT (_DCMD_I1553, 0x11, i1553_cfg_t)
#define	DCMD_I1553_GET_DRVINFO	__DIOF (_DCMD_I1553, 0x12, i1553_drvinfo_t)
#define	DCMD_I1553_GET_DEVINFO	__DIOF (_DCMD_I1553, 0x13, i1553_devinfo_t)


/*
 * SPI API calls
 */
int	i1553_open(const char *path);
int i1553_close(int fd);
int i1553_setcfg(int fd, uint32_t device, spi_cfg_t *cfg);
int i1553_getdevinfo(int fd, uint32_t device, spi_devinfo_t *devinfo);
int i1553_getdrvinfo(int fd, spi_drvinfo_t *drvinfo);
int i1553_read(int fd, uint32_t device, void *buf, int len);
int i1553_write(int fd, uint32_t device, void *buf, int len);
int i1553_xchange(int fd, uint32_t device, void *wbuf, void *rbuf, int len);
int i1553_cmdread(int fd, uint32_t device, void *cbuf, int16_t clen, void *rbuf, int rlen);
int i1553_dma_xchange(int fd, uint32_t device, void *wbuf, void *rbuf, int len);



/*
 * For SPI driver interface
 */

/*
 * Hardware interface for low level driver
 */
typedef struct {
	/* size of this structure */
	size_t	size;

	/*
	 * Initialize master interface.
	 * Returns a handle associated with this driver
	 * Returns:
	 * !NULL    success
	 * NULL     failure
	 */
	void*	(*init)(void *hdl, char *options);

	/*
	 * Clean up driver.
	 * Frees memory associated with "hdl".
	 */
	void	(*fini)(void *hdl);

	/*
	 * Get driver information
	 */
	int		(*drvinfo)(void *hdl, i1553_drvinfo_t *info);

	/*
	 * Get device information
	 */
	int		(*devinfo)(void *hdl, uint32_t device, i1553_devinfo_t *info);

	/*
	 * Set i1553 configuration
	 */
	int		(*setcfg)(void *hdl, uint16_t device, i1553_cfg_t *cfg);

	/*
	 * i1553 read function calls
	 */
	void*	(*read)(void *hdl, uint8_t *buf, int *len);

	/*
	 * i1553 write function calls
	 */
	int		(*write)(void *hdl, uint8_t *buf, int len);
} i1553_funcs_t;


/*
 * Low-level entry, has to be at the beginning of low-level handle
 */
typedef struct _i1553dev_entry_t {
	iofunc_attr_t	attr;
	iofunc_notify_t notify[3];   /* an array of three notification list -- one for each possible condition(_NOTIFY_COND_INPUT(notify[0]), _NOTIFY_COND_OUTPUT(notify[1] AND _NOTIFY_COND_OBAND) that a client can ask to be notified about */
	void			*hdl;		/* Pointer to high-level handle */
	void			*lock;		/* Pointer to lock list */
} I1553DEV;


#endif

/* __SRCVERSION("spi-master.h $Rev: 170072 $"); */
