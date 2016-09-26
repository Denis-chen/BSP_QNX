/** ============================================================================
 *  @file   dbc.h
 *
 *  @path   $(DSPLINK)/gpp/inc/usr/
 *
 *  @desc   Design by Contract
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (DBC_H)
#define DBC_H


/*  ----------------------------------- DSP/BIOS Link                   */
#include <dsplink160/dsplink.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/*  ============================================================================
 *  @macro  DBC_PRINTF
 *
 *  @desc   This macro expands to the print function. It makes the DBC
 *          macros portable across OSes.
 *  ============================================================================
 */
#define  DBC_PRINTF     PRINT_Printf


#if defined (DDSP_DEBUG)

/** ============================================================================
 *  @macro  DBC_Assert
 *
 *  @desc   Assert on expression.
 *  ============================================================================
 */
#define DBC_Assert(exp)                                                        \
        if (!(exp)) {                                                          \
            DBC_PRINTF ("Assertion failed ("#exp"). File : "__FILE__           \
                        " Line : %d\n", __LINE__) ;                            \
        }

/** ============================================================================
 *  @macro  DBC_Require
 *
 *  @desc   Function Precondition.
 *  ============================================================================
 */
#define DBC_Require    DBC_Assert

/** ============================================================================
 *  @macro  DBC_Ensure
 *
 *  @desc   Function Postcondition.
 *  ============================================================================
 */
#define DBC_Ensure     DBC_Assert

#else /* defined (DDSP_DEBUG) */

/*  ============================================================================
 *  @macro  DBC_Assert/DBC_Require/DBC_Ensure
 *
 *  @desc   Asserts defined out.
 *  ============================================================================
 */
#define DBC_Assert(exp)
#define DBC_Require(exp)
#define DBC_Ensure(exp)

#endif /* defined (DDSP_DEBUG) */


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (DBC_H) */
