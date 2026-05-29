/*
***************************************************************************************************
*
*   FileName : udma.c
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

#include <gic.h>
#include "udma.h"
#include "clock.h"
#include "clock_dev.h"
#include "reg_phys.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

// UART DMA Base Address
#define UDMA_CON0_BASE                  (MCU_BSP_UDMA_BASE)
#define UDMA_CON1_BASE                  (MCU_BSP_UDMA_BASE + 0x10000UL)
#define UDMA_CON2_BASE                  (MCU_BSP_UDMA_BASE + 0x20000UL)
#define UDMA_CON3_BASE                  (MCU_BSP_UDMA_BASE + 0x30000UL)
#define UDMA_CON4_BASE                  (MCU_BSP_UDMA_BASE + 0x40000UL)
#define UDMA_CON5_BASE                  (MCU_BSP_UDMA_BASE + 0x50000UL)

#define UDMA_CON_BASE(x)                ((uint32)MCU_BSP_UDMA_BASE + ((x) * 0x10000UL))

// UART DMA Register offsets
#define UDMA_INTSR                      (0x00UL)  // Interrupt Status Register
#define UDMA_ITCSR                      (0x04UL)  // Interrupt Terminal Count Status Register
#define UDMA_ITCCR                      (0x08UL)  // Interrupt Terminal Count Clear Register
#define UDMA_IESR                       (0x0CUL)  // Interrupt Error Status Register
#define UDMA_IECR                       (0x10UL)  // Interrupt Error Clear Register
#define UDMA_RITCSR                     (0x14UL)  // Raw Interrupt Terminal Count Status Register
#define UDMA_REISR                      (0x18UL)  // Raw Error Interrupt Status Register
#define UDMA_ECR                        (0x1CUL)  // Enabled Channel Register
#define UDMA_SBRR                       (0x20UL)  // Software Burst Request Register
#define UDMA_SSRR                       (0x24UL)  // Software Single Request Register
#define UDMA_SLBRR                      (0x28UL)  // Software Last Burst Request Register
#define UDMA_SLSRR                      (0x2CUL)  // Software Last Single Request Register
#define UDMA_CR                         (0x30UL)  // Configuration Register
#define UDMA_SR                         (0x34UL)  // Reserved

#define UDMA_CH_SRC_ADDR(x)             (((uint32)0x100UL + ((x) * 0x20UL)))
#define UDMA_CH_DST_ADDR(x)             (((uint32)0x104UL + ((x) * 0x20UL)))
#define UDMA_CH_LLI(x)                  (((uint32)0x108UL + ((x) * 0x20UL)))
#define UDMA_CH_CON(x)                  (((uint32)0x10CUL + ((x) * 0x20UL)))
#define UDMA_CH_CONFIG(x)               (((uint32)0x110UL + ((x) * 0x20UL)))

#define UDMA_CH0_FLAG                   ((uint32)1UL << 0UL)
#define UDMA_CH1_FLAG                   ((uint32)1UL << 1UL)


typedef struct UDMA_CONTROLLER
{
    uint32                              cController;
    UDMAInformation_t *                 cCh[2];
} UDMAController_t;

static void UDMA_ISR
(
    void *                              pArg
);

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

void UDMA_InterruptEnable
(
    UDMAInformation_t *                 sDmacon
)
{
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        // Enable terminal count interrupt
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CON(sDmacon->iCh);
        uiValue = (uint32)SAL_ReadReg(uiAddr) | ((uint32)1UL << 31UL);

        SAL_WriteReg(uiValue,uiAddr);

        // Mask terminal count interrupt
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CONFIG(sDmacon->iCh);
        uiValue = (uint32)SAL_ReadReg(uiAddr) | ((uint32)1UL << 15UL);
        SAL_WriteReg(uiValue,uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_InterruptDisable
(
    UDMAInformation_t *                 sDmacon
)
{
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        // Disable terminal count interrupt
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CON(sDmacon->iCh);
        uiValue = (uint32)SAL_ReadReg(uiAddr) & ~((uint32)1UL << 31UL);
        SAL_WriteReg(uiValue,uiAddr);

        // Unask terminal count interrupt
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CONFIG(sDmacon->iCh);
        uiValue = (uint32)SAL_ReadReg(uiAddr) & ~((uint32)1UL << 15UL);
        SAL_WriteReg(uiValue,uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_SetAddrIncrement
(
    UDMAInformation_t *                 sDmacon,
    uint8                               ucDestInc,
    uint32                              uiSrcInc
)
{
    uint32 inc;
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CON(sDmacon->iCh);
        uiValue = SAL_ReadReg(uiAddr);
        //clear desc, src increment
        uiValue &= ~(0x3UL << 26UL);

        inc     = (((uint32)ucDestInc & 0x1UL) << 1UL) | (uiSrcInc & 0x1UL);
        uiValue |= (inc << 26UL);

        SAL_WriteReg(uiValue, uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_SetTransferWidth
(
    UDMAInformation_t *                 sDmacon,
    uint8                               ucDestWidth,
    uint32                              uiSrcWidth
)
{
    uint32 transfer_width;
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CON(sDmacon->iCh);
        uiValue = SAL_ReadReg(uiAddr);
        //clear desc, src transfer width
        uiValue         &= ~(0x3FUL << 18UL);
        transfer_width  = (((uint32)ucDestWidth & 0x7UL) << 3UL) | (uiSrcWidth & 0x7UL);
        uiValue         |= (transfer_width << 18UL);

        SAL_WriteReg(uiValue, uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_SetBurstSize
(
    UDMAInformation_t *                 sDmacon,
    uint8                               ucDestBurst,
    uint32                              uiSrcBurst
)
{
    uint32 burst_size;
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CON(sDmacon->iCh);
        uiValue = SAL_ReadReg(uiAddr);
        //clear desc, src burst size
        uiValue     &= ~(0x3FUL << 12UL);
        burst_size  = (((uint32)ucDestBurst & 0x7UL) << 3UL) | (uiSrcBurst & 0x7UL);
        uiValue     |= (burst_size << 12UL);

        SAL_WriteReg(uiValue, uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_SetTransferSize
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiTransferSize
)
{
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CON(sDmacon->iCh);
        uiValue = SAL_ReadReg(uiAddr);
        //clear transfer size
        uiValue &= ~(0xFFFUL);
        uiValue |= (uiTransferSize & 0xFFFUL);
        SAL_WriteReg(uiValue, uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_SetFlowControl
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiFlow
)
{
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CONFIG(sDmacon->iCh);
        uiValue = SAL_ReadReg(uiAddr);
        uiValue &= ~(0x7UL << 11UL);
        uiValue |= ((uiFlow & 0x7UL) << 11UL);
        SAL_WriteReg(uiValue, uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

/*
***************************************************************************************************
*                                          UDMA_Reset
*
* Function to reset UDMA Channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t UDMA_Reset
(
    uint8                               ucCh
)
{
    sint32  ret;
    sint32  iClkBusId;

    ret = 0;
    iClkBusId   = (sint32)CLOCK_IOBUS_UDMA0 + (sint32)ucCh;

    /* SW reset */
    ret = CLOCK_SetSwReset(iClkBusId, TRUE);

    if(ret != (sint32)NULL)
    {
        return SAL_RET_FAILED;
    }

    /* Bit Clear */
    ret = CLOCK_SetSwReset(iClkBusId, FALSE);

    if(ret != (sint32)NULL)
    {
        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

void UDMA_SetPeri
(
    UDMAInformation_t *                 sDmacon,
    uint8                               ucDestPeri,
    uint8                               ucSrcPeri
)
{
    uint32 uiAddr;
    uint32 uiValue;

    uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CONFIG(sDmacon->iCh);
    uiValue = SAL_ReadReg(uiAddr);
    uiValue &= ~(0x7FEUL);
    uiValue |= ((uint32)ucDestPeri & 0xFUL) << 6UL;
    uiValue |= ((uint32)ucSrcPeri  & 0xFUL) << 1UL;
    SAL_WriteReg(uiValue, uiAddr);
}

uint32 UDMA_GetTransferSize
(
    UDMAInformation_t *                 sDmacon
)
{
    uint32 size;
    uint32 uiAddr;
    uint32 uiValue;

    uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CON(sDmacon->iCh);
    uiValue = SAL_ReadReg(uiAddr);
    size    = (uiValue & (0xFFFUL));

    return size;
}

static void UDMA_ISR
(
    void *                              pArg
)
{
    uint32              status;
    uint32              uiAddr;
    uint32              udma_cnt;
    UDMAController_t *  pConTable;
    UDMAInformation_t * dmacon;

    udma_cnt    = 0UL;
    pConTable   = (UDMAController_t *)pArg;
    dmacon      = NULL;

    if (pArg != NULL_PTR)
    {
        uiAddr = UDMA_CON_BASE(pConTable->cController) + UDMA_ITCSR;
        status = SAL_ReadReg(uiAddr);

        if (status != 0UL)
        {
            if ((status & UDMA_CH0_FLAG) != 0UL)
            {
                dmacon = (UDMAInformation_t *)pConTable->cCh[0UL];
                // Clear terminal count interrupt
                uiAddr = UDMA_CON_BASE(dmacon->iCon) + UDMA_ITCCR;
                SAL_WriteReg(UDMA_CH0_FLAG, uiAddr);
            }

            if ((status & UDMA_CH1_FLAG) != 0UL)
            {
                dmacon = (UDMAInformation_t *)pConTable->cCh[1UL];
                if (dmacon == NULL_PTR) //Codesonar : Null Test After Dereference
                {
                    return;
                }

                // Clear terminal count interrupt
                uiAddr = UDMA_CON_BASE(dmacon->iCon) + UDMA_ITCCR;
                SAL_WriteReg(UDMA_CH1_FLAG, uiAddr);

                UDMA_ChannelDisable(dmacon); // channel disabled

                // break loop if the FIFO is not empty after 2ms. time out.
                // 6250 loops for 1ms can be changed depending on Cortex-R5 Single Core clock speed.
                // 6250 loops for 1ms is based on 600MHz which is max clock speed for Cortex-R5
                // clock speed lower than 600MHz is okay because 1ms is guaranteed for 6250 loop to have enough time that isr is complete after FIFO is empty.
                // if it is higher than 600MHz, isr can be complete too fast before FIFO is empty.
                while (udma_cnt <= 12500UL) //// 12500 : 2ms, 6250 : 1ms
                {
                    // check the Active flag
                    // 0 = there is no data in the FIFO of the channel
                    uiAddr = UDMA_CON_BASE(dmacon->iCon) + UDMA_CH_CONFIG(dmacon->iCh);

                    if ((SAL_ReadReg(uiAddr) & (0x20000UL)) == 0UL)
                    {
                        break;
                    }
                    #if 0
                    else if(udma_cnt==12500) // 12500 : 2ms, 6250 : 1ms
                    {
                        break;
                    }
                    #endif

                    udma_cnt+=1UL;
                }

                dmacon->iIsRun = 0UL;

                return;
            }
        }

        if (dmacon == NULL_PTR)
        {
            return;
        }

        UDMA_ChannelDisable(dmacon); // channel disabled

        // break loop if the FIFO is not empty after 2ms. time out.
        // 6250 loops for 1ms can be changed depending on Cortex-R5 Single Core clock speed.
        // 6250 loops for 1ms is based on 600MHz which is max clock speed for Cortex-R5
        // clock speed lower than 600MHz is okay because 1ms is guaranteed for 6250 loop to have enough time that isr is complete after FIFO is empty.
        // if it is higher than 600MHz, isr can be complete too fast before FIFO is empty.
        while (udma_cnt <= 12500UL) // 12500 : 2ms, 6250 : 1ms
        {
            // check the Active flag
            // 0 = there is no data in the FIFO of the channel
            uiAddr = UDMA_CON_BASE(dmacon->iCon) + UDMA_CH_CONFIG(dmacon->iCh);

            if ((SAL_ReadReg(uiAddr) & (0x20000UL)) == 0UL)
            {
                break;
            }
            #if 0 //Msg(4:0771) More than one break statement is being used to terminate an iteration statement. MISRA-C:2004 Rule 14.6
            else if(udma_cnt==12500) // 12500 : 2ms, 6250 : 1ms
            {
                break;
            }
            #endif

            udma_cnt+=1UL;
        }

        UDMA_SetSrcAddr(dmacon, (uint32)dmacon->iSrcAddr);
        UDMA_SetDestAddr(dmacon, (uint32)dmacon->iDestAddr);
        UDMA_InterruptEnable(dmacon);
        UDMA_SetTransferWidth(dmacon, UDMA_TRANSFER_SIZE_BYTE, UDMA_TRANSFER_SIZE_BYTE);
        UDMA_SetTransferSize(dmacon, dmacon->iBufSize);

        // Run DMA
        UDMA_ChannelEnable(dmacon);

        dmacon->iIsRun = 0UL;
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_ChannelEnable
(
    UDMAInformation_t *                 sDmacon
)
{
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CONFIG(sDmacon->iCh);
        uiValue = SAL_ReadReg(uiAddr);
        uiValue &= ~(1UL << 18UL);  // enable DMA requests
        uiValue |= (1UL << 0UL);    // channel enabled

        SAL_WriteReg(uiValue, uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_ChannelDisable
(
    UDMAInformation_t *                 sDmacon
)
{
    uint32 uiAddr;
    uint32 uiValue;

    if (sDmacon != NULL_PTR)
    {
        uiAddr  = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_CONFIG(sDmacon->iCh);
        uiValue = SAL_ReadReg(uiAddr);
        uiValue |= (1UL << 18UL);   // ignore subsequent source DMA requests
        uiValue &= ~(1UL << 0UL);   // channel disabled

        SAL_WriteReg(uiValue, uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_SetSrcAddr
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiAddr
)
{
    uint32 uiRegAddr;

    if (sDmacon != NULL_PTR)
    {
        uiRegAddr = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_SRC_ADDR(sDmacon->iCh);
        SAL_WriteReg(uiAddr, uiRegAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_SetDestAddr
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiAddr
)
{
    uint32 uiRegAddr;

    if (sDmacon != NULL_PTR)
    {
        uiRegAddr = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_DST_ADDR(sDmacon->iCh);
        SAL_WriteReg(uiAddr, uiRegAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

void UDMA_SetlliAddr
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiLLIAddr
)
{
    uint32 uiAddr;

    if (sDmacon != NULL_PTR)
    {
        uiAddr = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_LLI(sDmacon->iCh);
        SAL_WriteReg(uiLLIAddr, uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

uint32 UDMA_GetDestAddr
(
    UDMAInformation_t *                 sDmacon
)
{
    uint32 ret;
    uint32 uiAddr;

    ret = 0UL;

    if (sDmacon != NULL_PTR)
    {
        uiAddr = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CH_DST_ADDR(sDmacon->iCh);
        ret = SAL_ReadReg(uiAddr);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }

    return ret;
}

void UDMA_TransferMem2Mem
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiSrcAddr,
    uint32                              uiDestAddr,
    uint32                              uiLen
)
{
    if (sDmacon != NULL_PTR)
    {
        // DMA Flow control set to memory to memory transfer
        UDMA_SetFlowControl(sDmacon, UDMA_FLOW_TYPE_M2M);
        UDMA_SetAddrIncrement(sDmacon, UDMA_INC, UDMA_INC);
        UDMA_SetBurstSize(sDmacon, UDMA_BURST_SIZE_256, UDMA_BURST_SIZE_256);
        UDMA_SetSrcAddr(sDmacon, uiSrcAddr);
        UDMA_SetDestAddr(sDmacon, uiDestAddr);
        UDMA_InterruptEnable(sDmacon);
        UDMA_SetTransferWidth(sDmacon, UDMA_TRANSFER_SIZE_WORD, UDMA_TRANSFER_SIZE_WORD);
        UDMA_SetTransferSize(sDmacon, uiLen);
        UDMA_ChannelEnable(sDmacon);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}

sint32 UDMA_Init
(
    UDMAInformation_t *                 sDmacon,
    uint32                              uiIntPrio
)
{
    sint32 ret;
    uint32 uiAddr;
    uint32 uiValue;
    sint32 iClkId;
    static UDMAController_t udma_con_table[6] =
    {
        {0UL, {NULL, NULL}},
        {1UL, {NULL, NULL}},
        {2UL, {NULL, NULL}},
        {3UL, {NULL, NULL}},
        {4UL, {NULL, NULL}},
        {5UL, {NULL, NULL}}
    };

    ret     = -1;
    iClkId  = 0;

    if (sDmacon != NULL_PTR)
    {
        iClkId = (sint32)CLOCK_IOBUS_UDMA0 + (sint32)sDmacon->iCon;
        (void)CLOCK_EnableIobus(iClkId, SALEnabled);

        udma_con_table[sDmacon->iCon].cController       = sDmacon->iCon;
        udma_con_table[sDmacon->iCon].cCh[sDmacon->iCh] = sDmacon;
        uiAddr = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CR;

        if ((SAL_ReadReg(uiAddr) & 0x1UL) == 0UL)
        {
            // Disable DMA Contorller
            uiValue = SAL_ReadReg(uiAddr);
            SAL_WriteReg(uiValue & ~(0x1UL), uiAddr);

            // Clear Interrupt
            uiAddr = UDMA_CON_BASE(sDmacon->iCon) + UDMA_ITCCR;
            SAL_WriteReg(0xFFUL, uiAddr);
            uiAddr = UDMA_CON_BASE(sDmacon->iCon) + UDMA_IECR;
            SAL_WriteReg(0xFFUL, uiAddr);

            (void)GIC_IntVectSet((uint32)GIC_UT_DMA0 + sDmacon->iCon, uiIntPrio, GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&UDMA_ISR, (void*)&udma_con_table[sDmacon->iCon]);
            (void)GIC_IntSrcEn((uint32)GIC_UT_DMA0 + sDmacon->iCon);

            // Enable DMA Controller
            uiAddr = UDMA_CON_BASE(sDmacon->iCon) + UDMA_CR;
            SAL_WriteReg(0x1UL, uiAddr);
            ret = (sint32)SAL_RET_SUCCESS;
        }
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }

    return ret;
}

