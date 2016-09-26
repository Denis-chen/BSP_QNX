/** ============================================================================
 *  @file   _dsplink.h
 *
 *  @path   $(DSPLINK)/gpp/inc/usr/
 *
 *  @desc   Consolidate Include file to include all internal generic definition
 *          include files.
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (_DSPLINK_H)
#define _DSPLINK_H


#include <dsplink160/_bitops.h>
#include <dsplink160/_dspdefs.h>
#include <dsplink160/_linkdefs.h>
#include <dsplink160/_safe.h>
#include <dsplink160/_intobject.h>
#include <dsplink160/loaderdefs.h>

#include <dsplink160/_loaderdefs.h>

#if defined (POOL_COMPONENT)
#include <dsplink160/_pooldefs.h>
#endif

#if defined (CHNL_COMPONENT)
#include <dsplink160/_datadefs.h>
#endif

#endif /* if !defined (_DSPLINK_H) */
