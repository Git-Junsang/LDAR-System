/*
***************************************************************************************************
*
*   FileName : can_demo.h
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

#ifndef CAN_DEMO_HEADER
#define CAN_DEMO_HEADER

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <can_config.h>
#include <can.h>


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

#define CAN_DEMO_TASK_STK_SIZE          (2048)
#define CAN_MAX_TEST_MSG_NUM            (9UL)

//#define CAN_DEMO_RESPONSE_TEST          //for CAN response test

typedef struct CANDemoTestInfo
{
    uint8                               tiRecv;
    uint8                               tiSendRecv;
} CANDemoTestInfo_t;

#if defined(EDUCATION_CAN_DEMO_EN)
#include <uart.h>

#define CAN_DEMO_ID_MIN                 (0x000)
#define CAN_DEMO_ID_MAX                 (0x7FF)

#define CAN_DEMO_STD_MSG_LEN_MAX        (8)
#define CAN_DEMO_EXT_MSG_LEN_MAX        (64)

#define CAN_DEMO_SEND_STATUS_STOP       (0x00)
#define CAN_DEMO_SEND_STATUS_START      (0x01)

#define CAN_DEMO_CHANNEL_VALUE_A        (0x01)
#define CAN_DEMO_CHANNEL_VALUE_B        (0x02)
#define CAN_DEMO_CHANNEL_VALUE_C        (0x04)

typedef struct CANDemoCounter
{
    uint32                              canID[CAN_DEMO_ID_MAX + 2];
} CANDemoRecvCounter_t;

typedef struct CANDemoControl
{
    uint8                               status;
    uint8                               isRepeat;
    uint32                              repeatInterval;
} CANDemoControl_t;

typedef struct CANDemoMeta
{
    uint8                               isFD;
    uint8                               channel;
    uint32                              baudRate;
    uint32                              fastBaudRate;
    uint32                              idMask;
    uint32                              idAcceptance;
} CANDemoMeta_t;

typedef struct CANDemoValue
{
    uint32                              id;
    uint32                              length;
    uint8                               data[64];
} CANDemoValue_t;

typedef struct CANDemoData
{
    CANDemoControl_t                    control;
    CANDemoMeta_t                       meta;
    CANDemoValue_t                      value;
} CANDemoData_t;
#endif

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/
#if defined(EDUCATION_CAN_DEMO_EN)
void CAN_Edu_resetReceiveFlag
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
);
void CAN_Edu_DemoCreateApp
(
    void
);
#endif

sint32 CAN_DemoInitialize
(
    void
);

void CAN_DemoTest
(
    uint8                               ucArgc,
    void *                              pArgv[]
);

void CAN_DemoCreateApp
(
    void
);

#endif // CAN_DEMO_HEADER

