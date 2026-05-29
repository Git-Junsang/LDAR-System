/*
***************************************************************************************************
*
*   FileName : gpsb.c
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

#include <gpsb_reg.h>

/*
***************************************************************************************************
*                                         DEFINITIONS
***************************************************************************************************
*/


/*************************************************************************************************/
/*                                      LOCAL VARIABLES                                          */
/*************************************************************************************************/
static uint32                           reg_offset      = 0;
static uint32                           dma_tx_dummy[8] = {0};
static uint32                           dma_rx_dummy[8] = {0};

/*
***************************************************************************************************
*                                      FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void GPSB_DmaStart
(
    uint32                              uiCh
);

static void GPSB_DmaStop
(
    uint32                              uiCh
);

static SALRetCode_t GPSB_DmaSetAddr
(
    uint32                              uiCh,
    const uint32                        uiTx,
    uint32                              uiRx
);

static SALRetCode_t GPSB_SetPort
(
    uint32                              uiCh,
    uint32                              uiSdo,
    uint32                              uiSdi,
    uint32                              uiSclk
);

static void GPSB_SetXferState
(
    uint32                              uiCh,
    uint32                              uiState
);

static uint32 GPSB_FifoWrite
(
    uint32                              uiCh,
    const uint32 *                      puiTxBuf,
    uint32                              uiTxBufSize
);

static uint32 GPSB_FifoRead
(
    uint32                              uiCh,
    uint32 *                            puiRxBuf,
    uint32                              uiRxBufSize
);

static uint32 GPSB_GetBpw
(
    uint32                              uiCh
);

static SALRetCode_t GPSB_SetCs
(
    int32                               iCs,
    uint32                              uiEnable,
    uint32                              uiCsHigh
);

static SALRetCode_t GPSB_WaitRxFifoValid
(
    uint32                              uiCh
);

static SALRetCode_t GPSB_FifoSyncXfer8
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
);

static SALRetCode_t GPSB_FifoSyncXfer16
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
);

static SALRetCode_t GPSB_FifoSyncXfer32
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
);

static SALRetCode_t GPSB_FiFoAsyncXfer
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
);

static SALRetCode_t GPSB_DmaAsyncXfer
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
);

static void GPSB_HwInit
(
    uint32                              uiCh
);

static void GPSB_Isr
(
    uint32                              uiCh
);

static void GPSB_Isr0
(
    void *                              pArg
);

static void GPSB_Isr1
(
    void *                              pArg
);

static void GPSB_Isr2
(
    void *                              pArg
);

static void GPSB_Isr3
(
    void *                              pArg
);

static void GPSB_DmaIsr0
(
    void *                              pArg
);

static void GPSB_DmaIsr1
(
    void *                              pArg
);

static void GPSB_DmaIsr2
(
    void *                              pArg
);

static void GPSB_DmaIsr3
(
    void *                              pArg
);

static uint32 GPSB_IsMaster
(
    uint32                              uiCh
);

static void GPSB_Enable
(
    uint32                              uiCh
);

static void GPSB_Disable
(
    uint32                              uiCh
);

static void GPSB_EnableIntr
(
    uint32                              uiCh
);

static void GPSB_DisableIntr
(
    uint32                              uiCh
);

static void delay1us
(
    uint32                              uiUs
);

#ifdef GPSB_DEBUG
static void GPSB_RegDump
(
    uint32                              uiCh
);
#endif

/*
***************************************************************************************************
*                                          delay1us
*
* Function to set delay with 'nop'.
*
* @param    uiUs
* @return
* Notes
*
***************************************************************************************************
*/

static void delay1us
(
    uint32                              uiUs
)
{
    uint32  i;
    uint32  sec;

    i   = 0xffUL;
    sec = 0;
    sec = uiUs * (uint32)500UL;

    for (i = 0 ; i < sec ; i++)
    {
        BSP_NOP_DELAY();
    }

    return;
}

/*
***************************************************************************************************
*                                          GPSB_RegDump
*
* Function to dump all of GPSB register about selected channel.
*
* @param    uiCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

#ifdef GPSB_DEBUG
void GPSB_RegDump
(
    uint32                              uiCh
)
{
    uint32  reg_offset;

    reg_offset = 0;

    if(uiCh > 3UL)
    {
        reg_offset = 1UL; //PORT_SEL1
    }

    GPSB_D("##### GPSB REG DUMP CH %d (@0x%X)#####\n", uiCh, gpsb[uiCh].dBase);
    GPSB_D("STAT    0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_STAT));
    GPSB_D("INTEN   0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_INTEN));
    GPSB_D("MODE    0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_MODE));
    GPSB_D("CTRL    0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_CTRL));
    GPSB_D("TXBASE  0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_TXBASE));
    GPSB_D("RXBASE  0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_RXBASE));
    GPSB_D("PACKET  0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_PACKET));
    GPSB_D("DMACTR  0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_DMACTR));
    GPSB_D("DMASTAT 0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_DMASTR));
    GPSB_D("DMAICR  0x%08X\n", SAL_ReadReg(gpsb[uiCh].dBase + GPSB_DMAICR));
    GPSB_D("PCFG    0x%08X\n", SAL_ReadReg((uint32)GPSB_PORT_CFG(reg_offset)));
}
#endif

/*
***************************************************************************************************
*                                          GPSB_BitClear
*
* Function to clear bit in GPSB register.
*
* @param    addr [in]       : Value of Register Address
*           mask [in]       : Value of bit mask
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_BitClear
(
    uint32                              uiAddr,
    uint32                              uiMask
)
{
    uint32  val;

    val = (uint32)SAL_ReadReg(uiAddr);
    BSP_BitClr32(val, uiMask);
    SAL_WriteReg(val, uiAddr);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_BitSet
*
* Function to set bit in GPSB register.
*
* @param    addr [in]       : Value of Register Address
*           mask [in]       : Value of bit mask
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_BitSet
(
    uint32                              uiAddr,
    uint32                              uiMask
)
{
    uint32  val;

    val = (uint32)SAL_ReadReg(uiAddr);
    BSP_BitClr32(val, uiMask);
    BSP_BitSet32(val, uiMask);
    SAL_WriteReg(val, uiAddr);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_BitCSet
*
* Function to clear & set bit in GPSB register.
*
* @param    addr [in]       : Value of Register Address
*           cmask [in]      : Value of clear bit mask
*           smask [in]      : Value of set bit mask
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_BitCSet
(
    uint32                              uiAddr,
    uint32                              uiCmask,
    uint32                              uiSmask
)
{
    uint32  val;

    val = (uint32)SAL_ReadReg(uiAddr);
    BSP_BitClr32(val, uiCmask);
    BSP_BitSet32(val, uiSmask);
    SAL_WriteReg(val, uiAddr);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_BitSClear
*
* Function to set & clear bit in GPSB register.
*
* @param    addr [in]       : Value of Register Address
*           smask [in]      : Value of set bit mask
*           cmask [in]      : Value of clear bit mask*
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_BitSClear
(
    uint32                              uiAddr,
    uint32                              uiSmask,
    uint32                              uiCmask
)
{
    uint32  val;

    val = (uint32)SAL_ReadReg(uiAddr);
    BSP_BitSet32(val, uiSmask);
    BSP_BitClr32(val, uiCmask);
    SAL_WriteReg(val, uiAddr);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_IsMaster
*
* Function to check gpsb chanel is master.
*
* @param    uiCh [in]       : Value of GPSB Channel.
* @return   TRUE or FALSE
* Notes
*
***************************************************************************************************
*/

static uint32 GPSB_IsMaster
(
    uint32                              uiCh
)
{
    if(gpsb[uiCh].dIsSlave == (uint32)FALSE)
    {
        return TRUE;
    }

    return FALSE;
}

/*
***************************************************************************************************
*                                          GPSB_Enable
*
* Function to check gpsb chanel is master.
*
* @param    uiCh [in]       : Value of GPSB Channel.
* @return   TRUE or FALSE
* Notes
*
***************************************************************************************************
*/

static void GPSB_Enable
(
    uint32                              uiCh
)
{
    /* Enable GPSB operation */
    GPSB_BitSet(gpsb[(uiCh)].dBase + GPSB_MODE, BSP_BIT_03);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_Disable
*
* Function to disable gpsb chanel.
*
* @param    uiCh [in]       : Value of GPSB Channel.
* @return   TRUE or FALSE
* Notes
*
***************************************************************************************************
*/

void GPSB_Disable
(
    uint32                              uiCh
)
{
    /* Disable GPSB operation */
    GPSB_BitClear(gpsb[(uiCh)].dBase + GPSB_MODE, BSP_BIT_03);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_EnableIntr
*
* Function to enable interrupt for gpsb chanel.
*
* @param    uiCh [in]       : Value of GPSB Channel.
* @return   TRUE or FALSE
* Notes
*
***************************************************************************************************
*/

void GPSB_EnableIntr
(
    uint32                              uiCh
)
{
    /* Enable GPSB Interrupt */
    GPSB_BitSet(gpsb[(uiCh)].dBase + GPSB_INTEN, BSP_BIT_00);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_DisableIntr
*
* Function to disable interrupt for gpsb chanel.
*
* @param    uiCh [in]       : Value of GPSB Channel.
* @return   TRUE or FALSE
* Notes
*
***************************************************************************************************
*/

void GPSB_DisableIntr
(
    uint32                              uiCh
)
{
    /* Disable GPSB Interrupt */
    GPSB_BitClear(gpsb[uiCh].dBase + GPSB_INTEN, BSP_BIT_00);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_DmaStart
*
* Function to start dma.
*
* @param    uiCh [in]       : Value of channel to control
* @return
* Notes
*           Now we support dma operation for channel 0 to 4 only
*           Start dma operation
*
***************************************************************************************************
*/

static void GPSB_DmaStart
(
    uint32                              uiCh
)
{
    /* Setup GPSB DMA */
    if(uiCh < DEDICATED_DMA_NUM)
    {
        /* Set GPSB DMA Address mode - Multiple address mode */
        //GPSB_BitClear(gpsb[uiCh].dBase + GPSB_DMACTR, (0x3 << 14) | (0x3 << 16));

        /* Enable DMA request for TX/RX FIFO */
        GPSB_BitSet(gpsb[uiCh].dBase + GPSB_INTEN, BSP_BIT_31 | BSP_BIT_30);

        /* Enable DMA Tx and Rx request */
        GPSB_BitSet(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_31 | BSP_BIT_30);

        /* Enbale DMA done interrupt and Disable DMA packet interrupt */
        GPSB_BitSClear(gpsb[uiCh].dBase + GPSB_DMAICR, BSP_BIT_17, BSP_BIT_16);

        /* Set DMA Rx interrupt */
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_DMAICR, BSP_BIT_20);

        /* Enable GPSB DMA operation */
        GPSB_BitSet(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_00);
    }

    return;
}

/*
***************************************************************************************************
*                                          GPSB_DmaStop
*
* Function to Stop dma.
*
* @param    uiCh [in]       : Value of channel to control
* @return
* Notes
*           Stop dma operation and clear interrupt status
*
***************************************************************************************************
*/

static void GPSB_DmaStop
(
    uint32                              uiCh
)
{
    if(uiCh < DEDICATED_DMA_NUM)
    {
        /* Disable DMA request for TX/RX FIFO */
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_INTEN, BSP_BIT_31 | BSP_BIT_30);

        /* Disable DMA Tx and Rx request */
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_31 | BSP_BIT_30);

        /* Clear DMA done and packet interrupt status */
        GPSB_BitSet(gpsb[uiCh].dBase + GPSB_DMAICR, BSP_BIT_29 | BSP_BIT_28);

        /* Disable GPSB DMA operation */
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_00);
    }
    else
    {
        /* Nothing to do */
    }

    return;
}

