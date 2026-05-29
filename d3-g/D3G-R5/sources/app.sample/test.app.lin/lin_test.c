/*
***************************************************************************************************
*
*   FileName : lin_test.c
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
#include <uart.h>
#include <lin.h>
#include <lin_test.h>
#include <gpio.h>
#include <debug.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

void LIN_TestUsage(void)
{
    mcu_printf("Lin Command Usage \n");
    mcu_printf("    : lin init \n");
    mcu_printf("    : lin send \n");
    mcu_printf("    : lin read \n");
    mcu_printf("\r\n");
}

void LIN_SampleTest(uint8 ucArgc, void * pArgv[])
{
    const uint8 *   pucStr;
    sint8           uiStatus;
    sint32          ret;
    LINConfig_t     sLinConfig;

    pucStr = NULL_PTR;
    uiStatus = LIN_OK;

    sLinConfig.uiLinCh = UART_CH2;
    sLinConfig.uiBaud = 9600U;
    sLinConfig.uiPortSel = 36U;;

#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1) //TCC8059
    sLinConfig.uiLinSlpPin = GPIO_GPK(8UL);
#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1) //TCC8050
    sLinConfig.uiLinSlpPin = GPIO_GPMC(8UL);
#endif

    //Unused Parameter
    (void)ucArgc;

    if(pArgv != NULL_PTR)
    {
        pucStr = (const uint8 *)pArgv[0];

        if(pucStr != NULL_PTR)
        {
            if ((SAL_StrNCmp(pucStr, (const uint8 *)"init", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                uiStatus = LIN_Init(sLinConfig);
                if (uiStatus != LIN_OK)
                {
                    mcu_printf("Fail LIN_Init");
                }
            }
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"send", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                uint8 uiSendId;
                uint8 ucSendSize;
                uint8 ucSendData[3] = {0xAA, 0xBB, 0xCC};

                uiSendId    = 0x11;
                ucSendSize  = 3;

                uiStatus = LIN_SendData(sLinConfig, uiSendId, ucSendData, ucSendSize);
                if (uiStatus != LIN_OK)
                {
                    mcu_printf("Fail LIN_SendData");
                }
            }
            else if ((SAL_StrNCmp(pucStr, (const uint8 *)"read", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                sint8 i;
                uint8 uiRendId;
                sint8 cReadSize;
                uint8 ucReadData[LIN_MAX_DATA_SIZE] = {0,};

                i           = 0;
                uiRendId    = 0x08;
                cReadSize   = 0;

                cReadSize = LIN_ReadData(sLinConfig, uiRendId, ucReadData);
                if (cReadSize > 0)
                {
                    mcu_printf("LIN_ReadData\n");
                    for ( ; i < cReadSize ; i++)
                    {
                        mcu_printf("[%d] :0x%x\n", i, ucReadData[i]);
                    }
                }
                else
                {
                    mcu_printf("Fail LIN_ReadData\n");
                }
            }
            else
            {
                mcu_printf("Wrong argument : %s\n", pucStr);
                LIN_TestUsage();
            }
        }
    }
}

