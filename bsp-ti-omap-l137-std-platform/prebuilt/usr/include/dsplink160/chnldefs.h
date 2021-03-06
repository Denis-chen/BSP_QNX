/** ============================================================================
 *  @file   chnldefs.h
 *
 *  @path   $(DSPLINK)/gpp/inc/usr/
 *
 *  @desc   Defines data types and structures used by CHNL component.
 *
 *  @ver    1.62
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */


#if !defined (CHNLDEFS_H)
#define CHNLDEFS_H


/*  ----------------------------------- DSP/BIOS Link                   */
#include <dsplink160/dsplink.h>
#include <dsplink160/procdefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/** ============================================================================
 *  @const  MAX_ALLOC_BUFFERS
 *
 *  @desc   Maximum number of buffers that can be allocated
 *          through CHNL_AllocateBuffer.
 *  ============================================================================
 */
#define MAX_ALLOC_BUFFERS      1000u

/** ============================================================================
 *  @const  MAX_CHANNELS
 *
 *  @desc   Maximum number of channels that can be created for each DSP.
 *  ============================================================================
 */
#define MAX_CHANNELS           16u


#if defined (CHNL_COMPONENT)
/** ============================================================================
 *  @macro  IS_VALID_CHNLID
 *
 *  @desc   Is the chnl ID valid.
 *  ============================================================================
 */
#define IS_VALID_CHNLID(dspId, chnlId)                                      \
                                (    IS_VALID_PROCID (dspId)                \
                                 &&  (chnlId < MAX_CHANNELS))
#else /* if defined (CHNL_COMPONENT) */
#define IS_VALID_CHNLID(dspId, chnlId)  FALSE
#endif /* if defined (CHNL_COMPONENT) */


/** ============================================================================
 *  @name   ChannelMode
 *
 *  @desc   Mode of a channel.
 *
 *  @field  ChannelMode_Input
 *              Indicates the channel as an Input channel (from DSP to GPP).
 *  @field  ChannelMode_Output
 *              Indicates the channel as an Output channel (from GPP to DSP).
 *  ============================================================================
 */
typedef enum {
    ChannelMode_Input  = 0x1u,
    ChannelMode_Output = 0x2u
} ChannelMode ;

/** ============================================================================
 *  @name   ChannelDataSize
 *
 *  @desc   Width of data being sent on channel.
 *
 *  @field  ChannelDataSize_16bits
 *              Indicates the data to be transferred through the channel
 *              as 16 bit data.
 *  @field  ChannelDataSize_32bits
 *              Indicates the data to be transferred through the channel
 *              as 32 bit data.
 *  ============================================================================
 */
typedef enum {
    ChannelDataSize_16bits = 1u,
    ChannelDataSize_32bits = 2u
} ChannelDataSize ;


/** ============================================================================
 *  @name   ChannelAttrs
 *
 *  @desc   Channel Attributes.
 *
 *  @field  endianism
 *              Endiannism information currently not used.
 *  @field  mode
 *              Mode of channel (Input or output).
 *  @field  size
 *              Size of data sent on channel (16 bits or 32 bits).
 *  ============================================================================
 */
typedef struct ChannelAttrs_tag {
    Endianism       endianism ;
    ChannelMode     mode      ;
    ChannelDataSize size      ;
} ChannelAttrs ;

/** ============================================================================
 *  @name   ChannelIOInfo
 *
 *  @desc   Information for adding or reclaiming a IO request.
 *
 *  @field  buffer
 *              Buffer pointer.
 *  @field  size
 *              Size of buffer.
 *  @field  arg
 *              Argument to receive or send.
 *  ============================================================================
 */
typedef struct ChannelIOInfo_tag {
    Char8 *   buffer ;
    Uint32    size ;
    Uint32    arg ;
} ChannelIOInfo ;


#if defined (__cplusplus)
}
#endif


#endif /* !defined (CHNLDEFS_H) */
