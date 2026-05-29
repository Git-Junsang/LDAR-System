/*
***************************************************************************************************
*
*   FileName : can_msg.c
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

#include "can_config.h"
#include "can_reg.h"
#include "can.h"
#include "can_par.h"
#include "can_drv.h"
#include "can_msg.h"

#include <sal_internal.h>

#if defined(EDUCATION_CAN_DEMO_EN)
#include <can_demo.h>
extern CANDemoMeta_t gNowMetaInfo;
#endif

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static CANRxRingBuffer_t RxRingBufferManager[ CAN_CONTROLLER_NUMBER ]; /* Rx Message */


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static uint8 CAN_MsgGetDataSizeBySizeType
(
    CANDataLength_t                     uiDataLengthCode
);

static CANDataLength_t CAN_MsgGetSizeTypebyDataSize
(
    uint8                               ucDataSize
);

static CANErrorType_t CAN_MsgCopyRxMessage
(
    const CANRamRxBuffer_t *            psSrcReceivedMsg,
    CANMessage_t *                      psRxBuffer
);

static CANMessage_t *CAN_MsgGetRxBufferFromRingBuffer
(
    uint8                               ucCh,
    CANRingBufferType_t                 ucType
);

static CANErrorType_t CAN_MsgCopyTxMessage
(
    CANRamTxBuffer_t *                  psTxMsg,
    const CANMessage_t *                psSrcMsg
);

static CANErrorType_t CAN_MsgRequestTxMessage
(
    CANController_t *                   psControllerInfo,
    uint8                               ucReqIndex
);

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

