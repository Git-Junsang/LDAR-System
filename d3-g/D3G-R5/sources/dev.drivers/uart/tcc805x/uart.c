/*
***************************************************************************************************
*
*   FileName : uart.c
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

#include <bsp.h>
#include "reg_phys.h"
#include "udma.h"
#include "uart_drv.h"
#include "uart.h"
#include "clock.h"
#include "clock_dev.h"
#include "gpio.h"
#include "mpu.h" //mem_get_dma_base

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#if (DEBUG_ENABLE)
#   include <debug.h>
#   define UART_D(fmt, args...)          {LOGD(DBG_TAG_UART, fmt, ## args)}
#   define UART_E(fmt, args...)          {LOGE(DBG_TAG_UART, fmt, ## args)}
#else
#   define UART_D(fmt, args...)
#   define UART_E(fmt, args...)
#endif

static UartStatus_t uart[UART_CH_MAX];

static uint8 uart_buff0[2][UART_BUFF_SIZE];
static uint8 uart_buff1[2][UART_BUFF_SIZE];
static uint8 uart_buff2[2][UART_BUFF_SIZE];
static uint8 uart_buff3[2][UART_BUFF_SIZE];
static uint8 uart_buff4[2][UART_BUFF_SIZE];
static uint8 uart_buff5[2][UART_BUFF_SIZE];

static uint8 *uart_buff[UART_CH_MAX][2] =
{
    { uart_buff0[0], uart_buff0[1] },
    { uart_buff1[0], uart_buff1[1] },
    { uart_buff2[0], uart_buff2[1] },
    { uart_buff3[0], uart_buff3[1] },
    { uart_buff4[0], uart_buff4[1] },
    { uart_buff5[0], uart_buff5[1] }
};

/*
***************************************************************************************************
                                     STATIC FUNCTIONS
***************************************************************************************************
*/

static uint32 UART_RegRead
(
    uint8                               ucCh,
    uint32                              uiAddr
);

static void UART_RegWrite
(
    uint8                               ucCh,
    uint32                              uiAddr,
    uint32                              uiSetValue
);

static SALRetCode_t UART_Reset
(
    uint8                               ucCh
);

static sint32 UART_SetGpio
(
    uint8                               ucCh,
    const UartBoardPort_t *             psInfo
);

static sint32 UART_SetPortConfig
(
    uint8                               ucCh,
    uint32                              uiPort
);

static SALRetCode_t UART_ClearGpio
(
    uint8                               ucCh
);

static sint32 UART_SetChannelConfig
(
    uint8                               ucCh,
    uint32                              uiBaud,
    uint8                               ucWordLength,
    uint8                               ucFIFO,
    uint8                               uc2StopBit,
    uint8                               ucParity
);

static sint32 UART_SetBaudRate
(
    uint8                               ucCh,
    uint32                              uiBaud
);

static void UART_StatusInit
(
    uint8                               ucCh
);

static sint32 UART_Probe
(
    uint8                               ucCh,
    uint32                              uiPriority,
    uint32                              uiBaudrate,
    uint8                               ucMode,
    uint8                               ucCtsRts,
    uint8                               ucPortCfg,
    uint8                               ucWordLength,
    uint8                               ucFIFO,
    uint8                               uc2StopBit,
    uint8                               ucParity,
    GICIsrFunc                          fnCallback
);

static sint32 UART_Rx
(
    uint8                               ucCh
);

static sint32 UART_TxChar
(
    uint8                               ucCh,
    uint8                               cChar
);

static sint32 UART_Tx
(
    uint8                               ucCh
);

static void UART_DmaRxIrq
(
    uint8                               ucCh
);

static void UART_EnableInterrupt
(
    uint8                               ucCh,
    uint32                              uiPriority,
    uint8                               ucFIFO,
    GICIsrFunc                          FnCallback
);

static void UART_DisableInterrupt
(
    uint8                               ucCh
);

static void UART_InterruptTxProbe
(
    uint8                               ucCh
);

static void UART_InterruptRxProbe
(
    uint8                               ucCh
);

static sint32 UART_InterruptWrite
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
);

static sint32 UART_InterruptRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
);

static void UART_DmaTxProbe
(
    uint8                               ucCh,
    uint32 *                            puiAddr
);

static void UART_DmaRxProbe
(
    uint8                               ucCh,
    uint32 *                            puiAddr
);

static sint32 UART_DmaTxEnable
(
    uint8                               ucCh,
    uint32                              uiSize,
    const UDMAInformation_t *           psDmacon
);

static sint32 UART_DmaRxEnable
(
    uint8                               ucCh,
    uint32                              uiSize,
    const UDMAInformation_t *           psDmacon
);

static sint32 UART_DmaWrite
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
);

static sint32 UART_DmaRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
);

static sint32 UART_DmaRxTriggerDma
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static uint32 UART_RegRead
(
    uint8                               ucCh,
    uint32                              uiAddr
)
{
    uint32 uiRet;
    uint32 uiRegAddr;

    uiRet       = 0;
    uiRegAddr   = uart[ucCh].sBase + uiAddr;
    uiRet       = SAL_ReadReg(uiRegAddr);

    return uiRet;

}

static void UART_RegWrite
(
    uint8                               ucCh,
    uint32                              uiAddr,
    uint32                              uiSetValue
)
{
    uint32 uiRegAddr;

    uiRegAddr   = uart[ucCh].sBase + uiAddr;
    SAL_WriteReg(uiSetValue, uiRegAddr);

    return;
}