/*
***************************************************************************************************
*                                          GPSB_DmaSetAddr
*
* Function to set dma buffer address.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiTx [in]       : TX dma buffer address
* @param    uiRx [in]       : RX dma buffer address
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_DmaSetAddr
(
    uint32                              uiCh,
    const uint32                        uiTx,
    uint32                              uiRx
)
{
    volatile uint32 tx;
    volatile uint32 rx;
    uint32 *        dummy;

    tx      = 0;
    rx      = 0;
    dummy   = (uint32 *)0;

    if(uiCh > DEDICATED_DMA_NUM)
    {
        GPSB_D("GPSB] ch %x is not support dma operation\n", uiCh);

        return SAL_RET_FAILED;
    }

    /* Set TXBASE and RXBASE */
    if(uiTx != (uint32)NULL)
    {
        /* Set multiple packet address mode */
        tx = GPSB_GET_PHY_ADDR((const uint32 *)uiTx);
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_17 | BSP_BIT_16);
    }
    else
    {
        /* Set fixed address mode */
        dummy   = dma_tx_dummy;
        tx      = GPSB_GET_PHY_ADDR(*dummy);
        GPSB_BitCSet(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_17 | BSP_BIT_16, BSP_BIT_16);
    }

    SAL_WriteReg(tx, (uint32)(gpsb[uiCh].dBase + GPSB_TXBASE));

    if(uiRx != (uint32)NULL)
    {
        /* Set multiple packet address mode */
        rx = GPSB_GET_PHY_ADDR((uint32 *)uiRx);
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_15 | BSP_BIT_14);
    }
    else
    {
        /* Set fixed address mode */
        dummy   = dma_rx_dummy;
        rx      = GPSB_GET_PHY_ADDR(*dummy);
        GPSB_BitCSet(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_15 | BSP_BIT_14, BSP_BIT_14);
    }

    SAL_WriteReg(rx, (uint32)(gpsb[uiCh].dBase + GPSB_RXBASE));

    /* Set Tx and Rx FIFO threshold for interrupt/DMA request */
    GPSB_BitCSet((gpsb[uiCh].dBase + (uint32)GPSB_INTEN), ((uint32)0x7UL << 16UL),
                   (((uint32)GPSB_CFGRTH & (uint32)0x7UL) << 16UL));
    GPSB_BitCSet((gpsb[uiCh].dBase + (uint32)GPSB_INTEN), ((uint32)0x7UL << 20UL),
                   (((uint32)GPSB_CFGWTH & (uint32)0x7UL) << 20UL));

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_Reset
*
* Function to reset gpsb channel.
*
* @param    uiCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*           Reset GPSB block
*
***************************************************************************************************
*/

