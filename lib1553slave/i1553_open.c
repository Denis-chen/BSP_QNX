/*
 * Halliburton ESG
 * 1553 Library API (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#include <fcntl.h>
#include <string.h>

/*
 * Open
 */

int i1553_open(const char *path)
{
	return open(path, O_RDWR);
}
