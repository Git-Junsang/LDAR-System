/*
***************************************************************************************************
*
*   FileName : console.c
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

#include <stdlib.h>
#include <string.h>

#include <uart.h>
#include <wdt.h>
#include <console.h>
#include <main.h>
#include "debug.h"
#include <app_cfg.h>

/* For Test Code(Verification Code) */
#include <wdt_test.h>
#include <timer_test.h>
#ifdef SSM_FEATURE_ISO
#   include <iso_test.h>
#endif
#include <gdma_test.h>
#include <dse_test.h>
#include <gic_test.h>
#include <lin_test.h>
#include <pdm_test.h>
#include <ictc_test.h>
#include <pmio_test.h>
#include <gpio_test.h>
#include <adc_test.h>
#include <fmu_test.h>
#include <can_demo.h>
#include <reset_demo.h>
#include <sdm.h>
#include <fwug.h>

#ifdef  MCU_BSP_T_RVC_INCLUDED
#   include <trvc_test.h>
#endif  // MCU_BSP_T_RVC_INCLUDED

#include <midf.h>
#include <gpsb_test.h>
#include <i2c_test.h>

#ifdef IP_VERIFICATION_DEFINED_GDMA_MIDF_TOP_CRD
#include <gdma_ip_test.h>
#include <mid_filter_test.h>
#include <mc_top.h>
#include <crd_test.h>
#endif
#include <hsm_test.h>

#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
#   include <ivi_cluster_demo.h>
#endif

/****************************************************************************************************/
/*                                       STATIC FUNCTIONS                                           */
/****************************************************************************************************/

static ConsoleCmdList_t *pGetConsoleCmdList
(
    uint32                              uiIndex
);

static void CSL_HelpList
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static int32 CSL_StringToHex
(
    const int8                          *param
);

static void CSL_ReadMemory
(
    uint8                               ucArgc,
    void                                *pArgv[]
);

static void CSL_WriteMemory
(
    uint8                               ucArgc,
    void                                *pArgv[]
);

static void CSL_SetAliveMessage
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceI2c
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceUart
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceCan
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceLin
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceMbox
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DevicePdm
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceIctc
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceAdc
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceGpio
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceTmr
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceWdt
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceMpu
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceFmu
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceGpsb
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceGic
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DevicePmio
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceCkc
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceGdma
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceSdm
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceTrvc
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DevicePmu
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceTOP
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceFwug
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
static void CSL_DeviceIVIClusterDemo
(
    uint8                               ucArgc,
    void *                              pArgv[]
);
#endif

static void CSL_DeviceDse
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceSsm
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceMidf
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_EnableLog
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static void CSL_DeviceHsm
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

static sint32 CSL_EmptyHistoryList
(
    void
);

static uint32 CSL_PullHistoryList
(
    uint8 *                             pucCmdBuf,
    uint32                              uiOldCmdSize
);

static void CSL_PushHistoryList
(
    uint8 *                             pucBuf,
    uint32                              uiCmdSize
);

static void ConsoleTask
(
    void *                              pArg
);

static uint32 CSL_GetOneWordLength
(
    uint8 *                             puiCmdBuf,
    uint32                              uiCmdLength
);

static uint32 CSL_GetCommandLength
(
    const uint8 *                       puiCmdBuf
);

static void CSL_ExecuteCommand
(
    uint8 *                             puiCmdBuf,
    uint32                              uiCmdLength
);

static void CSL_LogCmdType01
(
    void *                              pArgv[]
);

static void CSL_LogCmdType02
(
    void *                              pArgv[]
);

static void CSL_LogCmdType03
(
    void *                              pArgv[]
);

static void CSL_UsageLogCmd
(
    uint8 ucInfo,
    uint8 ucHelp
);

static void CSL_ShowLogInfo
(
    void
);


/*
***************************************************************************************************
*                                        DEFINITIONS
***************************************************************************************************
*/
//{ CMD_ENABLE, (uint8 *)string you want,  will be executed function}

