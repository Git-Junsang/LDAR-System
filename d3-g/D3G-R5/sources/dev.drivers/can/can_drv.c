/*
***************************************************************************************************
*
*   FileName : can_drv.c
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

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <gic.h>
#include <clock.h>
#include "can_config.h"
#include "can_reg.h"
#include "can.h"
#include "can_par.h"
#include "can_drv.h"
#include "can_msg.h"
#include "can_porting.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static CANErrorType_t CAN_DrvSetTimeStamp
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetTimeoutValue
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetWatchDog
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetControlConfig
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvCopyFilterList
(
    CANController_t *                   psControllerInfo,
    CANIdType_t                         uiFilterType
);

static CANErrorType_t CAN_DrvSetFilterConfig
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetBitrate
(
    CANController_t *                   psControllerInfo,
    CANBitRateTiming_t                  ucTimingType
);

static CANErrorType_t CAN_DrvInitTiming
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvStartConfigSetting
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvFinishConfigSetting
(
    CANController_t *                   psControllerInfo
);

static uint32 CAN_DrvGetSizeOfRamMemory
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvRegisterParameterAll
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvInitBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiMemory,
    uint32                              uiMemorySize
);

static CANErrorType_t CAN_DrvDeinitBuffer
(
    CANController_t *                   psControllerInfo
);

static void CAN_DrvIRQHandler
(
    void *pArg
);

static CANErrorType_t CAN_DrvSetInterruptConfig
(
    CANController_t *                   psControllerInfo
);

static void CAN_DrvProcessIRQ
(
    uint8                               ucCh
);

static void CAN_DrvCallbackNotifyTxEvent
(
    uint8                               ucCh,
    CANTxInterruptType_t                uiIntType
);

static void CAN_DrvCallbackNotifyErrorEvent
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
);

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

CANErrorType_t CAN_DrvInitChannel
(
    CANController_t *                   psControllerInfo
)
{
    uint8               ucCh;
    uint32              uiMemSize;
    uint32              uiMemAddr;
    CANRegBaseAddr_t *  psConfigBaseRegAddr;
    CANErrorType_t      ret;

    uiMemAddr           = 0UL;
    ret                 = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL_PTR ) && ( psControllerInfo->cMode == CAN_MODE_NO_INITIALIZATION ) )
    {
        /* Set Config */
        ucCh = psControllerInfo->cChannelHandle;
        psControllerInfo->cRegister = ( CANControllerRegister_t * ) CAN_PortingGetControllerRegister( ucCh );

        /* Set HW_Init */
        ( void ) CAN_PortingInitHW( psControllerInfo );
        ( void ) CAN_PortingSetControllerClock( psControllerInfo, CAN_CONTROLLER_CLOCK );
        ( void ) CAN_PortingResetDriver( psControllerInfo );

        /* Set Memory */
        uiMemSize = CAN_DrvGetSizeOfRamMemory( psControllerInfo );

        if( 0UL < uiMemSize )
        {
            uiMemAddr = CAN_PortingAllocateNonCacheMemory( ucCh, uiMemSize );
        }

        if( uiMemAddr != 0UL )
        {
            psConfigBaseRegAddr = ( CANRegBaseAddr_t * ) CAN_PortingGetMessageRamBaseAddr( ucCh );

            if( psConfigBaseRegAddr != NULL_PTR )
            {
                psConfigBaseRegAddr->rFReg.rfBASE_ADDR = ( SALReg32 ) ( ( uint32 ) ( uiMemAddr >> ( uint32 ) 16UL ) & ( uint32 ) 0xFFFFUL );
            }

            ( void ) CAN_DrvRegisterParameterAll( psControllerInfo );

            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );
            ( void ) CAN_DrvSetControlConfig( psControllerInfo );

            /* Set Buffer */
            ( void ) SAL_MemSet( ( void * ) uiMemAddr, 0, uiMemSize );
            ( void ) CAN_DrvInitBuffer( psControllerInfo, uiMemAddr, uiMemSize );
            ( void ) CAN_MsgInit( psControllerInfo );

            /* Set Filter */
            ( void ) CAN_DrvSetFilterConfig( psControllerInfo );

            /* Set Timing */
            ( void ) CAN_DrvInitTiming( psControllerInfo );

            /* Set Interrupt */
            ( void ) CAN_DrvSetInterruptConfig( psControllerInfo );

            /* Set TimeStamp */
            ( void ) CAN_DrvSetTimeStamp( psControllerInfo );

            /* Set TimeOut */
            ( void ) CAN_DrvSetTimeoutValue( psControllerInfo );

            /* Set WatchDog */
            ( void ) CAN_DrvSetWatchDog( psControllerInfo );

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_OPERATION;
        }
        else
        {
            ret = CAN_ERROR_ALLOC;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_DrvDeinitChannel
(
    CANController_t *                   psControllerInfo
)
{
    uint8               ucCh;
    CANRegBaseAddr_t *  psConfigBaseRegAddr;
    CANErrorType_t      ret;

    psConfigBaseRegAddr = NULL_PTR;
    ret                 = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        /* Set Config */
        ucCh = psControllerInfo->cChannelHandle;

        /* Set HW_Init */
        ret = CAN_DrvStartConfigSetting( psControllerInfo );

        /* Set Buffer */
        if( ret == CAN_ERROR_NONE )
        {
            ( void ) CAN_MsgInit( psControllerInfo );
        }

        /* Free Memory */
        if( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr != 0UL )
        {
            CAN_PortingDeallocateNonCacheMemory( ucCh, ( uint32 * ) &( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr ) );
        }

        ( void ) CAN_DrvDeinitBuffer( psControllerInfo );

        /* Set Message RAM base address */
        psConfigBaseRegAddr = ( CANRegBaseAddr_t * ) CAN_PortingGetMessageRamBaseAddr( ucCh );

        if( psConfigBaseRegAddr != NULL_PTR )
        {
            psConfigBaseRegAddr->rFReg.rfBASE_ADDR = 0;
        }

        psControllerInfo->cMode = CAN_MODE_NO_INITIALIZATION;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Config */
CANErrorType_t CAN_DrvSetNormalOperationMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            if( psControllerInfo->cMode == CAN_MODE_MONITORING )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfMON = FALSE;
            }
            else if( psControllerInfo->cMode == CAN_MODE_INTERNAL_TEST )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = FALSE;
                psControllerInfo->cRegister->crTest.rFReg.rfLBCK = FALSE;
                psControllerInfo->cRegister->crCCControl.rFReg.rfMON = FALSE;
            }
            else if( psControllerInfo->cMode == CAN_MODE_EXTERNAL_TEST )
            {
                psControllerInfo->cRegister->crTest.rFReg.rfLBCK = FALSE;
                psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = FALSE;
            }
            else
            {
                ; /* Nothing to do */
            }

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_OPERATION;

            CAN_D( "[CAN][Ch:%d] Set OPERATION Mode \r\n", psControllerInfo->cChannelHandle );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

