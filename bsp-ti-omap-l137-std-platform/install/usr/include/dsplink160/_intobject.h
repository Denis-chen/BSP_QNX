/** ============================================================================
 *  @file   _intobject.h
 *
 *  @path   $(DSPLINK)/gpp/inc/usr/
 *
 *  @desc   Declares an object that encapsulates the interrupt information
 *          reqiured by Linux.
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (_INTOBJECT_H)
#define _INTOBJECT_H


/*  ----------------------------------- DSP/BIOS Link                   */
#include <dsplink160/gpptypes.h>


#if defined (__cplusplus)
extern "C" {
#endif


/** ============================================================================
 *  @func   IPS_checkIntGen
 *
 *  @desc   This function implements check routine to check the interrupt
 *          generation. In case of shared interrupts, other devices can also
 *          generate same ints.
 *
 *  @modif  None.
 *  ============================================================================
 */
typedef Bool (*INT_checkGen) (IN Pvoid refData) ;


/** ============================================================================
 *  @name   InterruptObject
 *
 *  @desc   Object encapsulating the OS dependent interrupt information.
 *
 *  @field  dspId
 *              Processor identifier
 *  @field  intId
 *              Interrupt identifier
 *  @field  shared
 *              Tells whether the interrupt is shared with other devices.
 *  @field  checkFunc
 *              Function to check for interrupt generating device.
 *  @field  param
 *              Parameter to be passed to check function.
 *  ============================================================================
 */
typedef struct InterruptObject_tag {
    ProcessorId  dspId     ;
    Int32        intId     ;
    Bool         shared    ;
    INT_checkGen checkFunc ;
    Pvoid        param     ;
} InterruptObject ;


#if defined (__cplusplus)
}
#endif


#endif /* !defined (_INTOBJECT_H) */
