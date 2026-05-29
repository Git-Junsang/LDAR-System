/*
***************************************************************************************************
*
*   FileName : pdm.h
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


#if !defined(PDM_HEADER)
#define PDM_HEADER
/****************************************************************************************************
*                                             INCLUDE FILES
****************************************************************************************************/
#include "clock.h"
#include "clock_dev.h"
#include "sal_com.h"
#include "pdm_dev.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#if (DEBUG_ENABLE)
    #include "debug.h"

    #define PDM_D(fmt, args...)         {LOGD(DBG_TAG_PDM, fmt, ## args)}
    #define PDM_Err(fmt, args...)       {LOGE(DBG_TAG_PDM, fmt, ## args)}
#else
    #define PDM_D(fmt, args...)
    #define PDM_Err(fmt, args...)
#endif

#define PDM_SAFETY_FEATURE_ENABLED

#define PDM_STATE_BUSY                  (1UL)
#define PDM_STATE_IDLE                  (0UL)

#define PDM_ON                          (1UL)
#define PDM_OFF                         (0UL)

#ifdef PDM_SAFETY_FEATURE_ENABLED
#define PMM_ON                          (1UL)
#define PMM_OFF                         (0UL)
#endif

/* The PWM controller consist of 3 PWM modules (PWM-0/1/2). */
#define PDM_TOTAL_MODULES               (3UL)

/* Each PWM module has four separate PWM blocks (PWM-A/B/C/D). */
#define PDM_TOTAL_CH_PER_MODULE         (4UL)

/* total 12channels */
#define PDM_TOTAL_CHANNELS              (PDM_TOTAL_MODULES * PDM_TOTAL_CH_PER_MODULE)

/* PDM MAX clk 400Mhz */
#define PDM_PERI_CLOCK                  (25UL * 1000UL * 1000UL)

/* Clock divide max value 3(divide 16) */
#define PDM_DIVID_MAX                   (3UL)

/*Operation Mode Phase 1 & 2*/
#define PDM_OUTPUT_MODE_PHASE_1         (0x0001UL) //0b0001UL //=> green hills compiler
#define PDM_OUTPUT_MODE_PHASE_2         (0x0009UL) //0b1001UL //=> green hills compiler

/*Operation Mode Register 1 & 2*/
#define PDM_OUTPUT_MODE_REGISTER_1      (0x0002UL) //0b0010UL //=> green hills compiler
#define PDM_OUTPUT_MODE_REGISTER_2      (0x0004UL) //0b0100UL //=> green hills compiler
#define PDM_OUTPUT_MODE_REGISTER_3      (0x0003UL) //0b0110UL //=> green hills compiler

#define REG_OUTPUT_PATTERN_HIGH         (0xFFFFFFFFUL)
#define REG_OUTPUT_PATTERN_LOW          (0x00000000UL)

#define PDM_CONNECT_INTERNAL            (0x00000000UL)
#define PDM_CONNECT_EXTERNAL            (0x00000001UL)

#define PDM_CH_MAX                      (12UL)

/****************************************************************************
 * struct
 ****************************************************************************/

#ifdef PDM_SAFETY_FEATURE_ENABLED
typedef struct PMMISRData
{
    uint32 idChannel;
    uint32 idPmmIrqState;
} PMMISRData_t;
#endif