SALRetCode_t GPSB_Reset
(
    uint32                              uiCh
)
{
    sint32  ret;

    ret = 0;

    /* SW reset */
    ret = CLOCK_SetSwReset((sint32)(gpsb[uiCh].dIobusName), 1);

    if(ret != 0)
    {
        return SAL_RET_FAILED;
    }

    ret = CLOCK_SetSwReset((sint32)(gpsb[uiCh].dIobusName), 0);

    if(ret != 0)
    {
        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_SetPort
*
* Function to set hw port info.
*
* @param    uiCh [in]       : Value of channel to control
* @param    iSdo [in]       : SDO GPIO index (= MOSI)
* @param    iSdi [in]       : SDI GPIO index (= MISO)
* @param    iSclk [in]      : SCLK GPIO index
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_SetPort
(
    uint32                              uiCh,
    uint32                              uiSdo,
    uint32                              uiSdi,
    uint32                              uiSclk
)
{

    SALRetCode_t    ret;
    uint32          i;
    uint32          match_port;
    uint32          port;
    uint32          cbit;
    uint32          sbit;

    ret         = SAL_RET_SUCCESS;
    i           = 0xffUL;
    match_port  = 0;
    port        = 0;
    cbit        = 0UL;
    sbit        = 0UL;

    if(uiCh > (uint32)GPSB_CH_PORT_NUM)
    {
        reg_offset = 1; //PORT_SEL1
    }

    for (i = 0 ; i < GPSB_PORT_NUM ; i++)
    {
        if (uiSclk == gpsbport[i][GPSB_SCLK])
        {
            if (uiSdo == gpsbport[i][GPSB_SDO])
            {
                if (uiSdi == gpsbport[i][GPSB_SDI])
                {
                    match_port  = 1;
                    port        = i;
                }
            }
        }
    }

    if (match_port == 1UL)
    {
        gpsb[uiCh].dPort = port;
        GPSB_D("%s: GPSB port 0x%x\n", __func__, gpsb[uiCh].dPort);
    }
    else
    {
        GPSB_D("%s: GPSB gpsb port is wrong\n", __func__);

        return SAL_RET_FAILED;
    }

    for (i = 0 ; i < GPSB_CH_NUM ; i++)
    {
        if ((i != uiCh) && (gpsb[i].dPort == port) && (gpsb[i].dPort != (uint32)NULL))
        {
            GPSB_D("%s: 0x%x port is already used by ch 0x%x\n", __func__, port, i);
            return SAL_RET_FAILED;
        }
    }

    ret = (SALRetCode_t)GPIO_Config((uint32)uiSdo, (uint32)(GPIO_FUNC((uint32)6UL) | GPIO_OUTPUT));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    ret = (SALRetCode_t)GPIO_Config((uint32)uiSdi, (uint32)((GPIO_FUNC((uint32)6UL) | GPIO_INPUT) | GPIO_INPUTBUF_EN));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    ret = (SALRetCode_t)GPIO_Config((uint32)uiSclk, (uint32)(GPIO_FUNC((uint32)6UL) | GPIO_OUTPUT));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    cbit = (uint32)GPSB_PORT_SEL_MASK << ((uiCh % 4UL) * 8UL);
    sbit = gpsb[uiCh].dPort << ((uiCh % 4UL) * 8UL);
    GPSB_BitCSet((uint32)GPSB_PORT_CFG(reg_offset), cbit, sbit);
    GPSB_D("%s: PCFG 0x%08x\n", __func__, SAL_ReadReg((uint32)GPSB_PORT_CFG(reg_offset)));

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_SetXferState
*
* Function to change xfer state.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiState [in]    : Xfer state (GPSB_XFER_STATE_DISABLED/IDLE/RUNNING)
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_SetXferState
(
    uint32                              uiCh,
    uint32                              uiState
)
{
    switch(uiState)
    {
        case GPSB_XFER_STATE_DISABLED:
        case GPSB_XFER_STATE_IDLE:
        case GPSB_XFER_STATE_RUNNING:
        {
            //(void)SAL_CoreCriticalEnter();
            GPSB_D("%s: ch %d xfer state is set to 0x%x\n", __func__, uiCh, uiState);
            gpsb[uiCh].dState = (uint8)uiState;
            //(void)SAL_CoreCriticalExit();
            break;
        }

        default:
        {
            GPSB_E("%s: state is not supported!\n", __func__);
            break;
        }
    }

    return;
}

/*
***************************************************************************************************
*                                          GPSB_FifoWrite
*
* Function to write data to PORT regitster with interrupt xfer mode.
*
* @param    uiCh [in]       : Value of channel to control
* @param    puiTxBuf [in]   : TX data buff
* @param    uiTxBufSize [in]: TX data size
* @return   write data size
* Notes
*
***************************************************************************************************
*/

static uint32 GPSB_FifoWrite
(
    uint32                              uiCh,
    const uint32 *                      puiTxBuf,
    uint32                              uiTxBufSize
)
{
    uint32          i;
    uint32          word_size;
    const uint32 *  tx;
    uint32          cnt;
    uint32          write_data;
    uint32          ret;

    i           = 0xff;
    word_size   = 0;
    tx          = NULL;
    cnt         = 0;
    write_data  = 0;
    ret         = 0;
    word_size   = (uint32)((SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_MODE)) >> 8UL) & 0x1FUL) + 1UL;
    word_size   = word_size / 8UL;

    /* Get empty fifo count */
    cnt = (SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_STAT)) >> 24UL) & 0xFUL;

    if(word_size > 2UL)
    {
        /*
         * With BPW=32 and higher speed, ROR error occurs.
         * In order to prevent ROR error, we limit FIFO size (8->4).
         * We confirmed operation at speed up to 25 MHz.
         * We recommend DMA transfer at high speed.
         */
        cnt = 4UL - cnt;
    }
    else
    {
        cnt = 8UL - cnt;
    }

    tx = puiTxBuf;

    for(i = 0 ; i < cnt ; i++)
    {
        write_data  = 0;

        if(tx != (uint32 *)NULL_PTR)
        {
            if(word_size <= 1UL)
            {
                write_data = tx[0];
            }
            else if(word_size <= 2UL)
            {
                write_data = (((uint32)tx[1] << 8UL) | (uint32)tx[0]);
            }
            else
            {
                write_data = (((uint32)tx[3] << 24UL) | ((uint32)tx[2] << 16UL)
                               |((uint32)tx[1] << 8UL) | ((uint32)tx[0] << 0));
            }

            tx += word_size;
        }

        SAL_WriteReg(write_data, (uint32)(gpsb[uiCh].dBase + GPSB_PORT));
        ret += word_size;

        if(ret >= uiTxBufSize)
        {
            break;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_FifoRead
*
* Function to read data to PORT regitster with interrupt xfer mode.
*
* @param    uiCh [in]       : Value of channel to control
* @param    puiRxBuf [out]  : RX data buff
* @param    uiRxBufSize [in]: RX data size
* @return   read data size
* Notes
*
***************************************************************************************************
*/

static uint32 GPSB_FifoRead
(
    uint32                              uiCh,
    uint32 *                            puiRxBuf,
    uint32                              uiRxBufSize
)
{
    uint32      i;
    uint32      word_size;
    uint32 *    rx;
    uint32      cnt;
    uint32      read_data;
    uint32      ret;

    i           = 0xff;
    word_size   = 0;
    rx          = NULL_PTR;
    cnt         = 0;
    read_data   = 0;
    ret         = 0;

    /* Get empty fifo count */
    cnt         = (SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_STAT)) >> 16UL) & 0xFUL;
    word_size   = (((SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_MODE)) >> 8UL) & 0x1FUL) + 1UL);
    word_size   = word_size / (uint32)8UL;
    rx          = puiRxBuf;

    for(i = 0 ; i < cnt ; i++)
    {
        read_data = SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_PORT));

        if(rx != (uint32 *)NULL_PTR)
        {
            if(word_size <= (uint32)1UL)
            {
                rx[0] = (uint32)(read_data & 0xFFUL);
            }
            else if(word_size <= (uint32)2UL)
            {
                rx[0] = (uint32)(read_data & 0xFFUL);
                rx[1] = (uint32)((read_data >> 8UL) & 0xFFUL);
            }
            else
            {
                rx[0] = (uint32)(read_data & 0xFFUL);
                rx[1] = (uint32)((read_data >> 8UL) & 0xFFUL);
                rx[2] = (uint32)((read_data >> 16UL) & 0xFFUL);
                rx[3] = (uint32)((read_data >> 24UL) & 0xFFUL);
            }

            rx += word_size;
        }

        ret += word_size;

        if(ret >= uiRxBufSize)
        {
            break;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SetSpeed
*
* Function to set GPSB operattion clock speed. (= SCLK)
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiSpeedInHz [in]: SCLK speed
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SetSpeed
(
    uint32                              uiCh,
    uint32                              uiSpeedInHz
)
{
    uint32  pclk;
    uint32  divldv;
    sint32  ret;

    pclk    = 0;
    divldv  = 0;
    ret     = 0;
    divldv  = ((SAL_ReadReg((uint32)(gpsb[uiCh].dBase + (uint32)GPSB_MODE)) >> 24UL) & 0xFFUL);
    pclk    = uiSpeedInHz * (divldv + 1UL) * 2UL;
    GPSB_D("%s: speed: %d / divldv: %d / pclk: %d\n", __func__, uiSpeedInHz, divldv, pclk);
    ret     = CLOCK_SetPeriRate((sint32)gpsb[uiCh].dPeriName, pclk);

    if(ret != 0)
    {
        GPSB_D("%s: ch %d failed to set peri. clock\n", __func__, uiCh);

        return SAL_RET_FAILED;
    }

    (void)CLOCK_EnablePeri((sint32)(gpsb[uiCh].dPeriName));

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_SetBpw
*
* Function to set transfer bpw value.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiBpw [in]      : Bit per word
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SetBpw
(
    uint32                              uiCh,
    uint32                              uiBpw
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;

    if(((uiBpw != (uint32)8UL) && (uiBpw != (uint32)16UL)) && (uiBpw != (uint32)32UL))
    {
        GPSB_D("%s: ch %d bpw is not supported\n", __func__, uiBpw);
        return SAL_RET_FAILED;
    }

    GPSB_BitCSet((gpsb[uiCh].dBase + (uint32)GPSB_MODE) , (0x1FUL << 8UL), (((uiBpw - 1UL) & 0x1FUL) << 8UL));

    /*
     * According to bits per word, Tx/Rx half-word and byte swap should be set
     */

    if(uiBpw == 8UL)
    {
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_INTEN, (BSP_BIT_24 | BSP_BIT_25 | BSP_BIT_26 | BSP_BIT_27));
    }
    else if(uiBpw == 16UL)
    {
        GPSB_BitCSet(gpsb[uiCh].dBase + GPSB_INTEN, (BSP_BIT_24 | BSP_BIT_25 | BSP_BIT_26 | BSP_BIT_27), (BSP_BIT_24 | BSP_BIT_26));
    }
    else
    {
        GPSB_BitSet(gpsb[uiCh].dBase + GPSB_INTEN, (BSP_BIT_24 | BSP_BIT_25 | BSP_BIT_26 | BSP_BIT_27));
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SetBpwNoSwap
*
* Function to set transfer bpw value.
* @param    uiCh [in]       : Value of channel to control
* @param    ucBpw [in]      : Bit per word
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*           If you don't have a specific reason, please use tcc_GPSB_master_set_bpw().
*           In some cases, need to send/receive the data without half-word and byte swapping.
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SetBpwNoSwap
(
    uint32                               uiCh,
    uint32                               uiBpw
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;

    if(((uiBpw != 8UL) && (uiBpw != 16UL)) && (uiBpw != 32UL))
    {
        GPSB_D("%s ch %d bpw is not supported\n", __func__, uiBpw);
        return SAL_RET_FAILED;
    }

    GPSB_BitCSet((gpsb[uiCh].dBase + (uint32)GPSB_MODE), 0x1FUL << 8UL, (uint32)(((uiBpw - 1UL) & 0x1FUL)) << 8UL);
    GPSB_BitClear((gpsb[uiCh].dBase + (uint32)GPSB_INTEN), (BSP_BIT_24 | BSP_BIT_25 | BSP_BIT_26 | BSP_BIT_27));

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_GetBpw
*
* Function to set transfer bpw value.
*
* @param    uiCh [in]       : Value of channel to control
* @return   bpw value from register read value.
* Notes
*
***************************************************************************************************
*/

static uint32 GPSB_GetBpw
(
    uint32                              uiCh
)
{
    uint32  bpw;

    bpw = 0;
    bpw = ((SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_MODE)) >> 8UL) & 0x1FUL);

    return (bpw + 1UL);
}

/*
***************************************************************************************************
*                                          GPSB_SetMode
*
* Function to set gpsb operate mode.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiMode [in]     : Mode
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_SetMode
(
    uint32                              uiCh,
    uint32                              uiMode
)
{
    if((uiMode & GPSB_CPOL) != (uint32)NULL)
    {
        GPSB_BitSet((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_16);
    }
    else
    {
        GPSB_BitClear((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_16);
    }

    if((uiMode & GPSB_CPHA) != (uint32)NULL)
    {
        GPSB_BitSet((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_18 | BSP_BIT_17);
    }
    else
    {
        GPSB_BitClear((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_18 | BSP_BIT_17);
    }

    if((uiMode & GPSB_CS_HIGH) != (uint32)NULL)
    {
        GPSB_BitSet((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_20 | BSP_BIT_19);
    }
    else
    {
        GPSB_BitClear((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_20 | BSP_BIT_19);
    }

    if((uiMode & GPSB_LSB_FIRST) != (uint32)NULL)
    {
        GPSB_BitSet((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_07);
    }
    else
    {
        GPSB_BitClear((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_07);
    }

    if((uiMode & GPSB_LOOP) != (uint32)NULL)
    {
        GPSB_BitSet((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_06);
    }
    else
    {
        GPSB_BitClear((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_06);
    }

    if((uiMode & GPSB_SLAVE) != (uint32)NULL)
    {
        GPSB_BitSet((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_02);
    }
    else
    {
        GPSB_BitClear((uint32)(gpsb[uiCh].dBase + GPSB_MODE), BSP_BIT_02);
    }

    GPSB_D("%s: ch %d GPSB MODE 0x%x\n", __func__, uiCh, SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_MODE)));

    return;
}

/*
***************************************************************************************************
*                                          GPSB_CsAlloc
*
* Function to config CS GPIO.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiCs [in]       : CS GPIO index
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_CsAlloc
(
    uint32                              uiCh,
    uint32                              uiCs
)
{
    SALRetCode_t    ret;
    sint32          gfb_cs;

    ret     = SAL_RET_SUCCESS;
    gfb_cs  = 0;

    /* Configure CS_GPIO as gpio function */
    if(uiCs > 0UL)
    {
        gfb_cs = (sint32)GPIO_ToNum(uiCs);

        if(gfb_cs == (sint32)SAL_RET_FAILED)
        {
            return SAL_RET_FAILED;
        }

        /* Configure CS_GPIO as GFB function and GPSB controls CS */
        ret = GPIO_Config((uint32)uiCs, (uint32)(GPIO_FUNC((uint32)6UL) | GPIO_OUTPUT));

        if(ret != SAL_RET_SUCCESS)
        {
            GPSB_E("%s : GPIO config fail. GPSB channel %d is closed. \n", __func__, uiCh);
            (void)GPSB_Close(uiCh);

            return SAL_RET_FAILED;
        }
    }
    else
    {
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_SetCs
*
* Function to set CS value to high or low.
*
* @param    iCs [in]        : CS GPIO index
* @param    uiEnable [in]   : Active or Deactive
* @param    uiCsHigh [in]   : Default high or low (active high or active low)
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_SetCs
(
    int32                               iCs,
    uint32                              uiEnable,
    uint32                              uiCsHigh
)
{
    SALRetCode_t    ret;
    uint32          enable;

    ret     = SAL_RET_SUCCESS;
    enable  = uiEnable;

    if(iCs < 0)
    {
        return SAL_RET_FAILED;
    }

    if(uiCsHigh != FALSE)
    {
        (void)GPIO_Set((uint32)iCs, enable);
    }
    else
    {
        if(uiEnable > 0UL)
        {
            enable = FALSE;
        }
        else
        {
            enable = (uint32)TRUE;
        }

        (void)GPIO_Set((uint32)iCs, enable);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_CsInit
*
* Function to initialize CS GPIO to default config.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiCs [in]       : CS GPIO index
* @param    uiCsHigh [in]   : default high or low
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_CsInit
(
    uint32                              uiCh,
    uint32                              uiCs,
    uint32                              uiCsHigh
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;

    /* Configure CS_GPIO as gpio function */
    ret = GPIO_Config((uint32)uiCs, (uint32)(GPIO_FUNC((uint32)0UL) | GPIO_OUTPUT));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    /* Initial state is deactivated state */
    ret = GPSB_SetCs((int32)uiCs, (uint32)FALSE, uiCsHigh);

    if(ret != SAL_RET_SUCCESS)
    {
        GPSB_D("%s: ch %d deactivating cs_gpios(%d) is failed\n", __func__, uiCh, uiCs);
        return ret;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_CsActivate
* Function to set active CS GPIO.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiCs [in]        : CS GPIO index
* @param    uiCsHigh        : Default High or Low
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_CsActivate
(
    uint32                              uiCh,
    uint32                              uiCs,
    uint32                              uiCsHigh
)
{
    SALRetCode_t    ret;

    ret = GPSB_SetCs((int32)uiCs, (uint32)TRUE, uiCsHigh);

    if(ret != SAL_RET_SUCCESS)
    {
        GPSB_E("%s Fail. GPSB channel %d is closed.\n", __func__, uiCh);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_CsDeactivate
*
* Function to set active CS GPIO.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiCs [in]       : CS GPIO index
* @param    uiCsHigh [in]   : Default High or Low
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_CsDeactivate
(
    uint32                              uiCh,
    uint32                              uiCs,
    uint32                              uiCsHigh
)
{
    SALRetCode_t    ret;

    ret = GPSB_SetCs((int32)uiCs, (uint32)FALSE, uiCsHigh);

    if(ret != SAL_RET_SUCCESS)
    {
        GPSB_E("%s Fail. GPSB channel %d is closed.\n", __func__, uiCh);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_WaitRxFifoValid
*
* Function to check RX FIFO count is valid after TX data transfer.
*
* @param    uiCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_WaitRxFifoValid
(
    uint32                              uiCh
)
{
    uint32  cnt;

    cnt = 0;

    while(((SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_STAT)) & BSP_BIT_00) == 0UL))
    {
        cnt++;

        if(cnt > GPSB_POLL_TIMEOUT)
        {
            (void)GPSB_Disable(uiCh);
            GPSB_D("%s ch %d pio polling mode timeout err\n", __func__, uiCh);

            return SAL_RET_FAILED;
        }
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_FifoSyncXfer8
*
* Function to Write/Read data with 8 BPW.
*
* @param    uiCh [in]       : Value of channel to control
* @param    pTx [in]        : TX data buffer
* @param    pRx [out]       : RX data buffer
* @param    uiDataSize [in] : Write/Read data size
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_FifoSyncXfer8
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
)
{
    uint32          data_num;
    uint32          i;
    uint32          dummy;
    const uint8 *   tx;
    uint8 *         rx;
    SALRetCode_t    err;

    tx          = pTx;
    data_num    = 0;
    i           = 0xffUL;
    dummy       = 0;
    rx          = pRx;
    err         = SAL_RET_SUCCESS;

    if(uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("%s %d channel is wrong\n", __func__, uiCh);
        return SAL_RET_FAILED;
    }

    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_RUNNING);
    GPSB_Enable(uiCh);
    data_num = uiDataSize;

    for(i = 0 ; i < data_num ; i++)
    {
        /* Write data to TXFIFO */
        if(tx != (uint8 *)NULL_PTR)
        {
            SAL_WriteReg(tx[i],(uint32)(gpsb[uiCh].dBase + GPSB_PORT));
        }

        /* Wait until Read FIFO is valid */
        err = GPSB_WaitRxFifoValid(uiCh);

        if(err != SAL_RET_SUCCESS)
        {
            GPSB_Disable(uiCh);
            GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);
            GPSB_D("%s ch %d wait rx fifo valid err %d\n", __func__, uiCh, err);

            return err;
        }

        /* Read data from RX FIFO */
        dummy = SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_PORT));

        if(rx != (uint8 *)NULL_PTR)
        {
            rx[i] = (uint8)(dummy & 0xFFUL);
        }
    }

    GPSB_Disable(uiCh);
    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_FifoSyncXfer16
*
* Function to Write/Read data with 16 BPW.
*
* @param    uiCh [in]       : Value of channel to control
* @param    pTx [in]        : TX data buffer
* @param    pRx [out]       : RX data buffer
* @param    uiDataSize [in] : Write/Read data size
* @return   SAL_RET_SUCCESS or SAL_ERR
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_FifoSyncXfer16
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
)
{
    uint32          data_num;
    uint32          i;
    uint32          dummy;
    const uint16 *  tx;
    uint16 *        rx;
    SALRetCode_t    err;

    tx          = pTx;
    data_num    = 0;
    i           = 0xffUL;
    dummy       = 0;
    rx          = pRx;
    err         = SAL_RET_SUCCESS;

    if(uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("%s %d channel is wrong\n", __func__, uiCh);
        return SAL_RET_FAILED;
    }

    if((uiDataSize % 2UL) != 0UL)
    {
        GPSB_D("%s ch %d data size(%d) is not alligned\n", __func__, uiCh, uiDataSize);
        return SAL_RET_FAILED;
    }

    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_RUNNING);
    GPSB_Enable(uiCh);
    data_num    = uiDataSize / 2UL;

    for(i = 0 ; i < data_num ; i++)
    {
        /* Write data to TXFIFO */
        if(tx != (uint16 *)NULL_PTR)
        {
            SAL_WriteReg(tx[i], (uint32)(gpsb[uiCh].dBase + GPSB_PORT));
        }

        /* Wait until Read FIFO is valid */
        err = GPSB_WaitRxFifoValid(uiCh);

        if(err != SAL_RET_SUCCESS)
        {
            GPSB_Disable(uiCh);
            GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);
            GPSB_D("%s ch %d wait rx fifo valid err %d\n", __func__, uiCh, err);

            return err;
        }

        /* Read data from RX FIFO */
        dummy = SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_PORT));

        if(rx != (uint16 *)NULL_PTR)
        {
            rx[i] = (uint16)(dummy & 0xFFFFUL);
        }
    }

    GPSB_Disable(uiCh);
    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_FifoSyncXfer32
*
* Function to Write/Read data with 32 BPW.
*
* @param    uiCh [in]       : Value of channel to control
* @param    pTx [in]        : TX data buffer
* @param    pRx [out]       : RX data buffer
* @param    uiDataSize [in] : Write/Read data size
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_FifoSyncXfer32
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
)
{
    uint32          data_num;
    uint32          i;
    uint32          dummy;
    const uint32 *  tx;
    uint32 *        rx;
    SALRetCode_t    err;

    tx          = pTx;
    data_num    = 0;
    i           = 0xffUL;
    dummy       = 0;
    rx          = pRx;
    err         = SAL_RET_SUCCESS;

    if(uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("%s %d channel is wrong\n", __func__, uiCh);

        return SAL_RET_FAILED;
    }

    if((uiDataSize % 4UL) != 0UL)
    {
        GPSB_D("%s ch %d data size(%d) is not alligned\n", __func__, uiCh, uiDataSize);

        return SAL_RET_FAILED;
    }

    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_RUNNING);
    GPSB_Enable(uiCh);
    data_num    = uiDataSize / 4UL;

    for(i = 0 ; i < data_num ; i++)
    {
        /* Write data to TXFIFO */
        if(tx != (uint32 *)NULL_PTR)
        {
            SAL_WriteReg((uint32)(tx[i] & 0xFFFFFFFFUL), (uint32)(gpsb[uiCh].dBase + GPSB_PORT));
        }
        else
        {
            SAL_WriteReg(0, (uint32)(gpsb[uiCh].dBase + GPSB_PORT));
        }

        /* Wait until Read FIFO is valid */
        err = GPSB_WaitRxFifoValid(uiCh);

        if(err != SAL_RET_SUCCESS)
        {
            GPSB_Disable(uiCh);
            GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);
            GPSB_D("%s ch %d wait rx fifo valid err %d\n", __func__, uiCh, err);

            return err;
        }

        //delay1us(1);

        /* Read data from RX FIFO */
        dummy   = SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_PORT));

        if(rx != (uint32 *)NULL_PTR)
        {
            rx[i] = (uint32)(dummy & 0xFFFFFFFFUL);
        }
    }

    GPSB_Disable(uiCh);
    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_FiFoAsyncXfer
