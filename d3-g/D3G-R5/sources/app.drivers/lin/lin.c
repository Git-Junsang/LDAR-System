/*
***************************************************************************************************
*
*   FileName : lin.c
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

#include <uart.h>
#include <lin.h>
#include <lin_reg.h>
#include <reg_phys.h>
#include <gpio.h>
#include <pmio.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/* LIN_READ_LENGTH = break field(1) + sync field(1) + pid(1) + data size(8) + checksum(1) */
#define LIN_READ_LENGTH                 (12U)
#define LIN_MAX_BAUDRATE                (20000U)

/* Lin Header */
#define LIN_SYNC_BREAK                  (0x00U)
#define LIN_SYNC_FIELD                  (0x55U)

/* Used to carry diagnostic and configuration data. Use only classic checksum */
#define LIN_CONF_ID1                    (0x3CU)
#define LIN_CONF_ID2                    (0x3DU)

/* UART Config for using as Lin communication*/

#define LIN_MODE                        (UART_POLLING_MODE)
#define LIN_RTSCTS_USE                  (0U)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static sint8 LIN_CalculateId
(
    uint8                               ucId,
    uint8 *                             pucPid
);

static uint8 LIN_CalClassicChecksum
(
    uint8 *                             pucData,
    uint8                               ucSize
);

static uint8 LIN_CalEnhancedChecksum
(
    uint8                               ucPid,
    uint8 *                             pucData,
    uint8                               ucSize
);

static void LIN_SendHeader
(
    LINConfig_t                         sLinCfg,
    uint8                               ucPid
);


/*
***************************************************************************************************
*                                          LIN_CalculateId
*
* @param        ucId [in] ucId is frame identifier
* @param        *pucPid [in] pucPid is protected identifier
* @return       sint8 return [out] return LIN_OK or LIN_ERROR
*
* Notes
*         Frame identifier consists of Six bits, Values in the range 0 to 63 can be used.
*
*         A protected identifier field consists of two sub-fields; the frame identifier and the parity.
*         Bits 0 to 5 are the frame identifier and bits 6 and 7 are the parity.
*
***************************************************************************************************
*/

static sint8 LIN_CalculateId
(
    uint8                               ucId,
    uint8 *                             pucPid
)
{
    sint8 cRet;
    uint8 ucP0;
    uint8 ucP1;

    cRet    = LIN_OK;
    ucP0    = 0;
    ucP1    = 0;

    if (ucId > 63UL)
    {
        LIN_E("frame id value should be from 0 to 63. current frame id is [%d] \n", ucId);
        cRet = LIN_ERROR;

        return cRet;
    }

    if( pucPid == NULL_PTR)
    {
        LIN_E("Pointer Parameter is NULL\n");
        cRet = LIN_ERROR;

        return cRet;
    }

    /* P0 = D0 + D1 + D2 + D4 */
    ucP0 = (ucId & 0x01U) + ((ucId >> 1U) & 0x01U) + ((ucId >> 2U) & 0x01U) + ((ucId >> 4U) & 0x01U);
    ucP0 = ucP0 & 0x01U;

    /* P1 = D1 + D3 + D4 + D5 */
    ucP1 = ((ucId >> 1U) & 0x01U) + ((ucId >> 3U) & 0x01U) + ((ucId >> 4U) & 0x01U) + ((ucId >> 5U) & 0x01U);
    ucP1 = ~(ucP1 & 0x01U);

    *pucPid = (uint8)(((uint32)ucP1 << 7U) | ((uint32)ucP0 << 6U) | (uint32)ucId);

    LIN_D("Frame identifier [0x%02X], Protected identifier [0x%02X] \n", ucId , *pucPid);

    return cRet;
}



/*
***************************************************************************************************
*                                          LIN_CalClassicChecksum
*
* @param        pucData [in] Send data
* @param        ucSize [in] Size of send data
* @return       uint8 [out] Classic checksum
*
* Notes
*         Checksum calculation over the data bytes only is called classic checksum and
*         it is used for the master request frame, slave response frame and
*         communication with LIN 1.x slaves.
*         Frame identifiers 60 (0x3C) to 61 (0x3D) shall always use classic checksum.
*
***************************************************************************************************
*/