/*
***************************************************************************************************
*                                          UART_Reset
*
* Function to reset UART Channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t UART_Reset
(
    uint8                               ucCh
)
{
    sint32  ret;
    sint32  iClkBusId;

    ret = 0;
    iClkBusId   = (sint32)CLOCK_IOBUS_UART0 + (sint32)ucCh;

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

static sint32 UART_SetGpio
(
    uint8                               ucCh,
    const UartBoardPort_t *             psInfo
)
{
    //void    *portcfg = (void *)UARTPORTCFG; //green hills compile error
    uint32          portcfg;
    uint32          portcfg_val;
    sint32          ret;
    SALRetCode_t    ret1;
    SALRetCode_t    ret2;
    SALRetCode_t    ret3;
    SALRetCode_t    ret4;
    uint32          reg_byte;

    portcfg = UARTPORTCFG;
    ret     = -2;

    if (psInfo != NULL_PTR)
    {
        if(ucCh > 3U)
        {
            portcfg += 0x04U; //PORT_SEL1
        }

        reg_byte = ucCh & 0x03U;

        /* set portcfg */
        portcfg_val = SAL_ReadReg(portcfg);
        portcfg_val = (portcfg_val & ~((uint32)0xFFUL << (reg_byte * 8UL))) | (psInfo->bPortCfg << (reg_byte * 8UL));
        SAL_WriteReg(portcfg_val, portcfg);

        /* set debug port */
        ret1 = GPIO_Config(psInfo->bPortTx, (psInfo->bPortFs)); // TX
        ret2 = GPIO_Config(psInfo->bPortRx, (psInfo->bPortFs | GPIO_INPUT | GPIO_INPUTBUF_EN)); // RX

        uart[ucCh].sPort.bPortCfg = psInfo->bPortCfg;
        uart[ucCh].sPort.bPortTx = psInfo->bPortTx;
        uart[ucCh].sPort.bPortRx = psInfo->bPortRx;
        uart[ucCh].sPort.bPortFs = psInfo->bPortFs;

        if (uart[ucCh].sCtsRts != 0UL)
        {
            ret3 = GPIO_Config(psInfo->bPortRts, psInfo->bPortFs); // RTS
            ret4 = GPIO_Config(psInfo->bPortCts, psInfo->bPortFs); // CTS

            if((ret1 != SAL_RET_SUCCESS) || (ret2 != SAL_RET_SUCCESS) || (ret3 != SAL_RET_SUCCESS) || (ret4 != SAL_RET_SUCCESS))
            {
                ret = -1;
            }
            else
            {
                uart[ucCh].sPort.bPortCts = psInfo->bPortCts;
                uart[ucCh].sPort.bPortRts = psInfo->bPortRts;
            }
        }
        else if((ret1 != SAL_RET_SUCCESS) || (ret2 != SAL_RET_SUCCESS))
        {
            ret = -1;
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static sint32 UART_SetPortConfig
(
    uint8                               ucCh,
    uint32                              uiPort
)
{
    uint32  idx;
    sint32  ret;
    static const UartBoardPort_t board_serial[UART_PORT_TBL_SIZE] =
    {
#if 0
        /* Coretex-R5 is not allowed to use the GPIO_SD0-2 ports */
        { 0UL,  TCC_GPSD0(11UL), TCC_GPSD0(12UL), TCC_GPSD0(13UL), TCC_GPSD0(14UL), GPIO_FUNC(7UL) },
        { 1UL,  TCC_GPSD1(0UL),  TCC_GPSD1(1UL),  TCC_GPSD1(2UL),  TCC_GPSD1(3UL),  GPIO_FUNC(7UL) },
        { 2UL,  TCC_GPSD1(6UL),  TCC_GPSD1(7UL),  TCC_GPSD1(8UL),  TCC_GPSD1(9UL),  GPIO_FUNC(7UL) },
        { 3UL,  TCC_GPSD2(0UL),  TCC_GPSD2(1UL),  TCC_GPSD2(2UL),  TCC_GPSD2(3UL),  GPIO_FUNC(7UL) },
        { 4UL,  TCC_GPSD2(6UL),  TCC_GPSD2(7UL),  TCC_GPNONE,    TCC_GPNONE,    GPIO_FUNC(7UL) },
#endif
        { 5UL,  GPIO_GPA(0UL),    GPIO_GPA(1UL),    GPIO_GPA(2UL),    GPIO_GPA(3UL),    GPIO_FUNC(7UL) },
        { 6UL,  GPIO_GPA(6UL),    GPIO_GPA(7UL),    TCC_GPNONE,       TCC_GPNONE,       GPIO_FUNC(7UL) },
        { 7UL,  GPIO_GPA(12UL),   GPIO_GPA(13UL),   TCC_GPNONE,       TCC_GPNONE,       GPIO_FUNC(7UL) },
        { 8UL,  GPIO_GPA(18UL),   GPIO_GPA(19UL),   TCC_GPNONE,       TCC_GPNONE,       GPIO_FUNC(7UL) },
        { 9UL,  GPIO_GPA(24UL),   GPIO_GPA(25UL),   GPIO_GPA(26UL),   GPIO_GPA(27UL),   GPIO_FUNC(7UL) },
        { 10UL, GPIO_GPB(0UL),    GPIO_GPB(1UL),    GPIO_GPB(2UL),    GPIO_GPB(3UL),    GPIO_FUNC(7UL) },
        { 11UL, GPIO_GPB(6UL),    GPIO_GPB(7UL),    TCC_GPNONE,       TCC_GPNONE,       GPIO_FUNC(7UL) },
        { 12UL, GPIO_GPB(12UL),   GPIO_GPB(13UL),   TCC_GPNONE,       TCC_GPNONE,       GPIO_FUNC(7UL) },
        { 13UL, GPIO_GPB(19UL),   GPIO_GPB(20UL),   GPIO_GPB(21UL),   GPIO_GPB(22UL),   GPIO_FUNC(7UL) },
        { 14UL, GPIO_GPB(25UL),   GPIO_GPB(26UL),   GPIO_GPB(27UL),   GPIO_GPB(28UL),   GPIO_FUNC(7UL) },
        { 15UL, GPIO_GPC(0UL),    GPIO_GPC(1UL),    GPIO_GPC(2UL),    GPIO_GPC(3UL),    GPIO_FUNC(7UL) },
        { 16UL, GPIO_GPC(4UL),    GPIO_GPC(5UL),    GPIO_GPC(6UL),    GPIO_GPC(7UL),    GPIO_FUNC(7UL) },
        { 17UL, GPIO_GPC(8UL),    GPIO_GPC(9UL),    GPIO_GPC(10UL),   GPIO_GPC(11UL),   GPIO_FUNC(7UL) },
        { 18UL, GPIO_GPC(10UL),   GPIO_GPC(11UL),   GPIO_GPC(12UL),   GPIO_GPC(13UL),   GPIO_FUNC(8UL) },
        { 19UL, GPIO_GPC(12UL),   GPIO_GPC(13UL),   GPIO_GPC(14UL),   GPIO_GPC(15UL),   GPIO_FUNC(7UL) },
        { 20UL, GPIO_GPC(16UL),   GPIO_GPC(17UL),   GPIO_GPC(18UL),   GPIO_GPC(19UL),   GPIO_FUNC(7UL) },
        { 21UL, GPIO_GPC(20UL),   GPIO_GPC(21UL),   GPIO_GPC(22UL),   GPIO_GPC(23UL),   GPIO_FUNC(7UL) },
        { 22UL, GPIO_GPC(26UL),   GPIO_GPC(27UL),   GPIO_GPC(28UL),   GPIO_GPC(29UL),   GPIO_FUNC(7UL) },
        { 23UL, GPIO_GPG(0UL),    GPIO_GPG(1UL),    GPIO_GPG(2UL),    GPIO_GPG(3UL),    GPIO_FUNC(7UL) },
        { 24UL, GPIO_GPG(7UL),    GPIO_GPG(8UL),    GPIO_GPG(9UL),    GPIO_GPG(10UL),   GPIO_FUNC(7UL) },
        { 25UL, GPIO_GPE(5UL),    GPIO_GPG(6UL),    GPIO_GPG(7UL),    GPIO_GPG(8UL),    GPIO_FUNC(7UL) },
        { 26UL, GPIO_GPE(11UL),   GPIO_GPE(12UL),   GPIO_GPG(13UL),   GPIO_GPG(14UL),   GPIO_FUNC(7UL) },
        { 27UL, GPIO_GPE(16UL),   GPIO_GPE(17UL),   GPIO_GPG(18UL),   GPIO_GPG(19UL),   GPIO_FUNC(7UL) },
        { 28UL, GPIO_GPH(4UL),    GPIO_GPH(5UL),    TCC_GPNONE,       TCC_GPNONE,       GPIO_FUNC(7UL) },
        { 29UL, GPIO_GPH(6UL),    GPIO_GPH(7UL),    TCC_GPNONE,       TCC_GPNONE,       GPIO_FUNC(7UL) },
        { 30UL, GPIO_GPH(0UL),    GPIO_GPH(1UL),    GPIO_GPH(2UL),    GPIO_GPH(3UL),    GPIO_FUNC(7UL) },
        { 31UL, GPIO_GPMA(0UL),   GPIO_GPMA(1UL),   GPIO_GPMA(2UL),   GPIO_GPMA(3UL),   GPIO_FUNC(7UL) },
        { 32UL, GPIO_GPMA(6UL),   GPIO_GPMA(7UL),   GPIO_GPMA(8UL),   GPIO_GPMA(9UL),   GPIO_FUNC(7UL) },
        { 33UL, GPIO_GPMA(12UL),  GPIO_GPMA(13UL),  GPIO_GPMA(14UL),  GPIO_GPMA(15UL),  GPIO_FUNC(7UL) },
        { 34UL, GPIO_GPMA(18UL),  GPIO_GPMA(19UL),  GPIO_GPMA(20UL),  GPIO_GPMA(21UL),  GPIO_FUNC(7UL) },
        { 35UL, GPIO_GPMA(24UL),  GPIO_GPMA(25UL),  GPIO_GPMA(26UL),  GPIO_GPMA(27UL),  GPIO_FUNC(7UL) },
        { 36UL, GPIO_GPK(9UL),    GPIO_GPK(10UL),   GPIO_GPK(11UL),   GPIO_GPK(12UL),   GPIO_FUNC(2UL) },
        { 37UL, GPIO_GPMB(0UL),   GPIO_GPMB(1UL),   GPIO_GPMB(2UL),   GPIO_GPMB(3UL),   GPIO_FUNC(7UL) },
        { 38UL, GPIO_GPMB(6UL),   GPIO_GPMB(7UL),   GPIO_GPMB(8UL),   GPIO_GPMB(9UL),   GPIO_FUNC(7UL) },
        { 39UL, GPIO_GPMB(12UL),  GPIO_GPMB(13UL),  GPIO_GPMB(14UL),  GPIO_GPMB(15UL),  GPIO_FUNC(7UL) },
        { 40UL, GPIO_GPMB(18UL),  GPIO_GPMB(19UL),  GPIO_GPMB(20UL),  GPIO_GPMB(21UL),  GPIO_FUNC(7UL) },
        { 41UL, GPIO_GPMB(24UL),  GPIO_GPMB(25UL),  GPIO_GPMB(26UL),  GPIO_GPMB(27UL),  GPIO_FUNC(7UL) },
        { 42UL, GPIO_GPMC(0UL),   GPIO_GPMC(1UL),   GPIO_GPMC(2UL),   GPIO_GPMC(3UL),   GPIO_FUNC(7UL) },
        { 43UL, GPIO_GPMC(6UL),   GPIO_GPMC(7UL),   GPIO_GPMC(8UL),   GPIO_GPMC(9UL),   GPIO_FUNC(7UL) },
        { 44UL, GPIO_GPMC(12UL),  GPIO_GPMC(13UL),  GPIO_GPMC(14UL),  GPIO_GPMC(15UL),  GPIO_FUNC(7UL) },
        { 45UL, GPIO_GPMC(18UL),  GPIO_GPMC(19UL),  GPIO_GPMC(20UL),  GPIO_GPMC(21UL),  GPIO_FUNC(7UL) },
        { 46UL, GPIO_GPMC(24UL),  GPIO_GPMC(25UL),  GPIO_GPMC(26UL),  GPIO_GPMC(27UL),  GPIO_FUNC(7UL) }
    };

    ret = 0;
    for(idx = 0UL ; idx <= UART_PORT_CFG_MAX ; idx++)
    {
        if(board_serial[idx].bPortCfg == uiPort)
        {
            ret = UART_SetGpio(ucCh, &board_serial[idx]);
            break;
        }
    }

    return ret;
}

static SALRetCode_t UART_ClearGpio
(
    uint8                               ucCh
)
{
    uint32          gpio_Tx = 0;
    uint32          gpio_Rx = 0;
    uint32          gpio_Cts = 0;
    uint32          gpio_Rts = 0;
    uint32          portcfg;
    uint32          portcfg_val;
    uint32          reg_byte;
    SALRetCode_t    ret;


    ret = SAL_RET_SUCCESS;

    if(ucCh >= UART_CH_MAX)
    {
        return SAL_RET_FAILED;
    }

    gpio_Tx = uart[ucCh].sPort.bPortTx;
    gpio_Rx = uart[ucCh].sPort.bPortRx;

    /* Reset gpio */
    ret = GPIO_Config(gpio_Tx, GPIO_FUNC(0UL));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    ret = GPIO_Config(gpio_Rx, GPIO_FUNC(0UL));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    if(uart[ucCh].sCtsRts == ON)
    {
        gpio_Cts = uart[ucCh].sPort.bPortCts;
        gpio_Rts = uart[ucCh].sPort.bPortRts;

        ret = GPIO_Config(gpio_Cts, GPIO_FUNC(0UL));

        if(ret != SAL_RET_SUCCESS)
        {
            return SAL_RET_FAILED;
        }

        ret = GPIO_Config(gpio_Rts, GPIO_FUNC(0UL));

        if(ret != SAL_RET_SUCCESS)
        {
            return SAL_RET_FAILED;
        }
    }

    /* Reset i2c pcfg */
    portcfg = UARTPORTCFG;

    if(ucCh > 3U)
    {
        portcfg += 0x04U; //PORT_SEL1
    }

    reg_byte = ucCh & 0x03U;

    /* set portcfg */
    portcfg_val = SAL_ReadReg(portcfg);
    portcfg_val |= ((uint32)0xFFUL << (reg_byte * 8UL));
    SAL_WriteReg(portcfg_val, portcfg);

    return SAL_RET_SUCCESS;
}

static sint32 UART_SetChannelConfig
(
    uint8                               ucCh,
    uint32                              uiBaud,
    uint8                               ucWordLength,
    uint8                               ucFIFO,
    uint8                               uc2StopBit,
    uint8                               ucParity
)
{
    uint32  cr_data = 0;
    uint32  lcr_data = 0;
    sint32  ret;
    sint32  iClkBusId;
    sint32  iClkPeriId;

    /* Enable the UART controller peri clock */
    iClkBusId   = (sint32)CLOCK_IOBUS_UART0 + (sint32)ucCh;
    (void)CLOCK_SetIobusPwdn(iClkBusId, SALDisabled);
    iClkPeriId  = (sint32)CLOCK_PERI_UART0 + (sint32)ucCh;
    ret         = CLOCK_SetPeriRate(iClkPeriId, UART_DEBUG_CLK);
    (void)CLOCK_EnablePeri(iClkPeriId);

    if (ret == 0)
    {
        (void)UART_SetBaudRate(ucCh, uiBaud);

        // line control setting
        // Word Length
        lcr_data = UART_LCRH_WLEN(ucWordLength);

        // Enables FIFOs
        if (ucFIFO == ENABLE_FIFO)
        {
            lcr_data |= UART_LCRH_FEN;
        }

        // Two Stop Bits
        if (uc2StopBit == ON)
        {
            lcr_data |= UART_LCRH_STP2;
        }

        // Parity Enable
        switch (ucParity)
        {
            case PARITY_SPACE:
                lcr_data &= ~(UART_LCRH_PEN);
                break;
            case PARITY_EVEN:
                lcr_data |= ((UART_LCRH_PEN | UART_LCRH_EPS) & ~(UART_LCRH_SPS));
                break;
            case PARITY_ODD:
                lcr_data |= ((UART_LCRH_PEN & ~(UART_LCRH_EPS)) & ~(UART_LCRH_SPS));
                break;
            case PARITY_MARK:
                lcr_data |= ((UART_LCRH_PEN & ~(UART_LCRH_EPS)) | UART_LCRH_SPS);
                break;
            default:
                break;
        }

        UART_RegWrite(ucCh, UART_REG_LCRH, lcr_data);

        // control register setting
        cr_data = UART_CR_EN;
        cr_data |= UART_CR_TXE;
        cr_data |= UART_CR_RXE;

        if (uart[ucCh].sCtsRts != 0UL)
        { //brace
            cr_data |= (UART_CR_RTSEN | UART_CR_CTSEN);
        }

        UART_RegWrite(ucCh, UART_REG_CR, cr_data);
    }

    return ret;
}

static sint32 UART_SetBaudRate
(
    uint8                               ucCh,
    uint32                              uiBaud
) // (uint32 => sint32)return type mismatched
{
    uint32  u32_div;
    uint32  mod;
    uint32  brd_i;
    uint32  brd_f;
    uint32  pclk;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        // Read the peri clock
        pclk = CLOCK_GetPeriRate((sint32)CLOCK_PERI_UART0 + (sint32)ucCh);

        if (pclk == 0UL)
        {
            ret = -1;
        }
        else
        {
            // calculate integer baud rate divisor
            u32_div = 16UL * uiBaud;
            brd_i   = pclk / u32_div;
            UART_RegWrite(ucCh, UART_REG_IBRD, brd_i);

            // calculate faction baud rate divisor
            // NOTICE : fraction maybe need sampling
            mod     = pclk % (16UL * uiBaud);
            u32_div = ((((1UL << 3UL) * 16UL) * mod) / (16UL * uiBaud));
            brd_f   = u32_div / 2UL;
            UART_RegWrite(ucCh, UART_REG_FBRD, brd_f);
            ret = (sint32)SAL_RET_SUCCESS;
        }
    }
    return ret;
}