*
* Function to Async transfer with FIFO(PIO) mode.
*
* @param    uiCh [in]       : Value of channel to control
* @param    pTx [in]        : TX data buffer
* @param    pRx [out]       : RX data buffer
* @param    uiDataSize [in] : Write/Read data size
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_FiFoAsyncXfer
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
)
{
    uint32  ret;

    ret = 0;

    GPSB_D("%s: ch %d\n", __func__, uiCh);

    if((pTx == NULL_PTR)|| (pRx == NULL_PTR))
    {
        return SAL_RET_FAILED;
    }

    gpsb[uiCh].dAsyncTxBuf      = pTx;
    gpsb[uiCh].dAsyncRxBuf      = pRx;
    gpsb[uiCh].dAsyncTxDataSize = uiDataSize;
    gpsb[uiCh].dAsyncRxDataSize = uiDataSize;

    /* Write data to FIFO */
    ret = GPSB_FifoWrite(uiCh, gpsb[uiCh].dAsyncTxBuf, gpsb[uiCh].dAsyncTxDataSize);

    if(gpsb[uiCh].dAsyncTxBuf != NULL_PTR)
    {
        gpsb[uiCh].dAsyncTxBuf = (gpsb[uiCh].dAsyncTxBuf + ret);
    }

    gpsb[uiCh].dAsyncTxDataSize -= ret;

    /* Enable operation */
    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_RUNNING);
    GPSB_EnableIntr(uiCh);
    GPSB_Enable(uiCh);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_DmaAsyncXfer
