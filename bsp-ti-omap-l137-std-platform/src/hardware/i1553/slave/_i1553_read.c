/*
 * _i1553_read.c
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */


#include "proto.h"
#include "verbose.h"



typedef struct
{
	struct _io_read        read;
	struct _xtype_offset   offset;
} io_pead_t;

/*****************************************************************************************
* Function name		: _i1553_read(resmgr_context_t *ctp, io_read_t *msg, i1553_ocb_t *ocb)
* 	 returns		: bytes read
* 	 arg1			: resmgr context_t
* 	 arg2			: io message
* 	 arg3			: ocb
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: 1553 read operation
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
******************************************************************************************/
int _i1553_read(resmgr_context_t *ctp, io_read_t *msg, i1553_ocb_t *ocb)
{
	uint16_t       *buf;
	I1553DEV       *drvhdl = (I1553DEV *)ocb->hdr.attr;            //find the attr pointer, then the driver handle is followed define I1553DEV
	i1553_dev_t    *dev = drvhdl->hdl;
	int            nonblock = 0;
	int            status;
	int            nbytes;
	i1553_spi_t    *i1553dev;


	i1553dev = (i1553_spi_t *)drvhdl;

	if ((status= iofunc_read_verify(ctp, msg, &ocb->hdr, &nonblock)) != EOK)
	{
		return status;
	}

	if(msg->i.nbytes <= 0)
	{
		_IO_SET_READ_NBYTES(ctp, 0);
		return _RESMGR_NPARTS(0);
	}

	nbytes = msg->i.nbytes;


	if (NULL == (buf = malloc(nbytes)))
	{
		return ENOMEM;
	}

	/* copy the data from the i1553 driver ring buffer */
	buf = dev->funcs->read(drvhdl, buf, &nbytes);

	_IO_SET_READ_NBYTES(ctp, nbytes);
	return _RESMGR_PTR(ctp, buf, nbytes);

}