static ConsoleCmdList_t ConsoleCmdList[CSL_CMD_NUM_MAX] =
{
    { CMD_ENABLE,   (const uint8 *)"help",     CSL_HelpList},
    { CMD_ENABLE,   (const uint8 *)"md",       CSL_ReadMemory},
    { CMD_ENABLE,   (const uint8 *)"mm",       CSL_WriteMemory},
    { CMD_ENABLE,   (const uint8 *)"alive",    CSL_SetAliveMessage},
    { CMD_ENABLE,   (const uint8 *)"i2c",      CSL_DeviceI2c},
    { CMD_ENABLE,   (const uint8 *)"uart",     CSL_DeviceUart},
    { CMD_ENABLE,   (const uint8 *)"can",      CSL_DeviceCan},
    { CMD_ENABLE,   (const uint8 *)"lin",      CSL_DeviceLin},
    { CMD_ENABLE,   (const uint8 *)"mbox",     CSL_DeviceMbox},
    { CMD_ENABLE,   (const uint8 *)"pdm",      CSL_DevicePdm},
    { CMD_ENABLE,   (const uint8 *)"ictc",     CSL_DeviceIctc},
    { CMD_ENABLE,   (const uint8 *)"adc",      CSL_DeviceAdc},
    { CMD_ENABLE,   (const uint8 *)"gpio",     CSL_DeviceGpio},
    { CMD_ENABLE,   (const uint8 *)"tmr",      CSL_DeviceTmr},
    { CMD_ENABLE,   (const uint8 *)"wdt",      CSL_DeviceWdt},
    { CMD_ENABLE,   (const uint8 *)"mpc",      CSL_DeviceMpu},
    { CMD_ENABLE,   (const uint8 *)"fmu",      CSL_DeviceFmu},
    { CMD_ENABLE,   (const uint8 *)"gpsb",     CSL_DeviceGpsb},
    { CMD_ENABLE,   (const uint8 *)"gic",      CSL_DeviceGic},
    { CMD_ENABLE,   (const uint8 *)"pmio",     CSL_DevicePmio},
    { CMD_ENABLE,   (const uint8 *)"ckc",      CSL_DeviceCkc},
    { CMD_ENABLE,   (const uint8 *)"gdma",     CSL_DeviceGdma},
    { CMD_ENABLE,   (const uint8 *)"sdm",      CSL_DeviceSdm},
    { CMD_ENABLE,   (const uint8 *)"dse",      CSL_DeviceDse},
    { CMD_ENABLE,   (const uint8 *)"ssm",      CSL_DeviceSsm},
    { CMD_ENABLE,   (const uint8 *)"midf",     CSL_DeviceMidf},
    { CMD_ENABLE,   (const uint8 *)"log",      CSL_EnableLog},
    { CMD_ENABLE,   (const uint8 *)"hsm",      CSL_DeviceHsm},
    { CMD_ENABLE,   (const uint8 *)"trvc",     CSL_DeviceTrvc},
    { CMD_ENABLE,   (const uint8 *)"pmu",      CSL_DevicePmu},
    { CMD_ENABLE,   (const uint8 *)"top",      CSL_DeviceTOP},
    { CMD_ENABLE,   (const uint8 *)"fwug",     CSL_DeviceFwug},
#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
    { CMD_ENABLE,   (const uint8 *)"clust",     CSL_DeviceIVIClusterDemo},
#endif
    { CMD_DISABLE,  (const uint8 *)"",         NULL}
};

static ConsoleLogFunc_t ConsoleLogFunc[CSL_LOG_NUM_MAX] =
{
    {CSL_LogCmdType01},
    {CSL_LogCmdType02},
    {CSL_LogCmdType03}
};

#ifdef CONSOLE_HISTORY_ENABLE
    static CSLHistoryManager_t CSHistoryManager;
#endif

/*
***************************************************************************************************
*                                    FUNCTION PROTOTYPES
***************************************************************************************************
*/

static ConsoleCmdList_t *pGetConsoleCmdList(uint32 uiIndex)
{
    return &ConsoleCmdList[uiIndex];
}

static void CSL_HelpList(uint8 ucArgc, void *pArgv[])
{
    (void)  ucArgc;
    (void)  pArgv;

    UART_Write(UART_CH0, (const uint8 *)"\r\n================================================", 50);
    UART_Write(UART_CH0, (const uint8 *)"\r\n============== How to use command ==============", 50);
    UART_Write(UART_CH0, (const uint8 *)"\r\n================================================", 50);

    UART_Write(UART_CH0, (const uint8 *)"\r\n [command_string] [arg1] ... [arg10]", 38);
    UART_Write(UART_CH0, (const uint8 *)"\n", 1);
    UART_Write(UART_CH0, (const uint8 *)"\r\n 1. display alive message : [alive] [on/off]", 46);

    UART_Write(UART_CH0, (const uint8 *)"\n", 1);
}

static void CSL_DeviceHsm(uint8 ucArgc, void *pArgv[])
{
	int32 ret = -1;

    if (pArgv != NULL_PTR)
    {
        if (pArgv[0] != NULL)
        {
            ret = HSM_Test((void *)pArgv[0]);
            if(ret != 0){
                mcu_printf("\n HSM_Test Fail(%d)\n", ret);
            }
        }
        else
        {
            (void)mcu_printf("\n Wrong argument, argv(%s)\n", pArgv[0]);
        }
    }
}

static int32 CSL_StringToHex(const int8 *pucData)
{
    int32 hex = -1;
    uint32 count = 0UL;
    uint32 i;

    if (pucData != NULL_PTR)
    {
        (void)SAL_StrLength(pucData, (SALSize *)&count);

        for (i = 0UL ; i < count; i++)
        {
            if((pucData[i] >= '0') && (pucData[i] <= '9'))
            {
                hex = (((hex * 16) + pucData[i]) - '0');
            }
            else if ((pucData[i] >= 'A') && (pucData[i] <= 'F'))
            {
                hex = ((((hex * 16) + *pucData) - 'A') + 10);
            }
            else if ((pucData[i] >= 'a') && (pucData[i] <= 'f'))
            {
                hex = ((((hex * 16) + pucData[i]) - 'a') + 10);
            }
            else
            {
                ;
            }

        }
    }

    return hex;
}

static void CSL_ReadMemory(uint8 ucArgc, void *pArgv[])
{
    int32   addr;
    uint32  reg;

    if (pArgv != NULL_PTR)
    {
        if (pArgv[0] != NULL)
        {
            addr = CSL_StringToHex(pArgv[0]);

            if (addr >= 0)
            {
                reg = SAL_ReadReg(addr);
                mcu_printf("\n          0x%x    ==>    0x%x\n", addr, reg);
            }
        }
        else
        {
            mcu_printf("\n Wrong argument, argv(%s)\n", pArgv[0]);
            mcu_printf("\n [md] [hex address] \n");
        }
    }
}

static void CSL_WriteMemory(uint8 ucArgc, void *pArgv[])
{
    int32   addr;
    int32   val;
    uint32  reg;

    if (pArgv != NULL_PTR)
    {
        if (pArgv[0] != NULL)
        {
            addr = CSL_StringToHex(pArgv[0]);
            val = CSL_StringToHex(pArgv[1]);

            if ((addr >= 0) && (val >= 0))
            {
                SAL_WriteReg(val, addr);

                reg = SAL_ReadReg(addr);
                mcu_printf("\n          0x%x    ==>    0x%x\n", addr, reg);
            }
        }
        else
        {
            mcu_printf("\n Wrong argument, argv(%s)\n", pArgv[1]);
            mcu_printf("\n [mm] [hex address] [value] \n");
        }
    }
}


