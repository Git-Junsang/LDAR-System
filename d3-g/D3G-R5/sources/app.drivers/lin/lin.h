/*
***************************************************************************************************
*
*   FileName : lin.h
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

#ifndef LIN_HEADER
#define LIN_HEADER

/****************************************************************************************************
*                                             INCLUDE FILES
****************************************************************************************************/

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define LIN_OK                          (0)
#define LIN_ERROR                       (-1)

#if (DEBUG_ENABLE)
#include "debug.h"
#define LIN_D(fmt, args...)             {LOGD(DBG_TAG_LIN, fmt, ## args)}
#define LIN_E(fmt, args...)             {LOGE(DBG_TAG_LIN, fmt, ## args)}
#else
#define LIN_D(fmt, args...)
#define LIN_E(fmt, args...)
#endif


/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

#define LIN_MAX_DATA_SIZE               (8U)

typedef struct LINConfig
{
    uint32                              uiLinCh;
    uint32                              uiPortSel;
    uint32                              uiBaud;
    uint32                              uiLinSlpPin;
} LINConfig_t;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

sint8 LIN_Init
(
    LINConfig_t                         sLinCfg
);

sint8 LIN_SendData
(
    LINConfig_t                         sLinCfg,
    uint8                               ucId,
    uint8 *                             pucData,
    uint8                               ucSize
);

sint8 LIN_ReadData
(
    LINConfig_t                         sLinCfg,
    uint8                               ucId,
    uint8 *                             pucData
);

sint8 LIN_TransceiverWakeup
(
    LINConfig_t                         sLinCfg,
    boolean                             bStatus
);

#endif  // LIN_HEADER

