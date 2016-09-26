/* Implementation for verbose debugging printing
*
* Language: C
*
* Created by: 	David Wong
* Date:		2015/06/11
*
* Revision History	:
*			Date, Developer Name
*			Detail description
*
 ******************************************************************************/
/* Include section
* (add all #includes here)
*
 ******************************************************************************/
#include "verbose.h"
#include <stdio.h>
#include <stdarg.h>

volatile int verbose_level = 0;

int myprintf(const char *prog, const char *fmt, ...)
{
    // Print out the program name, then forward the rest onto printf
    printf("%s: ", prog);

    va_list ap;
    va_start(ap, fmt);
    int ret = vprintf(fmt, ap);
    va_end(ap);

    return ret;
}