*
* Function to transfer data with dma mode.
*
* @param    uiCh [in]       : Value of channel to control
* @param    pTx [in]        : TX data buffer
* @param    pRx [out]       : RX data buffer
* @param    uiDataSize [in] : Write/Read data size
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GPSB_DmaAsyncXfer
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize
)
{
    uint32  copy_length;
    uint32  bpw;

    copy_length = 0;
    bpw         = 0;

    /* Check wrong GPSB channel number */
    if (uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("%d channel is wrong\n", uiCh);

        return SAL_RET_FAILED;
    }

    if(gpsb[uiCh].dSupportDma != TRUE)
    {
        GPSB_D("%s ch %d does not support dma transfer\n", __func__, uiCh);

        return SAL_RET_FAILED;
    }

    /*
     * For channels which is able to use a dedicated dma,
     * set byte endian mode according to bits per word.
     */
    bpw = GPSB_GetBpw(uiCh);

    if(bpw != 32UL)
    {
        GPSB_BitSet(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_28);
    }
    else
    {
        /* When bits per word is equal to 32, disable half-word and byte */
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_INTEN, (BSP_BIT_24 | BSP_BIT_25 | BSP_BIT_26 | BSP_BIT_27));
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_28);
    }

    /* Clear packet counter (CH 0-2) */
    GPSB_BitSet(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_02);
    GPSB_BitClear(gpsb[uiCh].dBase + GPSB_DMACTR, BSP_BIT_02);

    if(uiDataSize > gpsb[uiCh].dTxDma.dbSize)
    {
        copy_length = gpsb[uiCh].dTxDma.dbSize;
    }
    else
    {
        copy_length = uiDataSize;
    }

    if(pTx != NULL_PTR)
    {
        (void)SAL_MemCopy((void *)(gpsb[uiCh].dTxDma.dbAddr), pTx, copy_length);
    }

    /* Set DMA TXBASE and RXBASE */
    (void)GPSB_DmaSetAddr(uiCh, gpsb[uiCh].dTxDma.dbAddr, gpsb[uiCh].dRxDma.dbAddr);

    /* Set Packet size and count */
    SAL_WriteReg((uint32)((copy_length & 0x1FFFUL) << 0UL), (uint32)(gpsb[uiCh].dBase + GPSB_PACKET));

    gpsb[uiCh].dAsyncTxBuf      = pTx;
    gpsb[uiCh].dAsyncRxBuf      = pRx;
    gpsb[uiCh].dAsyncTxDataSize = uiDataSize;
    gpsb[uiCh].dAsyncRxDataSize = 0;

    /* Enable dma operation */
    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_RUNNING);
    GPSB_Enable(uiCh);
    GPSB_DmaStart(uiCh);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_Xfer
*
* Function to transfer data. this function can be called from application.
*
* @param    uiCh [in]       : Value of channel to control
* @param    pTx [in]        : TX data buffer
* @param    pRx [out]       : RX data buffer
* @param    uiDataSize [in] : Write/Read data size
* @param    uiXferMode [in] : Transfer mode (DMA/PIO , Interrupt/Non-Interrupt)
* @return   SAl_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_Xfer
(
    uint32                              uiCh,
    const void *                        pTx,
    void *                              pRx,
    uint32                              uiDataSize,
    uint32                              uiXferMode
)
{
    SALRetCode_t    ret;
    uint32          bpw;
    uint32          word_size;

    ret         = SAL_RET_SUCCESS;
    bpw         = 0;
    word_size   = 0;

    if (uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("%d channel is wrong\n", uiCh);

        return SAL_RET_FAILED;
    }

    if(gpsb[uiCh].dState != GPSB_XFER_STATE_IDLE)
    {
        GPSB_D("%s: ch %d is not in idle state : %d\n", __func__, uiCh, gpsb[uiCh].dState);

        delay1us(1);

        if(gpsb[uiCh].dState != GPSB_XFER_STATE_IDLE)
        {
            return SAL_RET_FAILED;
        }
    }

    /* Check data length and bpw */
    if(uiDataSize == 0UL)
    {
        GPSB_D("%s: ch %d data size is zero\n", __func__, uiCh);

        return SAL_RET_FAILED;
    }

    bpw = GPSB_GetBpw(uiCh);

    if(bpw <= 8UL)
    {
        word_size = 1UL;
    }
    else if(bpw <= 16UL)
    {
        word_size = 2UL;
    }
    else
    {
        word_size = 4UL;
    }

    if((uiDataSize % word_size) != 0UL)
    {
        GPSB_D("%s: ch %d data size(%d) is not aligned by word size(%d)\n",
                __func__, uiCh, uiDataSize, word_size);

        return SAL_RET_FAILED;
    }

    /*
     * Set GPSB CTF(Continuous Transfer mode) according to the flag.
     * In case of transfer without CTF mode, you should configure
     * GPSB PCFG FRM settings by using tcc_GPSB_master_alloc_cs() before
     * transfer.
     * In case of transfer with CTF mode, you can control CS as GPIO
     * or assign CS to GPSB FRM.
     */
    if((uiXferMode & GPSB_XFER_MODE_WITHOUT_CTF) != 0UL)
    {
        GPSB_BitClear(gpsb[uiCh].dBase + GPSB_MODE, BSP_BIT_04);
    }
    else
    {
        GPSB_BitSet(gpsb[uiCh].dBase + GPSB_MODE, BSP_BIT_04);
    }

    /* Start transfer according to xfer mode */
    if((uiXferMode & GPSB_XFER_MODE_WITH_INTERRUPT) != 0UL)
    {
        /* w/ interrupt mode (support non-bloning mode only) */
        if((uiXferMode & GPSB_XFER_MODE_DMA) != 0UL)
        {
            /* DMA mode */
            ret = GPSB_DmaAsyncXfer(uiCh, pTx, pRx, uiDataSize);
        }
        else
        {
            /* Non-DMA mode */
            ret = GPSB_FiFoAsyncXfer(uiCh, pTx, pRx, uiDataSize);
        }
    }
    else if((uiXferMode & GPSB_XFER_MODE_WITHOUT_INTERRUPT) != 0UL)
    {
        /* w/o interrupt mode (support blocking mode only) */
        if(bpw <= 8UL)
        {
            ret = GPSB_FifoSyncXfer8(uiCh, pTx, pRx, uiDataSize);
        }
        else if(bpw <= 16UL)
        {
            ret = GPSB_FifoSyncXfer16(uiCh, pTx, pRx, uiDataSize);
        }
        else
        {
            ret = GPSB_FifoSyncXfer32(uiCh, pTx, pRx, uiDataSize);
        }
    }
    else
    {
        GPSB_D("%s: ch %d unsupported transfer mode 0x%x\n", __func__, uiCh, uiXferMode);

        return SAL_RET_FAILED;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_XferAbort
*
* Function to stop transfer.
*
* @param    uiCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_XferAbort
(
    uint32                              uiCh
)
{
    /* Stop operate due to error */
    GPSB_DmaStop(uiCh);
    GPSB_DisableIntr(uiCh);
    GPSB_Disable(uiCh);
    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);

    gpsb[uiCh].dAsyncTxBuf      = NULL;
    gpsb[uiCh].dAsyncRxBuf      = NULL;
    gpsb[uiCh].dAsyncTxDataSize = 0;
    gpsb[uiCh].dAsyncRxDataSize = 0;
}


/*
***************************************************************************************************
*                                          GPSB_HwInit
*
* Function to set default hw settings.
*
* @param    uiCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_HwInit
(
    uint32                              uiCh
)
{
    sint32  ret;

    ret = 0;

    /* Enable iobus */
    ret = CLOCK_SetIobusPwdn((gpsb[uiCh].dIobusName), FALSE);

    if(ret != 0)
    {
        GPSB_D("%s: ch %d failed to enable iobus pwdn\n", __func__, uiCh);
    }
    else
    {
        /* Disable operation */
        GPSB_Disable(uiCh);

        /* Set GPSB DIVLDV */
        GPSB_BitCSet(gpsb[uiCh].dBase + GPSB_MODE, (0xFFUL << 24UL), ((GPSB_DEF_DIVLDV & 0xFFUL) << 24UL));

        /* Set GPSB MODE register */
        GPSB_BitCSet(gpsb[uiCh].dBase + GPSB_MODE, ((BSP_BIT_02 | BSP_BIT_01) | BSP_BIT_00),  /* Set GPSB master */
                                                     (BSP_BIT_04));                            /* Set CTF mode */

        /* Set interrup for handle error status (for ROR, WUR, RUR, WOR) */
        GPSB_BitCSet(gpsb[uiCh].dBase + GPSB_INTEN, (0x3FFUL),
                       (BSP_BIT_05 | BSP_BIT_06 | BSP_BIT_07 | BSP_BIT_08));


        /* Set GPSB FIFO Theshold for generating interrupt request */
        GPSB_BitCSet(gpsb[uiCh].dBase + GPSB_INTEN, (0x7UL << 20UL), (GPSB_CFGWTH << 20UL));
        GPSB_BitCSet(gpsb[uiCh].dBase + GPSB_INTEN, (0x7UL << 16UL), (GPSB_CFGRTH << 16UL));
    }

}