static void UART_StatusInit
(
    uint8                               ucCh
)
{
    //uart[ucCh].sIsProbed                = 0;
    uart[ucCh].sBase                    = UART_GET_BASE(ucCh);
    uart[ucCh].sCh                      = ucCh;
    uart[ucCh].sOpMode                  = UART_POLLING_MODE;
    uart[ucCh].sCtsRts                  = 0;
    uart[ucCh].sWordLength              = 0;
    uart[ucCh].s2StopBit                = 0;
    uart[ucCh].sParity                  = 0;

    /* Interupt mode init */
    uart[ucCh].sTxIntr.iXmitBuf         = NULL;
    uart[ucCh].sTxIntr.iHead            = -1;
    uart[ucCh].sTxIntr.iTail            = -1;
    uart[ucCh].sTxIntr.iSize            = 0;
    uart[ucCh].sRxIntr.iXmitBuf         = NULL;
    uart[ucCh].sRxIntr.iHead            = -1;
    uart[ucCh].sRxIntr.iTail            = -1;
    uart[ucCh].sRxIntr.iSize            = 0;

    /* DMA mode init */
    uart[ucCh].sTxDma.iCon              = 0xFF;
    uart[ucCh].sTxDma.iCh               = 0xFF;
    uart[ucCh].sTxDma.iSrcAddr          = NULL;
    uart[ucCh].sTxDma.iDestAddr         = NULL;
    uart[ucCh].sTxDma.iBufSize          = UDMA_BUFF_SIZE;
    uart[ucCh].sTxDma.iTransSize        = 0;
    uart[ucCh].sTxDma.iIsRun            = 0;
    uart[ucCh].sRxDma.iCon              = 0xFF;
    uart[ucCh].sRxDma.iCh               = 0xFF;
    uart[ucCh].sRxDma.iSrcAddr          = NULL;
    uart[ucCh].sRxDma.iDestAddr         = NULL;
    uart[ucCh].sRxDma.iBufSize          = UDMA_BUFF_SIZE;
    uart[ucCh].sRxDma.iTransSize        = 0;
    uart[ucCh].sRxDma.iIsRun            = 0;
}

