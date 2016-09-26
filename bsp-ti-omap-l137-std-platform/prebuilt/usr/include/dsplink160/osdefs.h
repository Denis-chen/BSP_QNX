/** ============================================================================
 *  @file   osdefs.h
 *
 *  @path   $(DSPLINK)/gpp/inc/sys/nto/
 *
 *  @desc   Defines OS-specific constants and interfaces for configuration of
 *          DSP/BIOS LINK.
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (OSDEFS_OS_H)
#define OSDEFS_OS_H


/*  ----------------------------------- DSP/BIOS LINK Headers       */
#include <dsplink160/dsplink.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*  ============================================================================
 *  @macro  TICKS_PER_MILLI_SEC
 *
 *  @desc   CPU Ticks per miliseconds.
 *  ============================================================================
 */
#define TICKS_PER_MILLI_SEC   10u


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (OSDEFS_H) */