/*
***************************************************************************************************
*                                          GPSB_Open
*
* Function to initialize GPSB channel.
*
* @param    uiCh [in]           : Value of channel to control
* @param    uiSdo [in]          : SDO GPIO index
* @param    uiSdi [in]          : SDI GPIO index
* @param    uiSclk [in]         : SCLK GPIO index
* @param    uiDmaAddrTx [in]    : DMA TX buffer physical address
* @param    uiDmaAddrRx [in]    : DMA RX buffer physical address
* @param    uiDmaBufSize [in]   : DMA buffer size
* @param    fbCallback [in]     : callback function pointer
* @param    pArg
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/


SALRetCode_t GPSB_Open
(
    uint32                              uiCh,
    uint32                              uiSdo,
    uint32                              uiSdi,
    uint32                              uiSclk,
    uint32                              uiDmaAddrTx,
    uint32                              uiDmaAddrRx,
    uint32                              uiDmaBufSize,
    GPSBCallback                        fbCallback,
    void *                              pArg
)
{
    SALRetCode_t    err;

    err = SAL_RET_SUCCESS;

    /* Check wrong GPSB channel number */
    if(uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("%s: %d channel is wrong\n", __func__, uiCh);

        return SAL_RET_FAILED;
    }

    (void)SAL_CoreCriticalEnter();

    if(gpsb[uiCh].dState != GPSB_XFER_STATE_DISABLED)
    {
        (void)SAL_CoreCriticalExit();
        GPSB_D("%s: %d channel is already opened\n", __func__, uiCh);

        return SAL_RET_FAILED;
    }

    gpsb[uiCh].dState = GPSB_XFER_STATE_IDLE;
    (void)SAL_CoreCriticalExit();

    /* Initialize the gpsb structure */
    gpsb[uiCh].dSdo                     = uiSdo;
    gpsb[uiCh].dSdi                     = uiSdi;
    gpsb[uiCh].dSclk                    = uiSclk;
    gpsb[uiCh].dComplete.xcCallback     = fbCallback;
    gpsb[uiCh].dComplete.xcArg          = pArg;
    gpsb[uiCh].dSupportDma              = FALSE;
    gpsb[uiCh].dTxDma.dbAddr            = NULL;
    gpsb[uiCh].dTxDma.dbSize            = 0;

    if((uiDmaBufSize != (uint32)NULL) && ((uiDmaAddrTx != (uint32)NULL) || (uiDmaAddrRx != (uint32)NULL)))
    {
        gpsb[uiCh].dSupportDma = TRUE;

        if(uiDmaAddrTx != (uint32)NULL)
        {
            gpsb[uiCh].dTxDma.dbAddr = uiDmaAddrTx;
            gpsb[uiCh].dTxDma.dbSize = uiDmaBufSize;
            GPSB_D("%s: tx_dma.paddr: 0x%08X      / size: %d\n",
                     __func__, gpsb[uiCh].dTxDma.dbAddr,
                     gpsb[uiCh].dTxDma.dbSize);
        }

        if(uiDmaAddrRx != (uint32)NULL)
        {
            gpsb[uiCh].dRxDma.dbAddr = uiDmaAddrRx;
            gpsb[uiCh].dRxDma.dbSize = uiDmaBufSize;
            GPSB_D("%s: rx_dma.Addr: 0x%08X\n",
                     __func__, gpsb[uiCh].dRxDma.dbAddr,
                     gpsb[uiCh].dRxDma.dbSize);
        }
    }

    /* Reset GPSB block */
    err = GPSB_Reset(uiCh);

    if(err != SAL_RET_SUCCESS)
    {
        GPSB_D("%s: ch %d failed to reset gpsb err %d\n", __func__, uiCh, err);

        return err;
    }

    if((uiSdo != (uint32)NULL) && (uiSdo != (uint32)NULL) && (uiSclk != (uint32)NULL))
    {
        if((uiCh != CH_SDM) && (uiCh != CH_RVC))
        {
            /* Set port and gpio configuration */
            err = GPSB_SetPort(uiCh, uiSdo, uiSdi, uiSclk);
            if(err != SAL_RET_SUCCESS)
            {
                GPSB_D("%s: ch %d gpsb set port failed err %d\n", __func__, uiCh, err);

                return err;
            }
        }
    }
    /* Stop dma operation */
    GPSB_DmaStop(uiCh);

    /* Initialize GPSB for GPSB master*/
    GPSB_HwInit(uiCh);

    /* Set gpsb init and mode state */
    gpsb[uiCh].dIsSlave = FALSE;

    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);

    return SAL_RET_SUCCESS;
}


/*
***************************************************************************************************
*                                          GPSB_Close
*
* Function to de-init GPSB channel.
*
* @param    uiCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_Close
(
    uint32                              uiCh
)
{
    sint32  err;
    uint32  cbit;

    err     = 0;
    cbit    = 0UL;

    /* Check wrong GPSB channel number */
    if(uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("%s %d channel is wrong\n", __func__, uiCh);

        return SAL_RET_FAILED;
    }

    (void)SAL_CoreCriticalEnter();

    if(gpsb[uiCh].dState == GPSB_XFER_STATE_DISABLED)
    {
        (void)SAL_CoreCriticalExit();
        GPSB_D("%s %d channel is already closed\n", __func__, uiCh);

        return SAL_RET_SUCCESS;
    }

    gpsb[uiCh].dState = GPSB_XFER_STATE_DISABLED;
    (void)SAL_CoreCriticalExit();

    /* Stop the operation */
    GPSB_DmaStop(uiCh);
    GPSB_DisableIntr(uiCh);
    GPSB_Disable(uiCh);

    /* Disable iobus */
    err = CLOCK_SetIobusPwdn(gpsb[uiCh].dIobusName, TRUE);

    if(err != NULL)
    {
        GPSB_D("%s ch %d failed to disable iobus pwdn\n", __func__, uiCh);

        return SAL_RET_FAILED;
    }

    gpsb[uiCh].dPort = NULL;

    if(uiCh < (GPSB_CH_NUM-1UL))
    {
        cbit = (uint32)GPSB_PORT_SEL_MASK << (uiCh * 8UL);
    }
    else
    {
        cbit = (uint32)GPSB_PORT_SEL_MASK << ((uiCh-4UL) * 8UL);
    }

    GPSB_BitClear((uint32)GPSB_PORT_CFG(reg_offset), cbit);
    GPSB_SetXferState(uiCh, GPSB_XFER_STATE_DISABLED);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_Isr
*
* Function to process GPSB interrupt.
*
* @param    uiCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_Isr
(
    uint32                              uiCh
)
{
    uint32  dmaicr;
    uint32  status;
    uint32  event;
    uint32  ret;
    uint32  copy_length;
    uint32  data_size;

    dmaicr      = 0;
    status      = 0;
    event       = 0;
    ret         = 0;
    copy_length = 0;
    data_size   = 0;

    if((GPSB_IsMaster(uiCh)) > 0UL)
    {
        /* Check error status */
        status = SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_STAT));

        if((status & (0xFUL << 5UL)) != 0UL)
        {
            GPSB_D("%s: ch %d error status occur status 0x%08x\n", __func__, uiCh, status);

            if((status & BSP_BIT_05) != 0UL)
            {
                event |= (uint32)GPSB_EVENT_ERR_ROR;
                GPSB_D("%s: ch %d read fifo over-run error\n", __func__, uiCh);
            }

            if((status & BSP_BIT_06) != 0UL)
            {
                event |= (uint32)GPSB_EVENT_ERR_WUR;
                GPSB_D("%s: ch %d write fifo under-run error\n", __func__, uiCh);
            }

            if((status & BSP_BIT_07) != 0UL)
            {
                event |= (uint32)GPSB_EVENT_ERR_RUR;
                GPSB_D("%s: ch %d read fifo under-run error\n", __func__, uiCh);
            }

            if((status & BSP_BIT_08) != 0UL)
            {
                event |= (uint32)GPSB_EVENT_ERR_WOR;
                GPSB_D("%s: ch %d write fifo over-run error\n", __func__, uiCh);
            }

            /* Stop operate due to error */
            GPSB_Disable(uiCh);
            GPSB_DmaStop(uiCh);
            GPSB_SetXferState(uiCh, GPSB_XFER_STATE_IDLE);

            /* Clear error status */
            GPSB_BitSet(gpsb[uiCh].dBase + GPSB_STAT,
                    (BSP_BIT_05 | BSP_BIT_06 | BSP_BIT_07 | BSP_BIT_08));

            /* Notify error status */
            if(gpsb[uiCh].dComplete.xcCallback != NULL_PTR)
            {
                gpsb[uiCh].dComplete.xcCallback(uiCh, event, gpsb[uiCh].dComplete.xcArg);
            }

            return;
        }

        if((status & BSP_BIT_00) != 0UL)
        {
            GPSB_D("%s: ch %d async pio transfer\n", __func__, uiCh);
            ret = GPSB_FifoRead(uiCh, gpsb[uiCh].dAsyncRxBuf, gpsb[uiCh].dAsyncRxDataSize);

            if(gpsb[uiCh].dAsyncRxBuf != NULL_PTR)
            {
                gpsb[uiCh].dAsyncRxBuf = gpsb[uiCh].dAsyncRxBuf + ret;
            }

            gpsb[uiCh].dAsyncRxDataSize -= ret;

            if(gpsb[uiCh].dAsyncTxDataSize > 0UL)
            {
                ret = GPSB_FifoWrite(uiCh, gpsb[uiCh].dAsyncTxBuf, gpsb[uiCh].dAsyncTxDataSize);

                if(gpsb[uiCh].dAsyncTxBuf != NULL_PTR)
                {
                    gpsb[uiCh].dAsyncTxBuf = gpsb[uiCh].dAsyncTxBuf + ret;
                }

                gpsb[uiCh].dAsyncTxDataSize -= ret;
            }

            if(gpsb[uiCh].dAsyncRxDataSize <= 0UL)
            {
                GPSB_D("%s: ch %d complete async pio transfer\n", __func__, uiCh);

                gpsb[uiCh].dAsyncTxBuf      = NULL;
                gpsb[uiCh].dAsyncRxBuf      = NULL;
                gpsb[uiCh].dAsyncTxDataSize = 0;

                GPSB_DisableIntr(uiCh);
                GPSB_Disable(uiCh);
                gpsb[uiCh].dState = GPSB_XFER_STATE_IDLE;

                /* Notify the completion of transfer */
                event = GPSB_EVENT_COMPLETE;

                if(gpsb[uiCh].dComplete.xcCallback != NULL_PTR)
                {
                    gpsb[uiCh].dComplete.xcCallback(uiCh, event, gpsb[uiCh].dComplete.xcArg);
                }
            }
        }

        dmaicr = SAL_ReadReg((uint32)(gpsb[uiCh].dBase + GPSB_DMAICR));

        if((dmaicr & BSP_BIT_29) != 0UL)
        {
            GPSB_D("%s: ch %d dma done interrupt\n", __func__, uiCh);
            SAL_WriteReg(dmaicr, (uint32)(gpsb[uiCh].dBase + GPSB_DMAICR));
            data_size = gpsb[uiCh].dAsyncTxDataSize;

            if(data_size > gpsb[uiCh].dTxDma.dbSize)
            {
                copy_length = gpsb[uiCh].dTxDma.dbSize;
            }
            else
            {
                copy_length = data_size;
            }

            if(gpsb[uiCh].dAsyncRxBuf != NULL_PTR)
            {
                (void)SAL_MemCopy(gpsb[uiCh].dAsyncRxBuf, (void *)(gpsb[uiCh].dRxDma.dbAddr), copy_length);
            }

            gpsb[uiCh].dAsyncTxDataSize -= copy_length;
            gpsb[uiCh].dAsyncTxBuf      = (gpsb[uiCh].dAsyncTxBuf + copy_length);
            gpsb[uiCh].dAsyncRxBuf      = (gpsb[uiCh].dAsyncRxBuf + copy_length);

            GPSB_D("%s: ch %d xfer in this time %d remainder %d\n",
                    __func__, uiCh, copy_length, gpsb[uiCh].dAsyncTxDataSize);

            if(gpsb[uiCh].dAsyncTxDataSize != 0UL)
            {
                data_size = gpsb[uiCh].dAsyncTxDataSize;

                if(data_size > gpsb[uiCh].dTxDma.dbSize)
                {
                    copy_length = gpsb[uiCh].dTxDma.dbSize;
                }
                else
                {
                    copy_length = data_size;
                }

                if(gpsb[uiCh].dAsyncTxBuf  != NULL_PTR)
                {
                    (void)SAL_MemCopy((void *)(gpsb[uiCh].dTxDma.dbAddr), gpsb[uiCh].dAsyncTxBuf, copy_length);
                }

                /* Set Packet size and count */
                SAL_WriteReg(((copy_length & 0x1FFFUL) << 0UL), (uint32)(gpsb[uiCh].dBase + GPSB_PACKET));

                /* Re-start DMA for transferring remainder */
                GPSB_DmaStart(uiCh);

            }
            else
            {
                gpsb[uiCh].dAsyncTxBuf      = NULL;
                gpsb[uiCh].dAsyncRxBuf      = NULL;
                gpsb[uiCh].dAsyncTxDataSize = 0;
                gpsb[uiCh].dAsyncRxDataSize = 0;

                /* DMA done interrupt */
                GPSB_DmaStop(uiCh);
                GPSB_Disable(uiCh);
                gpsb[uiCh].dState   = GPSB_XFER_STATE_IDLE;

                /* Notify the completion of transfer */
                event = GPSB_EVENT_COMPLETE;

                if(gpsb[uiCh].dComplete.xcCallback != NULL_PTR)
                {
                    gpsb[uiCh].dComplete.xcCallback(uiCh, event, gpsb[uiCh].dComplete.xcArg);
                }
            }
        }
    }
}

