/** ============================================================================
 *  @file   dsplink.h
 *
 *  @path   $(DSPLINK)/gpp/inc/usr/
 *
 *  @desc   Defines data types and structures used by DSP/BIOS(tm) Link.
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (DSPLINK_H)
#define DSPLINK_H


/*  ----------------------------------- DSP/BIOS Link                   */
#include <dsplink160/gpptypes.h>
#include <dsplink160/constants.h>
#include <dsplink160/errbase.h>
#include <dsplink160/archdefs.h>
#include <dsplink160/linkcfgdefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*  ============================================================================
 *  @const  MAX_IPS
 *
 *  @desc   Maximum number of IPS objects supported for each DSP.
 *
 *  ============================================================================
 */
#define MAX_IPS             16u

/** ============================================================================
 *  @const  WAIT_FOREVER
 *
 *  @desc   Wait indefinitely.
 *  ============================================================================
 */
#define WAIT_FOREVER           (~((Uint32) 0u))

/** ============================================================================
 *  @const  WAIT_NONE
 *
 *  @desc   Do not wait.
 *  ============================================================================
 */
#define WAIT_NONE              ((Uint32) 0u)


/** ============================================================================
 *  @macro  IS_GPPID
 *
 *  @desc   Is the GPP ID valid.
 *  ============================================================================
 */
#define IS_GPPID(id)        (id == ID_GPP)


#if defined (__cplusplus)
}
#endif


#endif /* !defined (DSPLINK_H) */
