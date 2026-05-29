/*
***************************************************************************************************
*
*   FileName : can_porting.h
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

#ifndef MCU_BSP_CAN_PORTING_HEADER
#define MCU_BSP_CAN_PORTING_HEADER

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include "gic.h"
#include "pmio.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

#if defined (MCU_BSP_BOARD_VER_TCC8059_EVM_0_1)
    #define CAN_0_FS                        (PMIO_GPK(14)|PMIO_GPK(13)|PMIO_GPK(8)|PMIO_GPK(6))
    #define CAN_0_TX                        (GPIO_GPK(13UL))
    #define CAN_0_RX                        (GPIO_GPK(14UL))
    #define CAN_0_STB                       (GPIO_GPK(8UL))
    #define CAN_0_INH                       (GPIO_GPK(6UL))

    #define CAN_1_FS                        (PMIO_GPK(16)|PMIO_GPK(15))
    #define CAN_1_TX                        (GPIO_GPK(15UL))
    #define CAN_1_RX                        (GPIO_GPK(16UL))
    #define CAN_1_STB                       (GPIO_GPMA(9UL))
    #define CAN_1_INH                       (0UL) //No INH for CAN1

    #define CAN_2_FS                        (PMIO_GPK(18)|PMIO_GPK(17))
    #define CAN_2_TX                        (GPIO_GPK(17UL))
    #define CAN_2_RX                        (GPIO_GPK(18UL))
    #define CAN_2_STB                       (GPIO_GPMA(10UL))
    #define CAN_2_INH                       (0UL) //No INH for CAN2
#elif defined (MCU_BSP_BOARD_VER_TCC8050_EVM_0_1)
    #define CAN_0_FS                        (PMIO_GPK(14)|PMIO_GPK(13)|PMIO_GPK(8)|PMIO_GPK(6))
    #define CAN_0_TX                        (GPIO_GPK(13UL))
    #define CAN_0_RX                        (GPIO_GPK(14UL))
    #define CAN_0_STB                       (GPIO_GPK(8UL))
    #define CAN_0_INH                       (GPIO_GPK(6UL))

    #define CAN_1_FS                        (PMIO_GPK(16)|PMIO_GPK(15))
    #define CAN_1_TX                        (GPIO_GPK(15UL))
    #define CAN_1_RX                        (GPIO_GPK(16UL))
    #define CAN_1_STB                       (GPIO_GPMC(0UL))
    #define CAN_1_INH                       (0UL) //No INH for CAN1

    #define CAN_2_FS                        (PMIO_GPK(18)|PMIO_GPK(17))
    #define CAN_2_TX                        (GPIO_GPK(17UL))
    #define CAN_2_RX                        (GPIO_GPK(18UL))
    #define CAN_2_STB                       (GPIO_GPMC(1UL))
    #define CAN_2_INH                       (0UL) //No INH for CAN2
#else
    ***** ERROR - SELECT EVB *****
#endif


#define CAN_SAL_DELAY_SUPPORT           (0)

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

void CAN_PortingDelay
(
    uint32                              uiMsDelay
);

CANErrorType_t CAN_PortingInitHW
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_PortingSetControllerClock
(
    CANController_t *                   psControllerInfo,
    uint32                              uiFreq
);

CANErrorType_t CAN_PortingResetDriver
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_PortingSetInterruptHandler
(
    CANController_t *                   psControllerInfo,
    GICIsrFunc                          fnIsr
);

CANErrorType_t CAN_PortingDisableControllerInterrupts
(
    uint8                               ucCh
);

CANErrorType_t CAN_PortingEnableControllerInterrupts
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofStandardIDFilterList
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofExtendedIDFilterList
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofRxFIFO0
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofRxFIFO1
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofRxBuffer
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofTxEventBuffer
(
    uint8                               ucCh
);

uint32 CAN_PortingGetSizeofTxBuffer
(
    uint8                               ucCh
);

uint8 CAN_PortingGetBitRateSwitchEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetFDEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetStandardIDFilterEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetExtendedIDFilterEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetStandardIDRemoteRejectEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetExtendedIDRemoteRejectEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetTxEventFIFOEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetWatchDogEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetTimeOutEnable
(
    uint8                               ucCh
);

uint8 CAN_PortingGetTimeStampEnable
(
    uint8                               ucCh
);

uint32 CAN_PortingAllocateNonCacheMemory
(
    uint8                               ucCh,
    uint32                              uiMemSize
);

void CAN_PortingDeallocateNonCacheMemory
(
    uint8                               ucCh,
    uint32 *                            puiMemAddr
);

uint32 CAN_PortingGetControllerRegister
(
    uint8                               ucCh
);

uint32 CAN_PortingGetConfigBaseAddr
(
    void
);

uint32 CAN_PortingGetMessageRamBaseAddr
(
    uint8                               ucCh
);

uint32 CAN_PortingGetConfigEXTS0Addr
(
    uint8                               ucCh
);

uint32 CAN_PortingGetConfigEXTS1Addr
(
    uint8                               ucCh
);

uint32 CAN_PortingGetConfigWritePasswordAddr
(
    uint8                               ucCh
);

uint32 CAN_PortingGetConfigWriteLockAddr
(
    uint8                               ucCh
);

#endif // __MCU_BSP_CAN_PORTING_HEADER__

