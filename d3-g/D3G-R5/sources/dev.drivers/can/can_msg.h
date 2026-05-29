/*
***************************************************************************************************
*
*   FileName : can_msg.h
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

#ifndef MCU_BSP_CAN_MSG_HEADER
#define MCU_BSP_CAN_MSG_HEADER

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

typedef struct CANRxRingBuffer
{
    uint32                              rrbInitFlag;
    uint32                              rrbMsgBufferMax;
    uint32                              rrbHeadIdx;
    uint32                              rrbTailIdx;
    CANMessage_t                        rrbMsg[ CAN_RX_MSG_RING_BUFFER_MAX ];
} CANRxRingBuffer_t;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

CANErrorType_t CAN_MsgInit
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_MsgPutRxMessage
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
);

uint32 CAN_MsgGetCountOfRxMessage
(
    uint8                               ucCh
);

CANErrorType_t CAN_MsgGetRxMessage
(
    uint8                               ucCh,
    CANMessage_t *                      psRxMsg
);

CANErrorType_t CAN_MsgSetTxMessage
(
    CANController_t *                   psControllerInfo,
    const CANMessage_t *                psMsg,
    uint8 *                             pucTxBufferIndex
);

/* Not used function */
/*
CANErrorType_t CAN_MsgRequestTxAllMessage
(
    CANController_t *                   psControllerInfo,
    uint32                              uiTxBufferAllIndex
);
*/

CANErrorType_t CAN_MsgRequestTxMessageCancellation
(
    CANController_t *                   psControllerInfo,
    uint8                               ucTxBufferIndex
);

uint32 CAN_MsgGetCountOfTxEvent
(
    uint8                               ucCh
);

CANErrorType_t CAN_MsgGetTxEventMessage
(
    uint8                               ucCh,
    CANTxEvent_t *                      psTxEvtBuffer
);

#if defined(EDUCATION_CAN_DEMO_EN)
CANErrorType_t CAN_Edu_FilteringCanID
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
);

uint8 CAN_Edu_checkFiltering
(
    uint32                              bitTotal,
    uint32                              canId
);
#endif

#endif /* __MCU_BSP_CAN_MSG_HEADER__ */

