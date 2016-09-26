/*
 * Halliburton ESG
 * Utilities
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#include <stdio.h>
#include <stdlib.h>

#include "omapl137spi.h"
#include "omapl1xx_gpio.h"
#include "verbose.h"


/**************************************************
* Function name		: int32 initCircularBuffer(circularBuffer *buff,
*                     int32 *data, int32 size)
* 	 returns		: always returns 0
* 	 arg1			: buff - Circular buffer to be initialized
* 	 arg2			: data - Initialization data
* 	 arg3			: size - No. of elements
* Created by		: Wei Sun
* Date				: 07/28/2016
* Description		: Initialise circular buffer with provided data
* Notes		    	: None
* Revision History	:
*					7/28/2016, Wei Sun
*					- Created
**************************************************/
void initCircularBuffer(circular_buffer_t *buff, uint16_t *data, size_t size)
{

	buff->rdptr = buff->wrptr = buff->base = data;
	buff->fillLevel = 0;
    buff->maxSize = size;
    buff->underflow = buff->overflow = 0;
    buff->end = (uint32_t)(data + (size - 1) * sizeof(uint16_t));

    return;
}
