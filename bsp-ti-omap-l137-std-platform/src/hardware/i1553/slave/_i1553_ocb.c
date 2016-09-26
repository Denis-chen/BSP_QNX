/*
 * _i1553_ocb.c
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */


#include "proto.h"

/*****************************************************************************************
* Function name		: i1553_ocb_t * _i1553_ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *attr
* 	 returns		: bytes read
* 	 arg1			: resmgr context_t
* 	 arg2			: IO Function attribute
* 	 arg3			:
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: 1553 read operation
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
******************************************************************************************/

i1553_ocb_t *_i1553_ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *attr)
{
   i1553_ocb_t   *ocb;

   if(NULL==(ocb = calloc(1, sizeof(*ocb))))
	   return NULL;

   ocb->chip = I1553_DEV_ID_NONE;

   return ocb;
}
