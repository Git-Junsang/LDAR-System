/*
***************************************************************************************************
*
*   FileName : ictc_test.c
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

#include "ictc.h"
#include "ictc_test.h"
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

static void ICTC_TestGetPeriodAndDuty
(
    void
);

static void ICTC_TestSetGpio
(
    void
);

static void ICTC_TestSleepForSec
(
    uint32                              uiSec
);

static void ICTC_GetPeriodAndDutyCallBack
(
    uint32                              uiChannel,
    uint32                              uiPeriod,
    uint32                              uiDuty
);

/*************************************************************************************************/
/*                                                                                               */
/*                                                                                               */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*                                                                                               */
/*************************************************************************************************/

static void ICTC_GetPeriodAndDutyCallBack
(
    uint32                              uiChannel,
    uint32                              uiPeriod,
    uint32                              uiDuty
)
{
    if(uiChannel == (uint32)0UL)
    {
        mcu_printf("\n ICTC %d- SPEED DETECT period %d duty %d  \n", uiChannel, uiPeriod, uiDuty);
    }
    else if(uiChannel == (uint32)1UL)
    {
        mcu_printf("\n ICTC %d- ILL DETECT period %d duty %d  \n", uiChannel, uiPeriod, uiDuty);
    }
    else
    {
        mcu_printf("Invaid channel \n");
    }
}

/*************************************************************************************************/
/*                                                                                               */
/*                                                                                               */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*                                                                                               */
/*************************************************************************************************/

static void ICTC_TestSleepForSec
(
    uint32                              uiSec
)
{
    (void)SAL_TaskSleep(uiSec*(1000UL));
}

/*************************************************************************************************/
/*                                                                                               */
/*                                                                                               */
/* @param                                                                                        */
/* @return                                                                                       */
/*                                                                                               */
/* Notes                                                                                         */
/*                                                                                               */
/*************************************************************************************************/

static void ICTC_TestSetGpio
(
    void
)
{

#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1) //TCC8059
    /*8059*/
    /* VEHICLE SPEED DETECT */
    (void)GPIO_Config(GPIO_GPMA(20UL) , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_CD(0x3UL)));

    /* VEHICLE ILL DETECT */
    (void)GPIO_Config(GPIO_GPMA(17UL) , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_CD(0x3UL)));

#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1) //TCC8050
    /*8050*/
    /* VEHICLE SPEED DETECT */
    (void)GPIO_Config(GPIO_GPMB(1UL) , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_CD(0x3UL)));

    /* VEHICLE ILL DETECT */
    (void)GPIO_Config(GPIO_GPMB(25UL) , (GPIO_FUNC(0UL) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_CD(0x3UL)));

#endif

}