CANErrorType_t CAN_DrvSetMonitoringMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfMON = TRUE;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_MONITORING;

            CAN_D( "[CAN][Ch:%d] Set MONITORING Mode \r\n", psControllerInfo->cChannelHandle );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

CANErrorType_t CAN_DrvSetInternalTestMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = TRUE;
            psControllerInfo->cRegister->crTest.rFReg.rfLBCK = TRUE;
            psControllerInfo->cRegister->crCCControl.rFReg.rfMON = TRUE;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_INTERNAL_TEST;

            CAN_D( "[CAN][Ch:%d] Set INTERNAL_TEST Mode \r\n", psControllerInfo->cChannelHandle );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

CANErrorType_t CAN_DrvSetExternalTestMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = TRUE;
            psControllerInfo->cRegister->crTest.rFReg.rfLBCK = TRUE;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_EXTERNAL_TEST;

            CAN_D( "[CAN][Ch:%d] Set External_TEST Mode \r\n", psControllerInfo->cChannelHandle );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

CANErrorType_t CAN_DrvSetWakeUpMode
(
    CANController_t *                   psControllerInfo
)
{
    sint32          iTimeOutValue;
    CANErrorType_t  ret;

    iTimeOutValue   = 400;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( psControllerInfo->cMode == CAN_MODE_NO_INITIALIZATION )
            {
                ret = CAN_Init();
            }
            else if( psControllerInfo->cMode == CAN_MODE_SLEEP )
            {
                ( void ) CAN_PortingSetControllerClock( psControllerInfo, CAN_CONTROLLER_CLOCK );
                ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

                psControllerInfo->cRegister->crCCControl.rFReg.rfCSR = FALSE;

                ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

                while( psControllerInfo->cRegister->crCCControl.rFReg.rfCSA == TRUE )
                {
                    CAN_PortingDelay( 1 );

                    if( iTimeOutValue == 0 )
                    {
                        ret = CAN_ERROR_TIMEOUT;

                        break;
                    }

                    iTimeOutValue--;
                }
            }
            else
            {
                ; /* Nothing to do */
            }

            psControllerInfo->cMode = CAN_MODE_WAKE_UP;
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

CANErrorType_t CAN_DrvSetSleepMode
(
    CANController_t *                   psControllerInfo
)
{
    sint32          iTimeOutValue;
    CANErrorType_t  ret;

    iTimeOutValue   = 200;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( psControllerInfo->cMode == CAN_MODE_OPERATION )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfCSR = TRUE;

                while( psControllerInfo->cRegister->crCCControl.rFReg.rfCSA == FALSE )
                {
                    CAN_PortingDelay( 1 );

                    if( iTimeOutValue == 0 )
                    {
                        break;
                    }

                    iTimeOutValue--;
                }
            }
            else
            {
                ; /* Nothing to do */
            }

            psControllerInfo->cMode = CAN_MODE_SLEEP;
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetTimeStamp
(
    CANController_t *                   psControllerInfo
)
{
    CANRegExtTSCtrl0_t *    psExtTSCtrl0;
    CANRegExtTSCtrl1_t *    psEXTTSCtrl1;
    CANErrorType_t          ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( CAN_PortingGetTimeStampEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            if( psControllerInfo->cRegister != NULL_PTR )
            {
                psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTSS = 0x2;
                psExtTSCtrl0 = ( CANRegExtTSCtrl0_t * ) CAN_PortingGetConfigEXTS0Addr( psControllerInfo->cChannelHandle );
                psExtTSCtrl0->rFReg.rfENABLE = TRUE;

                psEXTTSCtrl1 = ( CANRegExtTSCtrl1_t * ) CAN_PortingGetConfigEXTS1Addr( psControllerInfo->cChannelHandle );
                psEXTTSCtrl1->rFReg.rfEXT_TIMESTAMP_DIV_RATIO = CAN_TIMESTAMP_RATIO;
                psEXTTSCtrl1->rFReg.rfEXT_TIMESTAMP_COMP = CAN_TIMESTAMP_COMP;

                CAN_D( "[CAN]Set External TimeStamp \r\n", psControllerInfo->cChannelHandle );

            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetTimeoutValue
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( CAN_PortingGetTimeOutEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            if( psControllerInfo->cRegister != NULL_PTR )
            {
                if( psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTCP == 0UL )
                {
                    psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTCP = CAN_TIMESTAMP_PRESCALER;
                }

                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfTOP = CAN_TIMEOUT_VALUE;
                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfTOS = CAN_TIMEOUT_TYPE;
                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfETOC = TRUE;
            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetWatchDog
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( CAN_PortingGetWatchDogEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            if( psControllerInfo->cRegister != NULL_PTR )
            {
                psControllerInfo->cRegister->crRAMWatchdog.rFReg.rfWDC = CAN_WATCHDOG_VALUE;
            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetControlConfig
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( CAN_PortingGetFDEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfFDOE = TRUE;
            }
            if( CAN_PortingGetBitRateSwitchEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfBRSE = TRUE;
            }
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvCopyFilterList
(
    CANController_t *                   psControllerInfo,
    CANIdType_t                         uiFilterType
)
{
    uint8                           ucCh;
    uint8                           ucNumOfFilter;
    uint32                          uiTempAddr;
    CANIdFilterList_t *             psFilterList;
    CANRamStdIDFilterElement_t *    psStdFilterElement;
    CANRamExtIDFilterElement_t *    psExtFilterElement;
    CANErrorType_t                  ret;

    ucCh            = 0;
    uiTempAddr      = 0;
    psFilterList    = NULL_PTR;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        ucCh = psControllerInfo->cChannelHandle;

        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( uiFilterType == CAN_ID_TYPE_STANDARD )
            {
                if( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr != 0UL )
                {
                    psStdFilterElement = ( CANRamStdIDFilterElement_t * ) psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr;
                    psFilterList = ( CANIdFilterList_t * ) StandardIDFilterPar.idfList[ ucCh ];

                    if( psFilterList != NULL_PTR )
                    {
                        for( ucNumOfFilter = 0U ; ucNumOfFilter < CAN_STANDARD_ID_FILTER_NUMBER ; ucNumOfFilter++ )
                        {
                            psStdFilterElement->rFReg.rfSFT      = ( SALReg32 ) ( ( uint32 ) ( psFilterList->idflFilterType ) & 0x3UL );
                            psStdFilterElement->rFReg.rfSFEC     = ( SALReg32 ) ( ( uint32 ) ( psFilterList->idflFilterElementConfiguration ) & 0x7UL );
                            psStdFilterElement->rFReg.rfSFID1    = ( SALReg32 ) ( psFilterList->idflFilterID1 & 0x7FFUL );
                            psStdFilterElement->rFReg.rfSFID2    = ( SALReg32 ) ( psFilterList->idflFilterID2 & 0x7FFUL );

                            uiTempAddr          = ( uint32 ) psStdFilterElement;
                            uiTempAddr          += sizeof( CANRamStdIDFilterElement_t );
                            psStdFilterElement  = ( CANRamStdIDFilterElement_t * ) uiTempAddr;

                            uiTempAddr      = ( uint32 ) psFilterList;
                            uiTempAddr      += sizeof( CANIdFilterList_t );
                            psFilterList    = ( CANIdFilterList_t * ) uiTempAddr;
                        }

                        psControllerInfo->cNumOfStdIDFilterList = ucNumOfFilter;
                        psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = ucNumOfFilter;
                    }
                }
                else
                {
                    ret = CAN_ERROR_NOT_INIT;
                }
            }
            else if( uiFilterType == CAN_ID_TYPE_EXTENDED )
            {
                if( psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr != 0UL )
                {
                    psExtFilterElement = ( CANRamExtIDFilterElement_t * ) psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr;
                    psFilterList = ( CANIdFilterList_t * ) ExtendedIDFilterPar.idfList[ ucCh ];

                    if( psFilterList != NULL_PTR )
                    {
                        for( ucNumOfFilter = 0U ; ucNumOfFilter < CAN_EXTENDED_ID_FILTER_NUMBER ; ucNumOfFilter++ )
                        {
                            psExtFilterElement->rF0.rFReg.rfEFEC    = ( SALReg32 ) ( ( uint32 ) ( psFilterList->idflFilterElementConfiguration ) & 0x7UL );
                            psExtFilterElement->rF1.rFReg.rfEFT     = ( SALReg32 ) ( ( uint32 ) ( psFilterList->idflFilterType ) & 0x3UL );
                            psExtFilterElement->rF0.rFReg.rfEFID1   = ( SALReg32 ) ( psFilterList->idflFilterID1 & 0x1FFFFFFFUL );
                            psExtFilterElement->rF1.rFReg.rfEFID2   = ( SALReg32 ) ( psFilterList->idflFilterID2 & 0x1FFFFFFFUL );

                            uiTempAddr          = ( uint32 ) psExtFilterElement;
                            uiTempAddr          += sizeof( CANRamExtIDFilterElement_t );
                            psExtFilterElement  = ( CANRamExtIDFilterElement_t * ) uiTempAddr;

                            uiTempAddr          = ( uint32 ) psFilterList;
                            uiTempAddr          += sizeof( CANIdFilterList_t );
                            psFilterList        = ( CANIdFilterList_t * ) uiTempAddr;
                        }

                        psControllerInfo->cNumOfExtIDFilterList = ucNumOfFilter;
                        psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = ucNumOfFilter;
                    }
                }
                else
                {
                    ret = CAN_ERROR_NOT_INIT;
                }
            }
            else
            {
                ret = CAN_ERROR_BAD_PARAM;
            }
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetFilterConfig
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                ret = CAN_DrvCopyFilterList( psControllerInfo, CAN_ID_TYPE_STANDARD );
            }

            if( ret == CAN_ERROR_NONE )
            {
                if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
                {
                    ret = CAN_DrvCopyFilterList( psControllerInfo, CAN_ID_TYPE_EXTENDED );
                }
            }

            psControllerInfo->cRegister->crGlobalFilterConfiguration.rFReg.rfRRFS = CAN_PortingGetStandardIDRemoteRejectEnable( psControllerInfo->cChannelHandle );
            psControllerInfo->cRegister->crGlobalFilterConfiguration.rFReg.rfRRFE = CAN_PortingGetExtendedIDRemoteRejectEnable( psControllerInfo->cChannelHandle );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Timing */
static CANErrorType_t CAN_DrvSetBitrate
(
    CANController_t *                   psControllerInfo,
    CANBitRateTiming_t                  ucTimingType
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( ucTimingType == CAN_BIT_RATE_TIMING_ARBITRATION )
        {
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNSJW    = ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpSJW ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNBRP    = ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpBRP ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNTSEG1  = ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPROP ) + ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPhaseSeg1 ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNTSEG2  = ( SALReg32 ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPhaseSeg2 ) - 1UL;
        }
        else if( ucTimingType == CAN_BIT_RATE_TIMING_DATA )
        {
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDSJW   = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpSJW ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDBRP   = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpBRP ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDTSEG1 = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpPROP ) + ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpPhaseSeg1 ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDTSEG2 = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpPhaseSeg2 ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfTDC    = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDC );
            psControllerInfo->cRegister->crTransmitterDelayCompensation.rFReg.rfTDCO = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDCOffset );
            psControllerInfo->cRegister->crTransmitterDelayCompensation.rFReg.rfTDCF = ( SALReg32 ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDCFilterWindow );

            psControllerInfo->cRegister->crCCControl.rFReg.rfBRSE = TRUE;
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvInitTiming
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cArbiPhaseTimeInfo != NULL_PTR )
        {
            /* Arbitration Phase Bit Timing */
            ( void ) CAN_DrvSetBitrate( psControllerInfo, CAN_BIT_RATE_TIMING_ARBITRATION );

            /* Data Phase Bit Timing */
            if( CAN_PortingGetBitRateSwitchEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                if( psControllerInfo->cDataPhaseTimeInfo != NULL_PTR )
                {
                    ( void ) CAN_DrvSetBitrate( psControllerInfo, CAN_BIT_RATE_TIMING_DATA );
                }
            }
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvStartConfigSetting
(
    CANController_t *                   psControllerInfo
)
{
    uint32          uiCnt;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crCCControl.rFReg.rfINIT = 1;

            for( uiCnt = 0UL ; uiCnt < 0x100UL ; uiCnt++ ) {
                ;
            }

            psControllerInfo->cRegister->crCCControl.rFReg.rfCCE = 1;
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvFinishConfigSetting
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crCCControl.rFReg.rfCCE = 0;
            psControllerInfo->cRegister->crCCControl.rFReg.rfINIT = 0;
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static uint32 CAN_DrvGetSizeOfRamMemory
(
    const CANController_t *             psControllerInfo
)
{
    uint32 uiStdIDFilterListSize;
    uint32 uiExtIDFilterListSize;
    uint32 uiRxFIFO0Size;
    uint32 uiRxFIFO1Size;
    uint32 uiRxBufferSize;
    uint32 uiTxEvtFIFOSize;
    uint32 uiTxBufferSize;
    uint32 ret;

    ret = 0;

    if( psControllerInfo != NULL_PTR )
    {
        /* StandardIDFilterListSize */
        uiStdIDFilterListSize = CAN_PortingGetSizeofStandardIDFilterList( psControllerInfo->cChannelHandle );

        /* ExtendedIDFilterListSize */
        uiExtIDFilterListSize = CAN_PortingGetSizeofExtendedIDFilterList( psControllerInfo->cChannelHandle );

        /* RxFIFO0Size */
        uiRxFIFO0Size = CAN_PortingGetSizeofRxFIFO0( psControllerInfo->cChannelHandle );

        /* RxFIFO1Size */
        uiRxFIFO1Size = CAN_PortingGetSizeofRxFIFO1( psControllerInfo->cChannelHandle );

        /* RxBufferSize */
        uiRxBufferSize = CAN_PortingGetSizeofRxBuffer( psControllerInfo->cChannelHandle );

        /* TxEventFIFOSize */
        uiTxEvtFIFOSize = CAN_PortingGetSizeofTxEventBuffer( psControllerInfo->cChannelHandle );

        /* TxBufferSize */
        uiTxBufferSize = CAN_PortingGetSizeofTxBuffer( psControllerInfo->cChannelHandle );

        ret = uiStdIDFilterListSize + uiExtIDFilterListSize + uiRxFIFO0Size + uiRxFIFO1Size + uiRxBufferSize + uiTxEvtFIFOSize + uiTxBufferSize;
    }

    return ret;
}

static CANErrorType_t CAN_DrvRegisterParameterAll
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        /* ArbitrationPhaseTiming */
        psControllerInfo->cArbiPhaseTimeInfo = ( CANTimingParam_t * ) &ArbitrationPhaseTimingPar[ psControllerInfo->cChannelHandle ];

        /* DATAPhaseTiming */
        psControllerInfo->cDataPhaseTimeInfo = ( CANTimingParam_t * ) &DataPhaseTimingPar[ psControllerInfo->cChannelHandle ];

        /* TxBufferInfo */
        psControllerInfo->cTxBufferInfo = ( CANTxBuffer_t * ) &TxBufferInfoPar[ psControllerInfo->cChannelHandle ];

        /* RxDedicatedBufferInfo */
        psControllerInfo->cDedicatedBufferInfo = ( CANRxBuffer_t * ) &RxBufferInfoPar[ psControllerInfo->cChannelHandle ];

        /* RxBufferFIFO0Info */
        psControllerInfo->cFIFO0BufferInfo = ( CANRxBuffer_t * ) &RxBufferFIFO0InfoPar[ psControllerInfo->cChannelHandle ];

        /* RxBufferFIFO1Info */
        psControllerInfo->cFIFO1BufferInfo = ( CANRxBuffer_t * ) &RxBufferFIFO1InfoPar[ psControllerInfo->cChannelHandle ];

        psControllerInfo->cCallbackFunctions = ( CANCallBackFunc_t * ) &CANCallbackFunctions;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Set Buffer */
static CANErrorType_t CAN_DrvInitBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32                        uiMemory,
    uint32                              uiMemorySize
)
{
    uint32          uiStartAddr;
    CANErrorType_t  ret;

    uiStartAddr     = 0UL;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( ( uiMemory != 0UL ) && ( 0UL < uiMemorySize ) )
        {
            uiStartAddr = uiMemory;

            if( psControllerInfo->cRegister != NULL_PTR )
            {
                /* StandardIDFilterListSize */
                if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
                {
                    psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr = uiStartAddr;

                    psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = 0;
                    psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfFLSSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;
                    // psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfFLSSA = ( uiStartAddr & 0x0000UL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofStandardIDFilterList( psControllerInfo->cChannelHandle );
                }

                /* ExtendedIDFilterListSize */
                if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
                {
                    psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr = uiStartAddr;

                    psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = 0;
                    psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfFLESA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofExtendedIDFilterList( psControllerInfo->cChannelHandle );
                }

                /* RxBufferFIFO0Info */
                if( psControllerInfo->cFIFO0BufferInfo != NULL_PTR )
                {
                    psControllerInfo->cRamAddressInfo.raRxFIFO0Addr = uiStartAddr;

                    psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF0DS = ( SALReg32 ) psControllerInfo->cFIFO0BufferInfo->rbDataFieldSize;
                    psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxOM = ( SALReg32 ) psControllerInfo->cFIFO0BufferInfo->rbOperationMode;
                    psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxWM = 0xA; //Watermark
                    psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxS = psControllerInfo->cFIFO0BufferInfo->rbNumberOfElement;
                    psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofRxFIFO0( psControllerInfo->cChannelHandle );
                }

                /* RxBufferFIFO1Info */
                if( psControllerInfo->cFIFO1BufferInfo != NULL_PTR )
                {
                    psControllerInfo->cRamAddressInfo.raRxFIFO1Addr = uiStartAddr;

                    psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF1DS = ( SALReg32 ) psControllerInfo->cFIFO1BufferInfo->rbDataFieldSize;
                    psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxOM = ( SALReg32 ) psControllerInfo->cFIFO1BufferInfo->rbOperationMode;
                    psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxWM = 0xA; //Watermark
                    psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxS = psControllerInfo->cFIFO1BufferInfo->rbNumberOfElement;
                    psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofRxFIFO1( psControllerInfo->cChannelHandle );
                }

                /* RxDedicatedBufferInfo */
                if( psControllerInfo->cDedicatedBufferInfo != NULL_PTR )
                {
                    psControllerInfo->cRamAddressInfo.raRxBufferAddr = uiStartAddr;

                    psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfRBDS = ( SALReg32 ) psControllerInfo->cDedicatedBufferInfo->rbDataFieldSize;
                    psControllerInfo->cRegister->crRxBufferConfiguration.rFReg.rfRBSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofRxBuffer( psControllerInfo->cChannelHandle );
                }

                /* TxBufferInfo */
                if( psControllerInfo->cTxBufferInfo != NULL_PTR )
                {
                    psControllerInfo->cRamAddressInfo.raTxBufferAddr = uiStartAddr;

                    psControllerInfo->cRegister->crTxBufferElementSizeConfiguration.rFReg.rfTBDS = psControllerInfo->cTxBufferInfo->tbInterruptConfig;
                    psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQM = ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbTxFIFOorQueueMode;
                    psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQS = psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
                    psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfNDTB = psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer;
                    psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTBSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofTxBuffer( psControllerInfo->cChannelHandle );

                    if( CAN_PortingGetTxEventFIFOEnable( psControllerInfo->cChannelHandle ) == TRUE )
                    {
                        psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr = uiStartAddr;

                        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFWM = ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer + ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
                        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFS = ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer + ( SALReg32 ) psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
                        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                        ( void ) CAN_PortingGetSizeofTxEventBuffer( psControllerInfo->cChannelHandle );
                    }
                }
            }
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvDeinitBuffer
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL_PTR ) && ( psControllerInfo->cRegister != NULL_PTR ) )
    {
        /* StandardIDFilterListSize */
        if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr = 0;

            psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = 0;
            psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfFLSSA = 0;
        }

        /* ExtendedIDFilterListSize */
        if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == TRUE )
        {
            psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr = 0;

            psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = 0;
            psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfFLESA = 0;
        }

        /* RxBufferFIFO0Info */
        if( psControllerInfo->cFIFO0BufferInfo != NULL_PTR )
        {
            psControllerInfo->cRamAddressInfo.raRxFIFO0Addr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF0DS = 0;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxOM = ( SALReg32 ) psControllerInfo->cFIFO0BufferInfo->rbOperationMode;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxWM = 0xA; //Watermark
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxS = 0;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxSA = 0;
        }
        /* RxBufferFIFO1Info */
        if( psControllerInfo->cFIFO1BufferInfo != NULL_PTR )
        {
            psControllerInfo->cRamAddressInfo.raRxFIFO1Addr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF1DS = 0;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxOM = ( SALReg32 ) psControllerInfo->cFIFO1BufferInfo->rbOperationMode;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxWM = 0xA; //Watermark
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxS = 0;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxSA = 0;
        }

        /* RxDedicatedBufferInfo */
        if( psControllerInfo->cDedicatedBufferInfo != NULL_PTR )
        {
            psControllerInfo->cRamAddressInfo.raRxBufferAddr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfRBDS = 0;
            psControllerInfo->cRegister->crRxBufferConfiguration.rFReg.rfRBSA = 0;
        }

        /* TxBufferInfo */
        if( psControllerInfo->cTxBufferInfo != NULL_PTR )
        {
            psControllerInfo->cRamAddressInfo.raTxBufferAddr = 0;

            psControllerInfo->cRegister->crTxBufferElementSizeConfiguration.rFReg.rfTBDS = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQM = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQS = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfNDTB = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTBSA = 0;

            if( CAN_PortingGetTxEventFIFOEnable( psControllerInfo->cChannelHandle ) == TRUE )
            {
                psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr = 0;

                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFWM = 0;
                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFS = 0;
                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFSA = 0;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}


/* ERROR */
uint32 CAN_DrvGetProtocolStatus
(
    const CANController_t *             psControllerInfo
)
{
    uint32 ret;

    ret = 0;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ret = ( uint32 ) psControllerInfo->cRegister->crProtocolStatusRegister.rNReg;
        }
    }

    return ret;
}

/*  IRQ  */
static void CAN_DrvIRQHandler
(
    void *                              pArg
)
{
    uint8 ucCh;
    CANController_t * psControllerInfo;

    psControllerInfo = NULL_PTR;

    if( pArg != NULL_PTR )
    {
        ucCh = * ( uint8 * ) pArg;

        if( ucCh < CAN_CONTROLLER_NUMBER )
        {
            psControllerInfo = CAN_GetControllerInfo( ucCh );

            if( ( psControllerInfo != NULL_PTR ) && ( psControllerInfo->cMode != CAN_MODE_NO_INITIALIZATION ) && ( psControllerInfo->cMode != CAN_MODE_SLEEP ) )
            {
                CAN_DrvProcessIRQ( ucCh );
            }
        }
    }
}

static CANErrorType_t CAN_DrvSetInterruptConfig
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
#if defined(EDUCATION_CAN_DEMO_EN)
        // CANRegFieldIR_t - 27(rfPEA) bit off
        psControllerInfo->cRegister->crInterruptEnable.rNReg = (CAN_INTERRUPT_ENABLE & 0xF7FFFFFFUL);
#else
        psControllerInfo->cRegister->crInterruptEnable.rNReg = CAN_INTERRUPT_ENABLE;
#endif
        psControllerInfo->cRegister->crInterruptLineEnable.rNReg = CAN_INTERRUPT_LINE_ENABLE;
        psControllerInfo->cRegister->crInterruptLineSelect.rNReg = CAN_INTERRUPT_LINE_SEL;

        psControllerInfo->cRegister->crTxBufferTransmissionInterruptEnable = 0xFFFFFFFFUL;
        psControllerInfo->cRegister->crTxBufferCancellationFinishedInterruptEnable = 0xFFFFFFFFUL;

        ( void ) CAN_PortingSetInterruptHandler( psControllerInfo, &CAN_DrvIRQHandler );
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static void CAN_DrvProcessIRQ
(
    uint8                               ucCh
)
{
    CANController_t *   psControllerInfo;
    CANRegFieldIR_t     sIRStatus;
    CANRegFieldIR_t     sIRClearStatus;

    psControllerInfo = NULL_PTR;

    psControllerInfo = CAN_GetControllerInfo( ucCh );
    if( psControllerInfo != NULL_PTR )
    {
        ( void ) SAL_MemSet ( &sIRClearStatus, 0, sizeof( CANRegFieldIR_t ) );

        sIRStatus = psControllerInfo->cRegister->crInterruptRegister.rFReg;

        // 29 : Access to Reserved Address
        if( sIRStatus.rfARA != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_ACCESS_RESERVED_ADDRESS );

            CAN_D( "[CAN Channel : %d] Access to Reserved Address \r\n", ucCh );

            sIRClearStatus.rfARA = 1;
        }

        // 28 : Protocol Error in Data Phase
        if( sIRStatus.rfPED != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PROTOCOL );

            CAN_D( "[CAN Channel : %d] Protocol Error in Data Phase \r\n", ucCh );

            sIRClearStatus.rfPED = 1;
        }

        // 27 : Protocol Error in Arbitration Phase
        if( sIRStatus.rfPEA != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PROTOCOL );

            CAN_D( "[CAN Channel : %d] Protocol Error in Arbitration Phase \r\n", ucCh );

            sIRClearStatus.rfPEA = 1;
        }

        // 26 : Watchdog Interrupt
        if( sIRStatus.rfWDI != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Watchdog Interrupt \r\n", ucCh );

            sIRClearStatus.rfWDI = 1;
        }

        // 25 : Bus_Off Status
        if( sIRStatus.rfBO != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Bus_Off Status \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BUS_OFF );

            sIRClearStatus.rfBO = 1;
        }

        // 24 : Warning Status
        if( sIRStatus.rfEW != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Warning Status \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_WARNING );

            sIRClearStatus.rfEW = 1;
        }

        // 23 : Error Passive
        if( sIRStatus.rfEP != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Error Passive \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PASSIVE );

            sIRClearStatus.rfEP = 1;
        }

        // 22 : Error Logging Overflow
        if( sIRStatus.rfELO != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Error Logging Overflow \r\n", ucCh );

            sIRClearStatus.rfELO = 1;
        }

        // 21 : Bit Error Uncorrected
        if( sIRStatus.rfBEU != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Bit Error Uncorrected \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BIT );

            sIRClearStatus.rfBEU = 1;
        }

        // 20 : Bit Error Corrected
        if( sIRStatus.rfBEC != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Bit Error Corrected \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BIT );

            sIRClearStatus.rfBEC = 1;
        }

        // 19 : Message stored to Dedicated Rx Buffer
        if( sIRStatus.rfDRX != 0UL )
        {
#if defined(EDUCATION_CAN_DEMO_EN)
            CANErrorType_t ret;
            ret = CAN_ERROR_NONE;

            // Filtering can id
            ret = CAN_Edu_FilteringCanID( ucCh, CAN_RX_BUFFER_TYPE_DBUFFER );
            if (ret == CAN_ERROR_NONE)
            {
                ret = CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_DBUFFER );
                sIRClearStatus.rfDRX = 1;
            }