/*
***************************************************************************************************
*                                          GPSB_SetSdmExtPort
*
* Function to set sdm external hw port info.
*
* @param    iPort [in]      : Port number connected sdm.
* @return   GPSB_PASSED or GPSB_FAILED
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SetSdmExtPort
(
    uint32                              uiPort
)
{
    uint32  reg;
    uint32  val;

    reg = 0UL;
    val = 0UL;

    if(uiPort >= 64UL)
    {
        GPSB_D("[%s] Invaild port number. (vaild : 0~63) \n", __func__);

        return SAL_RET_FAILED;
    }

    val = ((uiPort & SDM_RVC_PORT_MASK) << SDM_PORT_SHIFT);
    reg = gpsb[0].dCfgAddr + (uint32)GPSB_PORT_SEL1;

    SAL_WriteReg(val, reg);
    GPSB_D("Set SDM external port : 0x%x \n",
             (SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_PORT_SEL1)) >> SDM_PORT_SHIFT) & SDM_RVC_PORT_MASK);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_SetRvcExtPort
*
* Function to set rvc external hw port info.
*
* @param    iPort [in]      : Port number connected rvc.
* @return   GPSB_PASSED or GPSB_FAILED
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SetRvcExtPort
(
    uint32                              uiPort
)
{
    if(uiPort >= 64UL)
    {
        GPSB_D("[%s] Invaild port number. (vaild : 0~63) \n", __func__);

        return SAL_RET_FAILED;
    }

    SAL_WriteReg(((uiPort & SDM_RVC_PORT_MASK) << RVC_PORT_SHIFT), (uint32)(gpsb[0].dCfgAddr + GPSB_PORT_SEL1));
    GPSB_D("Set RVC external port : 0x%x \n",
             (SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_PORT_SEL1)) >> RVC_PORT_SHIFT) & SDM_RVC_PORT_MASK);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_SetSdmChannel
*
* Function to set sdm channel.
*
* @param    uiCh [in]       : Value of channel to control
* @return   GPSB_PASSED or GPSB_FAILED
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SetSdmChannel
(
    uint32                              uiCh
)
{
    uint32  reg;

    reg = 0UL;

    if((SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_PCFG_WR_LOCK)) & GPSB_BIT_WR_LOCK) == 1UL)
    {
        GPSB_D("GPSB CFG register was locked. Can't access it.\n");
    }

    reg = SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_SDM_RVC_CTRL_SEL));
    GPSB_D("Read GPSB SDM Channel : 0x%x \n", reg & SDM_CHANNEL_MASK);

    reg &= ~(SDM_CHANNEL_MASK);
    reg |= uiCh;

    GPSB_D("Write GPSB SDM Channel : 0x%x \n", uiCh);
    SAL_WriteReg(reg , (uint32)(gpsb[0].dCfgAddr + GPSB_SDM_RVC_CTRL_SEL));

    reg = SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_SDM_RVC_CTRL_SEL)) & SDM_CHANNEL_MASK;

    if(reg != uiCh)
    {
        GPSB_D("Fail to set GPSB SDM Channel. (expected : %d, read value : %d)\n", uiCh, reg);

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_SetRvcChannel
*
* Function to set rvc channel.
*
* @param    uiCh [in]       : Value of channel to control
* @return   GPSB_PASSED or GPSB_FAILED
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SetRvcChannel
(
    uint32                              uiCh
)
{
    uint32  reg;

    reg = 0UL;

    if((SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_PCFG_WR_LOCK)) & GPSB_BIT_WR_LOCK) == 1UL)
    {
        GPSB_D("GPSB CFG register was locked. Can't access it.\n");
    }

    reg = SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_SDM_RVC_CTRL_SEL));
    GPSB_D("Read GPSB RVC Channel : 0x%x \n", (reg >> RVC_CHANNEL_SHIFT) & RVC_CHANNEL_MASK);

    reg &= ~(RVC_CHANNEL_MASK << RVC_CHANNEL_SHIFT);
    reg |= (uiCh << RVC_CHANNEL_SHIFT);

    GPSB_D("Write GPSB RVC Channel : 0x%x \n", uiCh);
    SAL_WriteReg(reg , (uint32)(gpsb[0].dCfgAddr + GPSB_SDM_RVC_CTRL_SEL));

    reg = (SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_SDM_RVC_CTRL_SEL)) >> RVC_CHANNEL_SHIFT) & RVC_CHANNEL_MASK;

    if(reg != uiCh)
    {
        GPSB_D("Fail to set GPSB RVC Channel. (expected : %d, read value : %d)\n", uiCh, reg);

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_SetLbCfg
*
* Function to set channel to enable loopback.
*
* @param    uiMstCh [in]    : Master Channel index
* @param    uiLoopCh [in]   : Loopback Channel index
* @param    uiEnable [in]   : Loopback enable
* @return   GPSB_PASSED or GPSB_FAILED
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SetLbCfg
(
    uint32                              uiMstCh,
    uint32                              uiLoopCh,
    uint32                              uiEnable
)
{
    uint32  reg;

    reg = 0UL;

    if((uiMstCh >= GPSB_CH_NUM) || (uiLoopCh >= GPSB_CH_NUM))
    {
        GPSB_D("Invaild Channel value. \n");

        return SAL_RET_FAILED;
    }

    reg = SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_LB_CFG));
    reg |= (((uiEnable + ((uiMstCh & 0x7UL) << 1UL))) << uiLoopCh) * 4UL;

    SAL_WriteReg(reg, (uint32)(gpsb[0].dCfgAddr + (uint32)GPSB_LB_CFG));

    GPSB_D("GPSB LB_CFG : %x\n", reg);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_ClearLbCfg
*
* Function to unset channel to disable loopback.
*
* @return   GPSB_PASSED
* Notes
*
***************************************************************************************************
*/


SALRetCode_t GPSB_ClearLbCfg
(
    void
)
{
    SAL_WriteReg(0x0UL , (uint32)(gpsb[0].dCfgAddr + GPSB_LB_CFG));

    GPSB_D("Done.Clear LB_CFG : %x \n", SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_LB_CFG)));

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPSB_Isr0
*
* Function to process interrupt about channel 0.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_Isr0
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter ISR0\n", __func__);
    GPSB_Isr(0);
}

/*
***************************************************************************************************
*                                          GPSB_Isr1
*
* Function to process interrupt about channel 1.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_Isr1
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter ISR1\n", __func__);
    GPSB_Isr(1);
}

/*
***************************************************************************************************
*                                          GPSB_Isr2
*
* Function to process interrupt about channel 2.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_Isr2
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter ISR2\n", __func__);
    GPSB_Isr(2);
}

/*
***************************************************************************************************
*                                          GPSB_Isr3
*
* Function to process interrupt about channel 3.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_Isr3
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter ISR3\n", __func__);
    GPSB_Isr(3);
}

/*
***************************************************************************************************
*                                          GPSB_Isr4
*
* Function to process interrupt about channel 4.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_Isr4
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter ISR4\n", __func__);
    GPSB_Isr(4);
}

/*
***************************************************************************************************
*                                          GPSB_Isr5
*
* Function to process interrupt about channel 5.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_Isr5
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter ISR5\n", __func__);
    GPSB_Isr(5);
}

/*
***************************************************************************************************
*                                          GPSB_DmaIsr0
*
* Function to process dma interrupt about channel 0.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_DmaIsr0
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter DMA-ISR0\n", __func__);
    GPSB_Isr(0);
}

/*
***************************************************************************************************
*                                          GPSB_DmaIsr1
*
* Function to process dma interrupt about channel 1.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_DmaIsr1
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter DMA-ISR1\n", __func__);
    GPSB_Isr(1);
}

/*
***************************************************************************************************
*                                          GPSB_DmaIsr2
*
* Function to process dma interrupt about channel 2.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_DmaIsr2
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter DMA-ISR2\n", __func__);
    GPSB_Isr(2);
}

/*
***************************************************************************************************
*                                          GPSB_DmaIsr3
*
* Function to process dma interrupt about channel 3.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/


static void GPSB_DmaIsr3
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter DMA-ISR3\n", __func__);
    GPSB_Isr(3);
}

/*
***************************************************************************************************
*                                          GPSB_DmaIsr4
*
* Function to process dma interrupt about channel 4.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_DmaIsr4
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter DMA-ISR4\n", __func__);
    GPSB_Isr(4);
}

/*
***************************************************************************************************
*                                          GPSB_DmaIsr5
*
* Function to process dma interrupt about channel 5.
*
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void GPSB_DmaIsr5
(
    void *                              pArg
)
{
    (void)pArg;
    GPSB_D("%s: enter DMA-ISR5\n", __func__);
    GPSB_Isr(5);
}

/*
***************************************************************************************************
*                                          GPSB_Init
*
* Function to register interrupt handle and enable interrupt.
*
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_Init
(
    void
)
{
    /* Register gpsb irq handler */
    (void)GIC_IntVectSet((uint32)GIC_GPSB, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_Isr0, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB);

    (void)GIC_IntVectSet((uint32)GIC_GPSB1, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_Isr1, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB1);

    (void)GIC_IntVectSet((uint32)GIC_GPSB2, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_Isr2, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB2);

    (void)GIC_IntVectSet((uint32)GIC_GPSB3, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_Isr3, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB3);

    (void)GIC_IntVectSet((uint32)GIC_GPSB4, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_Isr4, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB4);

    (void)GIC_IntVectSet((uint32)GIC_GPSB5, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_Isr5, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB5);

    // DMA Intr

    (void)GIC_IntVectSet((uint32)GIC_GPSB0_DMA, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_DmaIsr0, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB0_DMA);

    (void)GIC_IntVectSet((uint32)GIC_GPSB1_DMA, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_DmaIsr1, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB1_DMA);

    (void)GIC_IntVectSet((uint32)GIC_GPSB2_DMA, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_DmaIsr2, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB2_DMA);

    (void)GIC_IntVectSet((uint32)GIC_GPSB3_DMA, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_DmaIsr3, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB3_DMA);

    (void)GIC_IntVectSet((uint32)GIC_GPSB4_DMA, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_DmaIsr4, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB4_DMA);

    (void)GIC_IntVectSet((uint32)GIC_GPSB5_DMA, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, &GPSB_DmaIsr5, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_GPSB5_DMA);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_Deinit