static sint32 UART_Probe
(
    uint8                               ucCh,
    uint32                              uiPriority,
    uint32                              uiBaudrate,
    uint8                               ucMode,
    uint8                               ucCtsRts,
    uint8                               ucPortCfg,
    uint8                               ucWordLength,
    uint8                               ucFIFO,
    uint8                               uc2StopBit,
    uint8                               ucParity,
    GICIsrFunc                          fnCallback
)
{
    static uint32 * dma_tx_buf;
    static uint32 * dma_rx_buf;
    uint32          uiDmaRxAddr;
    uint32          uiDmaTxAddr;
    sint32          ret;

    ret = -1;

    if ((ucCh < UART_CH_MAX) && (uart[ucCh].sIsProbed == OFF))
    {
        uart[ucCh].sOpMode = ucMode;
        uart[ucCh].sCtsRts = ucCtsRts;
        uart[ucCh].sWordLength = ucWordLength;
        uart[ucCh].s2StopBit = uc2StopBit;
        uart[ucCh].sParity = ucParity;

        // Set port config
        ret = UART_SetPortConfig(ucCh, ucPortCfg);

        if (ret != -1)
        {
            ret = UART_SetChannelConfig(ucCh, uiBaudrate, ucWordLength, ucFIFO, uc2StopBit, ucParity);

            if (ret != -1)
            {
                // Configure for interrupt mode
                if (uart[ucCh].sOpMode == UART_INTR_MODE)
                {
                    UART_InterruptTxProbe(ucCh);
                    UART_InterruptRxProbe(ucCh);
                    UART_EnableInterrupt(ucCh, uiPriority, ucFIFO, fnCallback);
                }
                else if (uart[ucCh].sOpMode == UART_DMA_MODE)
                {
                    uiDmaRxAddr = MPU_GetDMABaseAddress() + UDMA_ADDRESS_UNIT_CH_RX(ucCh);
                    uiDmaTxAddr = MPU_GetDMABaseAddress() + UDMA_ADDRESS_UNIT_CH_TX(ucCh);
                    dma_rx_buf  = (uint32 *)(uiDmaRxAddr);
                    dma_tx_buf  = (uint32 *)(uiDmaTxAddr);

                    (void)SAL_MemSet((void *)dma_rx_buf, 0, UDMA_BUFF_SIZE);
                    (void)SAL_MemSet((void *)dma_tx_buf, 0, UDMA_BUFF_SIZE);
                    UART_DmaTxProbe(ucCh, dma_tx_buf);
                    UART_DmaRxProbe(ucCh, dma_rx_buf);
                    UART_EnableInterrupt(ucCh, uiPriority, ucFIFO, fnCallback);
                }
                else
                {
	                //UART_POLLING_MODE
                }

                uart[ucCh].sIsProbed = ON;
            }
        }
    }

    return ret;
}

static sint32 UART_Rx
(
    uint8                               ucCh
)
{
    uint32  status;
    uint32  max_count;
    uint32  data;
    uint8 * buf;
    sint32  ret;

    max_count   = UART_BUFF_SIZE;
    buf         = NULL;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        buf = uart[ucCh].sRxIntr.iXmitBuf;

        while(max_count > 0UL)
        {
            status = UART_RegRead(ucCh, UART_REG_FR);

            if((status & UART_FR_RXFE) != 0UL)
            {
                break;
            }

            data = UART_RegRead(ucCh, UART_REG_DR);

            buf[uart[ucCh].sRxIntr.iHead] = (uint8)(data & 0xFFUL);
            uart[ucCh].sRxIntr.iHead++;

            if(uart[ucCh].sRxIntr.iHead >= uart[ucCh].sRxIntr.iSize)
            {
                uart[ucCh].sRxIntr.iHead = 0;
            }

            max_count--;
        }

        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static sint32 UART_TxChar
(
    uint8                               ucCh,
    uint8                               cChar
)
{
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_TXFF) != 0UL)
        {
            ret = -1;
        }
        else
        {
            UART_RegWrite(ucCh, UART_REG_DR, cChar);
            ret = (sint32)SAL_RET_SUCCESS;
        }
    }

    return ret;
}