static uint8 LIN_CalClassicChecksum
(
    uint8 *                             pucData,
    uint8                               ucSize
)
{
    uint8   ucI;
    uint8   ucChecksum;
    uint32  uhwTemp;

    ucI         = 0;
    ucChecksum  = 0;
    uhwTemp     = 0;

    if(pucData != NULL_PTR)
    {
        for ( ; ucI < ucSize ; ucI++)
        {
            uhwTemp += pucData[ucI];
            if (uhwTemp > 0xFFU)
            {
                uhwTemp &= 0x00FFU;
                uhwTemp += 0x0001U;
            }
        }

        ucChecksum = (uint8)(~uhwTemp);

        LIN_D("ucChecksum : 0x%02x", ucChecksum);
    }
    else
    {
        LIN_E("Receive Wrong Parameter");
    }

    return ucChecksum;
}


/*
***************************************************************************************************
*                                          LIN_CalEnhancedChecksum
*
* @param        ucPid [in] protected identifier
* @param        pucData [in] send data
* @param        ucSize [in] size of send data
* @return       uint8 [out] Enhanced checksum
*
* Notes
*         The checksum model used in the LIN specification versions starting from LIN 2.0
*         for all frames,except the diagnostic frames.
*         The enhanced checksum includes the protected identifier and the data bytes.
*
***************************************************************************************************
*/

static uint8 LIN_CalEnhancedChecksum
(
    uint8                               ucPid,
    uint8 *                             pucData,
    uint8                               ucSize
)
{
    uint8   ucI;
    uint8   ucChecksum;
    uint32  uhwTemp;

    ucI         = 0;
    ucChecksum  = 0;
    uhwTemp     = ucPid;

    if(pucData != NULL_PTR)
    {
        for ( ; ucI < ucSize ; ucI++)
        {
            uhwTemp += pucData[ucI];
            if (uhwTemp > 0xFFU)
            {
                uhwTemp &= 0x00FFU;
                uhwTemp += 0x0001U;
            }
        }

        ucChecksum = (uint8)(~uhwTemp);

        LIN_D("ucChecksum : 0x%02x", ucChecksum);
    }
    else
    {
        LIN_E("Receive Wrong Parameter");
    }

    return ucChecksum;
}


/*
***************************************************************************************************
*                                          LIN_SendHeader
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @param        ucPid [in] protected identifier
*
* Notes
*         The header starts at the falling edge of the break field and
*         ends after the end of the stop bit of the protected identifier (PID) field.
*
***************************************************************************************************
*/

void LIN_SendHeader
(
    LINConfig_t                         sLinCfg,
    uint8                               ucPid
)
{
    LIN_D("");

    /* Send Sync Break*/
    if(sLinCfg.uiBaud > 15000U)
    {
        /*Send Break, a low-level is continually output on the TXD output*/
        UART_SetLineControlReg(sLinCfg.uiLinCh, UART_LCRH_BRK, SALEnabled);
        (void)SAL_TaskSleep(1);                /* 1ms */
        UART_SetLineControlReg(sLinCfg.uiLinCh, UART_LCRH_BRK, SALDisabled);
    }
    else
    {
        UART_SetLineControlReg(sLinCfg.uiLinCh, UART_LCRH_BRK, SALEnabled);
        (void)SAL_TaskSleep(2);                /* 2ms */
        UART_SetLineControlReg(sLinCfg.uiLinCh, UART_LCRH_BRK, SALDisabled);
    }

    /* Send Sync Field */
    (void)UART_PutChar(sLinCfg.uiLinCh, LIN_SYNC_FIELD);

    /* Send Idetifier */
    (void)UART_PutChar(sLinCfg.uiLinCh, ucPid);

}

/*
***************************************************************************************************
*                                          LIN_Init
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @return       sint8 [out] LIN_OK is Success. Other value means fail
*
* Notes
*         Lin communication use UART.
*         Wake up Lin Transceiver setting Lin Sleep Pin as high.
*
***************************************************************************************************
*/

