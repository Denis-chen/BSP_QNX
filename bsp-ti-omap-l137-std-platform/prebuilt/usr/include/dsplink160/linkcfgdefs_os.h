/** ============================================================================
 *  @file   linkcfgdefs_os.h
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


#if !defined (LINKCFGDEFS_OS_H)
#define LINKCFGDEFS_OS_H


/*  ----------------------------------- DSP/BIOS LINK Headers       */
#include <dsplink160/dsplink.h>


#if defined (__cplusplus)
EXTERN "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @name   LINKCFG_GppOs
 *
 *  @desc   This structure defines the configuration structure for the GPP OS
 *          specific configuration.
 *
 *  @field  reserved
 *              Reserved for any OS-specific configuration information.
 *  ============================================================================
 */
typedef struct LINKCFG_GppOs_tag {
    Bool               handleSignals ;
    Uint32             numSignals ;
    Uint32 *           sigNumArray ;
} LINKCFG_GppOs ;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (LINKCFGDEFS_OS_H) */
