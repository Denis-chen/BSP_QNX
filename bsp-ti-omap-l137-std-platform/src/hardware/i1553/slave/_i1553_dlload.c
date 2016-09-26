/*
 * Halliburton ESG
 * 1553 Driver (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */


#include <dlfcn.h>
#include "proto.h"

void *_i1553_dlload(void **hdl, const char *optarg)
{
	char        dllpath[_POSIX_PATH_MAX + 1];
	void        *dlhdl,   *entry;

	if(strchr(optarg, '/') != NULL)
		strcpy(dllpath, optarg);

	else
		sprintf(dllpath, "i1553-%s.so", optarg);

	dlhdl = dlopen(dllpath, 0);

	if(dlhdl != NULL){
		entry = dlsym(dlhdl, "i1553_drv_entry");

		if(entry != NULL){
			*hdl = dlhdl;
			return entry;
		}
		dlclose(dlhdl);
	}

    return NULL;
}