sint8 LIN_Init
(
    LINConfig_t                         sLinCfg
)
{
    sint8   cRet;

    cRet = LIN_OK;

    LIN_D("");

    /* Check valid baudrate */
    if (sLinCfg.uiBaud > LIN_MAX_BAUDRATE)
    {
        LIN_E("Allow for bit rates up to 20 kBit/sec, current bit rates [%d]bit/s", sLinCfg.uiBaud);

        return LIN_ERROR;
    }

    //UART_DecideGpioPort(sLinCfg.uiLinCh, sLinCfg.uiPortSel);
    cRet = (sint8)UART_Open(sLinCfg.uiLinCh, GIC_PRIORITY_NO_MEAN, sLinCfg.uiBaud, LIN_MODE, LIN_RTSCTS_USE, sLinCfg.uiPortSel,
                                WORD_LEN_8, DISABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, NULL_PTR);

    if (cRet != LIN_OK)
    {
        LIN_E("Fail Init Uart Port for LIN [%d]", cRet);

        return cRet;
    }

    /* If GPIO_GPK9,10 use as lin communication port, GPIO_GPK9,10 need to set as normal port */
    if (sLinCfg.uiPortSel == LIN_SEL_PMIO_PORT)
    {
        PMIO_SetGPK( PMIO_GPK(10)|PMIO_GPK(9) ); //uart set
    }

    /* Set GPIO port for controlling SLP_N */
    cRet = (sint8)GPIO_Config(sLinCfg.uiLinSlpPin, (uint32)(GPIO_FUNC(0U) | GPIO_OUTPUT));

    if (cRet != LIN_OK)
    {
        LIN_E("Fail to set LIN_SLP pin as GPIO_OUTPUT");

        return cRet;
    }

    /* Wake up lin transceiver */
    cRet = LIN_TransceiverWakeup(sLinCfg, SALEnabled);

    return cRet;
}

/*
***************************************************************************************************
*                                          LIN_SendData
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @param        ucId [in] ucId is frame identifier
* @param        pucData [in] Data for Send
* @param        ucSize [in] Size of Send Data
* @return       sint8 [out] LIN_OK is Success. Other value means fail.
*
* Notes
*
***************************************************************************************************
*/

sint8 LIN_SendData
(
    LINConfig_t                         sLinCfg,
    uint8                               ucId,
    uint8 *                             pucData,
    uint8                               ucSize
)
{
    uint8 i;
    uint8 ucProtectedId;
    uint8 ucChecksum;
    sint8 cRet;

    i             = 0;
    ucProtectedId = 0;
    ucChecksum    = 0;
    cRet          = LIN_OK;

    /* Calculate idetifier */
    cRet = LIN_CalculateId(ucId, &ucProtectedId);

    if (cRet == LIN_ERROR)
    {
        return cRet;
    }

    /* Calculate Checksum */
    if ((ucId == LIN_CONF_ID1) || (ucId == LIN_CONF_ID2))
    {
        ucChecksum = LIN_CalClassicChecksum(pucData, ucSize);
    }
    else
    {
        ucChecksum = LIN_CalEnhancedChecksum(ucId, pucData, ucSize);
    }

    /* Send Message Header */
    LIN_SendHeader(sLinCfg, ucProtectedId);

    /* Send Data Field */
    for ( i = 0 ; i < ucSize ; i++)
    {
        cRet = (sint8)UART_PutChar(sLinCfg.uiLinCh, pucData[i]);
    }

    /* Send Checksum */
    cRet = (sint8)UART_PutChar(sLinCfg.uiLinCh, ucChecksum);

    return cRet;
}

/*
***************************************************************************************************
*                                          LIN_ReadData
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @param        ucId [in] ucId is frame identifier.
* @param        pucData [out] Save read data this param.
* @return       sint8 [out] return Read Data Size. If fail to read data, return LIN_ERROR.
*
* Notes
*
***************************************************************************************************
*/