CANErrorType_t CAN_MsgInit
(
    const CANController_t *             psControllerInfo
)
{
    uint8           ucCh;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        ucCh = psControllerInfo->cChannelHandle;

        RxRingBufferManager[ ucCh ].rrbTailIdx = 0;
        RxRingBufferManager[ ucCh ].rrbHeadIdx = 0;
        RxRingBufferManager[ ucCh ].rrbMsgBufferMax = CAN_RX_MSG_RING_BUFFER_MAX;

        ( void ) SAL_MemSet( &RxRingBufferManager[ ucCh ].rrbMsg[ 0 ], 0, ( sizeof( CANMessage_t ) * RxRingBufferManager[ ucCh ].rrbMsgBufferMax ) );

        RxRingBufferManager[ ucCh ].rrbInitFlag = TRUE;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static uint8 CAN_MsgGetDataSizeBySizeType
(
    CANDataLength_t                     uiDataLengthCode
)
{
    uint8 ucDataSize;

    ucDataSize = 0;

    switch( uiDataLengthCode )
    {
        case CAN_DATA_LENGTH_12:    // 9,
        {
            ucDataSize = 12;

            break;
        }

        case CAN_DATA_LENGTH_16:    // 10,
        {
            ucDataSize = 16;

            break;
        }

        case CAN_DATA_LENGTH_20:    // 11,
        {
            ucDataSize = 20;

            break;
        }

        case CAN_DATA_LENGTH_24:    // 12,
        {
            ucDataSize = 24;

            break;
        }

        case CAN_DATA_LENGTH_32:    // 13,
        {
            ucDataSize = 32;

            break;
        }

        case CAN_DATA_LENGTH_48:    // 14,
        {
            ucDataSize = 48;

            break;
        }

        case CAN_DATA_LENGTH_64:    // 15,
        {
            ucDataSize = 64;

            break;
        }

        default:
        {
            if( uiDataLengthCode <= CAN_DATA_LENGTH_8 )
            {
                ucDataSize = ( uint8 ) uiDataLengthCode;
            }
            else
            {
                ucDataSize = 64;
            }

            break;
        }
    }

    return ucDataSize;
}

static CANDataLength_t CAN_MsgGetSizeTypebyDataSize
(
    uint8                               ucDataSize
)
{
    CANDataLength_t uiDataLengthCode;

    uiDataLengthCode = CAN_DATA_LENGTH_0;

    if( ucDataSize <= 8U )
    {
        uiDataLengthCode = ( CANDataLength_t ) ucDataSize;
    }
    else if( ucDataSize <= 12U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_12;
    }
    else if( ucDataSize <= 16U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_16;
    }
    else if( ucDataSize <= 20U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_20;
    }
    else if( ucDataSize <= 24U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_24;
    }
    else if( ucDataSize <= 32U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_32;
    }
    else if( ucDataSize <= 48U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_48;
    }
    else if( ucDataSize <= 64U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_64;
    }
    else
    {
        uiDataLengthCode = CAN_DATA_LENGTH_64;
    }

    return uiDataLengthCode;
}

static CANErrorType_t CAN_MsgCopyRxMessage
(
    const CANRamRxBuffer_t *            psSrcReceivedMsg,
    CANMessage_t *                      psRxBuffer
)
{
    uint8           i;
    uint8           j;
    uint8           ucRemainLength;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( ( psSrcReceivedMsg != NULL_PTR ) && ( psRxBuffer != NULL_PTR ) )
    {
        psRxBuffer->mErrorStateIndicator = ( uint8 ) psSrcReceivedMsg->rR0.rFReg.rfESI;
        psRxBuffer->mRemoteTransmitRequest = ( uint8 ) psSrcReceivedMsg->rR0.rFReg.rfRTR;
        psRxBuffer->mFDFormat = ( uint8 ) psSrcReceivedMsg->rR1.rFReg.rfFDF;
        psRxBuffer->mExtendedId = ( uint8 ) psSrcReceivedMsg->rR0.rFReg.rfXTD;

        if( psRxBuffer->mExtendedId == TRUE )
        {
            psRxBuffer->mId = ( uint32 ) psSrcReceivedMsg->rR0.rFReg.rfID;
        }
        else
        {
            psRxBuffer->mId = ( uint32 ) ( ( ( uint32 ) ( psSrcReceivedMsg->rR0.rFReg.rfID ) >> 18UL ) & 0x7FFUL );
        }

        psRxBuffer->mDataLength = CAN_MsgGetDataSizeBySizeType( ( CANDataLength_t ) psSrcReceivedMsg->rR1.rFReg.rfDLC );

        if( 0U < psRxBuffer->mDataLength )
        {
            ucRemainLength = psRxBuffer->mDataLength % 4U;

            for( i = 0U ; i < ( psRxBuffer->mDataLength / 4U ) ; i++ )
            {
                for( j = 0U ; j < 4U ; j++ )
                {
                    psRxBuffer->mData[ j + ( 4U * i ) ] = ( uint8 ) ( ( ( uint32 ) ( ( uint32 ) psSrcReceivedMsg->rData[ i ] >> ( j * 8U ) ) ) & 0xFFU );
                }
            }

            if( 0U < ucRemainLength )
            {
                for( j = 0U ; j < ucRemainLength ; j++ )
                {
                    psRxBuffer->mData[ j + ( 4U * i ) ] = ( uint8 ) ( ( ( uint32 ) ( ( uint32 )psSrcReceivedMsg->rData[ i ] >> ( j * 8U ) ) ) & 0xFFU );
                }
            }
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

static CANMessage_t * CAN_MsgGetRxBufferFromRingBuffer
(
    uint8                               ucCh,
    CANRingBufferType_t                 ucType
)
{
    uint32          uiTotalNum;
    CANMessage_t *  psBuffer;

    uiTotalNum      = 0;
    psBuffer        = NULL_PTR;

    if( RxRingBufferManager[ ucCh ].rrbInitFlag == TRUE )
    {
        uiTotalNum = CAN_MsgGetCountOfRxMessage( ucCh );

        if( uiTotalNum < CAN_RX_MSG_RING_BUFFER_MAX )
        {
            if( ( uiTotalNum == 0UL ) || ( ucType == CAN_RING_BUFFER_LAST ) )
            {
                psBuffer = &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbTailIdx ];

                RxRingBufferManager[ ucCh ].rrbTailIdx++;

                if( RxRingBufferManager[ ucCh ].rrbMsgBufferMax <= RxRingBufferManager[ ucCh ].rrbTailIdx )
                {
                    RxRingBufferManager[ ucCh ].rrbTailIdx = 0;
                }
            }
            else
            {
                if( 0UL < RxRingBufferManager[ ucCh ].rrbHeadIdx )
                {
                    RxRingBufferManager[ ucCh ].rrbHeadIdx--;
                }
                else
                {
                    RxRingBufferManager[ ucCh ].rrbHeadIdx = RxRingBufferManager[ ucCh ].rrbMsgBufferMax - 1UL;
                }

                psBuffer = &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbHeadIdx ];
            }
        }
    }

    return psBuffer;
}

CANErrorType_t CAN_MsgPutRxMessage
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
)
{
    uint8               uiSavedFlag;
    uint32              i;
    uint32              j;
    uint32              uiIndex;
    uint32              uiRxMsgAddr;
    uint32              uiRxFIFOAddr;
    uint32              uiCheckFlag;
    uint32              uiGetIdx;
    uint32              uiPutIdx;
    uint32              uiNewMsgFlag[ 2 ];
    CANRegFieldRXFxS_t  sRxFIFOStatus_bReg;
    CANRegFieldRXFxA_t *psRxFIFOAcknowledge_bReg;
    CANMessage_t *      psRxBuffer;
    CANRamRxBuffer_t *  psReceivedMsg;
    CANController_t *   psControllerInfo;
    CANRingBufferType_t ucRingBufferType;
    CANErrorType_t      ret;

    uiSavedFlag         = FALSE;
    uiCheckFlag         = 0;
    uiNewMsgFlag[ 0 ]   = 0;
    uiNewMsgFlag[ 1 ]   = 0;
    psRxBuffer          = NULL_PTR;
    ret                 = CAN_ERROR_NONE;

    psControllerInfo = CAN_GetControllerInfo(ucCh);

    if( psControllerInfo != NULL_PTR )
    {
        if( RxRingBufferManager[ ucCh ].rrbInitFlag == TRUE )
        {
            if( uiBufferType == CAN_RX_BUFFER_TYPE_DBUFFER )
            {
                uiNewMsgFlag[ 0 ] = psControllerInfo->cRegister->crNewData1.rNReg;
                uiNewMsgFlag[ 1 ] = psControllerInfo->cRegister->crNewData2.rNReg;

                for( i = 0UL ; i < 2UL ; i++ )
                {
                    for( j = 0UL ; j < 32UL ; j++ )
                    {
                        uiCheckFlag = ( uint32 ) 1UL << j;

                        if( ( uiNewMsgFlag[ i ] & uiCheckFlag ) != 0UL )
                        {
                            uiIndex = j + ( 32UL * i );

                            psRxBuffer = CAN_MsgGetRxBufferFromRingBuffer( ucCh, CAN_RING_BUFFER_LAST );

                            if( psRxBuffer != NULL_PTR )
                            {
                                psRxBuffer->mBufferIndex = ( uint8 ) uiIndex;
                                uiRxMsgAddr = psControllerInfo->cRamAddressInfo.raRxBufferAddr + ( sizeof( CANRamRxBuffer_t ) * ( uiIndex ) );
                                psReceivedMsg = ( CANRamRxBuffer_t * ) uiRxMsgAddr;
                                psRxBuffer->mBufferType = CAN_RX_BUFFER_TYPE_DBUFFER;

                                ret = CAN_MsgCopyRxMessage( psReceivedMsg, psRxBuffer );

                                if( ret == CAN_ERROR_NONE )
                                {
                                    uiSavedFlag = TRUE;
                                }
                            }
                            else
                            {
                                ret = CAN_ERROR_NO_BUFFER;
                            }

                            break;
                        }
                    }

                    if( uiSavedFlag == TRUE )
                    {
                        break;
                    }
                }

                if( uiSavedFlag == TRUE )
                {
                    if( i == 0UL )
                    {
                        psControllerInfo->cRegister->crNewData1.rNReg = uiNewMsgFlag[ 0 ];
                    }
                    else
                    {
                        psControllerInfo->cRegister->crNewData2.rNReg = uiNewMsgFlag[ 1 ];
                    }
                }
            }
            else
            {
                if( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO0 )
                {
                    sRxFIFOStatus_bReg = psControllerInfo->cRegister->crRxFIFO0Status.rFReg;
                    psRxFIFOAcknowledge_bReg = &psControllerInfo->cRegister->crRxFIFO0Acknowledge.rFReg;
                    uiRxFIFOAddr = psControllerInfo->cRamAddressInfo.raRxFIFO0Addr;
                }
                else if( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO1 )
                {
                    sRxFIFOStatus_bReg = psControllerInfo->cRegister->crRxFIFO1Status.rFReg;
                    psRxFIFOAcknowledge_bReg = &psControllerInfo->cRegister->crRxFIFO1Acknowledge.rFReg;
                    uiRxFIFOAddr = psControllerInfo->cRamAddressInfo.raRxFIFO1Addr;
                }
                else
                {
                    /* unknown buffer type */
                    ret = CAN_ERROR_NO_MESSAGE;
                }

                if( ( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO0 ) || ( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO1 ) )
                {
                    uiGetIdx = sRxFIFOStatus_bReg.rfFxGI;
                    uiPutIdx = sRxFIFOStatus_bReg.rfFxPI;

                    if( uiGetIdx != uiPutIdx )
                    {
                        uiIndex = uiGetIdx;

                        if( ( uint32 ) psControllerInfo->cRegister->crHighPriorityMessageStatus.rFReg.rfBIDX == uiIndex )
                        {
                            ucRingBufferType = CAN_RING_BUFFER_FIRST;
                        }
                        else
                        {
                            ucRingBufferType = CAN_RING_BUFFER_LAST;
                        }

                        psRxBuffer = CAN_MsgGetRxBufferFromRingBuffer( ucCh, ucRingBufferType );

                        if( psRxBuffer != NULL_PTR )
                        {
                            uiRxMsgAddr = uiRxFIFOAddr + ( sizeof( CANRamRxBuffer_t ) * uiIndex );
                            psReceivedMsg = ( CANRamRxBuffer_t * ) uiRxMsgAddr;
                            psRxBuffer->mBufferType = uiBufferType;
                            psRxBuffer->mBufferIndex = ( uint8 ) uiIndex;

                            ret = CAN_MsgCopyRxMessage( psReceivedMsg, psRxBuffer );

                            if( ret == CAN_ERROR_NONE )
                            {
                                uiSavedFlag = TRUE;
                                psRxFIFOAcknowledge_bReg->rfFxAI = uiIndex;
                            }
                        }
                        else
                        {
                            ret = CAN_ERROR_NO_BUFFER;
                        }
                    }
                }
            }

            if( uiSavedFlag == TRUE )
            {
                CAN_DrvCallbackNotifyRxEvent( ucCh, ( uint32 )psRxBuffer->mBufferIndex, uiBufferType, CAN_ERROR_NONE );
            }
            else
            {
                ;//CAN_Edu_resetReceiveFlag( ucCh, CAN_ERROR_NONE );
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

uint32 CAN_MsgGetCountOfRxMessage
(
    uint8                               ucCh
)
{
    uint32 ret;

    ret = 0;

    if( RxRingBufferManager[ ucCh ].rrbInitFlag == TRUE )
    {
        if( RxRingBufferManager[ ucCh ].rrbHeadIdx <= RxRingBufferManager[ ucCh ].rrbTailIdx )
        {
            ret = RxRingBufferManager[ ucCh ].rrbTailIdx - RxRingBufferManager[ ucCh ].rrbHeadIdx;
        }
        else
        {
            ret = ( CAN_RX_MSG_RING_BUFFER_MAX - RxRingBufferManager[ ucCh ].rrbHeadIdx ) + RxRingBufferManager[ ucCh ].rrbTailIdx;
        }
    }

    return ret;
}

CANErrorType_t CAN_MsgGetRxMessage
(
    uint8                               ucCh,
    CANMessage_t *                      psRxMsg
)
{
    uint32          uiMsgCount;
    CANErrorType_t  ret;

    uiMsgCount      = 0;
    ret             = CAN_ERROR_NONE;

    if( ( RxRingBufferManager[ ucCh ].rrbInitFlag == TRUE ) && ( psRxMsg != NULL_PTR ) )
    {
        uiMsgCount = CAN_MsgGetCountOfRxMessage( ucCh );

        if( 0UL < uiMsgCount )
        {
            ( void ) SAL_MemCopy( psRxMsg, &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbHeadIdx ], sizeof( CANMessage_t ) );

            RxRingBufferManager[ ucCh ].rrbHeadIdx++;

            if( CAN_RX_MSG_RING_BUFFER_MAX <= RxRingBufferManager[ ucCh ].rrbHeadIdx )
            {
                RxRingBufferManager[ ucCh ].rrbHeadIdx = 0;

                ret = CAN_ERROR_BAD_PARAM;
            }
        }
        else
        {
            ret = CAN_ERROR_NO_MESSAGE;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Tx Message */
static CANErrorType_t CAN_MsgCopyTxMessage
(
    CANRamTxBuffer_t *                  psTxMsg,
    const CANMessage_t *                psSrcMsg
)
{
    uint8           i;
    uint8           j;
    uint8           ucRemainLength;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( ( psTxMsg != NULL_PTR ) && ( psSrcMsg != NULL_PTR ) )
    {
        psTxMsg->rT0.rFReg.rfXTD = psSrcMsg->mExtendedId;

        if( psSrcMsg->mExtendedId == TRUE )
        {
            psTxMsg->rT0.rFReg.rfID = psSrcMsg->mId;
        }
        else
        {
            psTxMsg->rT0.rFReg.rfID = ( SALReg32 ) ( ( psSrcMsg->mId & 0x7FFUL ) << 18UL );
        }

        psTxMsg->rT0.rFReg.rfRTR    = psSrcMsg->mRemoteTransmitRequest;
        psTxMsg->rT0.rFReg.rfESI    = psSrcMsg->mErrorStateIndicator;
        psTxMsg->rT1.rFReg.rfBRS    = psSrcMsg->mBitRateSwitching;
        psTxMsg->rT1.rFReg.rfEFC    = psSrcMsg->mEventFIFOControl;
        psTxMsg->rT1.rFReg.rfFDF    = psSrcMsg->mFDFormat;
        psTxMsg->rT1.rFReg.rfMM     = psSrcMsg->mMessageMarker;
        psTxMsg->rT1.rFReg.rfDLC    = (SALReg32)CAN_MsgGetSizeTypebyDataSize(psSrcMsg->mDataLength);

        if( 0U < psSrcMsg->mDataLength )
        {
            for( i = 0U ; i < ( psSrcMsg->mDataLength / 4U ) ; i++ )
            {
                psTxMsg->rData[ i ] = 0;

                for( j = 0U ; j < 4U ; j++)
                {
                    psTxMsg->rData[ i ] |= ( ( uint32 ) ( psSrcMsg->mData[ j + ( i * 4U ) ] ) << ( 8U * j ) );
                }
            }

            if( 0U < ( psSrcMsg->mDataLength % 4U ) )
            {
                psTxMsg->rData[ i ] = 0UL;

                ucRemainLength = psSrcMsg->mDataLength % 4U;

                for( j = 0U ; j < ucRemainLength ; j++ )
                {
                    psTxMsg->rData[ i ] |= ( ( uint32 ) ( psSrcMsg->mData[ j + ( i * 4U ) ] ) << ( 8U * j ) );
                }
            }
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_MsgSetTxMessage
(
    CANController_t *                   psControllerInfo,
    const CANMessage_t *                psMsg,
    uint8 *                             pucTxBufferIndex
)
{
    uint8                       ucReqIndex;
    uint32                      uiTxMsgAddr;
    CANRamTxBuffer_t *          psTxMsg;
    CANControllerRegister_t *   psControllerReg;
    CANErrorType_t              ret;

    ret = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL_PTR ) && ( psMsg != NULL_PTR ) )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerReg = psControllerInfo->cRegister;

            if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_FIFO )
            {
                ucReqIndex = ( uint8 ) psControllerReg->crTxFIFOOrQueueStatus.rFReg.rfTFQPI;
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ( uint32 ) ucReqIndex );
                psTxMsg = ( CANRamTxBuffer_t * ) uiTxMsgAddr;

                *pucTxBufferIndex = ucReqIndex;
            }
            else if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_QUEUE )
            {
                ucReqIndex = psMsg->mBufferIndex + ( uint8 ) psControllerReg->crTxFIFOOrQueueStatus.rFReg.rfTFQPI;
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ( uint32 ) ucReqIndex );
                psTxMsg = ( CANRamTxBuffer_t * ) uiTxMsgAddr;

                *pucTxBufferIndex = ucReqIndex;
            }
            else if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_DBUFFER )
            {
                ucReqIndex = psMsg->mBufferIndex;
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ( uint32 ) ucReqIndex );
                psTxMsg = ( CANRamTxBuffer_t * ) uiTxMsgAddr;

                *pucTxBufferIndex = ucReqIndex;
            }
            else
            {
                /* unknown buffer type */
                ret = CAN_ERROR_NO_MESSAGE;
            }

            if( ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_FIFO ) || ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_QUEUE ) || ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_DBUFFER ) )
            {
                ( void ) CAN_MsgCopyTxMessage( psTxMsg, psMsg );

                if( psControllerInfo->cTxBufferInfo->tbTxFIFOorQueueMode != CAN_TX_BUFFER_MODE_QUEUE ) {
                    ret = CAN_MsgRequestTxMessage( psControllerInfo, ucReqIndex );
                } else {
                    ret = CAN_ERROR_NONE;
                }
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

static CANErrorType_t CAN_MsgRequestTxMessage
(
    CANController_t *                   psControllerInfo,
    uint8                               ucReqIndex
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crTxBufferAddRequest |= ( uint32 ) ( ( uint32 ) 1UL << ucReqIndex );
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

/* Not used function */
/*
CANErrorType_t CAN_MsgRequestTxAllMessage
(
    CANController_t *                   psControllerInfo,
    uint32                              uiTxBufferAllIndex
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crTxBufferAddRequest |= uiTxBufferAllIndex;
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
*/

CANErrorType_t CAN_MsgRequestTxMessageCancellation
(
    CANController_t *                   psControllerInfo,
    uint8                               ucTxBufferIndex
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            psControllerInfo->cRegister->crTxBufferCancellationRequest |= ( uint32 ) ( ( uint32 ) 1UL << ucTxBufferIndex );
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

uint32 CAN_MsgGetCountOfTxEvent
(
    uint8                               ucCh
)
{
    CANController_t *   psControllerInfo;
    uint32              ret;

    ret = 0;

    psControllerInfo = CAN_GetControllerInfo( ucCh );

    if( psControllerInfo != NULL_PTR )
    {
        if( psControllerInfo->cRegister != NULL_PTR )
        {
            ret = psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFFL;
        }
    }

    return ret;
}

CANErrorType_t CAN_MsgGetTxEventMessage
(
    uint8                               ucCh,
    CANTxEvent_t *                      psTxEvtBuffer
)
{
    uint32                  uiIndex;
    uint32                  uiTxEvtMsgAddr;
    CANController_t *       psControllerInfo;
    CANRamTxEventFIFO_t *   psSavedTxEvt;
    CANErrorType_t          ret;

    uiIndex                 = 0;
    psControllerInfo        = NULL_PTR;
    ret                     = CAN_ERROR_NONE;

    if( psTxEvtBuffer != NULL_PTR )
    {
        psControllerInfo = CAN_GetControllerInfo( ucCh );

        if( ( psControllerInfo != NULL_PTR ) && ( psControllerInfo->cRegister != NULL_PTR ) )
        {
            if( psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFFL != 0UL )
            {
                uiIndex = psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFGI;
                uiTxEvtMsgAddr = psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr + ( sizeof( CANRamTxEventFIFO_t ) * uiIndex );
                psSavedTxEvt = ( CANRamTxEventFIFO_t * ) uiTxEvtMsgAddr;

                psTxEvtBuffer->teExtendedId = ( uint8 ) psSavedTxEvt->rE0.rFReg.rfXTD;

                if( psTxEvtBuffer->teExtendedId == 0U )
                {
                    psTxEvtBuffer->teId = ( uint32 ) ( ( ( uint32 ) ( psSavedTxEvt->rE0.rFReg.rfID ) >> 18UL ) & 0x7FFUL );
                }
                else
                {
                    psTxEvtBuffer->teId = ( uint32 ) ( psSavedTxEvt->rE0.rFReg.rfID );
                }

                psTxEvtBuffer->teRemoteTransmitRequest = ( uint8 ) psSavedTxEvt->rE0.rFReg.rfRTR;
                psTxEvtBuffer->teFDFormat = ( uint8 ) psSavedTxEvt->rE1.rFReg.rfFDF;
                psTxEvtBuffer->teBitRateSwitching = ( uint8 ) psSavedTxEvt->rE1.rFReg.rfBRS;
                psTxEvtBuffer->teDataLength = CAN_MsgGetDataSizeBySizeType( ( CANDataLength_t ) psSavedTxEvt->rE1.rFReg.rfDLC );

                psControllerInfo->cRegister->crTxEventFIFOAcknowledge.rFReg.rfEFAI = uiIndex;
            }
            else
            {
                ret = CAN_ERROR_NO_MESSAGE;
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


#if defined(EDUCATION_CAN_DEMO_EN)
CANErrorType_t CAN_Edu_FilteringCanID
(
    uint8                               ucCh,
    CANMessageBufferType_t              uiBufferType
)
{
    uint32              i;
    uint32              j;
    uint32              ret;
    uint32              uiCheckFlag;
    uint32              uiIndex;
    uint32              uiNewMsgFlag[ 2 ];
    uint32              uiRxMsgAddr;
    CANRamRxBuffer_t *  psReceivedMsg;
    CANController_t *   psControllerInfo;

    CANRegFieldRXFxS_t  sRxFIFOStatus_bReg;
    uint32              uiRxFIFOAddr;
    uint32              uiGetIdx;
    uint32              uiPutIdx;

    uint8               isExtendedId = FALSE;
    uint32              getId = 0;
    uint32              bitTotal = 0;

    psControllerInfo = CAN_GetControllerInfo(ucCh);
    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL_PTR )
    {
        if( uiBufferType == CAN_RX_BUFFER_TYPE_DBUFFER )
        {
            uiNewMsgFlag[ 0 ] = psControllerInfo->cRegister->crNewData1.rNReg;
            uiNewMsgFlag[ 1 ] = psControllerInfo->cRegister->crNewData2.rNReg;

            for( i = 0UL ; i < 2UL ; i++ )
            {
                for( j = 0UL ; j < 32UL ; j++ )
                {
                    uiCheckFlag = ( uint32 ) 1UL << j;

                    if( ( uiNewMsgFlag[ i ] & uiCheckFlag ) != 0UL )
                    {
                        uiIndex = j + ( 32UL * i );

                        uiRxMsgAddr = psControllerInfo->cRamAddressInfo.raRxBufferAddr + ( sizeof( CANRamRxBuffer_t ) * ( uiIndex ) );
                        psReceivedMsg = ( CANRamRxBuffer_t * ) uiRxMsgAddr;

                        //// check Can ID
                        isExtendedId = ( uint8 ) psReceivedMsg->rR0.rFReg.rfXTD;
                        if( isExtendedId == TRUE )
                        {
                            getId = ( uint32 ) psReceivedMsg->rR0.rFReg.rfID;
                            bitTotal = 29;
                        }
                        else
                        {
                            getId = ( uint32 ) ( ( ( uint32 ) ( psReceivedMsg->rR0.rFReg.rfID ) >> 18UL ) & 0x7FFUL );
                            bitTotal = 11;
                        }

                        // CAN_D( "[1] getId = 0x%03x, idMask = 0x%03x, idAcceptance = 0x%03x", getId, gNowMetaInfo.idMask, gNowMetaInfo.idAcceptance);

                        if (CAN_Edu_checkFiltering(bitTotal, getId) == TRUE)
                        {
                            // CAN_D( "[1-1] will filtering!!");
                            ret = CAN_ERROR_CODE_FILTERED;
                            break;
                        }
                    }   // end for
                }

                if( ret != CAN_ERROR_NONE )
                {
                    break;
                }
            }

            if( ret != CAN_ERROR_NONE )
            {
                if( i == 0UL )
                {
                    psControllerInfo->cRegister->crNewData1.rNReg = uiNewMsgFlag[ 0 ];
                }
                else
                {
                    psControllerInfo->cRegister->crNewData2.rNReg = uiNewMsgFlag[ 1 ];
                }
            }
        }
        else
        {
            CANRegFieldRXFxA_t *psRxFIFOAcknowledge_bReg;

            if( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO0 )
            {
                sRxFIFOStatus_bReg = psControllerInfo->cRegister->crRxFIFO0Status.rFReg;
                psRxFIFOAcknowledge_bReg = &psControllerInfo->cRegister->crRxFIFO0Acknowledge.rFReg;
                uiRxFIFOAddr = psControllerInfo->cRamAddressInfo.raRxFIFO0Addr;
            }
            else if( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO1 )
            {
                sRxFIFOStatus_bReg = psControllerInfo->cRegister->crRxFIFO1Status.rFReg;
                psRxFIFOAcknowledge_bReg = &psControllerInfo->cRegister->crRxFIFO1Acknowledge.rFReg;
                uiRxFIFOAddr = psControllerInfo->cRamAddressInfo.raRxFIFO1Addr;
            }
            else
            {
                /* unknown buffer type */
                ret = CAN_ERROR_NO_MESSAGE;
            }

            if( ( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO0 ) || ( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO1 ) )
            {
                uiGetIdx = sRxFIFOStatus_bReg.rfFxGI;
                uiPutIdx = sRxFIFOStatus_bReg.rfFxPI;

                if( uiGetIdx != uiPutIdx )
                {
                    uiIndex = uiGetIdx;

                    uiRxMsgAddr = uiRxFIFOAddr + ( sizeof( CANRamRxBuffer_t ) * uiIndex );
                    psReceivedMsg = ( CANRamRxBuffer_t * ) uiRxMsgAddr;

                    //// check Can ID
                    isExtendedId = ( uint8 ) psReceivedMsg->rR0.rFReg.rfXTD;
                    if( isExtendedId == TRUE )
                    {
                        getId = ( uint32 ) psReceivedMsg->rR0.rFReg.rfID;
                        bitTotal = 29;
                    }
                    else
                    {
                        getId = ( uint32 ) ( ( ( uint32 ) ( psReceivedMsg->rR0.rFReg.rfID ) >> 18UL ) & 0x7FFUL );
                        bitTotal = 11;
                    }

                    // CAN_D( "[2] getId = 0x%03x, idMask = 0x%03x, idAcceptance = 0x%03x", getId, gNowMetaInfo.idMask, gNowMetaInfo.idAcceptance);

                    if (CAN_Edu_checkFiltering(bitTotal, getId) == TRUE)
                    {
                        ret = CAN_ERROR_CODE_FILTERED;
                        psRxFIFOAcknowledge_bReg->rfFxAI = uiIndex;
                        // CAN_D( "[2-1] will filtering!!");
                    }
                }
            }
        }
    }

    return ret;
}

uint8 CAN_Edu_checkFiltering
(
    uint32                              bitTotal,
    uint32                              canId
)
{
    uint32 bitIdx;
    uint32 compareId = 0;
    uint8 ret = FALSE;

    // 1. Get compareId for filtering
    for ( bitIdx = 0; bitIdx < bitTotal; bitIdx++ )
    {
        // bit 0: don't care
        if ( ( gNowMetaInfo.idMask & ( 1 << bitIdx ) ) == 0 )
        {
            continue;
        }

        // bit 0: don't need to set this bit
        if ( ( canId & ( 1 << bitIdx ) ) == 0 )
        {
            continue;
        }

        // set bit to 1. make compareId
        compareId |= ( 1 << bitIdx );
    }

    // 2. Compare Acceptance and compareId for filtering
    for ( bitIdx = 0; bitIdx < bitTotal; bitIdx++ )
    {
        // bit 0: don't care
        if ( ( gNowMetaInfo.idMask & ( 1 << bitIdx ) ) == 0 )
        {
            continue;
        }

        // must be the same
        if ( ( gNowMetaInfo.idAcceptance & ( 1 << bitIdx ) ) != ( compareId & ( 1 << bitIdx ) ) )
        {
            // If the bit is not same, filtering
            ret = TRUE;
            break;
        }
    }

    return ret;
}
#endif
