/*
 * $QNXLicenseC:
 * Copyright 2010, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#ifndef __EDMA_L1XX_H__
#define __EDMA_L1XX_H__

#define OMAPL1xx_MAX_CC_NO	2

#define OMAPL1xx_EDMA_QCHMAP_TRWORD_CCNT	(0X7 << 2)
#define OMAPL1xx_CC_NOS				2
#define OMAPL1xx_EDMA0_TCNOS		2
#define OMAPL1xx_EDMA1_TCNOS		1
#define OMAPL1xx_MAX_PARAM_NO		127

	/* for channel controller 0 */
#define OMAPL1xx_EDMA0_IRQ_NO		0xc100

#define OMAPL1xx_EDMA0_NUM_CHAN	32
#define OMAPL1xx_QDMA0_NUM_CHAN	8

#define OMAPL1xx_EDMA0_FIRST_CHAN	0
#define OMAPL1xx_EDMA0_LAST_CHAN	31
#define OMAPL1xx_QDMA0_FIRST_CHAN	0
#define OMAPL1xx_QDMA0_LAST_CHAN	7

#define OMAPL1xx_EDMA0_MAX_CHAN	(OMAPL1xx_EDMA0_NUM_CHAN + OMAPL1xx_QDMA0_NUM_CHAN)

	/* for channel controller 1 */
#define OMAPL1xx_EDMA1_IRQ_NO		0xc120

#define OMAPL1xx_EDMA1_NUM_CHAN		32
#define OMAPL1xx_QDMA1_NUM_CHAN		8

#define OMAPL1xx_EDMA1_FIRST_CHAN	0
#define OMAPL1xx_EDMA1_LAST_CHAN	31
#define OMAPL1xx_QDMA1_FIRST_CHAN	0
#define OMAPL1xx_QDMA1_LAST_CHAN	7

#define OMAPL1xx_EDMA1_MAX_CHAN	(OMAPL1xx_EDMA1_NUM_CHAN + OMAPL1xx_QDMA1_NUM_CHAN)

	/* for total edma */
#define OMAPL1xx_EDMA_NUM_CHAN	(OMAPL1xx_EDMA0_NUM_CHAN + OMAPL1xx_EDMA1_NUM_CHAN)
#define OMAPL1xx_QDMA_NUM_CHAN	(OMAPL1xx_QDMA0_NUM_CHAN + OMAPL1xx_QDMA1_NUM_CHAN)

#define OMAPL1xx_MAX_NUM_CHANNELS	(OMAPL1xx_EDMA_NUM_CHAN + OMAPL1xx_QDMA_NUM_CHAN)

#define OMAPL1xx_DMA_NAME		"DMA Engine For OMAPL1xx"
#define OMAPL1xx_LIB_REVISION	1
#define OMAPL1xx_DMA_NUM_CHAN	OMAPL1xx_MAX_NUM_CHANNELS
#define OMAPL1xx_MAXPRIO		0

#define OMAPL1xx_MEM_NOCROSS		0
#define OMAPL1xx_MEM_LOWER_LIM	0x00700000	/* actually this can be 0, but l137 doesnot have any addressable memory lower than this */
#define OMAPL1xx_MEM_UPPER_LIM	0xffffffff
#define OMAPL1xx_MAX_ACNT		((1 << 16) - 1UL)
#define OMAPL1xx_MAX_BCNT		((1 << 16) - 1UL)
#define OMAPL1xx_MAX_XFER_SIZ	(OMAPL1xx_MAX_BCNT * OMAPL1xx_MAX_ACNT)
#define OMAPL1xx_MAX_SRC_SEG		OMAPL1xx_MAX_BCNT
#define OMAPL1xx_MAX_DST_SEG		OMAPL1xx_MAX_BCNT
#define OMAPL1xx_MAX_SRC_FRAG	5
#define OMAPL1xx_MAX_DST_FRAG	5
#define OMAPL1xx_XFER_UNIT_SIZ	(16 | 8 | 4 | 2 | 1)		/* DBS set in sysconfig */

/****************************************************************************/

#endif