#else
            ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_DBUFFER );

            sIRClearStatus.rfDRX = 1;
#endif
        }

        // 18 : Timeout Occurred
        if( sIRStatus.rfTOO != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Timeout Occurred \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TIMEOUT );

            sIRClearStatus.rfTOO = 1;
        }

        // 17 : Message RAM Access Failure
        if( sIRStatus.rfMRAF != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Message RAM Access Failure \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_RAM_ACCESS_FAIL );

            sIRClearStatus.rfMRAF = 1;
        }

        // 16 : Timestamp Wraparound
        if( sIRStatus.rfTSW != 0UL )
        {
            sIRClearStatus.rfTSW = 1;
        }

        // 15 : Tx Event FIFO Element Lost
        if( sIRStatus.rfTEFL != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TX_EVENT_LOST );

            sIRClearStatus.rfTEFL = 1;
        }

        // 14 : Tx Event FIFO Full
        if( sIRStatus.rfTEFF != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TX_EVENT_FULL );

            sIRClearStatus.rfTEFF = 1;
        }

        // 13 : Tx Event FIFO Watermark Reached
        if( sIRStatus.rfTEFW != 0UL )
        {
            sIRClearStatus.rfTEFW = 1;
        }

        // 12 : Tx Event FIFO New Entry
        if( sIRStatus.rfTEFN != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_NEW_EVENT );

            sIRClearStatus.rfTEFN = 1;
        }

        // 11 : Tx FIFO Empty
        if( sIRStatus.rfTFE != 0UL )
        {
            sIRClearStatus.rfTFE = 1;
        }

        // 10 : Transmission Cancellation Finished
        if( sIRStatus.rfTCF != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_TRANSMIT_CANCEL_FINISHED );

            sIRClearStatus.rfTCF = 1;
        }

        // 9 : Transmission Completed
        if( sIRStatus.rfTC != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_TRANSMIT_COMPLETED );

            sIRClearStatus.rfTC = 1;
        }

        // 8 : High Priority Message
        if( sIRStatus.rfHPM != 0UL )
        {
            CAN_D( "[CAN Channel : %d] High Priority Message \r\n", ucCh );

            sIRClearStatus.rfHPM = 1;
        }

        // 7 : Rx FIFO 1 Message Lost
        if( sIRStatus.rfRF1L != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_LOST );

            sIRClearStatus.rfRF1L = 1;
        }

        // 6 : Rx FIFO 1 Full
        if( sIRStatus.rfRF1F != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_FULL );

            sIRClearStatus.rfRF1F = 1;
        }

        // 5 : Rx FIFO 1 Watermark Reached
        if( sIRStatus.rfRF1W != 0UL )
        {
            sIRClearStatus.rfRF1W = 1;
        }

        // 4 : Rx FIFO 1 New Message
        if( sIRStatus.rfRF1N != 0UL )
        {
#if defined(EDUCATION_CAN_DEMO_EN)
            CANErrorType_t ret;
            ret = CAN_ERROR_NONE;

            // Filtering can id
            ret = CAN_Edu_FilteringCanID( ucCh, CAN_RX_BUFFER_TYPE_FIFO1 );
            if (ret == CAN_ERROR_NONE)
            {
                ret = CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO1 );
                sIRClearStatus.rfRF1N = 1;
            }