*
* Function to disable interrupt.
*
* @return
* Notes
*
***************************************************************************************************
*/

void GPSB_Deinit(void)
{
    (void)GIC_IntSrcDis(GIC_GPSB);
    (void)GIC_IntSrcDis(GIC_GPSB1);
    (void)GIC_IntSrcDis(GIC_GPSB2);
    (void)GIC_IntSrcDis(GIC_GPSB3);
    (void)GIC_IntSrcDis(GIC_GPSB4);
    (void)GIC_IntSrcDis(GIC_GPSB5);

    (void)GIC_IntSrcDis(GIC_GPSB0_DMA);
    (void)GIC_IntSrcDis(GIC_GPSB1_DMA);
    (void)GIC_IntSrcDis(GIC_GPSB2_DMA);
    (void)GIC_IntSrcDis(GIC_GPSB3_DMA);
    (void)GIC_IntSrcDis(GIC_GPSB4_DMA);
    (void)GIC_IntSrcDis(GIC_GPSB5_DMA);

    return;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyEnableLock
*
* Function to enable write lock.
*
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SafetyEnableLock
(
    void
)
{
    SALRetCode_t    ret;
    uint32          tmp;

    ret = SAL_RET_SUCCESS;
    tmp = 0;

    SAL_WriteReg(GPSB_PORT_PSWD, (uint32)(gpsb[0].dCfgAddr + GPSB_PCFG_WR_PW));
    SAL_WriteReg(GPSB_PCFG_WR_EN_BIT, (uint32)(gpsb[0].dCfgAddr + GPSB_PCFG_WR_LOCK));
    tmp = SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_PCFG_WR_LOCK));

    if ((tmp & GPSB_PCFG_WR_EN_BIT) != GPSB_PCFG_WR_EN_BIT)
    {
        GPSB_D("GPSB write lock fail : %d\n", tmp);
        ret = SAL_RET_FAILED;

        return ret;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyDisableLock
*
* Function to disable write lock.
*
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SafetyDisableLock
(
    void
)
{
    SALRetCode_t    ret;
    uint32          tmp;

    ret = SAL_RET_SUCCESS;
    tmp = 0;

    SAL_WriteReg(GPSB_PORT_PSWD, (uint32)(gpsb[0].dCfgAddr + GPSB_PCFG_WR_PW));
    SAL_WriteReg(~(GPSB_PCFG_WR_EN_BIT), (uint32)(gpsb[0].dCfgAddr + GPSB_PCFG_WR_LOCK));
    tmp = SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_PCFG_WR_LOCK));

    if ((tmp & GPSB_PCFG_WR_EN_BIT) == GPSB_PCFG_WR_EN_BIT)
    {
        GPSB_D("GPSB write lock fail : 0x%08x\n", tmp);
        ret = SAL_RET_FAILED;

        return ret;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyEnableLoopback
*
* Function to enable loopback between master & slave channel.
*
* @param    uiMstCh [in] : Master channel index
* @param    uiSlvCh [in] : Slave channel index
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SafetyEnableLoopback
(
    uint32                              uiMstCh,
    uint32                              uiSlvCh
)
{
    SALRetCode_t    ret;
    uint32          tmp;

    ret = SAL_RET_SUCCESS;
    tmp = 0UL;

    GPSB_D("loopback: ch%d(m) -> ch%d(s)\n", uiMstCh, uiSlvCh);

    if ((uiMstCh >= (GPSB_CH_NUM)) || (uiSlvCh >= (GPSB_CH_NUM)))
    {
        GPSB_D("loopback supports max %d channels.\n", (GPSB_CH_NUM));
        ret = SAL_RET_FAILED;
    }

    if (ret == SAL_RET_SUCCESS)
    {
        tmp = (uiMstCh << ((uiSlvCh * 4UL) + 1UL)) | ( (uint32)0x1UL << (uiSlvCh * 4UL));
        SAL_WriteReg(tmp, (uint32)(gpsb[0].dCfgAddr + GPSB_LB_CFG));
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyDisableLoopback
*
* Function to disable loopback between master & slave channel.
*
* @param    uiMstCh [in] : Master channel index
* @param    uiSlvCh [in] : Slave channel index
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SafetyDisableLoopback
(
    uint32                              uiMstCh,
    uint32                              uiSlvCh
)
{
    SALRetCode_t    ret;
    uint32          tmp;

    ret = SAL_RET_SUCCESS;
    tmp = 0UL;

    if ((uiMstCh >= (GPSB_CH_NUM)) || (uiSlvCh >= (GPSB_CH_NUM)))
    {
        GPSB_D("loopback supports max %d channels.\n", (GPSB_CH_NUM-1UL));
        ret = SAL_RET_FAILED;
    }

    if (ret == SAL_RET_SUCCESS)
    {
        tmp = SAL_ReadReg((uint32)(gpsb[0].dCfgAddr + GPSB_LB_CFG));
        tmp &= (~((0x3UL << 1UL) << (uiSlvCh * 4UL)));
        SAL_WriteReg(tmp, (uint32)(gpsb[0].dCfgAddr + GPSB_LB_CFG));
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyEnableMonitor
*
* Function to enable safety monitor.
*
* @param    uiCh [in]       : Value of channel to control
* @param    uiMonMode[in]   : Select Protocol/Data stuck monitor
* @param    uiMonWidth      : Value of width of monitor
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SafetyEnableMonitor
(
    uint32                              uiCh,
    uint32                              uiMonMode,
    uint32                              uiMonWidth
)
{
    SALRetCode_t    ret;
    uint32          tmp;

    ret = SAL_RET_SUCCESS;
    tmp = 0UL;

    if (uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("IO monitor doesn't support ch %d.\n", uiCh);
        ret = SAL_RET_FAILED;

        return ret;
    }

    /* Enable I/O monitor */
    tmp = BSP_BIT_16 | uiMonWidth; // Protocol & monitor enable

    if(uiMonMode == (uint32)GPSB_SM_MON_STUCK_BOTH)
    {
        tmp |= BSP_BIT_20 | BSP_BIT_21;
    }
    else if(uiMonMode == (uint32)GPSB_SM_MON_STUCK_HIGH)
    {
        tmp |= BSP_BIT_20;
    }
    else if(uiMonMode == (uint32)GPSB_SM_MON_STUCK_LOW)
    {
        tmp |= BSP_BIT_21;
    }
    else
    {
        GPSB_D("%s : unknown GPSB_SM_MODE.\n",__func__);
    }

    if((SAL_ReadReg((uint32)(GPSB_CH_BASE(uiCh)+GPSB_MODE)) & BSP_BIT_16) > 0UL)
    {
        tmp |= BSP_BIT_17; // SCK Polarity set
    }
    else
    {
        tmp &= ~(BSP_BIT_17); // Clear SCK_POL
    }

    SAL_WriteReg(tmp, gpsb_sm[uiCh].sbBaseAddr);
    GPSB_D("Set IO Mon value Ch %d : 0x%x \n", uiCh , SAL_ReadReg(gpsb_sm[uiCh].sbBaseAddr));

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyDisableMonitor
*
* Function to disable safety monitor.
*
* @param    uiCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPSB_SafetyDisableMonitor
(
    uint32                              uiCh
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;

    if (uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("IO monitor doesn't supports ch %d.\n", uiCh);
        ret = SAL_RET_FAILED;

        return ret;
    }

    /* Disable I/O monitor */
    SAL_WriteReg(0x0, gpsb_sm[uiCh].sbBaseAddr);
    SAL_WriteReg((uint32)((uint32)0x1UL << (uint32)uiCh) , (uint32)(GPSB_IO_MONITOR_BASE + GPSB_IOMON_CLR));

    return ret;
}

/*
***************************************************************************************************
*                                          GPSB_SafetyGetStatus
* Function to get safety monitor status.
*
* @param    uiCh [in]       : Value of channel to control
* @return   GPSB_SM_ERR_NONE or GPSB_SM_ERR
* Notes
*
***************************************************************************************************
*/


SALRetCode_t GPSB_SafetyGetStatus
(
    uint32                              uiCh
)
{
    SALRetCode_t    ret;
    uint32          tmp;

    ret = SAL_RET_SUCCESS;
    tmp = 0;

    if (uiCh >= GPSB_CH_NUM)
    {
        GPSB_D("GPSB-sm support max %d channels.\n", GPSB_CH_NUM);
        ret = SAL_RET_FAILED;
    }

    if (ret == SAL_RET_SUCCESS)
    {
        /* Read status */
        tmp = SAL_ReadReg(gpsb_sm[uiCh].sbBaseAddr);
        GPSB_D("Get IO Monitor value [ch : %d] : 0x%x \n", uiCh ,tmp);

        if ((tmp & BSP_BIT_24) > 0UL)
        {
            GPSB_D("SM MONITOR : PROTOCOL ERROR \n");
        }

        if ((tmp & BSP_BIT_25) > 0UL)
        {
            GPSB_D("SM MONITOR : STUCK ERROR \n");
        }
    }

    return ret;
}

