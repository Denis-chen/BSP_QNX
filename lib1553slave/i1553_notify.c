/*
 * Halliburton ESG
 * 1553 Library API (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#include <fcntl.h>
#include <string.h>

int i1553_ionotify(int fd, const struct sigevent *event){

	struct sigevent *i1553event = event;
	int flag;



	ionotif(fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_OUTPUT, i1553event);                  // ask resource manager to notify when it's ready to accept data from client






}