static sint32 UART_Tx
(
    uint8                               ucCh
)
{
    uint8 * buf;
    sint32  ret;

    buf = NULL;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        buf = uart[ucCh].sTxIntr.iXmitBuf;

        // xmit buffer is empty
        if (uart[ucCh].sTxIntr.iHead == uart[ucCh].sTxIntr.iTail)
        {
            UART_RegWrite(ucCh, UART_REG_ICR, UART_INT_TXIS);
        }
        else
        {
            do
            {
                if (UART_TxChar(ucCh, buf[(uint32)uart[ucCh].sTxIntr.iTail]) != 0)
                {
                    break;
                }

                uart[ucCh].sTxIntr.iTail++;

                if (uart[ucCh].sTxIntr.iTail >= uart[ucCh].sTxIntr.iSize)
                {
                    uart[ucCh].sTxIntr.iTail = 0;
                }
            } while (uart[ucCh].sTxIntr.iHead != uart[ucCh].sTxIntr.iTail);
        }

        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static void UART_DmaRxIrq
(
    uint8                               ucCh
)
{
    uint32  dmacr;

    if (ucCh < UART_CH_MAX)
    {
        dmacr   = UART_RegRead(ucCh, UART_REG_DMACR);
        dmacr   &= ~UART_DMACR_RXDMAE;
        UART_RegWrite(ucCh, UART_REG_DMACR, dmacr);

        uart[ucCh].sRxDma.iIsRun = 0;

        UART_RegWrite(ucCh, UART_REG_ICR, UART_INT_OEIS | UART_INT_BEIS | UART_INT_PEIS | UART_INT_FEIS);

        (void)UART_DmaRxTriggerDma(ucCh);

        uart[ucCh].sRxDma.iIsRun = 0;
    }
}

static void UART_EnableInterrupt
(
    uint8                               ucCh,
    uint32                              uiPriority,
    uint8                               ucFIFO,
    GICIsrFunc                          fnCallback
)
{
    uint32  i;
    uint32  im = 0UL;

    if (ucCh < UART_CH_MAX)
    {
        (void)GIC_IntVectSet((uint32)GIC_UART0 + ucCh, uiPriority, GIC_INT_TYPE_LEVEL_HIGH, fnCallback, &(uart[ucCh]));
        (void)GIC_IntSrcEn((uint32)GIC_UART0 + ucCh);

        UART_RegWrite(ucCh, UART_REG_ICR, UART_INT_RXIS | UART_INT_TXIS | UART_INT_RTIS);

        for (i = 0UL ; i < (UART_RX_FIFO_SIZE * 2UL) ; ++i)
        {
            if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFF) != 0UL)
            {
                break;
            }

            (void)UART_RegRead(ucCh, UART_REG_DR);
        }

        if (ucFIFO == ENABLE_FIFO)
        {
            im = UART_INT_RTIS;
        }

        if (uart[ucCh].sRxDma.iIsRun == 0UL)
        {
            im |= UART_INT_RXIS;
        }

        UART_RegWrite(ucCh, UART_REG_IMSC, im);
    }
}

static void UART_DisableInterrupt
(
    uint8                               ucCh
)
{
    if (ucCh < UART_CH_MAX)
    {
        (void)GIC_IntVectSet((uint32)GIC_UART0 + ucCh, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, NULL_PTR, NULL_PTR);
        (void)GIC_IntSrcDis((uint32)GIC_UART0 + ucCh);

        UART_RegWrite(ucCh, UART_REG_ICR, 0x7FF);
    }
}

static void UART_InterruptTxProbe
(
    uint8                               ucCh
)
{
    if (ucCh < UART_CH_MAX)
    {
        uart[ucCh].sTxIntr.iXmitBuf = uart_buff[ucCh][UART_MODE_TX];
        uart[ucCh].sTxIntr.iSize    = (sint32)UART_BUFF_SIZE;
        uart[ucCh].sTxIntr.iHead    = 0;
        uart[ucCh].sTxIntr.iTail    = 0;
    }
}

static void UART_InterruptRxProbe
(
    uint8                               ucCh
)
{
    if (ucCh < UART_CH_MAX)
    {
        uart[ucCh].sRxIntr.iXmitBuf = uart_buff[ucCh][UART_MODE_RX];
        uart[ucCh].sRxIntr.iSize    = (sint32)UART_BUFF_SIZE;
        uart[ucCh].sRxIntr.iHead    = 0;
        uart[ucCh].sRxIntr.iTail    = 0;
    }
}

