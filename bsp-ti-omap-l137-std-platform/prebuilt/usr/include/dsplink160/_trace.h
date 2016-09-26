/** ============================================================================
 *  @file   _trace.h
 *
 *  @path   $(DSPLINK)/gpp/inc/usr/
 *
 *  @desc   Consolidate include file to include all trace header files used
 *          internally.
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (_TRACE_H)
#define _TRACE_H


/*  ----------------------------------- Trace & Debug               */
#include <dsplink160/_signature.h>
#include <dsplink160/dbc.h>


/** ============================================================================
 *  @const  TRC_ENTER/TRC_LEVELn/TRC_LEAVE
 *
 *  @desc   Severity levels for debug printing.
 *  ============================================================================
 */
#define TRC_ENTER           0x01u       /*  Lowest level of severity */
#define TRC_LEVEL1          0x02u
#define TRC_LEVEL2          0x03u
#define TRC_LEVEL3          0x04u
#define TRC_LEVEL4          0x05u
#define TRC_LEVEL5          0x06u
#define TRC_LEVEL6          0x07u
#define TRC_LEVEL7          0x08u      /*  Highest level of severity */
#define TRC_LEAVE           TRC_ENTER



#if defined (TRACE_ENABLE)

#if defined (TRACE_KERNEL)

/*Do Nothing */

#else /* defined (TRACE_KERNEL) */

#define TRC_ENABLE(map)
#define TRC_DISABLE(map)
#define TRC_SET_SEVERITY(level)

#define TRC_0PRINT(a,b)                 \
    PRINT_Printf (b)

#define TRC_1PRINT(a,b,c)               \
    PRINT_Printf ((b), (int)(c))

#define TRC_2PRINT(a,b,c,d)             \
    PRINT_Printf ((b), (int)(c),        \
                       (int)(d))

#define TRC_3PRINT(a,b,c,d,e)           \
    PRINT_Printf ((b),(int)(c),         \
                      (int)(d),         \
                       (int)(e))

#define TRC_4PRINT(a,b,c,d,e,f)         \
    PRINT_Printf ((b), (int) (c),       \
                       (int) (d),       \
                       (int) (e),       \
                       (int) (f))

#define TRC_5PRINT(a,b,c,d,e,f,g)       \
    PRINT_Printf ((b), (int) (c),       \
                       (int) (d),       \
                       (int) (e),       \
                       (int) (f),       \
                       (int) (g))

#define TRC_6PRINT(a,b,c,d,e,f,g,h)     \
    PRINT_Printf ((b), (int) (c),       \
                       (int) (d),       \
                       (int) (e),       \
                       (int) (f),       \
                       (int) (g),       \
                       (int) (h))

#endif  /* defined (TRACE_KERNEL) */

#define TRC_0ENTER(str)                     \
    TRC_0PRINT (TRC_ENTER,                  \
                "Entered " str " ()\n")

#define TRC_1ENTER(str,a)                   \
    TRC_1PRINT (TRC_ENTER,                  \
                "Entered " str " ()\n"    \
                "\t"#a"\t[0x%x]\n",         \
                a)

#define TRC_2ENTER(str,a,b)                 \
    TRC_2PRINT (TRC_ENTER,                  \
                "Entered " str " ()\n"    \
                "\t"#a"\t[0x%x]\n"          \
                "\t"#b"\t[0x%x]\n",         \
                a,b)

#define TRC_3ENTER(str,a,b,c)               \
    TRC_3PRINT (TRC_ENTER,                  \
                "Entered " str " ()\n"    \
                "\t"#a"\t[0x%x]\n"          \
                "\t"#b"\t[0x%x]\n"          \
                "\t"#c"\t[0x%x]\n",         \
                a,b,c)

#define TRC_4ENTER(str,a,b,c,d)             \
    TRC_4PRINT (TRC_ENTER,                  \
                "Entered " str " ()\n"    \
                "\t"#a"\t[0x%x]\n"          \
                "\t"#b"\t[0x%x]\n"          \
                "\t"#c"\t[0x%x]\n"          \
                "\t"#d"\t[0x%x]\n",         \
                a,b,c,d)

#define TRC_5ENTER(str,a,b,c,d,e)           \
    TRC_5PRINT (TRC_ENTER,                  \
                "Entered " str " ()\n"    \
                "\t"#a"\t[0x%x]\n"          \
                "\t"#b"\t[0x%x]\n"          \
                "\t"#c"\t[0x%x]\n"          \
                "\t"#d"\t[0x%x]\n"          \
                "\t"#e"\t[0x%x]\n",         \
                a,b,c,d,e)

#define TRC_6ENTER(str,a,b,c,d,e,f)         \
    TRC_6PRINT (TRC_ENTER,                  \
                "Entered " str " ()\n"    \
                "\t"#a"\t[0x%x]\n"          \
                "\t"#b"\t[0x%x]\n"          \
                "\t"#c"\t[0x%x]\n"          \
                "\t"#d"\t[0x%x]\n"          \
                "\t"#e"\t[0x%x]\n"          \
                "\t"#f"\t[0x%x]\n",         \
                a,b,c,d,e,f)

#define TRC_0LEAVE(str)                     \
    TRC_0PRINT (TRC_LEAVE,                  \
                "Leaving " str " ()\n")

#define TRC_1LEAVE(str,status)                                  \
    TRC_1PRINT (TRC_LEAVE,                                      \
                "Leaving " str " () \t"#status" [0x%x]\n",    \
                status)
#else  /* defined (TRACE_ENABLE) */


#define TRC_ENABLE(map)
#define TRC_DISABLE(map)
#define TRC_SET_SEVERITY(level)

#define TRC_0PRINT(a,b)
#define TRC_1PRINT(a,b,c)
#define TRC_2PRINT(a,b,c,d)
#define TRC_3PRINT(a,b,c,d,e)
#define TRC_4PRINT(a,b,c,d,e,f)
#define TRC_5PRINT(a,b,c,d,e,f,g)
#define TRC_6PRINT(a,b,c,d,e,f,g,h)

#define TRC_0ENTER(str)
#define TRC_1ENTER(str,a)
#define TRC_2ENTER(str,a,b)
#define TRC_3ENTER(str,a,b,c)
#define TRC_4ENTER(str,a,b,c,d)
#define TRC_5ENTER(str,a,b,c,d,e)
#define TRC_6ENTER(str,a,b,c,d,e,f)

#define TRC_0LEAVE(str)
#define TRC_1LEAVE(str,status)

#endif  /* defined (TRACE_ENABLE) */


#if defined (TRACE_USER)
/*  ----------------------------------------------------------------------------
 *  Extern declaration for printf to avoid compiler warning.
 *  ----------------------------------------------------------------------------
 */
extern int printf (const char * format, ...) ;

#define PRINT_Printf printf
#endif


#endif /* !defined (_TRACE_H) */

