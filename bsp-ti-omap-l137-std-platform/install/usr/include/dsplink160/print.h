/** ============================================================================
 *  @file   print.h
 *
 *  @path   $(DSPLINK)/gpp/src/osal/
 *
 *  @desc   Interface declaration of OS printf abstraction.
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

#include <sys/slogcodes.h>

/*  ----------------------------------- DSP/BIOS Link               */
#include <dsplink160/dsplink.h>
#include <dsplink160/_dsplink.h>

/** ============================================================================
 *  @func   PRINT_Initialize
 *
 *  @desc   Initializes the PRINT sub-component.
 *
 *  @arg    None
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EFAIL
 *              General error from GPP-OS.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */
EXPORT_API
DSP_STATUS
PRINT_Initialize (Void) ;


/** ============================================================================
 *  @func   PRINT_Finalize
 *
 *  @desc   Releases resources used by this sub-component.
 *
 *  @arg    None
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EFAIL
 *              General error from GPP-OS.
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */
EXPORT_API
DSP_STATUS
PRINT_Finalize (Void) ;


/** ============================================================================
 *  @func   PRINT_Printf
 *
 *  @desc   Provides standard printf functionality abstraction.
 *
 *  @arg    format
 *              Format string.
 *  @arg    ...
 *              Variable argument list.
 *
 *  @ret    None
 *
 *  @enter  None
 *
 *  @leave  None
 *
 *  @see    None
 *  ============================================================================
 */
#if defined (TRACE_KERNEL)
EXPORT_API
Void
PRINT_Printf (Pstr format, ...) ;
#endif

#if defined (TRACE_USER)
/*  ----------------------------------------------------------------------------
 *  Extern declaration for printf to avoid compiler warning.
 *  ----------------------------------------------------------------------------
 */
extern int printf (const char * format, ...) ;

#define PRINT_Printf printf
#endif

extern int Verbosity;

void dsplinklog(int severity, const char *func, int line, int verbose_level,
			 const char *fmt, ...);

#define DSP_INFO( verbose_level, format, args... ) \
            do { if(Verbosity >= verbose_level) dsplinklog( _SLOG_INFO, __PRETTY_FUNCTION__, __LINE__, verbose_level, format, ##args ); } while(0)
#define DSP__WARNING( verbose_level, format, args... ) \
            do { if(Verbosity >= verbose_level) dsplinklog( _SLOG_WARNING, __PRETTY_FUNCTION__, __LINE__, verbose_level, format, ##args ); } while(0)
#define DSP_ERROR( format, args... ) \
            do { dsplinklog( _SLOG_ERROR, __PRETTY_FUNCTION__, __LINE__, 0, format, ##args ); } while(0)

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