static sint32 UART_InterruptWrite
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
)
{
    uint32  i;
    uint32  imsc;
    sint32  ret;

    if (pucBuf != NULL_PTR)
    {
        // copy user buffer to tx buffer
        for(i = 0 ; i < uiSize ; i++)
        {
            uart[ucCh].sTxIntr.iXmitBuf[uart[ucCh].sTxIntr.iHead] = pucBuf[i];
            uart[ucCh].sTxIntr.iHead++;

            if(uart[ucCh].sTxIntr.iHead >= uart[ucCh].sTxIntr.iSize)
            {
                uart[ucCh].sTxIntr.iHead = 0;
            }
        }

        ret = UART_Tx(ucCh);

        if (ret == (sint32)SAL_RET_SUCCESS)
        {
            imsc = UART_RegRead(ucCh, UART_REG_IMSC);
            imsc |= UART_INT_TXIS;
            UART_RegWrite(ucCh, UART_REG_IMSC, imsc);
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static sint32 UART_InterruptRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
)
{
    sint32  cnt;
    sint32  i;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        if(uart[ucCh].sRxIntr.iHead > uart[ucCh].sRxIntr.iTail)
        {
            cnt = uart[ucCh].sRxIntr.iHead - uart[ucCh].sRxIntr.iTail;
        }
        else if (uart[ucCh].sRxIntr.iHead < uart[ucCh].sRxIntr.iTail)
        {
            cnt = uart[ucCh].sRxIntr.iSize - uart[ucCh].sRxIntr.iTail;
        }
        else
        {
            cnt = 0;
        }

        if (cnt > (sint32)uiSize)
        {
            cnt = (sint32)uiSize;
        }

        // copy rx buffer to user buffer
        for(i = 0 ; i < cnt ; i++)
        {
            pucBuf[i] = uart[ucCh].sRxIntr.iXmitBuf[uart[ucCh].sRxIntr.iTail];
            uart[ucCh].sRxIntr.iTail++;

            if(uart[ucCh].sRxIntr.iTail >= uart[ucCh].sRxIntr.iSize)
            {
                uart[ucCh].sRxIntr.iTail = 0;
            }
        }

        ret = cnt;
    }

    return ret;
}

static void UART_DmaTxProbe
(
    uint8                               ucCh,
    uint32 *                            puiAddr
)
{
    uint32 regData;

    if (ucCh < UART_CH_MAX)
    {
        uart[ucCh].sTxDma.iCon          = ucCh;
        uart[ucCh].sTxDma.iCh           = (uint32)UDMA_PERI_TX;
        uart[ucCh].sTxDma.iSrcAddr      = (uint8 *)(puiAddr);
        uart[ucCh].sTxDma.iDestAddr     = (uint8 *)(uart[ucCh].sBase);
        uart[ucCh].sTxDma.iBufSize      = UDMA_BUFF_SIZE;
        uart[ucCh].sTxDma.iTransSize    = 0;

        (void)UDMA_Init(&uart[ucCh].sTxDma, GIC_PRIORITY_NO_MEAN);

        // Enable Transmit DMA
        regData = (UART_RegRead(ucCh, UART_REG_DMACR) | UART_DMACR_DMAONERR | UART_DMACR_TXDMAE);
        UART_RegWrite(ucCh, UART_REG_DMACR, regData);

        UDMA_SetFlowControl(&uart[ucCh].sTxDma, UDMA_FLOW_TYPE_M2P);
        UDMA_SetAddrIncrement(&uart[ucCh].sTxDma, UDMA_NO_INC, UDMA_INC);
        UDMA_SetBurstSize(&uart[ucCh].sTxDma, UDMA_BURST_SIZE_1, UDMA_BURST_SIZE_1);

        UDMA_SetPeri(&uart[ucCh].sTxDma, (uint8)UDMA_PERI_TX, 0U);

        (void)UART_DmaTxEnable(ucCh, uart[ucCh].sTxDma.iBufSize, (const UDMAInformation_t *)&uart[ucCh].sTxDma);
    }
}

static void UART_DmaRxProbe
(
    uint8                               ucCh,
    uint32 *                            puiAddr
)
{
    uint32 regData;

    if (ucCh < UART_CH_MAX)
    {
        uart[ucCh].sRxDma.iCon          = ucCh;
        uart[ucCh].sRxDma.iCh           = (uint32)UDMA_PERI_RX;
        uart[ucCh].sRxDma.iSrcAddr      = (uint8 *)(uart[ucCh].sBase);
        uart[ucCh].sRxDma.iDestAddr     = (uint8 *)(puiAddr);
        uart[ucCh].sRxDma.iBufSize      = UDMA_BUFF_SIZE;
        uart[ucCh].sRxDma.iTransSize    = 0UL;

        (void)UDMA_Init(&uart[ucCh].sRxDma, GIC_PRIORITY_NO_MEAN);

        // Enable Receive DMA
        regData = (UART_RegRead(ucCh, UART_REG_DMACR) | UART_DMACR_DMAONERR | UART_DMACR_RXDMAE);
        UART_RegWrite(ucCh, UART_REG_DMACR, regData);

        UDMA_SetFlowControl(&uart[ucCh].sRxDma, UDMA_FLOW_TYPE_P2M);
        UDMA_SetAddrIncrement(&uart[ucCh].sRxDma, UDMA_INC, UDMA_NO_INC);
        UDMA_SetBurstSize(&uart[ucCh].sRxDma, UDMA_BURST_SIZE_1, UDMA_BURST_SIZE_1);

        UDMA_SetPeri(&uart[ucCh].sRxDma, 0U, (uint8)UDMA_PERI_RX);

        (void)UART_DmaRxEnable(ucCh, uart[ucCh].sRxDma.iBufSize, (const UDMAInformation_t *)&uart[ucCh].sRxDma);
    }
}

static sint32 UART_DmaTxEnable
(
    uint8                               ucCh,
    uint32                              uiSize,
    const UDMAInformation_t *           psDmacon
)
{
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        uart[ucCh].sTxDma.iSrcAddr  = (uint8 *)(psDmacon->iSrcAddr);
        uart[ucCh].sTxDma.iDestAddr = (uint8 *)(psDmacon->iDestAddr);
        UDMA_SetSrcAddr(&uart[ucCh].sTxDma, (uint32)(uart[ucCh].sTxDma.iSrcAddr));
        UDMA_SetDestAddr(&uart[ucCh].sTxDma, (uint32)(uart[ucCh].sTxDma.iDestAddr));
        UDMA_InterruptEnable(&uart[ucCh].sTxDma);
        UDMA_SetTransferWidth(&uart[ucCh].sTxDma, UDMA_TRANSFER_SIZE_BYTE, UDMA_TRANSFER_SIZE_BYTE);
        UDMA_SetTransferSize(&uart[ucCh].sTxDma, uiSize);
        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static sint32 UART_DmaRxEnable
(
    uint8                               ucCh,
    uint32                              uiSize,
    const UDMAInformation_t *           psDmacon
)
{
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        uart[ucCh].sRxDma.iSrcAddr  = (uint8 *)(psDmacon->iSrcAddr);
        uart[ucCh].sRxDma.iDestAddr = (uint8 *)(psDmacon->iDestAddr);
        UDMA_SetSrcAddr(&uart[ucCh].sRxDma, (uint32)(uart[ucCh].sRxDma.iSrcAddr));
        UDMA_SetDestAddr(&uart[ucCh].sRxDma, (uint32)(uart[ucCh].sRxDma.iDestAddr));
        UDMA_InterruptEnable(&uart[ucCh].sRxDma);
        UDMA_SetTransferWidth(&uart[ucCh].sRxDma, UDMA_TRANSFER_SIZE_BYTE, UDMA_TRANSFER_SIZE_BYTE);
        UDMA_SetTransferSize(&uart[ucCh].sRxDma, uiSize);

        // Run DMA
        UDMA_ChannelEnable(&uart[ucCh].sRxDma);
        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

static sint32 UART_DmaWrite
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
)
{
    uint32  i;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        if (uiSize > UDMA_BUFF_SIZE)
        {
            ret = -1;
        }
        else
        {
            for (i = 0; i < uiSize; i++)
            {
                uart[ucCh].sTxDma.iSrcAddr[i] = pucBuf[i];
            }

            UDMA_SetSrcAddr(&uart[ucCh].sTxDma, (uint32)(uart[ucCh].sTxDma.iSrcAddr));
            UDMA_SetDestAddr(&uart[ucCh].sTxDma, (uint32)(uart[ucCh].sTxDma.iDestAddr));
            UDMA_InterruptEnable(&uart[ucCh].sTxDma);
            UDMA_SetTransferWidth(&uart[ucCh].sTxDma, UDMA_TRANSFER_SIZE_BYTE, UDMA_TRANSFER_SIZE_BYTE);
            UDMA_SetTransferSize(&uart[ucCh].sTxDma, uiSize);

            // Run DMA
            UDMA_ChannelEnable(&uart[ucCh].sTxDma);
            ret = (sint32)SAL_RET_SUCCESS;
        }
    }

    return ret;
}

static sint32 UART_DmaRead
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32 uiSize
)
{
    uint32  cnt;
    uint32  post_cnt;
    uint32  i;
    uint32  num_brige;
    uint32  prev_buf;
    uint32  c_size;
    uint32  u_size;
    sint32  ret;

    cnt         = 0;
    post_cnt    = 0;
    num_brige   = 0;
    prev_buf    = 0;
    c_size      = 0;
    u_size      = uiSize;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        c_size = UDMA_GetTransferSize(&uart[ucCh].sRxDma);

        if (uart[ucCh].sRxDma.iIsRun != 0UL)
        {
            ret = -1;
        }
        else if(uart[ucCh].sRxDma.iTransSize != c_size)
        {
            if(uart[ucCh].sRxDma.iTransSize > c_size)
            {
                cnt         = uart[ucCh].sRxDma.iTransSize - c_size;
                prev_buf    = uart[ucCh].sRxDma.iBufSize - uart[ucCh].sRxDma.iTransSize;

                if(u_size >= cnt)
                {
                    for (i = 0 ; i < cnt ; i++)
                    {
                        pucBuf[i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[prev_buf+i]);
                    }

                    uart[ucCh].sRxDma.iTransSize = c_size;
                }
                else //else if(u_size < cnt)
                {
                    for (i = 0 ; i < u_size ; i++)
                    {
                        pucBuf[i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[prev_buf+i]);
                    }

                    uart[ucCh].sRxDma.iTransSize -=  u_size;
                    cnt = u_size;
                }
                ret = (sint32)cnt;
            }
            else if(c_size > uart[ucCh].sRxDma.iTransSize)
            {
                prev_buf    = uart[ucCh].sRxDma.iBufSize - uart[ucCh].sRxDma.iTransSize;
                cnt         = uart[ucCh].sRxDma.iBufSize - c_size;
                post_cnt    = uart[ucCh].sRxDma.iTransSize;

                if(u_size > post_cnt)
                {
                    u_size  = u_size - post_cnt;

                    for(i= 0UL ; i < uart[ucCh].sRxDma.iTransSize ; i++)
                    {
                        pucBuf[i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[prev_buf + i]);
                        num_brige=i+1UL;
                    }

                    if(u_size >= cnt)
                    {
                        for(i = 0UL ; i < cnt ; i++)
                        {
                            pucBuf[num_brige+i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[i]);
                        }

                        uart[ucCh].sRxDma.iTransSize = c_size;
                        cnt += post_cnt;
                    }
                    else //else if(u_size < cnt)
                    {
                        for(i = 0UL ; i < u_size ; i++)
                        {
                            pucBuf[num_brige+i] = (uint8)(uart[ucCh].sRxDma.iDestAddr[i]);
                        }

                        uart[ucCh].sRxDma.iTransSize =  uart[ucCh].sRxDma.iBufSize - u_size;
                        cnt = u_size + post_cnt;
                    }
                }
                else
                {
                    for(i= 0UL ; i < u_size ; i++)
                    {
                        pucBuf[i]   = (uint8)(uart[ucCh].sRxDma.iDestAddr[prev_buf + i]);
                        num_brige   = i + 1UL;
                    }

                    uart[ucCh].sRxDma.iTransSize -= u_size;
                    cnt = u_size;
                }

                ret = (sint32)cnt;
            }
            else
            {
                ret = 0;// empty statement
            }
        }
        else
        {
            ret = 0;// empty statement
        }
    }

    return ret;
}

