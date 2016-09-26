/*
 * _i1553_create_instance.c
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */


#include "proto.h"


/*
 * register the resource manager
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */
static int _i1553_register_interface(void* data)
{
	i1553_dev_t       *dev = data;
	I1553DEV          *drvhdl;
	resmgr_attr_t     rattr;
	char              devname[PATH_MAX + 1];

	if ((drvhdl = dev->funcs->init(dev, dev->opts)) == NULL)
	{
		free(dev->opts);
		return (!EOK);
	}

	dev->drvhdl = drvhdl;

	/*set up i/o handler functions */
	memset(&rattr, 0, sizeof(rattr));
	rattr.nparts_max    = I1553_RESMGR_NPARTS_MIN;        // Number may adjust for 1553 structure
	rattr.msg_max_size  = I1553_RESMGR_MSGSIZE_MIN;      // Number may adjust for 1553 data structure

	iofunc_attr_init(&drvhdl->attr, S_IFCHR | 0666, NULL, NULL);    // may change the File type bit to S_IFBLK(block special)
    drvhdl->attr->mount = &_i1553_mount;

    /* register device name */
    snprintf(devname, PATH_MAX, "/dev/i1553_dev%d", dev->devnum);
    if(-1 == (dev->id = resmgr_attach(dev->dpp, &rattr, devname, _FTYTYPE_ANY, 0,
    	           &_i1553_connect_funcs, &_i1553_io_funcs, (void*)drvhdl))) {
    	perror("resmgr_attach() failed");
    	goto failed1;
    }

    resmgr_devino(dev->id, &drvhdl->attr.mount->dev, &drvhdl->attr.inode);

    if((dev->ctp = dispatch_context_alloc(dev->dpp)) != NULL)
    {
    	return (EOK);
    }

    perror("dispatch_context_alloc failed");

    resmgr_detach(dev->dpp, dev->id, _RESMGR_DETACH_ALL);

failed1:
    dev->funcs->fini(drvhdl);

    return(!EOK);

}

/*
 * Driver thread start message blocking
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */
static void* _i1553_driver_thread(void *data)
{
	i1553_dev_t   *dev = data;
	if(_i1553_register_interface(data) != EOK)
	{
		return NULL;
	}
	while(1)
	{
		if((dev->ctp = dispatch_block(dev->ctp)) != NULL)
			dispatch_handler(dev->ctp);
		else
			break;
	}

	return NULL;
}

/*
 * Create 1553 instance
 *
 *  Created on: Jul 13, 2016
 *      Author: hx32927
 */

int _i1553_create_instance(i1553_dev_t *dev)
{
	pthread_attr_t       pattr;
	struct sched_param   param;

	if(NULL == (dev->dpp = dispatch_create())) {
		perror("dispatch_create() failed");
		goto failed0;
	}

	pthread_attr_init(&pattr);
	pthread_attr_setschedpolicy(&pattr, SCHED_RR);
	param.sched_priority = 21;                     // may need to adjust the priority later to higher
	pthread_attr_setschedparam(&pattr, &param);
	pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);

	// Create thread for this interface
	if(pthread_create(NULL, &pattr, (void *)_i1553_driver_thread, dev) != EOK)
	{
		perror("pthread_create() failed");
		goto failed1;
	}

	return (EOK);

failed1:
    dispatch_destroy(dev->dpp);
failed0:
    return (-1);

}