static void CSL_SetAliveMessage(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 onbuf[2]    = "on";
    const uint8 offbuf[3]   = "off";
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, onbuf, 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            gALiveMsgOnOff  = ON;
        }
        else if((SAL_StrNCmp(str, offbuf, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            gALiveMsgOnOff  = OFF;
        }
        else
        {
            mcu_printf("Wrong argument : on or off %s\n", pArgv[0]);
        }
    }
}

static void CSL_DeviceI2c(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
	sint32		ret;

	(void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            I2C_TestMain();
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}

static void CSL_DeviceUart(uint8 ucArgc, void *pArgv[])
{

}

static void CSL_DeviceCan(uint8 ucArgc, void *pArgv[])
{
#if (ACFG_APP_CAN_DEMO_EN == 1)
    CAN_DemoTest(ucArgc, pArgv);
#endif
}

static void CSL_DeviceLin(uint8 ucArgc, void *pArgv[])
{
    LIN_SampleTest(ucArgc, pArgv);
}

static void CSL_DeviceMbox(uint8 ucArgc, void *pArgv[])
{

}

static void CSL_DevicePdm(uint8 ucArgc, void *pArgv[])
{
    uint32   ucMode;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        ucMode  = strtoul(pArgv[0], NULL_PTR, 16UL);
        PDM_SelectTestCase(ucMode);
    }
}

static void CSL_DeviceIctc(uint8 ucArgc, void *pArgv[])
{
    uint32   ucMode;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        ucMode  = strtoul(pArgv[0], NULL_PTR, 10UL);
        ICTC_SelectTestCase(ucMode);
    }
}

static void CSL_DeviceAdc(uint8 ucArgc, void *pArgv[])
{
#if (ACFG_APP_KEY_EN == 0)
    int32 ucMode;
    if (pArgv != NULL_PTR)
    {
        ucMode  = strtol(pArgv[0], NULL_PTR, 10UL);
        ADC_StartAdcTest(ucMode);
    }
#else
	mcu_printf("Please set ACFG_APP_KEY_EN = 0 !!\n");
#endif
}

static void CSL_DeviceGpio(uint8 ucArgc, void *pArgv[])
{
    int32   ucMode;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        ucMode  = strtol(pArgv[0], NULL_PTR, 16UL);
        GPIO_StartGpioTest(ucMode);
    }
}

static void CSL_DeviceTmr(uint8 ucArgc, void *pArgv[])
{
    int32       iMode;
    uint32      uiTime;
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
    sint32      ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];
        iMode   = strtol(pArgv[1], NULL_PTR, 10UL);
        uiTime  = strtol(pArgv[2], NULL_PTR, 10UL);

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            TIMER_StartTimerTest(iMode, uiTime);
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }
}

static void CSL_DeviceWdt(uint8 ucArgc, void *pArgv[])
{
    int32       iMode;
    uint32      uiTime;
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
    sint32      ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];
        iMode   = strtol(pArgv[1], NULL_PTR, 10UL);
        uiTime  = strtol(pArgv[2], NULL_PTR, 10UL);

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            WDT_StartTest(iMode, uiTime);
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}

static void CSL_DeviceMpu(uint8 ucArgc, void *pArgv[])
{

}

static void CSL_DeviceFmu(uint8 ucArgc, void *pArgv[])
{
    int32 ucMode;
    if (pArgv != NULL_PTR)
    {
        ucMode  = strtol(pArgv[0], NULL_PTR, 10UL);
        FMU_StartFmuTest(ucMode);
    }
}

static void CSL_DeviceGpsb(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4] = "test";
	sint32		ret;

	(void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str     = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GPSB_Test();
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}

