/*
 * Halliburton ESG
 * 1553 Driver (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#ifndef _I1553_PROTO_H_INCLUDED
#define _I1553_PROTO_H_INCLUDED

struct i1553_ocb;
#define IOFUNC_OCB_T    struct i1553_ocb


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <hw/i1553-slave.h>

#define I1553_RESMGR_NPARTS_MIN   2
#define I1553_RESMGR_MSGSIZE_MIN  2048

typedef struct i1553_ocb{
	iofunc_ocb_t   hdr;
	uint32_t       chip;
}i1553_ocb_t;


typedef struct i1553_lock {
	I1553_ocb_t *owner;
	uint32_t    device;
	struct i1553_lock *next;
}i1553_lock_t;

typedef struct i1553_dev{
	dispatch_t           *dpp;
	dispatch_context_t   *ctp;
	int                   id;
	i1553_funcs_t         *funcs;

	uint8_t               *buf;
	uint8_t               *dmabuf;
	unsigned               buflen;

	void                  *drvhdl;
	void                  *dlhdl;

	pthread_t             tid;
	int                   chid;
	int                   coid;

	char                  *opts;
	int                   devnum;

	void                  *next;

}i1553_dev_t;