/*
***************************************************************************************************
*                                       ICTC_TestGetPeriodAndDuty
*
* @param
* @return
*
* Notes
*
***************************************************************************************************
*/
static void ICTC_TestGetPeriodAndDuty
(
    void
)
{
    mcu_printf(" \n===== GET DUTY ADN PERIOD TEST ICTC-%d ==== \n", ICTC_CH_0);

    /*Initialize ictc module*/
    ICTC_Init();

    /*Set GPIO*/
    ICTC_TestSetGpio();

    /*Reset*/
    ICTC_SetOpEnCtrlCounter((uint32)ICTC_CH_0, ICTC_OPEN_CTRL_RESET_VALUE);
    ICTC_SetOpEnCtrlCounter((uint32)ICTC_CH_1, ICTC_OPEN_CTRL_RESET_VALUE);

    /*Reset*/
    (void)ICTC_SetOpModeCtrlReg((uint32)ICTC_CH_0, ICTC_OPMODE_CTRL_RESET_VALUE);
    (void)ICTC_SetOpModeCtrlReg((uint32)ICTC_CH_1, ICTC_OPMODE_CTRL_RESET_VALUE);

    /*set callback*/
    ICTC_SetCallBackFunc((uint32)ICTC_CH_0, (ICTCCallback)&ICTC_GetPeriodAndDutyCallBack);
    ICTC_SetCallBackFunc((uint32)ICTC_CH_1, (ICTCCallback)&ICTC_GetPeriodAndDutyCallBack);

    /*set timeout value*/
    ICTC_SetTimeoutValue((uint32)ICTC_CH_0, 0x0fffffffUL);
    ICTC_SetTimeoutValue((uint32)ICTC_CH_1, 0x0fffffffUL);

    /*set Matching value*/
    ICTC_SetEdgeMatchingValue((uint32)ICTC_CH_0, 0x00000100UL, 0x00000100UL, 0x0000FFFFUL);
    ICTC_SetEdgeMatchingValue((uint32)ICTC_CH_1, 0x00000100UL, 0x00000100UL, 0x0000FFFFUL);

    /*set Round value*/
    ICTC_SetCompareRoundValue((uint32)ICTC_CH_0, 0x0ffff800UL, 0x0ffff800UL);
    ICTC_SetCompareRoundValue((uint32)ICTC_CH_1, 0x0ffff800UL, 0x0ffff800UL);

#if (TCC_EVM_BD_VERSION == TCC8059_BD_VER_0_1) //TCC8059
    /*set OP_MODE value*/
    (void)ICTC_SetOpModeCtrlReg((uint32)ICTC_CH_0,
                          (ICTC_OPMODE_CTRL_ABS_SEL |
                          ICTC_OPMODE_CTRL_RISING_EDGE |
                          ICTC_OPMODE_CTRL_TCLK_BYPASS |
                          ICTC_OPMODE_CTRL_FLT_IMM_F_MODE |
                          ICTC_OPMODE_CTRL_FLT_IMM_R_MODE |
                          ICTC_OPMODE_CTRL_DUTY_CMP_ISR |
                          (154UL /*MA17*/ << ICTC_OPMODE_CTRL_F_IN_SEL_OFFSET)));

    (void)ICTC_SetOpModeCtrlReg((uint32)ICTC_CH_1,
                          (ICTC_OPMODE_CTRL_ABS_SEL |
                          ICTC_OPMODE_CTRL_RISING_EDGE |
                          ICTC_OPMODE_CTRL_TCLK_BYPASS |
                          ICTC_OPMODE_CTRL_FLT_IMM_F_MODE |
                          ICTC_OPMODE_CTRL_FLT_IMM_R_MODE |
                          ICTC_OPMODE_CTRL_DUTY_CMP_ISR |
                          (151UL /*MA20*/ << ICTC_OPMODE_CTRL_F_IN_SEL_OFFSET)));

#elif (TCC_EVM_BD_VERSION == TCC8050_BD_VER_0_1) //TCC8050
    /*set OP_MODE value*/
    (void)ICTC_SetOpModeCtrlReg((uint32)ICTC_CH_0,
                          (ICTC_OPMODE_CTRL_ABS_SEL |
                          ICTC_OPMODE_CTRL_RISING_EDGE |
                          ICTC_OPMODE_CTRL_TCLK_BYPASS |
                          ICTC_OPMODE_CTRL_FLT_IMM_F_MODE |
                          ICTC_OPMODE_CTRL_FLT_IMM_R_MODE |
                          ICTC_OPMODE_CTRL_DUTY_CMP_ISR |
                          (165UL /*MB1*/ << ICTC_OPMODE_CTRL_F_IN_SEL_OFFSET)));

    (void)ICTC_SetOpModeCtrlReg((uint32)ICTC_CH_1,
                          (ICTC_OPMODE_CTRL_ABS_SEL |
                          ICTC_OPMODE_CTRL_RISING_EDGE |
                          ICTC_OPMODE_CTRL_TCLK_BYPASS |
                          ICTC_OPMODE_CTRL_FLT_IMM_F_MODE |
                          ICTC_OPMODE_CTRL_FLT_IMM_R_MODE |
                          ICTC_OPMODE_CTRL_DUTY_CMP_ISR |
                          (189UL /*MB25*/ << ICTC_OPMODE_CTRL_F_IN_SEL_OFFSET)));
#endif


    /*set OP_EN Counter value*/
    ICTC_SetOpEnCtrlCounter((uint32)ICTC_CH_0,
                            ICTC_OPEN_CTRL_TSCNT_EN |
                            ICTC_OPEN_CTRL_TOCNT_EN |
                            ICTC_OPEN_CTRL_FLTCNT_EN |
                            ICTC_OPEN_CTRL_PDCMPCNT_EN);

    ICTC_SetOpEnCtrlCounter((uint32)ICTC_CH_1,
                            ICTC_OPEN_CTRL_TSCNT_EN |
                            ICTC_OPEN_CTRL_TOCNT_EN |
                            ICTC_OPEN_CTRL_FLTCNT_EN |
                            ICTC_OPEN_CTRL_PDCMPCNT_EN);

    /*set IRQ value*/
    ICTC_SetIRQCtrlReg((uint32)ICTC_CH_0,
                       ICTC_IRQ_CTRL_DT_ERR_IRQ |
                       ICTC_IRQ_CTRL_PRD_ERR_IRQ );

    ICTC_SetIRQCtrlReg((uint32)ICTC_CH_1,
                       ICTC_IRQ_CTRL_DT_ERR_IRQ |
                       ICTC_IRQ_CTRL_PRD_ERR_IRQ );

    /* set TCLK and ICTC_EN value */
    ICTC_EnableCapture((uint32)ICTC_CH_0);
    ICTC_EnableCapture((uint32)ICTC_CH_1);

    /*Test for 30sec*/
   	ICTC_TestSleepForSec(10UL);

    /* disable TCLK and ICTC_EN value */
    ICTC_DisableCapture((uint32)ICTC_CH_0);
    ICTC_DisableCapture((uint32)ICTC_CH_1);

    mcu_printf(" \n===== End of GET DUTY ADN PERIOD ==== \n");

    return;
}

/*
***************************************************************************************************
*                                       ICTC_SelectTestCase
*
* @param    test case number
* @return
*
* Notes
*
***************************************************************************************************
*/

void ICTC_SelectTestCase
(
    uint32                              uiTestCase
)
{
    switch(uiTestCase)
    {
        case    1:
        {
            ICTC_TestGetPeriodAndDuty();
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