#else
            ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO1 );

            sIRClearStatus.rfRF1N = 1;
#endif
        }

        // 3 : Rx FIFO 0 Message Lost
        if( sIRStatus.rfRF0L != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_LOST );

            sIRClearStatus.rfRF0L = 1;
        }

        // 2 : Rx FIFO 0 Full
        if( sIRStatus.rfRF0F != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_FULL );

            sIRClearStatus.rfRF0F = 1;
        }

        // 1 : Rx FIFO 0 Watermark Reached
        if( sIRStatus.rfRF0W != 0UL )
        {
            sIRClearStatus.rfRF0W = 1;
        }

        // 0 : Rx FIFO 0 New Message
        if( sIRStatus.rfRF0N != 0UL )
        {
#if defined(EDUCATION_CAN_DEMO_EN)
            CANErrorType_t ret;
            ret = CAN_ERROR_NONE;

            // Filtering can id
            ret = CAN_Edu_FilteringCanID( ucCh, CAN_RX_BUFFER_TYPE_FIFO0 );
            if (ret == CAN_ERROR_NONE)
            {
                ret = CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO0 );
                sIRClearStatus.rfRF0N = 1;
            }
#else
            ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO0 );

            sIRClearStatus.rfRF0N = 1;
#endif
        }

        psControllerInfo->cRegister->crInterruptRegister.rFReg = sIRClearStatus;
    }
}

