/******************************************************************************
* Module name: (verbose.h)
*
* Copyright 2014-2015 Halliburton, All Rights Reserved.
*
* The information contained herein is confidential
* property of Halliburton. The user, copying, transfer or
* disclosure of such information is prohibited except
* by written agreement with Halliburton.
*
* Module Description:
* Implementation for verbose debugging printing
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
#ifndef VERBOSE_H_
#define VERBOSE_H_

#define VERBOSE_LEVEL_MAX 2

#define LOG_INFO(...) myprintf(APP_NAME, __VA_ARGS__)
#define LOG_DEBUG(...) if(verbose_level > 0) myprintf(APP_NAME, __VA_ARGS__)
#define LOG_DETAILS(...) if(verbose_level > 1) myprintf(APP_NAME, __VA_ARGS__)

extern volatile int verbose_level;

int myprintf(const char *prog, const char *fmt, ...);

#endif /* VERBOSE_H_ */