sint8 LIN_ReadData
(
    LINConfig_t                         sLinCfg,
    uint8                               ucId,
    uint8 *                             pucData
)
{
    uint8 i;
    uint8 ucProtectedId;
    sint8 cRet;

    uint8 ucRxChecksum;
    uint8 ucCalChecksum;

    uint8 ucRxSize;
    uint8 ucDataSize;
    uint8 pucRxData[LIN_READ_LENGTH] = {0,};

    i               = 0;
    ucProtectedId   = 0;
    cRet            = LIN_OK;

    ucRxChecksum    = 0;
    ucCalChecksum   = 0;

    ucRxSize        = 0;
    ucDataSize      = 0;

    /* Check Valid of Parameter */
    if(pucData == NULL_PTR)
    {
        cRet = LIN_ERROR;

        return cRet;
    }

    /* Set FEN (Enable FIFOs) */
    UART_SetLineControlReg(sLinCfg.uiLinCh, UART_LCRH_FEN, SALEnabled);

    /* Calculate idetifier */

    cRet = LIN_CalculateId(ucId, &ucProtectedId);

    if (cRet == LIN_ERROR)
    {
        return cRet;
    }

    /* Send Message Header */
    LIN_SendHeader(sLinCfg, ucProtectedId);

    /* Read data , checksum */
    for ( ; i < LIN_READ_LENGTH ; i++)
    {
        sint8 cGetErr;
        uint32 uiRxData;

        cGetErr     = LIN_OK;
        uiRxData    = UART_GetData(sLinCfg.uiLinCh, (sint32)5000, (sint8 *)&cGetErr);

        if (cGetErr == LIN_ERROR)
        {
            LIN_E("Fail Read Lin Data\n");
            break;
        }

        LIN_D("i:%d, uiRxData:0x%x\n", i, uiRxData);
        pucRxData[i] = (uint8)uiRxData;
    }

    /* unset FEN (Disable FIFOs) */
    UART_SetLineControlReg(sLinCfg.uiLinCh, UART_LCRH_FEN, SALDisabled);

    if (i > 4U)
    {
        ucRxSize        = i - 1U; /* data size without checksum */
        ucRxChecksum    = pucRxData[ucRxSize];
        ucDataSize      = ucRxSize - 3U;
    }
    else
    {
        LIN_E("Read Lin Data size must larger than 5\n");
        cRet = LIN_ERROR;

        return cRet;
    }

    /* Inspect checksum */
    if ((ucId == LIN_CONF_ID1) || (ucId == LIN_CONF_ID2))
    {
        ucCalChecksum = LIN_CalClassicChecksum(&pucRxData[3], ucDataSize);
    }
    else
    {
        ucCalChecksum = LIN_CalEnhancedChecksum(ucProtectedId, &pucRxData[3], ucDataSize);
    }

    if (ucRxChecksum != ucCalChecksum)
    {
        LIN_E("LIN Rx Checksum Error ucRxChecksum : 0x%02x, ucCalChecksum : 0x%02x", ucRxChecksum, ucCalChecksum);
        cRet = LIN_ERROR;

        return cRet;
    }

    (void) SAL_MemCopy ( (void*)pucData, (const void *)&pucRxData[3] , (SALSize) ucDataSize);

    /*return only data size*/
    return (sint8)ucDataSize;
}


/*
***************************************************************************************************
*                                          LIN_TransceiverWakeup
*
* @param        LINConfig_t [in] Lin Configuration Struct includes channel, baudrate, uart port select,
*               lin sleep pin informtation.
* @param        bStatus [in] SALEnabled : Set Lin Transceiver Wake up
*                            SALDisabled : Set Lin Transceiver Sleep
* @return       sint8 [out] return Read Data Size. If fail to read data, return LIN_ERROR.
*
* Notes
*
***************************************************************************************************
*/

sint8 LIN_TransceiverWakeup
(
    LINConfig_t                         sLinCfg,
    boolean                             bStatus
)
{
    sint8 cRet;

    cRet = LIN_OK;
    cRet = (sint8)GPIO_Set(sLinCfg.uiLinSlpPin, bStatus);

    if(cRet != LIN_OK)
    {
        LIN_E("Fail to control LIN transceiver Status [%d]", bStatus);
    }

    return cRet;
}

