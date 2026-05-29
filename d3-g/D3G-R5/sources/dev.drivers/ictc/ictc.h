/*
***************************************************************************************************
*
*   FileName : ictc.h
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

#if !defined(ICTC_HEADER)
#define ICTC_HEADER

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include "sal_com.h"
#include "ictc_dev.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#if (DEBUG_ENABLE)
    #include "debug.h"

    #define ICTC_D(fmt, args...)        {LOGD(DBG_TAG_ICTC, fmt, ## args)}
    #define ICTC_Err(fmt, args...)      {LOGE(DBG_TAG_ICTC, fmt, ## args)}
#else
    #define ICTC_D(fmt, args...)
    #define ICTC_Err(fmt, args...)
#endif

#define ICTC_SUB_ICTC_ENABLED

#define ICTC_TOTAL_CHANNEL_NUM          (3UL)
#define ICTC_DIFF_MARGIN                (0x800UL)
#define ICTC_DiffABSValue(X, Y)         (((X) > (Y)) ? ((X) - (Y)) : ((Y) - (X)))


/******************************************************************************
 * Record Time-stamp
 ******************************************************************************/

#define ICTC_TIMESTAMP_RECORD_ENABLE
#define ICTC_TIMESTAMP_RECORD_MAXCNT    (15UL)

/******************************************************************************
 * CLOCK
 ******************************************************************************/

/* ICTC MAX clock 200Mhz */
#define ICTC_PERI_CLOCK                 ((200UL) * (1000UL) * (1000UL))

#define ICTC_CH_0                       (0UL)
#define ICTC_CH_1                       (1UL)
#define ICTC_CH_2                       (2UL)

typedef void (*ICTCCallback)(uint32 uiChannel, uint32 uiPeriod, uint32 uiDuty);

/******************************************************************************
* struct
******************************************************************************/

typedef struct ICTCCallBack
{
    uint32 								cbChannel;
    ICTCCallback 						cbCallBackFunc;
} ICTCCallBack_t;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          ICTC_Init
*
* Initialize register map
* Notes
*
***************************************************************************************************
*/

void ICTC_Init
(
    void
);

/*
***************************************************************************************************
*                                          ICTC_Deinit
*
* De-Initialize register map
* Notes
*
***************************************************************************************************
*/

void ICTC_Deinit
(
    void
);

/*
***************************************************************************************************
*                                          ICTC_GetPrevPeriodCnt
*
* ICTC get previous period Count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetPrevPeriodCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetPreDutyCnt
*
* ICTC get previous Duty Count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetPreDutyCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetCurEdgeCnt
*
* ICTC get current edge count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetCurEdgeCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetPrvEdgeCnt
*
* ICTC get previous edge count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetPrvEdgeCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetREdgeTstmpCnt
*
* ICTC get rising edge timestamp count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetREdgeTstmpCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_GetFEdgeTstmpCnt
*
* ICTC get falling edge timestamp count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetFEdgeTstmpCnt
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_EnableCapture
*
* ICTC Enable Operation, Enable TCLK and ICTC EN
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

void ICTC_EnableCapture
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_DisableCapture
*
* ICTC Diable Operation, Clear TCLK and ICTC EN
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

void ICTC_DisableCapture
(
    uint32                              uiChannel
);

/*
***************************************************************************************************
*                                          ICTC_SetOpEnCtrlCounter
*
* ICTC set OP_EN_CTRL Counter Register
* @param    channel number 0 ~ 2.
* @param    counter value.
* Notes
*
***************************************************************************************************
*/

void ICTC_SetOpEnCtrlCounter
(
    uint32                              uiChannel,
    uint32                              uiCntEnValue
);

/*
***************************************************************************************************
*                                          ICTC_SetOpModeCtrlReg
*
* ICTC set OP_MODE_CTRL Register
* @param    channel number 0 ~ 2.
* @param    OP_MODE value.
* Notes
*
***************************************************************************************************
*/

SALRetCode_t ICTC_SetOpModeCtrlReg
(
    uint32                              uiChannel,
    uint32                              uiModeValue
);

/*
***************************************************************************************************
*                                          ICTC_SetIRQCtrlReg
*
* ICTC set IRQ_CTRL Register
* @param    channel number 0 ~ 2.
* @param    IRQ_EN value.
* Notes
*
***************************************************************************************************
*/

void ICTC_SetIRQCtrlReg
(
    uint32                              uiChannel,
    uint32                              uiIRQValue
);

/*
***************************************************************************************************
*                                          ICTC_SetCallBackFunc
*
* ICTC set CallbackFunction
* @param    channel number 0 ~ 2.
* @param    CallBack Function
* Notes
*
***************************************************************************************************
*/

void ICTC_SetCallBackFunc
(
    uint32                              uiChannel,
    ICTCCallback                        pCallbackFunc
);

/*
***************************************************************************************************
*                                          ICTC_SetTimeoutValue
*
* ICTC set Time-Out Value Register
* @param    channel number 0 ~ 2.
* @param    timeout value.
* Notes
*
***************************************************************************************************
*/

void ICTC_SetTimeoutValue
(
    uint32                              uiChannel,
    uint32                              uiTimeoutValue
);

/*
***************************************************************************************************
*                                          ICTC_SetEdgeMatchingValue
*
* ICTC set Edge Matching Value, R_EDGE, F_EDGE, EDGE_COUNTER
* @param    channel number 0 ~ 2.
* @param    rising edge matching value.
* @param    falling edge matching value.
* @param    edge counter matching value.
* @return
*
* Notes
*
***************************************************************************************************
*/

void ICTC_SetEdgeMatchingValue
(
    uint32                              uiChannel,
    uint32                              uiRisingEdgeMat,
    uint32                              uiFallingEdgeMat,
    uint32                              uiEdgeCounterMat
);

/*
***************************************************************************************************
*                                          ICTC_SetCompareRoundValue
*
* ICTC set Period/Duty compare round value configuration register
* @param    channel number 0 ~ 2.
* @param    period compare round value.
* @param    duty compare round value.
* Notes
*
***************************************************************************************************
*/

void ICTC_SetCompareRoundValue
(
    uint32                              uiChannel,
    uint32                              uiPeriodCompareRound,
    uint32                              uiDutyCompareRound
);
#endif //_ICTC_HEADER_

