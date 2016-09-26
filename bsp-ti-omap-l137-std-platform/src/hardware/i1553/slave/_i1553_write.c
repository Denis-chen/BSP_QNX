/*
 * _i1553_write.c
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */


#include "proto.h"
#include "verbose.h"

/*****************************************************************************************
* Function name		: _i1553_write(resmgr_context_t *ctp, io_read_t *msg, i1553_ocb_t *ocb)
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
int _i1553_write(resmgr_context_t *ctp, io_write_t *msg, i1553_ocb_t *ocb)
{

	uint8_t          *buf;
	int              nonblock;
    int              nbytes, status;
    I1553DEV         *drvhdl = (I1553DEV *)ocb->hdr->attr;
    i1553_dev_t      *dev = drvhdl->hdl;

    if((status = iofunc_write_verify(ctp, msg, &ocb->hdr, &nonblock)) != EOK)
      return status;

    /*
     *  Check to see if the device is locked
     */

    if((status = _i1553_lock_check(ctp, ocb->chip, ocb)) != EOK)
    	return status;

    nbytes = msg->i.nbytes;
    if(nbytes <= 0 )
    {
    	_IO_SET_WRITE_NBYTES(ctp, 0);
    	return _RESMGR_NPARTS(0);
    }


    /* check if message buffer is too short */

    if((sizeof(msg->i) + nbytes) > ctp->msg_max_size)
    {
    	if(dev->buflen < nbytes)
    	{
    		dev->buflen = nbytes;
    		if(dev->buf)
    			free(dev->buf);                     /* need to free the old one before create new bigger one */
    		if(NULL == (dev->buf = malloc(dev->buflen)))
    		{
    			dev->buflen = 0;
    			return ENOMEM;
    		}
    	}
    	status = resmgr_msgread(ctp, dev->buf, nbytes, sizeof(msg->i));

    	if(status < 0)
    		return errno;
    	if(status < nbytes)
    		return EFAULT;

    	buf = dev->buf;

    }
    else
    	buf = ((uint8_t *)msg) + sizeof(msg->i);

    status = dev->funcs->write(drvhdl, buf, &nbytes);                      /* return nbytes, not much useful here, since the nbytes are already updated in the driver */

    if(nbytes == 0)
    	return EAGAIN;

    if(nbytes > 0)
    {
    	_IO_SET_WRITE_NBYTES(ctp, nbytes);
    	return _RESMGR_NPARTS(0);
    }
}