static void CAN_DrvCallbackNotifyTxEvent
(
    uint8                               ucCh,
    CANTxInterruptType_t                uiIntType
)
{
    CANController_t * psControllerInfo;

    psControllerInfo = NULL_PTR;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = CAN_GetControllerInfo( ucCh );

        if( ( psControllerInfo != NULL_PTR ) && ( psControllerInfo->cCallbackFunctions != NULL_PTR ) )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyTxEvent != NULL_PTR )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyTxEvent( ucCh, uiIntType );
            }
        }
    }
}

void CAN_DrvCallbackNotifyRxEvent
(
    uint8                               ucCh,
    uint32                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
)
{
    CANController_t * psControllerInfo;

    psControllerInfo = NULL_PTR;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = CAN_GetControllerInfo( ucCh );

        if( ( psControllerInfo != NULL_PTR ) &&  (psControllerInfo->cCallbackFunctions != NULL_PTR ) )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyRxEvent != NULL_PTR )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyRxEvent( ucCh, uiRxIndex, uiRxBufferType, uiError );
            }
        }
    }
}

static void CAN_DrvCallbackNotifyErrorEvent
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
)
{
    CANController_t * psControllerInfo;

    psControllerInfo = NULL_PTR;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = CAN_GetControllerInfo( ucCh );

        if( ( psControllerInfo != NULL_PTR ) && ( psControllerInfo->cCallbackFunctions != NULL_PTR ) )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyErrorEvent != NULL_PTR )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyErrorEvent( ucCh, uiError );
            }
        }
    }
}

/* Not used function */
/*
CANErrorType_t CAN_DrvSetDisableAutomaticRetransmission
(
    CANController_t *                   psControllerInfo,
    boolean bDisable
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfDAR = bDisable;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}
*/