typedef struct PDMModeConfig
{
    uint32                              mcPortNumber;            /* pdm output port number */
    uint32                              mcOperationMode;         /* pdm mode & enable */
    uint32                              mcClockDivide;
    uint32                              mcLoopCount;

    uint32                              mcInversedSignal;

    uint32                              mcPeriodNanoSec1;        /* period in nanosecond */
    uint32                              mcDutyNanoSec1;          /* duty cycle in nanosecond */
    uint32                              mcPeriodNanoSec2;        /* period in nanosecond */
    uint32                              mcDutyNanoSec2;          /* duty cycle in nanosecond */

    uint32                              mcPosition1;             /* pstn1 for Phase mode 1,2 */
    uint32                              mcPosition2;             /* pstn2 for Phase mode 1,2 */
    uint32                              mcPosition3;             /* pstn3 for Phase mode 2 */
    uint32                              mcPosition4;             /* pstn4 for Phase mode 2 */

    uint32                              mcOutPattern1;           /* output pattern1 for register out mode 1,2,3 */
    uint32                              mcOutPattern2;           /* output pattern2 for register out mode 2,3 */
    uint32                              mcOutPattern3;           /* output pattern3 for register out mode 2,3 */
    uint32                              mcOutPattern4;           /* output pattern4 for register out mode 2,3 */
    uint32                              mcMaxCount;              /* max count for register mode3 */
} PDMModeConfig_t;

typedef struct PDMChannelHandler
{
    uint32                              chModuleId;              /* pdm module 0/1/2 */
    uint32                              chChannelId;             /* pdm channel A/B/C/D */
    uint32                              chEnable;                /* output enable */
    uint32                              chOutSignalInIdle;
    uint32                              chIdleState;             /* divide peri clock */
    PDMModeConfig_t                     chModeCfgInfo;

#ifdef PDM_SAFETY_FEATURE_ENABLED
    uint32                              chPMMEnable;             /* monitoring module enable */
    uint32                              chPMMInputCnect;         /* monitoring module input connection (internal or external) */
    uint32                              chPMMFaultStatus;        /* pdm fault status */
    uint32                              chPMMTimeoutValue;       /* for test duty margin*/
    uint32                              chPMMDutyMargin;         /* for test duty margin*/
    uint32                              chPMMErrChannel;         /* last error channel */
#endif
} PDMChannelHandler_t;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          PDM_EnableClock
*
* Enable PDM peripheral clock.
* Notes
*
***************************************************************************************************
*/

void PDM_EnableClock
(
    void
);

/*
***************************************************************************************************
*                                          PDM_DisableClock
*
* Disable PDM peripheral clock.
* Notes
*
***************************************************************************************************
*/

void PDM_DisableClock
(
    void
);

/*
***************************************************************************************************
*                                          PDM_Init
*
* Initialize variable for register map and setup FMU for error Handling.
*
* Notes
*
***************************************************************************************************
*/

void PDM_Init
(
    void
);

/*
***************************************************************************************************
*                                          PDM_Deinit
*
* De-initialize PDM descriptor.
*
* Notes
*
***************************************************************************************************
*/

void PDM_Deinit
(
    void
);

/*
***************************************************************************************************
*                                          PDM_GetChannelStauts
*
* Get channel status
*
* @param    channel number 0 ~ 11.
* @return   return value 1 means PDMn-X is busy status
*
* Notes
*
***************************************************************************************************
*/

uint32 PDM_GetChannelStatus
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          PDM_Enable
*
* Enable pdm module, setup OP_EN register, and Trigger output signal
*
* @param    channel number 0 ~ 11.
* @param    Enable PMM monitoring.
* @return
*
* Notes To operate PDM correctly MUST
        set all configuration and enable first
        set VUP to 1 to announce the end of register update
        set TRIG to 1, PDM output will be generated correctly.
*
***************************************************************************************************
*/

SALRetCode_t PDM_Enable
(
    uint32                              uiChannel,
    uint32                              uiMonitoring
);

/*
***************************************************************************************************
*                                          PDM_Disable
*
* Disable pdm module
*
* @param    channel number 0 ~ 11.
* @param    Disable PMM monitoring.
* @return
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t PDM_Disable
(
    uint32                              uiChannel,
    uint32                              uiMonitoring
);

/*
***************************************************************************************************
*                                          PDM_SetConfig
*
* Set Configuration value for PMM
*
* @param    channel number 0 ~ 11.
* @param    Operation Mode Configure Information.
* @return
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t PDM_SetConfig
(
    uint32                              uiChannel,
    PDMModeConfig_t                     *pModeConfig
);

#endif //_PDM_HEADER_

