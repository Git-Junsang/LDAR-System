/*
***************************************************************************************************
*
*   FileName : bsp.c
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

#include "bsp.h"

#include <reg_phys.h>
#include <debug.h>

#include <clock.h>
#include <gic.h>
#include <uart.h>
#include <timer.h>
#include <wdt.h>
#include <i2c.h>
#include <gpsb.h>
#include <adc.h>
#include <mbox.h>
#include <pmio.h>

#ifdef FMU_DRIVER
    #include <fmu.h>
#endif
#include <pmu_reset.h>

/*
***************************************************************************************************
                                         STATIC VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void BSP_PreGpioSel
(
    void
);

/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/

void BSP_PreInit
(
    void
)
{
    CLOCK_Init();
    GIC_Init();
    PMU_ResetPreCfg();
}

void BSP_Init
(
    void
)
{
    (void)TIMER_Init();
    (void)UART_Init();
    (void)WDT_Init();

    I2C_Init();
    GPSB_Init();
    ADC_Init(0);

#ifdef FMU_DRIVER
    (void)FMU_Init();
#endif

    MBOX_Init();
    PMIO_Init();

#ifndef MCU_BSP_SOC_REV_TCC805x_ES
    PMU_ResetSCWDTInit(); /* used for CS */
#endif

    mcu_printf("\nInitialize System done\n");
    mcu_printf("Welcome to Telechips MCU BSP\n");
}

/* R0 : ARM Exception ID, R1 : Dummy , R2 : Link Register(Return PC)*/
void BSP_UndefAbortExceptionHook
(
    uint32                              uiExceptId,
    uint32                              uiDummy,
    uint32                              uiLinkReg
)
{
    (void)uiExceptId;
    (void)uiDummy;
    (void)uiLinkReg;
/* Unused function by TCC805X
    SAL_WriteReg(uiExceptId, SRAM_EXCEPTION_SAVE_BASE);
    SAL_WriteReg(uiLinkReg, (SRAM_EXCEPTION_SAVE_BASE+4));
*/
}

/*
Warning !!!  After calliing this function. it will never return from exception except H/W reseting.
This function is designed for very restricted use case such as firmware upgrading.
*/
void BSP_SystemHalt
(
    void
)
{
    (void)SAL_CoreCriticalEnter();  // Interrupt disable(I/F)
    ARM_Reserved();                 // move ARM p.c on the sram.
}