static sint32 UART_DmaRxTriggerDma
(
    uint8                               ucCh
)
{
    uint32  dmacr;
    uint32  im;
    sint32  ret;

    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
    {
        dmacr = UART_RegRead(ucCh, UART_REG_DMACR);
        dmacr |= UART_DMACR_RXDMAE;
        UART_RegWrite(ucCh, UART_REG_DMACR, dmacr);

        uart[ucCh].sRxDma.iIsRun = 1;

        im = UART_RegRead(ucCh, UART_REG_IMSC);
        im |= ~UART_INT_RXIS;
        UART_RegWrite(ucCh, UART_REG_IMSC, im);

        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

/*
***************************************************************************************************
*                                         INTERFACE FUNCTIONS
***************************************************************************************************
*/

sint32 UART_Open
(
    uint8                               ucCh,
    uint32                              uiPriority,     // Interrupt Priority
    uint32                              uiBaudrate,     // Baudrate
    uint8                               ucMode,         // polling or interrupt or udma
    uint8                               ucCtsRts,       // on/off
    uint8                               ucPortCfg,      // port selection
    uint8                               ucWordLength,   // 5~6 bits
    uint8                               ucFIFO,         // on/off
    uint8                               uc2StopBit,     // on/off
    uint8                               ucParity,       // space, even, odd, mark
    GICIsrFunc                          fnCallback      // callback function
)
{
    SNORRomInfo_t   *rom_header;
    uint32          header_addr;
    uint8           port_cfg;   //config of debug port
    sint32          ret = -1;

    UART_StatusInit(ucCh);

    rom_header  = (SNORRomInfo_t *)MCU_BSP_SNOR_BASE;
    port_cfg  = 0;

    if (ucCh == UART_DEBUG_CH)
    {
        // Find Debug port from special location of SNOR Flash.
        if (rom_header->riRomId != SNOR_ROM_ID)
        {
            /* Secondary SNOR ROM Header */
            header_addr = (MCU_BSP_SNOR_BASE + 0x1000UL);
            rom_header  = (SNORRomInfo_t *)(header_addr);

            if (rom_header->riRomId != SNOR_ROM_ID)
            {
                port_cfg = ucPortCfg;
            }
            else
            {
                port_cfg = (uint8)rom_header->riDebugPort;
            }
        }
        else
        {
            port_cfg = (uint8)rom_header->riDebugPort;
        }
    }
    else if (ucCh < UART_CH_MAX)
    {
        port_cfg = ucPortCfg;
    }
    else
    {
        port_cfg = 0xFF;    //Error
    }

    if (port_cfg <= UART_PORT_CFG_MAX)
    {
        ret = UART_Probe(ucCh, uiPriority, uiBaudrate, ucMode, ucCtsRts, port_cfg,
                        ucWordLength, ucFIFO, uc2StopBit, ucParity, fnCallback);
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }

    return ret;
}

void UART_Close
(
    uint8                              ucCh
)
{
    sint32          iClkBusId;
    SALRetCode_t    ret;

    if (ucCh < UART_CH_MAX)
    {
        /* Disable the UART Interrupt */
        UART_DisableInterrupt(ucCh);

        /* Disable the UART controller Bus clock */
        iClkBusId   = (sint32)CLOCK_IOBUS_UART0 + (sint32)ucCh;
        (void)CLOCK_SetIobusPwdn(iClkBusId, TRUE);

        if (uart[ucCh].sOpMode == UART_DMA_MODE)
        {
            /* Disable the UDMA controller Bus clock */
            iClkBusId   = (sint32)CLOCK_IOBUS_UDMA0 + (sint32)ucCh;
            (void)CLOCK_SetIobusPwdn(iClkBusId, TRUE);
        }

        ret = UART_ClearGpio(ucCh);

        if (ret != SAL_RET_SUCCESS)
        {
            UART_D("%s ch %d failed to reset port\n", __func__, ucCh);
        }

        /* Initialize UART Structure */
        SAL_MemSet(&uart[ucCh], 0, sizeof(UartStatus_t));

        /* UART SW Reset */
        UART_Reset(ucCh);
        if (uart[ucCh].sOpMode == UART_DMA_MODE)
        {
            UDMA_Reset(ucCh);
        }
    }
    else
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
}


sint32 UART_Read
(
    uint8                               ucCh,
    uint8 *                             pucBuf,
    uint32                              uiSize
)
{
    uint32  i;
    sint32  ret;
    sint8   getc_err;

    ret = -1;

    if(pucBuf != NULL_PTR)
    {
        if(OFF != uart[ucCh].sIsProbed)
        {
            if (ucCh >= UART_CH_MAX)
            {
                (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
            }

            if (uart[ucCh].sOpMode == UART_DMA_MODE)
            {
                ret = UART_DmaRead(ucCh, pucBuf, uiSize);
            }
            else if (uart[ucCh].sOpMode == UART_INTR_MODE)
            {
                ret = UART_InterruptRead(ucCh, pucBuf, uiSize);
            }
            else
            {
                for (i = 0 ; i < uiSize ; i++)
                {
                    ret = UART_GetChar(ucCh, 0, (sint8 *)&getc_err);

                    if (ret >= 0)
                    {
                        pucBuf[i] = (uint8)((uint32)ret & 0xFFUL);
                    }
                    else
                    {
                        ret = (sint32)i;
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

sint32 UART_Write
(
    uint8                               ucCh,
    const uint8 *                       pucBuf,
    uint32                              uiSize
)
{
    uint32  i;
    sint32  ret;

    ret = -1;

    if (pucBuf != NULL_PTR)
    {
        if(OFF != uart[ucCh].sIsProbed)
        {
            if (ucCh >= UART_CH_MAX)
            {
                (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
            }
            else
            {
                if (uart[ucCh].sOpMode == UART_DMA_MODE)
                {
                    ret = UART_DmaWrite(ucCh, pucBuf, uiSize);
                }
                else if (uart[ucCh].sOpMode == UART_INTR_MODE)
                {
                    ret = UART_InterruptWrite(ucCh, pucBuf, uiSize);
                }
                else
                {
                    for (i = 0; i < uiSize; i++)
                    {
                        ret = UART_PutChar(ucCh, pucBuf[i]);
                    }
                }
            }
        }
    }

    return ret;
}

uint32 UART_GetData
(
    uint8                               ucCh,
    sint32                              iWait,
    sint8 *                             pcErr
)
{
    /* Use for Lin */
    uint32  data;
    uint32  ret;
    sint32  timeout;

    data    = 0;
    ret     = 0;
    timeout = 0;

    if (pcErr != NULL_PTR)
    {
        if ((ucCh >= UART_CH_MAX) || (OFF == uart[ucCh].sIsProbed))
        {
            (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
            *pcErr = (sint8)-1;
        }
        else
        {
            while ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_BUSY) != 0UL);

            while ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) != 0UL)
            {
                timeout++;

                if( timeout > iWait )
                {
                   (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_FAIL_GET_DATA, __FUNCTION__);
                    *pcErr = (sint8)-1;
                    break;
                }
            }

            if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) == 0UL)
            {
                data    = UART_RegRead(ucCh, UART_REG_DR);
                ret     = data;
            }
        }
    }

    return ret;
}

sint32 UART_GetChar
(
    uint8                               ucCh,
    sint32                              iWait,
    sint8 *                             pcErr
)
{
    uint32  data;
    sint32  ret;

    ret = -1;

    if (pcErr != NULL_PTR)
    {
        if ((ucCh >= UART_CH_MAX) || (OFF == uart[ucCh].sIsProbed))
        {
            (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
            *pcErr = (sint8)-1;
        }
        else
        {
            if(iWait == 1)
            {
                while ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) != 0UL)
                {
                    if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) == 0UL)
                    {
                        data    = UART_RegRead(ucCh, UART_REG_DR);
                        ret     = (sint32) data;
                        break;
                    }
                }
            }
            else
            {
                if((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_RXFE) != 0UL)
                {
                    *pcErr = (sint8)(-1);
                }
                else
                {
                    data    = UART_RegRead(ucCh, UART_REG_DR);
                    ret     = (sint32) data;
                }
            }
        }
    }

    return ret;
}

sint32 UART_PutChar
(
    uint8                               ucCh,
    uint8                               ucChar
)
{
    sint32  ret;

    ret = -1;

    if ((ucCh >= UART_CH_MAX) || (OFF == uart[ucCh].sIsProbed))
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
    }
    else
    {
        while ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_TXFF) != 0UL)
        {
            if ((UART_RegRead(ucCh, UART_REG_FR) & UART_FR_TXFF) == 0UL)
            {
                break;
            }
        }

        UART_RegWrite(ucCh, UART_REG_DR, ucChar);
        ret = (sint32)SAL_RET_SUCCESS;
    }

    return ret;
}

