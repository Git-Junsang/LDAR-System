/*
***************************************************************************************************
*
*   FileName : pmu_reset.h
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

#ifndef PMU_RESET_HEADER
#define PMU_RESET_HEADER

#include <pmu_reg.h>


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*******************************************************************************
*    Task definitions
********************************************************************************/
#define PMU_RST_SC_WDT_TASK_STK_SIZE    (1024)

/*******************************************************************************
*    MBOX definitions
********************************************************************************/
#define PMU_RST_MBOX_ID                 (0x01UL)

/*******************************************************************************
*    Timeout definitions
********************************************************************************/
#define PMU_RST_PWRDN_ACK_TIMEOUT       (100UL)


/*
***************************************************************************************************
*                                          LOCAL VARIABLES
***************************************************************************************************
*/


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

void PMU_ResetPreCfg
(
    void
);

void PMU_ResetSDMRstMskCfg
(
    void
);

void PMU_ResetTRVCRstMskCfg
(
    void
);

void PMU_ResetColdReset
(
    void
);

void PMU_ResetAPSystemReboot
(
    void
);

SALRetCode_t PMU_ResetSCWDTInit
(
    void
);


#endif  // PMU_RESET_HEADER

