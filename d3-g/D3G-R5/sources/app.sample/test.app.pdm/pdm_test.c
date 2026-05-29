/*
***************************************************************************************************
*
*   FileName : pdm_test.c
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

#include "pdm.h"
#include "pdm_test.h"
#include "gpio.h"
#include "debug.h"
#include "sal_internal.h"
#include "bsp.h"

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                           PDM_TestSleepForSec
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/

#define PDM_CH_0                        (0UL)

static void PDM_TestSleepForSec
(
    uint32                              uiSec
)
{
    (void)SAL_TaskSleep(uiSec*(1000UL));

    return;
}

/*
***************************************************************************************************
*                                           PDM_TestSetGpio
*
* @param
* @return
*
* Notes
* 8050 GPIO_Config(GPIO_PORT_MA(7) , (GPIO_FUNC(11) | GPIO_OUTPUT | GPIO_CD(0x3)));
***************************************************************************************************
*/
static void PDM_TestSetGpio
(
    void
)
{

#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1) //TCC8059
    /*8059*/
    /* gpio_ma27 lcd backlight en pin, jr5-pin7 */
    (void)GPIO_Config(GPIO_GPMA(27UL) , (GPIO_FUNC(10UL) | GPIO_OUTPUT | GPIO_CD(0x3UL)));
#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1) //TCC8050
    (void)GPIO_Config(GPIO_GPE(15UL) , (GPIO_FUNC(10UL) | GPIO_OUTPUT | GPIO_CD(0x3UL)));
#endif

    return;
}

/*
***************************************************************************************************
*                                           PDM_TestPhaseMode1
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void PDM_TestPhaseMode1
(
    void
)
{
    uint32          x;
    uint32          uiDond;
    PDMModeConfig_t ModeConfigInfo;

    x = 0;
    uiDond = 1UL;

    mcu_printf("\n== Start PDM Phase Mode1 Sample ==\n");

    PDM_TestSleepForSec(3);
    PDM_TestSetGpio();

    (void)PDM_Init();

    PDM_TestSleepForSec(2);

#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1) //TCC8059
    ModeConfigInfo.mcPortNumber = 105UL;

#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1) //TCC8050
    ModeConfigInfo.mcPortNumber = 80UL;
#endif

    ModeConfigInfo.mcOperationMode = PDM_OUTPUT_MODE_PHASE_1;
    ModeConfigInfo.mcClockDivide = 0UL;
    ModeConfigInfo.mcLoopCount = 0UL;
    ModeConfigInfo.mcInversedSignal = 0UL;

    ModeConfigInfo.mcPosition1 = 0UL;
    ModeConfigInfo.mcPosition2 = 0UL;
    ModeConfigInfo.mcPosition3 = 0UL;
    ModeConfigInfo.mcPosition4 = 0UL;

    ModeConfigInfo.mcOutPattern1 = 0UL;
    ModeConfigInfo.mcOutPattern2 = 0UL;
    ModeConfigInfo.mcOutPattern3 = 0UL;
    ModeConfigInfo.mcOutPattern4 = 0UL;
    ModeConfigInfo.mcMaxCount = 0UL;

    while (uiDond > 0UL)
    {
        /* sleep 3sec for checking signal */
        PDM_TestSleepForSec(5UL);

        mcu_printf("\n== Duty %d0 / 100 ==\n", x);

        ModeConfigInfo.mcDutyNanoSec1 = x * 100UL * 1000UL;
        ModeConfigInfo.mcPeriodNanoSec1 = 1000UL * 1000UL;
        ModeConfigInfo.mcDutyNanoSec2 = 0UL;
        ModeConfigInfo.mcPeriodNanoSec2 = 0UL;

        if(x != (uint32)0UL)
        {
            (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);
        }

        (void)PDM_SetConfig((uint32)PDM_CH_0, (PDMModeConfig_t *)&ModeConfigInfo);

        (void)PDM_Enable((uint32)PDM_CH_0, PMM_ON);

        x++;

        if(x == (uint32)11UL)
        {
            PDM_TestSleepForSec(2UL);

            break;
        }
    }

    (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);
    mcu_printf("\n== End of PDM Phase Mode1 Sample ==\n");

    return;
}

