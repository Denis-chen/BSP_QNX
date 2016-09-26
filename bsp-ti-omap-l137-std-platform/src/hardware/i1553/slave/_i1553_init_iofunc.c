/*
 * Halliburton ESG
 * 1553 Driver (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#include "proto.h"

iofunc_funcs_t          _i1553_ocb_funcs = { _IOFUNC_NFUNCS, _i1553_ocb_calloc, _i1553_ocb_free};
resmgr_io_funcs_t       _i1553_io_funcs;
resmgr_connect_funcs_t  _i1553_connect_funcs;
iofunc_mount_t          _i1553_mount = {0, 0, 0, 0, &_i1553_ocb_funcs };

int _i1553_init_iofunc(void)
{
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &_i1553_connect_funcs, _RESMGR_IO_NFUNCS, &i1553_io_funcs);
	_i1553_io_funcs.read = _i1553_read;
	_i1553_io_funcs.write= _i1553_write;
	_i1553_io_funcs.devctl = _i1553_devctl;
	_i1553_io_funcs.close_ocb = _i1553_close_ocb;
	_i1553_io_funcs.msg = _i1553_iomsg;
	_i1553_io_funcs.notify = _i1553_notify;

	return EOK;
}