void UART_SetLineControlReg
(
    uint8                               ucCh,
    uint32                              uiBits,
    uint8                               uiEnabled
)
{
    uint32  uiLcrh;

    uiLcrh = UART_RegRead(ucCh, UART_REG_LCRH);

    if (uiEnabled == 1U)
    {
        uiLcrh = uiLcrh | uiBits;
    }
    else //(uiEnabled == 0U)
    {
        uiLcrh = uiLcrh & ~(uiBits);
    }

    UART_RegWrite(ucCh, UART_REG_LCRH, uiLcrh);
}

void UART_SetInterruptClearReg
(
    uint8                               ucCh,
    uint32                              uiSetValue
)
{
    UART_RegWrite(ucCh, UART_REG_ICR, uiSetValue);
}

void UART_SetErrorClearReg
(
    uint8                               ucCh,
    uint32                              uiSetValue
)
{
    UART_RegWrite(ucCh, UART_REG_ECR, uiSetValue);
}

uint32 UART_GetReceiveStatusReg
(
    uint8                               ucCh
)
{
    uint32  uiRet;
    uiRet = UART_RegRead(ucCh, UART_REG_RSR);

    return uiRet;
}

uint32 UART_GetRawInterruptStatusReg
(
    uint8                               ucCh
)
{
    uint32  uiRet;
    uiRet = UART_RegRead(ucCh, UART_REG_RIS);

    return uiRet;
}

void UART_Init
(
    void
)
{
#ifdef EDUCATION_CAN_DEMO_EN
    // For LOG output
    (void)UART_Open(UART_CH0, GIC_PRIORITY_NO_MEAN, 115200UL, UART_POLLING_MODE, UART_CTSRTS_OFF,
                    46U, WORD_LEN_8, ENABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, NULL_PTR);
#else
    // For LOG output
    (void)UART_Open(UART_CH0, GIC_PRIORITY_NO_MEAN, 115200UL, UART_POLLING_MODE, UART_CTSRTS_OFF,
                    46U, WORD_LEN_8, DISABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, NULL_PTR);
#endif
}

void UART_ISR
(
    void *                              pArg
)
{
    UartStatus_t *  uartStat;
    uint32          status;
    uint32          imsc;
    sint32          max_count;

    uartStat    = pArg;
    max_count   = (sint32)UART_BUFF_SIZE;

    if (uartStat != NULL_PTR)
    {
        imsc    = UART_RegRead(uartStat->sCh, UART_REG_IMSC);
        status  = UART_RegRead(uartStat->sCh, UART_REG_MIS) & imsc;

        if (status != 0UL)
        {
            do
            {
                UART_RegWrite(uartStat->sCh, UART_REG_ICR, status & ~(UART_INT_RXIS | UART_INT_RTIS | UART_INT_TXIS));

                if((status & (UART_INT_RTIS | UART_INT_RXIS)) != 0UL)
                {
                    if (uart[uartStat->sCh].sOpMode == UART_DMA_MODE)
                    {
                        UART_DmaRxIrq(uartStat->sCh);
                    }
                    else
                    {
                        (void)UART_Rx(uartStat->sCh);
                    }

                    UART_RegWrite(uartStat->sCh, UART_REG_ICR, UART_INT_RXIS | UART_INT_RTIS);
                }

                if((status & UART_INT_TXIS) != 0UL)
                {
                    (void)UART_Tx(uartStat->sCh);
                }

                if (max_count > 0)
                {
                    break;
                }

                status = UART_RegRead(uartStat->sCh, UART_REG_MIS) & imsc;
                max_count--;
            } while(status != 0UL);
        }
    }
}

