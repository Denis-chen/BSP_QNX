/*
 * Halliburton ESG
 * 1553 Library API (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#include <fcntl.h>
#include <string.h>
#include <hw/spi-master.h>

int i1553_read(int fd, void *buf, int len)
{
	return read(fd, buf, len);
}