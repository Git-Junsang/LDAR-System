/*
***************************************************************************************************
*
*   FileName : udma.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*
*   TCC Version 1.0
*
*   This source code contains confidential information of Telechips.
*
*   Any unauthorized use without a written permission of Telechips including not limited to
*   re-distribution in source or binary form is strictly prohibited.
*
*   This source code is provided "AS IS" and nothing contained in this source code shall constitute
*   any express or implied warranty of any kind, including without limitation, any warranty of
*   merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
*   or other third party intellectual property right. No warranty is made, express or implied,
*   regarding the information's accuracy,completeness, or performance.
*
*   In no event shall Telechips be liable for any claim, damages or other liability arising from,
*   out of or in connection with this source code or the use in the source code.
*
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between
*   Telechips and Company.
*   This source code is provided "AS IS" and nothing contained in this source code shall constitute
*   any express or implied warranty of any kind, including without limitation, any warranty
*   (of merchantability, fitness for a particular purpose or non-infringement of any patent,
*   copyright or other third party intellectual property right. No warranty is made, express or
*   implied, regarding the information's accuracy, completeness, or performance.
*   In no event shall Telechips be liable for any claim, damages or other liability arising from,
*   out of or in connection with this source code or the use in the source code.
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
*   between Telechips and Company.
*
***************************************************************************************************
*/

#ifndef UDMA_HEADER
#define UDMA_HEADER

#include <sal_internal.h>

// UDMA address starts from 0xC003E000 and occupies 0x3FF for each of tx and rx.
// UDMA has 4 channels whose range of address is from 0xC003E000 to 0xC003FFFF(8KB).
// Valid address range of SRAM(micom) is only defined from 0xC0000000 to 0xC003FFFF(256KB).
// If it is needed to increase DMA buffer area in SRAM, refer to 4.6 Use fixed DMA buffer
// in TCC805x MICOM RTOS SDK-User Guide
#define UDMA_BUFF_SIZE                  (0x3ffUL) // 1023.
#define UDMA_BUFF_MARGIN                (0x0UL)

#define UDMA_CH0                        (0UL)
#define UDMA_CH1                        (1UL)

#define UDMA_INC                        (1U)
#define UDMA_NO_INC                     (0U)

#define UDMA_TRANSFER_SIZE_BYTE         (0U)
#define UDMA_TRANSFER_SIZE_HALF         (1U)
#define UDMA_TRANSFER_SIZE_WORD         (2U)

#define UDMA_MAX_LLI_NUM                (64UL)

#define UDMA_BURST_SIZE_1               (0U)
#define UDMA_BURST_SIZE_4               (1U)
#define UDMA_BURST_SIZE_8               (2U)
#define UDMA_BURST_SIZE_16              (3U)
#define UDMA_BURST_SIZE_32              (4U)
#define UDMA_BURST_SIZE_64              (5U)
#define UDMA_BURST_SIZE_128             (6U)
#define UDMA_BURST_SIZE_256             (7U)

#define UDMA_FLOW_TYPE_M2M              (0UL)
#define UDMA_FLOW_TYPE_M2P              (1UL)
#define UDMA_FLOW_TYPE_P2M              (2UL)
#define UDMA_FLOW_TYPE_P2P              (3UL)
#define UDMA_FLOW_TYPE_P2P_BY_DP        (4UL)
#define UDMA_FLOW_TYPE_M2P_BY_P         (5UL)
#define UDMA_FLOW_TYPE_P2M_BY_P         (6UL)
#define UDMA_FLOW_TYPE_P2P_BY_SP        (7UL)

// DMA Request Peripheral
enum
{
    UDMA_PERI_RX = 0,
    UDMA_PERI_TX = 1
};

typedef struct UDMA_INFORMATION
{
    uint32                              iCon;
    uint32                              iCh;
    uint8 *                             iSrcAddr;
    uint8 *                             iDestAddr;
    uint32                              iBufSize;
    uint32                              iTransSize;
    uint32                              iIsRun;
} UDMAInformation_t;

/*
***************************************************************************************************
*                                         EXTERN FUNCTIONS
***************************************************************************************************
*/

sint32 UDMA_Init
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiIntPrio
);

void UDMA_ChannelEnable
(
    UDMAInformation_t *                 sDmacon
);

void UDMA_ChannelDisable
(
    UDMAInformation_t *                 sDmacon
);

void UDMA_SetFlowControl
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiFlow
);

void UDMA_SetAddrIncrement
(
    UDMAInformation_t *                 sDmacon,
    uint8                               ucDestInc,
    uint32                              uiSrcInc
);

void UDMA_SetTransferWidth
(
    UDMAInformation_t *                 sDmacon,
    uint8                               ucDestWidth,
    uint32                              uiSrcWidth
);

void UDMA_SetBurstSize
(
    UDMAInformation_t *                 sDmacon,
    uint8                               ucDestBurst,
    uint32                              uiSrcBurst
);

void UDMA_SetTransferSize
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiTransferSize
);

void UDMA_InterruptEnable
(
    UDMAInformation_t *                 sDmacon
);

void UDMA_InterruptDisable
(
    UDMAInformation_t *                 sDmacon
);

SALRetCode_t UDMA_Reset
(
    uint8                               ucCh
);

void UDMA_SetPeri
(
    UDMAInformation_t *                 sDmacon,
    uint8                               ucDestPeri,
    uint8                               ucSrcPeri
);

void UDMA_SetSrcAddr
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiAddr
);

void UDMA_SetDestAddr
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiAddr
);

void UDMA_SetlliAddr
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiLLIAddr
);

uint32 UDMA_GetDestAddr
(
    UDMAInformation_t *                 sDmacon
);

void UDMA_TransferMem2Mem
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiSrcAddr,
    uint32                              uiDestAddr,
    uint32                              uiLen
);

uint32 UDMA_GetTransferSize
(
    UDMAInformation_t *                 sDmacon
);

#endif  /* _TCC_UDMA_H_ */