static void CSL_DeviceGic(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4]    = "test";
    const uint8 ucBufTest1[7]   = "sgitest";
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GIC_Test_SmInt();
        }
        else if((SAL_StrNCmp(str, ucBufTest1, 7, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GIC_Test_Sgi();
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }

    return;
}

static void CSL_DevicePmio(uint8 ucArgc, void *pArgv[])
{
    const uint8 ucCmdNum                = ucArgc;
    const uint8 ucStatus[6]             = "status";
    const uint8 ucRead[6]               = "read";
    const uint8 ucWrite[6]              = "write";
    const uint8 ucTest[6]               = "test";
    uint32 uiVa;
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        mcu_printf("\nPMIO TEST: cmd length %d\n", ucCmdNum);

        if ((SAL_StrNCmp(pArgv[0], ucStatus, 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            PMIO_TEST_PrintStatus();
        }
        else if ((SAL_StrNCmp(pArgv[0], ucRead, 1, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            PMIO_TEST_GetByName((uint8*)pArgv[1]);
        }
        else if ((SAL_StrNCmp(pArgv[0], ucWrite, 1, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            uiVa = (uint32)strtoul(pArgv[2], NULL, 16UL);
            PMIO_TEST_SetByName((uint8*)pArgv[1], uiVa);
        }
        else if ((SAL_StrNCmp(pArgv[0], ucTest, 1, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            uiVa = (uint32)strtoul(pArgv[1], NULL, 10UL);
            PMIO_TEST_Run(uiVa);
        }
        else
        {
            mcu_printf("PMIO_TEST: Unknown cmd \n");
        }
    }
}

static void CSL_DeviceCkc(uint8 ucArgc, void *pArgv[])
{

}

static void CSL_DeviceGdma(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4]    = "test";
    sint32 ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if ((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            GDMA_SampleForM2M();
        }
	 else if ((SAL_StrNCmp(str, (const uint8 *)"verf", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            #ifdef GDMA_IP_TEST_CODE
            uint32 test_id;
            int32 ret;

            test_id = (uint32)strtoul(pArgv[1], NULL, 10);
            ret = GDMA_IPTest(test_id);
            if ( ret == 0)
            {
                mcu_printf("PASS \n");
            }
            else
            {
                mcu_printf("FAIL \n");
            	}
            #else
            mcu_printf("Need to check GDMA_IP_TEST_CODE feature  \n");
            #endif
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }
}

static void CSL_DeviceSdm(uint8 ucArgc, void *pArgv[])
{
    const uint8 ucReadCmdNum    = 2;
    const uint8 ucWriteCmdNum   = 6;
    const uint8 ucCmdNum        = ucArgc;
    uint32 uiRegAddr            = 0;
    uint32 uiReadVa             = 0;
    uint8 ucWriteBuf[4]         = {0, 0, 0, 0};
    uint8 ucIdx;
    uint32 uiConvTemp;

    if (pArgv != NULL_PTR)
    {
        mcu_printf("\nSDM TEST cmd length %d, str0 %s\n", ucCmdNum, pArgv[0]); //Misra : ucIdx -> 0

        if(ucCmdNum == ucReadCmdNum)
        {
            uiRegAddr = (uint32)strtoul(pArgv[1], NULL, 16UL);

            if(SDM_ReadReg(uiRegAddr, &uiReadVa) == SDM_ERR_NONE)
            {
                mcu_printf("SDM Read Register x%06x: x%08x\n", uiRegAddr, uiReadVa);
            }
            else
            {
                mcu_printf("FAIL! SDM Read Register x%08x\n", uiRegAddr);
            }

        }
        else if(ucCmdNum == ucWriteCmdNum)
        {
            uiRegAddr = (uint32)strtoul(pArgv[1], NULL, 16UL);

            for(ucIdx = 0UL ; ucIdx < 4UL ; ucIdx++)
            {
                uiConvTemp = strtoul(pArgv[ucIdx+2UL], NULL, 16UL);
                if(uiConvTemp <= 0xffUL)
                {
                    ucWriteBuf[ucIdx] = (uint8)uiConvTemp;
                }
                else
                {
                    ucWriteBuf[ucIdx] = 0;
                }
            }

            if(SDM_WriteReg(uiRegAddr, (const uint8 *)ucWriteBuf, 4) != SDM_ERR_NONE)
            {
                mcu_printf("FAIL! SDM Write Register x%08x\n", uiRegAddr);
            }
        }
        else
        {
            mcu_printf("SDM_TEST: Unknown cmd \n");
        }
    }
}

static void CSL_DeviceTrvc(uint8 ucArgc, void *pArgv[])
{
#if (ACFG_APP_TRVC_EN == 1)
    if (pArgv != NULL_PTR)
    {
        TRVC_ConsoleCommandHandler(ucArgc, pArgv);
    }
#endif
}

static void CSL_DevicePmu(uint8 ucArgc, void *pArgv[])
{
#if (ACFG_APP_RESET_DEMO_EN == 1)
    PMU_ResetTest(ucArgc, pArgv);
#endif
}

static void CSL_DeviceTOP(uint8 ucArgc, void *pArgv[])
{
#ifdef MC_TOP_TEST // if you want to use mc top test fucntion, please check including mc_top.h header file
    int32 isAll =0;
     sint32 ret;

    // For the futrue to expand test part
    if ((SAL_StrNCmp(pArgv[0], (const uint8 *)"all", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
    {
        isAll = 1;
    }

    if ((isAll ==1) || ((SAL_StrNCmp(pArgv[0], (const uint8 *)"mcudsei", 7, &ret) == SAL_RET_SUCCESS) && (ret == 0))) // mcu default slave error
    {
    	mc_top_hbic03_mc_sub_tb_addr_dec_test();
    }
    if ((isAll ==1) || ((SAL_StrNCmp(pArgv[0], (const uint8 *)"peri", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))) // peri access test
    {
    	mc_top_mc_cr5_tb_peri_access_test();
    }
    if ((isAll ==1) || ((SAL_StrNCmp(pArgv[0], (const uint8 *)"apdsei", 6, &ret) == SAL_RET_SUCCESS) && (ret == 0))) // wait for default slave error interrrupt, when ap access invalid address of mcu
    {
    	mc_top_mc_cr5_wait_for_AP_default_error();
    }
    if ((isAll ==1) || ((SAL_StrNCmp(pArgv[0], (const uint8 *)"sr", 2, &ret) == SAL_RET_SUCCESS) && (ret == 0))) //secure wrapper isr wait
    {
    	mc_top_hbic03_mc_nsaid();
    }
    if ((isAll ==1) || ((SAL_StrNCmp(pArgv[0], (const uint8 *)"bus", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))) // bus access teset
    {
    	mcu_printf("MC_TOP :mc_top_03_mc_tb_sub_bus_test \n");
    	ret = mc_top_03_mc_tb_sub_bus_test();
    	if ( ret == 0)
    		mcu_printf("TEST PASS\n");
    	else
    		mcu_printf("TEST FAIL\n");
    }
    if ((isAll ==1) || ((SAL_StrNCmp(pArgv[0], (const uint8 *)"iso", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))) // bus isolation test
    {
    	mc_top_hbic03_mc_sub_bus_iso_test();
    }
    if ((isAll ==1) || ((SAL_StrNCmp(pArgv[0], (const uint8 *)"cfg", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))) // mc cfg
    {
    	mcu_printf("MC_TOP :mc_top_mc_cfg_test \n");
    	ret = mc_top_mc_cfg_test();
    	if ( ret == 0)
    		mcu_printf("TEST PASS\n");
    	else
    		mcu_printf("TEST FAIL\n");
    }
    if ((isAll ==1) || ((SAL_StrNCmp(pArgv[0], (const uint8 *)"alias", 5, &ret) == SAL_RET_SUCCESS) && (ret == 0))) // address alias test
    {
    	mc_top_xbic_tb_addr_alias_test();
    }

    if ((SAL_StrNCmp(pArgv[0], (const uint8 *)"dsei", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
    {
    	uint32 access_addr;
    	uint32 data;
    	if ((SAL_StrNCmp(pArgv[1], (const uint8 *)"wait", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
    	{
    		topDSEIWaitThreadCreate();
    	}

    	if ((SAL_StrNCmp(pArgv[1], (const uint8 *)"read", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
    	{
    		access_addr = (uint32)strtoul(pArgv[2], NULL, 16);
    		data = *(uint32 *)access_addr;
    		mcu_printf("read : 0x%x  value : 0x%x\n", access_addr, data);
    	}
    }
#else
    mcu_printf("Test Not Avaliable. Check MC_TOP_TEST feature.\r\n");
#endif
}

static void CSL_DeviceFwug(uint8 ucArgc, void *pArgv[])
{
    const uint8 *pucStr;
    const uint8 ucBufTest[4] = "test";
    sint32      ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        pucStr = (const uint8 *)pArgv[0];

        if ((SAL_StrNCmp(pucStr, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            FWUG_Start(FWUG_CMD_UPDATE_START);
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}

#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
static void CSL_DeviceIVIClusterDemo(uint8 ucArgc, void *pArgv[])
{
    IVI_Cluster_DemoTest(ucArgc, pArgv);
}
#endif

static void CSL_DeviceDse(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    const uint8 ucBufTest[4]    = "test";
    const uint8 ucBufTest1[7]   = "dup";
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            DSE_Test_R5AccessFault();
        }
        else if((SAL_StrNCmp(str, ucBufTest1, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            DSE_Test_SoftFaultCheck();
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }

    return;
}

static void CSL_DeviceSsm(uint8 ucArgc, void *pArgv[])
{
    const uint8 *strCategory;
    const uint8 *strAction;
    const uint8 ucBufTest[4] = "test";
    const uint8 ucBufIsolation[3] = "iso";
    const uint8 ucBufSramEcc[4] = "sram";
    int32       ucMode;
    sint32      ret;

    (void)ucArgc;

    if (pArgv != NULL_PTR)
    {
        strCategory = (const uint8 *)pArgv[0];
        strAction   = (const uint8 *)pArgv[1];
        ucMode      = strtol(pArgv[2], NULL_PTR, 10UL);

        if ((SAL_StrNCmp(strCategory, ucBufIsolation, 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            if ((SAL_StrNCmp(strAction, ucBufTest, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
#ifdef SSM_FEATURE_ISO
                ISO_StartTest(ucMode);
#endif
            }
        }
        else if ((SAL_StrNCmp(strCategory, ucBufSramEcc, 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            mcu_printf("SRAM ECC test is not supported. %d\n", ucMode);
        }
#ifdef CRD_TEST_ENABLE
        else if ((SAL_StrNCmp(strCategory, "crd", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            uint32 isAll =0;
            if ((SAL_StrNCmp(strAction, "all", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0))
            {
                isAll = 1;
            }
            // ucMode(Severity Level : 0 : Low, 1 : Mid, 2: High
            if ((isAll) || ((SAL_StrNCmp(strAction, "subsys", 6, &ret) == SAL_RET_SUCCESS) && (ret == 0)))
            {
                Crd_Test(0, ucMode);
            }
            if ((isAll) || ((SAL_StrNCmp(strAction, "cr5", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0)))
            {
                Crd_Test(1, ucMode);
            }
            if ((isAll) || ((SAL_StrNCmp(strAction, "mid", 3, &ret) == SAL_RET_SUCCESS) && (ret == 0)))
            {
                Crd_Test(2, ucMode);
            }
            if ((isAll) || ((SAL_StrNCmp(strAction, "sm", 2, &ret) == SAL_RET_SUCCESS) && (ret == 0)))
            {
                Crd_Test(3, ucMode);
            }
        }
#endif
        else
        {
            mcu_printf("\nWrong argument : %s\n", pArgv[0]);
        }
    }
}

static void CSL_DeviceMidf(uint8 ucArgc, void *pArgv[])
{
    const uint8 *str;
    sint32 ret;
    uint32 uiMemoryBase	= 0xC1000000UL; //SRAM1
    uint32 uiSize			= 0x1000UL; // 4KB
    uint32 uiID			= MIDF_ID_CAN0;
    uint32 uiType			=(MIDF_TYPE_READ|MIDF_TYPE_WRITE);

    if (pArgv != NULL_PTR)
    {
        str = (const uint8 *)pArgv[0];

        if((SAL_StrNCmp(str, (const uint8 *)"init", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            MIDF_FilterInitialize();
        }
        else if((SAL_StrNCmp(str, (const uint8 *)"reg", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            MIDF_UnregisterFilterRegion(uiMemoryBase, uiSize, uiID, uiType);
        }
        else if ((SAL_StrNCmp(str, (const uint8 *)"del", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
            MIDF_RegisterFilterRegion(uiMemoryBase, uiSize, uiID, uiType);
        }
	 else if ((SAL_StrNCmp(str, (const uint8 *)"verf", 4, &ret) == SAL_RET_SUCCESS) && (ret == 0))
        {
        	#ifdef MIDF_IP_TEST
		uint32 test_id;
		int32 ret;

		test_id = (uint32)strtoul(pArgv[1], NULL, 10);
		ret = MIDF_IPTest(test_id);
		if ( ret == 0)
		{
			mcu_printf("PASS \n");
		}
		else
		{
			mcu_printf("FAIL \n");
		}
		#else
		mcu_printf("Need to check MIDF_IP_TEST feature  \n");
		#endif
        }
        else
        {
            mcu_printf("\nWrong argument : %s\n", str);
        }
    }

    return;
}

static void CSL_EnableLog(uint8 ucArgc, void *pArgv[])
{
    if (pArgv != NULL_PTR)
    {
        if(DBG_VALID_ARGCOUNT(ucArgc))
        {
            ConsoleLogFunc[DBG_CNT_TO_IDX(ucArgc)].clFunc(pArgv);
        }
        else
        {
            CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
        }
    }
}


static void CSL_LogCmdType01(void *pArgv[])
{
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        if((SAL_StrNCmp(pArgv[0],  (const uint8 *)"help", DBG_HELP_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            CSL_UsageLogCmd(DBG_ENABLE, DBG_ENABLE);
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"on", DBG_ON_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            gDebugOption = DBG_ENABLE_ALL;
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"off", DBG_OFF_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            gDebugOption = DBG_DISABLE_ALL;
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"debug", DBG_LEVEL_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            gDebugOption = (gDebugOption & ~DBG_LEVEL_POSITION) | DBG_LEVEL_DEBUG;
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"error", DBG_LEVEL_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            gDebugOption = (gDebugOption & ~DBG_LEVEL_POSITION) | DBG_LEVEL_ERROR;
        }
        else if((SAL_StrNCmp(pArgv[0], (const uint8 *)"info", DBG_INFO_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
        {
            CSL_ShowLogInfo();
        }
        else
        {
            CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
        }
    }
}

static void CSL_LogCmdType02(void *pArgv[])
{
    uint32  tagIdx;
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        tagIdx = strtoul(pArgv[0], NULL_PTR, 10UL);

        if(DBG_VALID_TAGIDX(tagIdx))
        {
            if((SAL_StrNCmp(pArgv[1], (const uint8 *)"on", DBG_ON_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
            {
                gDebugOption = gDebugOption | (DBG_TAG_BIT(tagIdx));
            }
            else if((SAL_StrNCmp(pArgv[1], (const uint8 *)"off", DBG_OFF_LEN, &ret) == SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
            {
                gDebugOption = gDebugOption & ~(DBG_TAG_BIT(tagIdx));
            }
            else
            {
                CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
            }
        }
        else
        {
            CSL_UsageLogCmd(DBG_DISABLE, DBG_ENABLE);
        }
    }
}

static void CSL_LogCmdType03(void *pArgv[])
{
    uint32  tagIdx;
    sint32 ret;

    if (pArgv != NULL_PTR)
    {
        tagIdx = strtoul(pArgv[0], NULL_PTR, 10UL);

        if(DBG_VALID_TAGIDX(tagIdx))
        {
            if(DBG_EQUALS(SAL_StrNCmp(pArgv[1], (const uint8 *)"on", DBG_ON_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
            {
                gDebugOption = gDebugOption | (DBG_TAG_BIT(tagIdx));
                if(DBG_EQUALS(SAL_StrNCmp(pArgv[2], (const uint8 *)"debug", DBG_LEVEL_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
                {
                    gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_DEBUG;
                }
                else if(DBG_EQUALS(SAL_StrNCmp(pArgv[2], (const uint8 *)"error", DBG_LEVEL_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
                {
                    gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_ERROR;
                }
                else
                {
                    CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
                }
            }
            else if(DBG_EQUALS(SAL_StrNCmp(pArgv[1], (const uint8 *)"off", DBG_OFF_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
            {
                gDebugOption = gDebugOption & ~(DBG_TAG_BIT(tagIdx));

                if(DBG_EQUALS(SAL_StrNCmp(pArgv[2], (const uint8 *)"debug", DBG_LEVEL_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
                {
                    gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_DEBUG;
                }
                else if(DBG_EQUALS(SAL_StrNCmp(pArgv[2], (const uint8 *)"error", DBG_LEVEL_LEN, &ret), SAL_RET_SUCCESS) && DBG_EQUALS(ret, DBG_ERR_NONE))
                {
                    gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_ERROR;
                }
                else
                {
                    CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
                }
            }
            else
            {
                CSL_UsageLogCmd(DBG_ENABLE, DBG_DISABLE);
            }
        }
        else
        {
            CSL_UsageLogCmd(DBG_DISABLE, DBG_ENABLE);
        }
    }
}

static void CSL_UsageLogCmd(uint8 ucInfo, uint8 ucHelp)
{
    uint32 index;

    if(DBG_EQUALS(ucInfo, DBG_ENABLE))
    {
        mcu_printf("=== USAGE INFO ===\n");
        mcu_printf("log [Index] on/off debug/error\n");
        mcu_printf("log [Index] on/off\n");
        mcu_printf("log on/off\n");
        mcu_printf("log debug/error\n");
        mcu_printf("log info\n");
        mcu_printf("log help\n");
    }

    if(DBG_EQUALS(ucHelp, DBG_ENABLE))
    {
        mcu_printf("\n=== INDEX INFO ===\n");

        for(index = 0 ; index < DBG_MAX_TAGS ; index++)
        {
            mcu_printf("[%-5s] : [%2d]\n", dbgTags[index].dlTag, index);
        }
    }
}

static void CSL_ShowLogInfo(void)
{
    uint32 index;
    uint32 check = 0;
    uint32 cnt   = 0;

    mcu_printf("Enabled : ");

    for(index = 0 ; index < (DBG_MAX_TAGS - 2UL) ; index++)
    {
        check = (gDebugOption >> index) & ~DBG_LEVEL_POSITION;

        if((check & DBG_CHECK_BIT) == DBG_CHECK_BIT)
        {
            mcu_printf(" %s(%2d) ", dbgTags[index].dlTag, index);
            cnt++;
        }
    }

    if(cnt == DBG_ZERO)
    {
        mcu_printf(" None ");
    }

    mcu_printf("\nLevel   :");
    check = (gDebugOption & DBG_LEVEL_POSITION);

    if(check == 0UL)
    {
        mcu_printf(" Error\n");
    }
    else
    {
        mcu_printf(" Debug\n");
    }
}

#ifdef CONSOLE_HISTORY_ENABLE

static CSLHistoryManager_t *pGetCSHistoryManager(void)
{
    return &CSHistoryManager;
}

static sint32 CSL_EmptyHistoryList(void)
{
    CSLHistoryManager_t *   pCSHistoryManager;
    uint32                  index;
    uint32                  flag    = 0;
    sint32                  ret     = -1;

    pCSHistoryManager = pGetCSHistoryManager();

    for(index = 0 ; index < CSL_HISTORY_COUNT ; index++)
    {
        flag = pCSHistoryManager->hmList[index].hcFlag;

        if(flag == OFF)
        {
            ret = (sint32)index;
            break;
        }
    }

    return ret;
}

static uint32 CSL_PullHistoryList(uint8 *pucCmdBuf, uint32 uiOldCmdSize)
{
    uint32                  num;
    uint32                  currPtr;
    uint32                  newCmdSize = 0;
    uint8                   listFlag;
    uint8               *   pHistoryBuf;
    CSLHistoryManager_t *   pCSHistoryManager;

    SAL_MemSet(pucCmdBuf, 0, CSL_CMD_BUFF_SIZE);
    pCSHistoryManager   = pGetCSHistoryManager();
    currPtr             = pCSHistoryManager->hmPtr;

    /* Remove current command in console */
    for(num = 0 ; num < uiOldCmdSize ; num ++)
    {
        UART_Write(UART_CH0, (const uint8 *)"\b \b", 3);
    }

    for(num = 0 ; num < CSL_HISTORY_COUNT ; num++)
    {
        listFlag = pCSHistoryManager->hmList[currPtr].hcFlag;

        if(listFlag == ON)
        {
            pCSHistoryManager->hmPtr    = currPtr;
            pHistoryBuf = pCSHistoryManager->hmList[currPtr].hcBuf;
            newCmdSize  = pCSHistoryManager->hmList[currPtr].hcSize;

            UART_Write(UART_CH0, (const uint8 *)pHistoryBuf, newCmdSize);
            SAL_MemCopy(pucCmdBuf, pHistoryBuf, newCmdSize);

            if(currPtr == 0UL)
            {
                currPtr = CSL_HISTORY_COUNT;
            }

            currPtr--;
            pCSHistoryManager->hmPtr = currPtr;
            break;
        }
        else
        {
            if(currPtr == 0UL)
            {
                currPtr = CSL_HISTORY_COUNT;
            }

            currPtr--;
        }
    }

    return newCmdSize;
}

static void CSL_PushHistoryList(uint8 *pucBuf, uint32 uiCmdSize)
{
    sint32                  index = 0;
    CSLHistoryManager_t *   pCSHistoryManager;
    uint8               *   pHistoryBuf;

    index = CSL_EmptyHistoryList();

    if(index < 0)
    {
        index = 0;
    }

    pCSHistoryManager   = pGetCSHistoryManager();

    pHistoryBuf         = pCSHistoryManager->hmList[index].hcBuf;
    if(pHistoryBuf != NULL)
    {
        SAL_MemSet(pHistoryBuf, 0, CSL_CMD_BUFF_SIZE);
        pCSHistoryManager->hmPtr                    = (uint32)index;
        pCSHistoryManager->hmList[index].hcFlag     = ON;
        pCSHistoryManager->hmList[index].hcIndex    = (uint8)index;
        pCSHistoryManager->hmList[index].hcSize     = uiCmdSize;
        SAL_MemCopy(pHistoryBuf, pucBuf, uiCmdSize);
    }
}
#endif


/*****************************************************************************/
/*                                 FUNCTIONS                                 */
/*****************************************************************************/

static void ConsoleTask(void *pArg)
{
    uint8   cmdBuffer[CSL_CMD_BUFF_SIZE + CSL_CMD_PROMPT_SIZE];
    uint32  cmdLength   = 0;
    uint8   cmdStatus   = (uint8)CSL_NOINPUT;
    sint8   getc_err;
    sint32  ret         =0;
    uint8   c           =0;
    uint32  idx;

    (void)pArg;

    for(;;)
    {
        ret = UART_GetChar(UART_CH0, 0, (sint8 *)&getc_err);

        if(ret > 0)
        {
            c   = ((uint8)ret & 0xFFUL);

            switch(c)
            {
                case ARRIAGE_RETURN:
                case LINE_FEED:
                {
                    cmdStatus = (uint8)CSL_EXECUTE;
                    break;
                }

                case BACK_SPACE:
                {
                    if (cmdLength > 0UL)
                    {
                        cmdStatus = (uint8)CSL_INPUTING;
                        cmdLength--;
                        UART_Write(UART_CH0, (const uint8 *)"\b \b", 3UL);
                    }

                    break;
                }

                case NAK_KEY:
                {

                    /* Remove current command in command buffer */
                    for(idx = 0UL; idx < cmdLength; idx ++)
                    {
                        UART_Write(UART_CH0, (const uint8 *)"\b \b", 3UL);
                    }

                    cmdStatus   = (uint8)CSL_INPUTING;
                    cmdLength   = 0UL;
                    break;
                }

                case ESC_KEY:
                    break;

#ifdef CONSOLE_HISTORY_ENABLE
                case LBRACKET:
                {
                    cmdLength = CSL_PullHistoryList(cmdBuffer, cmdLength);
                    (void)UART_GetChar(UART_CH0, 0, (sint8 *)&getc_err);
                    break;
                }
#endif
                default:
                {
                    if(cmdLength < (CSL_CMD_BUFF_SIZE-1UL))
                    {
                        cmdStatus               = (uint8)CSL_INPUTING;
                        cmdBuffer[cmdLength]    = c;
                        cmdLength++;
                        /*mcu_printf("[%d]0x%x\n", __LINE__, c);\
                        mcu_printf("[%d][DATA:", __LINE__);
                        for(idx = 0UL; idx < cmdLength; idx ++)
                        {
                            mcu_printf("%c", cmdBuffer[idx]);
                        }
                        mcu_printf("][Len:%d]\n", cmdLength);*/
                        UART_Write(UART_CH0, &c, 1UL);
                    }

                    break;
                }
            }

            if(cmdStatus == (uint8)CSL_EXECUTE)
            {
                //mcu_printf("[%d]%s\n", __LINE__, cmdBuffer);

                cmdStatus = (uint8)CSL_NOINPUT;
                UART_Write(UART_CH0, (const uint8 *)CRLF, 2UL);
                CSL_ExecuteCommand(cmdBuffer, cmdLength);
                UART_Write(UART_CH0, (const uint8 *)CRLF, 2UL);
                UART_Write(UART_CH0, (const uint8 *)"> ", 2UL);  //Prompt

                if(cmdLength != 0UL)
                {
#ifdef CONSOLE_HISTORY_ENABLE
                    CSL_PushHistoryList(cmdBuffer, cmdLength);
#endif
                    cmdLength   = 0UL;
                    SAL_MemSet(cmdBuffer, 0, sizeof(cmdBuffer));
                }
            }
        }
        else
        {
#ifndef EDUCATION_CAN_DEMO_EN
            (void)SAL_TaskSleep(100);
#endif
            //mcu_printf("ERROR NO ACK\n");
        }
    }
}

static uint32 CSL_GetOneWordLength(uint8 * puiCmdBuf, uint32 uiCmdLength)
{
    uint32  idx;
    uint8   key;

    for(idx = 0UL ; idx < uiCmdLength ; idx++)
    {
        key = *(puiCmdBuf+idx);

        if(key == SPACE_BAR)
        {
            break;
        }
    }

    return idx;
}

static uint32 CSL_GetCommandLength(const uint8 * puiCmdBuf)
{
    uint32  idx = 0UL;
    uint8   key;

    if (puiCmdBuf != NULL_PTR)
    {
        for( ; idx < CSL_CMD_BUFF_SIZE ; idx++)
        {
            key = *(puiCmdBuf+idx);

            if(key == NUL_KEY)
            {
                break;
            }
        }
    }

    return idx;
}


static void CSL_ExecuteCommand(uint8 * puiCmdBuf, uint32 uiCmdLength)
{
    uint32              idx;
    uint32              totalCnt        = 0;
    uint32              realCmdLength   = 0;
    uint32              paramLength     = 0;
    uint32              clNameLength;   // name length in command list
    ConsoleCmdList_t *  pConsoleCommand;
    uint8               ucArgc;
    void *              pArgv[CSL_ARGUMENT_MAX] = {NULL};
    uint32              curPosition     =0;
    sint32              ret;

    if (puiCmdBuf != NULL_PTR)
    {
        realCmdLength   = CSL_GetOneWordLength(puiCmdBuf, uiCmdLength);
        curPosition     = realCmdLength;

        if(uiCmdLength > 0UL)
        {
            for(ucArgc = 0UL ; ucArgc < CSL_ARGUMENT_MAX ; ucArgc++)
            {
                if(uiCmdLength <= curPosition)
                {
                    break;
                }
                else
                {
                    curPosition += CSL_SPACE_1COUNT;
                    paramLength = CSL_GetOneWordLength((puiCmdBuf+curPosition), uiCmdLength-curPosition);

                    if(paramLength != 0UL)
                    {
                        pArgv[ucArgc]   = (void *)puiCmdBuf+curPosition;
                        puiCmdBuf[curPosition+paramLength] = 0; // Exchange SPACE to NULL
                        curPosition     += paramLength;
                    }
                }
            }

            totalCnt = sizeof(ConsoleCmdList) / sizeof(ConsoleCmdList[0]);

            for(idx = 0UL ; idx < totalCnt ; idx++)
            {
                pConsoleCommand = pGetConsoleCmdList(idx);

                if(pConsoleCommand->clEnable == CMD_ENABLE)
                {
                    clNameLength    = CSL_GetCommandLength(pConsoleCommand->clName);

                    if(clNameLength == realCmdLength)
                    {
                        if((SAL_StrNCmp(pConsoleCommand->clName, puiCmdBuf, realCmdLength, &ret) == SAL_RET_SUCCESS) && (ret == 0))
                        {
                            pConsoleCommand->clFunc(ucArgc, pArgv);
                        }
                    }
                }
            }
        }
    }
}

void CreateConsoleTask(void)
{
    static uint32   ConsoleTaskID = 0UL;
    static uint32   ConsoleTaskStk[CSL_TASK_STK_SIZE];

    (void)SAL_TaskCreate
    (
        &ConsoleTaskID,
        (const uint8 *)"Console Task",
        (SALTaskFunc)&ConsoleTask,
        &ConsoleTaskStk[0],
        CSL_TASK_STK_SIZE,
        SAL_PRIO_LOWEST,
        NULL
    );
}

