/*
 * Halliburton ESG
 * 1553 Driver (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */

#ifndef _I1553PROTOCAL_H_INCLUDED
#define _I1553PROTOCAL_H_INCLUDED

#define RX_MODE_CMD_MASK            0x00F0
#define RX_DATA_COUNTER_MASK        0x001F
#define TX_DATA_COUNTER_MASK        0x001F

#define RX_MODE_CMD_NO_DATA         0x00E0
#define RX_MODE_CMD_ONE_WORD_DATA   0x00F0
#define RX_BLIND_CMD_W_DATA1        0x0070          /* blind command only care the bit 7 to bit 5 to be 011, the bit 4 will
                                                       contribute to word counter can be either 1 or 0 so here we have two
                                                       define flag for blind command */
#define RX_BLIND_CMD_W_DATA2        0x0060
