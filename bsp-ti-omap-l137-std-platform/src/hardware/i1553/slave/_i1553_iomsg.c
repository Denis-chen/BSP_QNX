/*
 * _i1553_iomsg_read.c
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */


#include "proto.h"


/******************************************************************
* Function name		: int _i1553_iomsg(resmgr_context_t *ctp,
*                     io_msg_t *msg, i1553_ocb_t *ocb)
* 	 returns		:
* 	 arg1			:
* 	 arg2			:
* 	 arg3			:
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		:
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
*******************************************************************/

int _i1553_iomsg(resmgr_context_t *ctp, io_msg_t *msg, i1553_ocb_t *ocb)
{

   i1553_msg_t    *i1553msg = (i1553_msg_t *)msg;
   I1553DEV       *drvhdl = (I1553DEV *)ocb->hdr->attr;
   i1553_dev_t    *dev = drvhdl->hdl;
   i1553_spi_t    *i1553dev = (i1553_spi_t *)drvhdl;
   int  nbytes, err;
   uint8_t        *buf;




   if(((i1553msg->msg_hdr.i.combine_len < sizeof(i1553_msg_t))) || ((i1553msg->msg_hdr.i.mgrid != _IOMSG_I1553)))
	   return ENOSYS;


   nbytes = i1553msg->xlen;

   if(nbytes <= 0)
   {
	   _IO_SET_READ_NBYTES(ctp, 0);
	   return _RESMGR_NPARTS(0);
   }
/* we won't check the size here since the size should be very small for the RTU application
   if(nbytes > ctp->msg_max_size)
   {

   }
   */

   if((err == _i1553_lock_check(ctp, i1553msg->device, ocb)) != EOK)
	   return err;

   buf = (uint8_t *)msg;

   buf += sizeof(i1553_msg_t);

   switch(i1553msg->msg_hdr.i.subtype)
   {
   case _I1553_IO_MSG_RTU_ADDR:

	   i1553dev->rtu_addr = *((uint16_t *)buf);            //update the tool rtu address
	   break;

   case _I1553_IO_MSG_RTU_STATUS:
	   i1553dev->rtu_status = *((uint16_t *)buf);
	   break;

   case _I1553_IO_


   }




}
