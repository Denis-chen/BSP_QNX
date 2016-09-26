/*
 * _i1553_devlock.c
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */


#include "proto.h"

/*****************************************************************************************
* Function name		:i1553_lock_t * _i1553_islock(resmgr_context_t *ctp, uint32_t device, i1553_ocb_t *ocb)
* 	 returns		: lock
* 	 arg1			: resmgr context_t
* 	 arg2			: device number
* 	 arg3			: ocb
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: 1553 read operation
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
******************************************************************************************/
i1553_lock_t * _i1553_islock(resmgr_context_t *ctp, uint32_t device, i1553_ocb_t *ocb)
{
	I1553DEV         *drvhdl = (I1553DEV *)ocb->hdr->attr;
	i1553_lock_t     *lock = drvhdl->lock;

	if(lock == NULL)
		return NULL;

	device &= I1553_DEV_ID_MASK;

	while(lock)
	{
		if(lock->device == device)
			return lock;
		lock = lock->next;
	}

	return NULL;

}
/*****************************************************************************************
* Function name		:int _i1553_lock_dev(resmgr_context_t *ctp, uint32_t device, i1553_ocb_t ocb)
* 	 returns		: ERROR MESSAGE
* 	 arg1			: resmgr context_t
* 	 arg2			: device number
* 	 arg3			: ocb
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: lock 1553 device
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
******************************************************************************************/
int _i1553_lock_dev(resmgr_context_t *ctp, uint32_t device, i1553_ocb_t ocb)
{
	I1553DEV      *drvhdl = (I1553DEV *)ocb->hdr->attr;
	i1553_lock_t  *lock = drvhdl->lock;

	if(NULL == (lock = calloc(1, sizeof(i1553_lock_t))))
		return (ENOMEM);

	/*
	 * Simply add to the head of the lock list
	 */
	lock->owner = ocb;
	lock->device = device;
	lock->next = drvhdl->lock;
	drvhdl->lock = lock;

	return (EOK);
}

/*****************************************************************************************
* Function name		:int _i1553_unlock_dev(resmgr_context_t *ctp, uint32_t device, i1553_ocb_t ocb)
* 	 returns		: ERROR MESSAGE
* 	 arg1			: resmgr context_t
* 	 arg2			: device number
* 	 arg3			: ocb
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: unlock 1553 device
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
******************************************************************************************/
int _i1553_unlock_dev(resmgr_context_t *ctp, uint32_t device, i1553_ocb_t ocb)
{
	I1553DEV      *drvhdl = (I1553DEV *)ocb->hdr->attr;
	i1553_lock_t  *prev, *curr;


	prev = curr = drvhdl->lock;

	if(curr == NULL)
	  return EINVAL;

	device &= I1553_DEV_ID_MASK;

	while (curr)
	{
		if(((curr->device == device) || (device == I1553_DEV_ID_NONE)) && (curr->owner == ocb))
		{
			if(curr == prev)      /* on the top of the list */
			   drvhdl->lock = curr->next;
		   else
			 prev->next = curr->next;
		  free(curr);
		  return EOK;
		}

		prev = curr;
		curr = curr->next;
	}
	return (ENODEV);
}
