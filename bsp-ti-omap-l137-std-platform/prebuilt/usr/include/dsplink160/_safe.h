/** ============================================================================
 *  @file   _safe.h
 *
 *  @path   $(DSPLINK)/gpp/inc/usr/
 *
 *  @desc   Contains safe programming macros
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (_SAFE_H)
#define _SAFE_H

/*  ----------------------------------- DSP/BIOS Link               */
#include <dsplink160/dsplink.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @macro  IS_OBJECT_VALID
 *
 *  @desc   Checks validity of object by comparing against it's signature.
 *  ============================================================================
 */
#define IS_OBJECT_VALID(obj, sign)                                  \
    (((obj != NULL) && ((obj)->signature == sign)) ? TRUE : FALSE)


/** ============================================================================
 *  @macro  IS_RANGE_VALID
 *
 *  @desc   Checks if a value lies in given range.
 *  ============================================================================
 */
#define IS_RANGE_VALID(x,min,max) (((x) < (max)) && ((x) >= (min)))

/** ============================================================================
 *  @macro  MIN
 *
 *  @desc   Returns minumum of the two arguments
 *  ============================================================================
 */
#define MIN(a,b) (((a) < (b)) ? (a) : (b))


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (_SAFE_H) */