/*
***************************************************************************************************
*                                           PDM_TestPhaseMode2
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/

static void PDM_TestPhaseMode2
(
    void
)
{
    PDMModeConfig_t ModeConfigInfo;

    mcu_printf("\n== Start PDM Phase Mode2 Sample ==\n");

    PDM_TestSleepForSec(3UL);
    PDM_TestSetGpio();

    (void)PDM_Init();

    PDM_TestSleepForSec(2UL);

#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1)
    ModeConfigInfo.mcPortNumber = 105UL;
#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1)
    ModeConfigInfo.mcPortNumber = 80UL;
#endif

    ModeConfigInfo.mcOperationMode = PDM_OUTPUT_MODE_PHASE_2;
    ModeConfigInfo.mcClockDivide = 0UL;
    ModeConfigInfo.mcLoopCount = 0UL;
    ModeConfigInfo.mcInversedSignal = 0UL;
    ModeConfigInfo.mcPosition1 = 0UL;
    ModeConfigInfo.mcPosition2 = 0UL;
    ModeConfigInfo.mcPosition3 = 0UL;
    ModeConfigInfo.mcPosition4 = 0UL;
    ModeConfigInfo.mcOutPattern1 = 0UL;
    ModeConfigInfo.mcOutPattern2 = 0UL;
    ModeConfigInfo.mcOutPattern3 = 0UL;
    ModeConfigInfo.mcOutPattern4 = 0UL;
    ModeConfigInfo.mcMaxCount = 0UL;

    ModeConfigInfo.mcDutyNanoSec1 = ((350UL) * (1000UL));
    ModeConfigInfo.mcPeriodNanoSec1 = ((700UL) * (1000UL));
    ModeConfigInfo.mcDutyNanoSec2 = ((150UL) * (1000UL));
    ModeConfigInfo.mcPeriodNanoSec2 = ((300UL) * (1000UL));

    (void)PDM_SetConfig((uint32)PDM_CH_0, (PDMModeConfig_t *)&ModeConfigInfo);

    (void)PDM_Enable((uint32)PDM_CH_0, PMM_ON);

    PDM_TestSleepForSec(20UL);

    (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);
    mcu_printf("\n== End of PDM Phase Mode2 Sample ==\n");

    return;
}

/*
***************************************************************************************************
*                                           PDM_TestRegisterMode1
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/

static void PDM_TestRegisterMode1
(
    void
)
{
    PDMModeConfig_t ModeConfigInfo;

    mcu_printf("\n== Start PDM Register Mode1 Sample ==\n");

    PDM_TestSleepForSec(3UL);
    PDM_TestSetGpio();

    (void)PDM_Init();

    PDM_TestSleepForSec(2UL);

#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1)
    ModeConfigInfo.mcPortNumber = 105UL;
#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1)
    ModeConfigInfo.mcPortNumber = 80UL;
#endif

    ModeConfigInfo.mcOperationMode = PDM_OUTPUT_MODE_REGISTER_1;
    ModeConfigInfo.mcClockDivide = 1UL;
    ModeConfigInfo.mcLoopCount = 0UL;
    ModeConfigInfo.mcInversedSignal = 0UL;
    ModeConfigInfo.mcDutyNanoSec1 = 0UL;
    ModeConfigInfo.mcPeriodNanoSec1 = 0UL;
    ModeConfigInfo.mcDutyNanoSec2 = 0UL;
    ModeConfigInfo.mcPeriodNanoSec2 = 0UL;
    ModeConfigInfo.mcPosition1 = 0UL;
    ModeConfigInfo.mcPosition2 = 0UL;
    ModeConfigInfo.mcPosition3 = 0UL;
    ModeConfigInfo.mcPosition4 = 0UL;
    ModeConfigInfo.mcOutPattern1 = 0x0000FFFFUL;
    ModeConfigInfo.mcOutPattern2 = 0UL;
    ModeConfigInfo.mcOutPattern3 = 0UL;
    ModeConfigInfo.mcOutPattern4 = 0UL;
    ModeConfigInfo.mcMaxCount = 0UL;

    (void)PDM_SetConfig((uint32)PDM_CH_0, &ModeConfigInfo);

    (void)PDM_Enable((uint32)PDM_CH_0, PMM_ON);

    PDM_TestSleepForSec(20UL);

    (void)PDM_Disable((uint32)PDM_CH_0, PMM_ON);
    mcu_printf("\n== End of PDM Register Mode1 Sample ==\n");

    return;
}

/*
***************************************************************************************************
*                                           PDM_SelectTestCase
*
* @param test case number
* @return
*
* Notes
*
***************************************************************************************************
*/
void PDM_SelectTestCase
(
    uint32                              uiTestCase
)
{
    switch(uiTestCase)
    {
        case    1:
        {
            PDM_TestPhaseMode1();
            break;
        }

        case    2:
        {
            PDM_TestPhaseMode2();
            break;
        }

        case    3:
        {
            PDM_TestRegisterMode1();
            break;
        }

        default :
        {
            mcu_printf("\n== Invaild Test Case ==\n");
            break;
        }
    }

    return;
}

