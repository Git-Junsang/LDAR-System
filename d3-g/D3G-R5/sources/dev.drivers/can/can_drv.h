/*
***************************************************************************************************
*
*   FileName : can_drv.h
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

#ifndef MCU_BSP_CAN_DRV_HEADER
#define MCU_BSP_CAN_DRV_HEADER

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

typedef struct CANRamAddress
{
    uint32                              raStandardIDFilterListAddr;
    uint32                              raExtendedIDFilterListAddr;
    uint32                              raRxFIFO0Addr;
    uint32                              raRxFIFO1Addr;
    uint32                              raRxBufferAddr;
    uint32                              raTxEventFIFOAddr;
    uint32                              raTxBufferAddr;
} CANRamAddress_t;

typedef struct CANController
{
    /* Config */
    CANMode_t                           cMode;
    uint8                               cChannelHandle;
    uint32                              cFrequency;

    /* Timing */
    CANTimingParam_t *                  cArbiPhaseTimeInfo;
    CANTimingParam_t *                  cDataPhaseTimeInfo;

    /* Buffer */
    CANTxBuffer_t *                     cTxBufferInfo;
    CANRxBuffer_t *                     cDedicatedBufferInfo;
    CANRxBuffer_t *                     cFIFO0BufferInfo;
    CANRxBuffer_t *                     cFIFO1BufferInfo;

    /*Filter */
    uint8                               cNumOfStdIDFilterList;
    CANIdFilterList_t *                 cStdIDFilterList;
    uint8                               cNumOfExtIDFilterList;
    CANIdFilterList_t *                 cExtIDFilterList;

    /* Register & Address*/
    CANRamAddress_t                     cRamAddressInfo;
    CANControllerRegister_t *           cRegister;
    CANCallBackFunc_t *                 cCallbackFunctions;
} CANController_t;

typedef struct CANDriver
{
    uint8                               dIsInitDriver;
    uint8                               dNumOfController;
    CANController_t                     dControllerInfo[ CAN_CONTROLLER_NUMBER ];
} CANDriver_t;


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

CANErrorType_t CAN_DrvInitChannel
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvDeinitChannel
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetNormalOperationMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetMonitoringMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetInternalTestMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetExternalTestMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetWakeUpMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetSleepMode
(
    CANController_t *                   psControllerInfo
);

uint32 CAN_DrvGetProtocolStatus
(
    const CANController_t *             psControllerInfo
);

void CAN_DrvCallbackNotifyRxEvent
(
    uint8                               ucCh,
    uint32                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
);

/* Not used function */
/*
CANErrorType_t CAN_DrvSetDisableAutomaticRetransmission
(
    CANController_t *                   psControllerInfo,
    boolean                             bDisable
);
*/

#endif /* __MCU_BSP_CAN_DRV_HEADER__ */

