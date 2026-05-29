/*
***************************************************************************************************
*
*   FileName : pmio.h
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

#ifndef  MCU_BSP_PMIO_HEADER
#define  MCU_BSP_PMIO_HEADER

#include <sal_internal.h>
#include <gic_enum.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/* CONFIGURATION for PMIO FUNCTION ===================================================*/
/*
    PMIO_CONF_DEBUG_ALONE   : [Def] Print always about errors and debugs log.
                                        (No use a LOG_D/E console.)

    PMIO_CONF_PREVENT_LEAK  : [Def] Prevent power current leakage
                                        when sys enter power down or str mode.
                                        The settings are tailored to TC EVM environment only.

    PMIO_CONF_DET_BU        : [Def] Detect BU(GPK1) signal.
										refer to sources/dev.drivers/pmio/tcc805x/pmio_dev.c
										"#define PMIO_VA_BU_DET_PORT  (PMIO_VA_BIT1)"

    PMIO_CONF_DET_ACC       : [Def] Detect ACC(GPK3) signal.
										refer to sources/dev.drivers/pmio/tcc805x/pmio_dev.c
										"#define PMIO_VA_ACC_DET_PORT (PMIO_VA_BIT3)"

    PMIO_CONF_DET_CAN       : [Def] Detect CAN(GPK14) signal.
										refer to sources/dev.drivers/pmio/tcc805x/pmio_dev.c
										"#define PMIO_VA_CAN_DET_PORT (PMIO_VA_BIT14)"

    PMIO_CONF_STR_MODE      : [Def] Run in a suspend scenario, not a normal power down.
                                        Need to support H/W suspend mode.
                                        Need to support S/W Mailbox driver.
                                        Need to support S/W Timer driver.
                                        Need to support S/W AP's suspend mode and scenario.

    PMIO_CONF_MISC_DEV_FUNC : [Def] Supports miscellaneous test functions for further development.
                                        Append backup ram test (PMIO_FEATURE_DEV_TEST_BACKUP_RAM)
                                        Append cold reset test (PMIO_FEATURE_DEV_TEST_CRST)
                                            Need to support S/W Mailbox driver.
                                            Need to support S/W AP's cold rest test scenario.
*/

#define PMIO_CONF_DEBUG_ALONE
//#define PMIO_CONF_PREVENT_LEAK
//#define PMIO_CONF_DET_BU
#define PMIO_CONF_DET_ACC
#define PMIO_CONF_DET_CAN
#define PMIO_CONF_STR_MODE
//#define PMIO_CONF_MISC_DEV_FUNC

/* =================================================== CONFIGURATION for PMIO FUNCTION*/


/*PMIO interrupt*/
#define PMIO_VA_INTERRUPT_SRC           ((uint32)(GIC_PMGPIO))
#define PMIO_VA_RTC_INTERRUPT_SRC       ((uint32)(GIC_RTC))

#define PMIO_GPK(x)                     ((uint32)1<<(uint32)x)
#define PMIO_FUNC_MSEC_TO_USEC(x)       ((uint32)(x) * (uint32)(1000UL))
#define PMIO_FUNC_SEC_TO_USEC(x)        ((uint32)(x) * (uint32)(1000UL) * (uint32)(1000UL))

/*PMIO enum*/
typedef enum
{
    PMIO_MODE_NOT_SET                   = 0,
    PMIO_MODE_POWER_ON                  = 1,
    PMIO_MODE_ACTIVE                    = 2,
    PMIO_MODE_STR                       = 3,
    PMIO_MODE_POWER_DOWN                = 4,
    PMIO_MODE_MAX                       = 5
} PMIOMode_t;

typedef enum
{
    PMIO_REASON_NOT_SET                 = 0,
    PMIO_REASON_BU_ON                   = 1,
    PMIO_REASON_ACC_ON                  = 2,
    PMIO_REASON_CAN_ON                  = 3,
    PMIO_REASON_RTC_ON                  = 4,
    PMIO_REASON_MAX                     = 5
} PMIOReason_t;

#ifdef PMIO_CONF_STR_MODE
    typedef enum
    {
        PMIO_MSG_NOT_SET                = 0,
        PMIO_MSG_ACK                    = 1, //used      (ALL)(AP->R5 : for first mbox ready check)
        PMIO_MSG_ACC_ON                 = 2, //used      (R5->AP)
        PMIO_MSG_STR                    = 3, //used      (R5->AP)
        PMIO_MSG_STR_DONE               = 4, //used      (AP->R5)
        PMIO_MSG_POWER_DOWN             = 5, //not used  (R5->AP)
        PMIO_MSG_POWER_DOWN_DONE        = 6, //not used  (AP->R5)
        PMIO_MSG_AP_AWAKE               = 7, //used      (AP->R5)
        PMIO_MSG_MAX                    = 8
    } PMIOMsg_t;
#endif

typedef enum
{
        PMIO_PORT_SEL_RELEASE            = 0,

        PMIO_PORT_SEL_GPIO               = 1,

        PMIO_PORT_SEL_PMGPIO_IN          = 2,
        PMIO_PORT_SEL_PMGPIO_OUT_L       = 3,
        PMIO_PORT_SEL_PMGPIO_OUT_H       = 4,
        PMIO_PORT_SEL_PMGPIO_PU          = 5,
        PMIO_PORT_SEL_PMGPIO_PD          = 6,

        PMIO_PORT_SEL_PCTLIO_IN          = 7,
        PMIO_PORT_SEL_PCTLIO_OUT_L       = 8,
        PMIO_PORT_SEL_PCTLIO_OUT_H       = 9,
        PMIO_PORT_SEL_PCTLIO_PU          = 10,
        PMIO_PORT_SEL_PCTLIO_PD          = 11
} PMIOPortSel_t;

typedef enum
{
    PMIO_TIMER_REASON_NOT_SET             = 0,
    PMIO_TIMER_REASON_ACC_OFF             = 1,
    PMIO_TIMER_REASON_STR_REQ             = 2
} PMIOTimerReason_t;


/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

/**************************************************************************************************/
/*                                         FUNCTION PROTOTYPES                                    */
/**************************************************************************************************/

void PMIO_EarlyWakeUp
(
    void
);

void PMIO_Init
(
    void
);

void PMIO_PowerDown
(
    PMIOMode_t                          tMode
);

void PMIO_SetGPK
(
    uint32                               uiPort32
);

uint32 PMIO_GetBackupRam
(
    uint32                              uiAddr
);

sint8 PMIO_SetBackupRam
(
    uint32                              uiAddr,
    uint32                              uiVa
);

void PMIO_ClearBackupRAM
(
    void
);

PMIOReason_t PMIO_GetBootReason
(
    void
);

uint8 PMIO_GetWdtReset
(
    void
);

void PMIO_DebugPrintPowerStatus
(
    void
);

void PMIO_ForceSTRModeTest
(
    void
);

void PMIO_APBootOk
(
    void
);
#endif

