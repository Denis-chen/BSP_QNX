/** ============================================================================
 *  @file   _ringiodefs.h
 *
 *  @path   $(DSPLINK)/gpp/inc/usr/
 *
 *  @desc   Global definitions for RingIO module
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (_RINGIODEFS_H)
#define _RINGIODEFS_H


/*  ----------------------------------- DSP/BIOS LINK Headers       */
#include <dsplink160/gpptypes.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @name   RingIO_MemInfo
 *
 *  @desc   Contains memory information for the RingIO component.
 *
 *  @field  procId
 *              ID of the processor with which the RingIO region is shared
 *  @field  physAddr
 *              Physical address of the memory region for RingIO
 *  @field  kernAddr
 *              Kernel address of the memory region for RingIO
 *  @field  userAddr
 *              User address of the memory region for RingIO
 *  @field  size
 *              Size of the memory region for RingIO
 *  ============================================================================
 */
typedef struct RingIO_MemInfo_tag {
    ProcessorId procId ;
    Uint32      physAddr ;
    Uint32      kernAddr ;
    Uint32      userAddr ;
    Uint32      size ;
} RingIO_MemInfo ;


/** ============================================================================
 *  @macro  GET_ENTRY_ID
 *
 *  @desc   Gets the entry id of the RingIO
 *  ============================================================================
 */
#define GET_ENTRY_ID(control)  ((control)->entryId)

/** ============================================================================
 *  @macro  GET_LOCK_POOL_ID
 *
 *  @desc   Gets the pool id from which the MPCS lock object structure has been
 *          allocated.
 *  ============================================================================
 */
#define GET_LOCK_POOL_ID(entryId, procId)  POOL_makePoolId (procId, (ringIoState->entryPtr [entryId].lockPoolId))

/** ============================================================================
 *  @macro  GET_CTRL_POOL_ID
 *
 *  @desc   Gets the pool id from which the RingIO control structure has been
 *          allocated.
 *  ============================================================================
 */
#define GET_CTRL_POOL_ID(entryId, procId)   POOL_makePoolId (procId, (ringIoState->entryPtr [entryId].ctrlPoolId))

/** ============================================================================
 *  @macro  GET_DATA_POOL_ID
 *
 *  @desc   Gets the pool id from which the RingIO data buffer has been
 *          allocated.
 *  ============================================================================
 */
#define GET_DATA_POOL_ID(entryId, procId)   POOL_makePoolId (procId, (ringIoState->entryPtr [entryId].dataPoolId))

/** ============================================================================
 *  @macro  GET_ATTR_POOL_ID
 *
 *  @desc   Gets the pool id from which the RingIO attr buffer has been
 *          allocated.
 *  ============================================================================
 */
#define GET_ATTR_POOL_ID(entryId, procId)  POOL_makePoolId (procId, (ringIoState->entryPtr [entryId].attrPoolId))

/** ============================================================================
 *  @macro  IS_WRITER
 *
 *  @desc   Check if client is a writer.
 *  ============================================================================
 */
#define IS_WRITER(client)              ((client)->openMode == RINGIO_MODE_WRITE)

/** ============================================================================
 *  @macro  IS_READER
 *
 *  @desc   Check if client is a reader.
 *  ============================================================================
 */
#define IS_READER(client)              ((client)->openMode == RINGIO_MODE_READ)

/** ============================================================================
 *  @macro  GET_CUR_DATA_END
 *
 *  @desc   Get Current data buffer end.
 *  ============================================================================
 */
#define GET_CUR_DATA_END(control)   ((control)->curBufEnd + 1)

/** ============================================================================
 *  @macro  GET_CUR_ATTR_END
 *
 *  @desc   Get Current attr buffer end.
 *  ============================================================================
 */
#define GET_CUR_ATTR_END(control)   ((control)->curAttrBufEnd + 1)

/** ============================================================================
 *  @macro  ADJUST_END
 *
 *  @desc   Modulo operation
 *  ============================================================================
 */
#define ADJUST_MODULO(start, size, bufSize)   (((start) + (size)) % bufSize)

/** ============================================================================
 *  @macro  SIZEOF_ATTR
 *
 *  @desc   Size of RingIO attributes
 *  ============================================================================
 */
#define SIZEOF_ATTR(size)                  (sizeof (RingIO_Attr) + (size))

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* if !defined (_RINGIODEFS_H) */
