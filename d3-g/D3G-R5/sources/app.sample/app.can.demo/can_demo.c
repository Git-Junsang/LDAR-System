/*
***************************************************************************************************
*
*   FileName : can_demo.c
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

#include <app_cfg.h>

#if (ACFG_APP_CAN_DEMO_EN == 1)
#include "bsp.h"
#include "gic.h"
#include "gpio.h"
#include <debug.h>

#include "can_config.h"
#include "can_reg.h"
#include "can.h"
#include "can_par.h"
#include "can_drv.h"
#include "can_porting.h"
#include "can_demo.h"

#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN) || defined(EDUCATION_CAN_DEMO_EN)
#include "ipc.h"
#endif

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/
static uint32 s_ulxQueue = 0;

static CANDemoTestInfo_t sTestInfo;

#if defined(EDUCATION_CAN_DEMO_EN)
CANDemoMeta_t gNowMetaInfo;
CANDemoData_t gDemoData[CAN_CONTROLLER_NUMBER];
CANDemoRecvCounter_t gRecvCounter[CAN_CONTROLLER_NUMBER];
uint8 cntFlag = FALSE;

static uint8 CAN_DEMO_CHANNEL_VALUE_LIST[ CAN_CONTROLLER_NUMBER ] =
{
    CAN_DEMO_CHANNEL_VALUE_A, CAN_DEMO_CHANNEL_VALUE_B, CAN_DEMO_CHANNEL_VALUE_C
};
#endif

static CANFlagValue_t gCompletedFlag[3];
static CANFlagValue_t gErrorFlag[3];
static uint32 gReceiveFlag[3];

#ifdef CAN_DEMO_RESPONSE_TEST
static CANMessage_t sTxPreMessageInfo =
{
#if 1 /* Misra2012:9.3 - Partially Uninitialized Array */
 /* BufferType,                  Index, ESI, ExtendedID, RTR, ID,    FD, BRS, MM,   EventFIFO, DLC, DATA */
    CAN_TX_BUFFER_TYPE_FIFO,     0,     0,   0,          0,   0xFF,  1,  1,   0xFF, 1,         5,   {0, /* Data is definded as much as DLC in sending function */}
#else
 /* BufferType,                  Index, ESI, ExtendedID, RTR, ID,    FD, BRS, MM,   EventFIFO, DLC, DATA */
    CAN_TX_BUFFER_TYPE_FIFO,     0,     0,   0,          0,   0xFF,  1,  1,   0xFF, 1,         5,   {[0]=0x11,[1]=0x22,[2]=0x33,[3]=0x44,[4]=0x55}
#endif
};
#endif

static CANMessage_t sTxMessageInfo[CAN_MAX_TEST_MSG_NUM] =
{
#if 1 /* Misra2012:9.3 - Partially Uninitialized Array */
   /* BufferType,                 Index, ESI, ExtendedID, RTR, ID,    FD, BRS, MM,   EventFIFO, DLC, DATA */
#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
    { CAN_TX_BUFFER_TYPE_DBUFFER, 0,     0,   0,          0,   0x11,  0,  0,   0xFF, 1,         1,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 1,     0,   1,          0,   0x22,  0,  0,   0xFF, 1,         2,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 2,     0,   0,          0,   0x33,  0,  0,   0xFF, 1,         3,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 3,     0,   1,          0,   0x44,  0,  0,   0xFF, 1,         4,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x55,  0,  0,   0xFF, 1,         5,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x66,  0,  0,   0xFF, 1,         6,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x77,  0,  0,   0xFF, 1,         7,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x88,  0,  0,   0xFF, 1,         8,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x99,  0,  0,   0xFF, 1,         8,  {0, /* Data is definded as much as DLC in sending function */} },

#else
    { CAN_TX_BUFFER_TYPE_DBUFFER, 0,     0,   0,          0,   0x11,  1,  1,   0xFF, 1,         1,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 1,     0,   1,          0,   0x22,  1,  1,   0xFF, 1,         2,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 2,     0,   0,          0,   0x33,  1,  1,   0xFF, 1,         3,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 3,     0,   1,          0,   0x44,  1,  1,   0xFF, 1,         4,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x55,  1,  1,   0xFF, 1,         5,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x66,  1,  1,   0xFF, 1,         6,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x77,  1,  1,   0xFF, 1,         7,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x88,  1,  1,   0xFF, 1,         8,   {0, /* Data is definded as much as DLC in sending function */} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x99,  1,  1,   0xFF, 1,         12,  {0, /* Data is definded as much as DLC in sending function */} },
#endif
#else
   /* BufferType,                 Index, ESI, ExtendedID, RTR, ID,    FD, BRS, MM,   EventFIFO, DLC, DATA */
    { CAN_TX_BUFFER_TYPE_DBUFFER, 0,     0,   0,          0,   0x11,  1,  1,   0xFF, 1,         1,   {[0]=0x88} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 1,     0,   1,          0,   0x22,  1,  1,   0xFF, 1,         2,   {[0]=0x77,[1]=0x88} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 2,     0,   0,          0,   0x33,  1,  1,   0xFF, 1,         3,   {[0]=0x66,[1]=0x77,[2]=0x88} },
    { CAN_TX_BUFFER_TYPE_DBUFFER, 3,     0,   1,          0,   0x44,  1,  1,   0xFF, 1,         4,   {[0]=0x55,[1]=0x66,[2]=0x77,[3]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x55,  1,  1,   0xFF, 1,         5,   {[0]=0x44,[1]=0x55,[2]=0x66,[3]=0x77,[4]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x66,  1,  1,   0xFF, 1,         6,   {[0]=0x33,[1]=0x44,[2]=0x55,[3]=0x66,[4]=0x77,[5]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x77,  1,  1,   0xFF, 1,         7,   {[0]=0x22,[1]=0x33,[2]=0x44,[3]=0x55,[4]=0x66,[5]=0x77,[6]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,          0,   0x88,  1,  1,   0xFF, 1,         8,   {[0]=0x11,[1]=0x22,[2]=0x33,[3]=0x44,[4]=0x55,[5]=0x66,[6]=0x77,[7]=0x88} },
    { CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,          0,   0x99,  1,  1,   0xFF, 1,         12,  {[0]=0x00,[1]=0x11,[2]=0x22,[3]=0x33,[4]=0x44,[5]=0x55,[6]=0x66,[7]=0x77,[8]=0x88,[9]=0x99,[10]=0x10,[11]=0x12} },
#endif
};


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static void CAN_DemoCallbackTxEvent
(
    uint8                               ucCh,
    CANTxInterruptType_t                uiIntType
);

static void CAN_DemoCallbackRxEvent
(
    uint8                               ucCh,
    uint32                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
);

static void CAN_DemoCallbackErrorEvent
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
);

static void CAN_DemoSend
(
    uint8                               ucCh
);

#if defined(EDUCATION_CAN_DEMO_EN)
static void CAN_Edu_DemoInfo
(
    void
);

static void CAN_Edu_DemoSetInfo
(
    uint8                               index,
    int8 *                              value
);

static void CAN_Edu_DemoSend
(
    uint8                               ucCh
);

uint32 CAN_Edu_ConvIntegerToStr
(
    uint32                              from,
    uint8 *                             to
);

uint32  CAN_Edu_ConvHexStrToAscii
(
    uint8 *                             src,
    uint8                               srcLen,
    uint8 *                             dest
);

uint32 CAN_Edu_ConvAsciiToHexStr
(
    uint8 *                             from,
    uint8                               fromLen,
    uint8 *                             to
);

void CAN_Edu_DemoCreateApp
(
    void
);
static void CAN_Edu_DemoSendTask
(
    void *                              pArg
);

static void CAN_Edu_IpcCbFunc
(
    uint16                              uiCmd,
    uint8 *                             puData,
    uint16                              uiLength
);
#endif

static void CAN_DemoReceive
(
    uint8                               ucCh
);

static void CAN_DemoSendReceive
(
    void
);

static void CAN_DemoUsage
(
    void
);

static void CAN_DemoTask
(
    void *                              pArg
);


/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

static void CAN_DemoCallbackTxEvent
(
    uint8                               ucCh,
    CANTxInterruptType_t                uiIntType
)
{
    if( uiIntType == CAN_TX_INT_TYPE_TRANSMIT_COMPLETED )
    {
        gCompletedFlag[ucCh] = CAN_FLAG_TRUE;
    }
}

static void CAN_DemoCallbackRxEvent
(
    uint8                               ucCh,
    uint32                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
)
{
    if( uiError == CAN_ERROR_NONE )
    {
        gReceiveFlag[ucCh] = uiRxIndex + 1UL;
    }

    ( void ) uiRxBufferType;
}

#if defined(EDUCATION_CAN_DEMO_EN)
void CAN_Edu_resetReceiveFlag
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
)
{
    if( uiError == CAN_ERROR_NONE )
    {
        gReceiveFlag[ucCh] = 0;
    }
}
#endif

static void CAN_DemoCallbackErrorEvent
(
    uint8                               ucCh,
    CANErrorType_t                      uiError
)
{
    CAN_D( "[CAN ] chennel %d Error Event type: %d, ", ucCh, uiError );

    switch( uiError )
    {
        case CAN_ERROR_INT_ACCESS_RESERVED_ADDRESS:
        {
            CAN_E( "ACCESS_RESERVED_ADDRESS Error\r\n" );

            break;
        }

        case CAN_ERROR_INT_PROTOCOL:
        {
            CAN_E( "PROTOCO Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_BUS_OFF:
        {
            CAN_E( "BUS_OFF Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_WARNING:
        {
            CAN_E( "WARNING Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_PASSIVE:
        {
            CAN_E( "PASSIVE Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_BIT:
        {
            CAN_E( "BIT Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_TIMEOUT:
        {
            CAN_E( "TIMEOUT Error \r\n" );

            break;
        }

        case CAN_ERROR_INT_RAM_ACCESS_FAIL:
        {
            CAN_E( "RAM_ACCESS_FAIL \r\n" );

            break;
        }

        case CAN_ERROR_INT_TX_EVENT_FULL:
        {
            CAN_E( "TX_EVENT_FULL Error \r\n" );

            break;
        }

        case CAN_ERROR_INT_TX_EVENT_LOST:
        {
            CAN_E( "TX_EVENT_LOST Error \r\n" );

            break;
        }

        default:
        {
            CAN_E( "Error \r\n" );

            break;
        }
    }

    gErrorFlag[ucCh] = CAN_FLAG_TRUE;
}

static void CAN_DemoSend
(
    uint8                               ucCh
)
{
    uint8           ucTxMsgDlc;
    uint8           ucTxBufferIndex;
    uint32          uiTimeout;
    uint32          uiTxMsgCnt;
    uint32          uiTxEvtMsgCnt;
    CANMessage_t *  psTxMsg;
    CANTxEvent_t    sTxEvtMsg;
    CANErrorType_t  result;

    psTxMsg         = NULL_PTR;
    uiTxEvtMsgCnt   = 0;

    for( uiTxMsgCnt = 0UL ; uiTxMsgCnt < CAN_MAX_TEST_MSG_NUM ; uiTxMsgCnt++ )
    {
        uiTimeout = 10; //ms

        /* Fill data */
        for( ucTxMsgDlc = 0U ; ucTxMsgDlc < sTxMessageInfo[ uiTxMsgCnt ].mDataLength ; ucTxMsgDlc++ )
        {
            sTxMessageInfo[ uiTxMsgCnt ].mData[ ucTxMsgDlc ] = ucTxMsgDlc;
        }

        /* Send Tx message */
        psTxMsg = &sTxMessageInfo[ uiTxMsgCnt ];

        ( void ) CAN_SendMessage( ucCh, psTxMsg, &ucTxBufferIndex );

#if 0// defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN) //dgist
	CAN_PortingDelay(9);
	uiTimeout=uiTimeout;
	result=result;
	sTxEvtMsg=sTxEvtMsg;
	uiTxEvtMsgCnt=uiTxEvtMsgCnt;
#else    

        while ( ( gCompletedFlag[ucCh] == CAN_FLAG_FALSE ) && ( uiTimeout > 0UL ) )
        {
            CAN_PortingDelay( 1 );

            uiTimeout--;
        }

        /* Check send message */
        if( ( gCompletedFlag[ucCh] == CAN_FLAG_TRUE ) && ( uiTimeout != 0UL ) )
        {
            if( psTxMsg != NULL_PTR )
            {
                CAN_D( "[CAN DEMO] SEND     : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh, uiTxMsgCnt, psTxMsg->mId );

                uiTxEvtMsgCnt = CAN_CheckNewTxEvent( ucCh );

                if( 0UL < uiTxEvtMsgCnt )
                {
                    /* Get & Check Tx event message */
                    ( void ) SAL_MemSet( &sTxEvtMsg, 0, sizeof( CANTxEvent_t ) );

                    result = CAN_GetNewTxEvent( ucCh, &sTxEvtMsg );

                    if( result == CAN_ERROR_NONE )
                    {
                        if( psTxMsg->mId == sTxEvtMsg.teId )
                        {
                            CAN_D( "[CAN DEMO] TX_EVENT : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh, uiTxMsgCnt, psTxMsg->mId );
                        }
                        else
                        {
                            CAN_D( "[CAN DEMO] TX_EVENT : Fail(ID not match) - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh, uiTxMsgCnt, psTxMsg->mId );
                        }
                    }
                    else
                    {
                        CAN_D( "[CAN DEMO] TX_EVENT : Fail to get Tx Event Message \r\n" );
                    }
                }
            }

            gCompletedFlag[ucCh] = CAN_FLAG_FALSE;
        }
    }
#endif    
}

static void CAN_DemoReceive
(
    uint8                               ucCh
)
{
    uint32          uiRxMsgNum;
    CANMessage_t    sRxMsg;
    // CANErrorType_t  ret;

    uiRxMsgNum      = 0;

    if( 0UL < gReceiveFlag[ ucCh ] )
    {
        while( 1 )
        {
            uiRxMsgNum = CAN_CheckNewRxMessage( ucCh );

            if( 0UL < uiRxMsgNum )
            {
                ( void ) CAN_GetNewRxMessage( ucCh, &sRxMsg );
                // ret = CAN_GetNewRxMessage( ucCh, &sRxMsg );
                // mcu_printf( "[CAN DEMO] ret %d\n", ret );

#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN) || defined(EDUCATION_CAN_DEMO_EN)
                int32 ipc_ret = 0;
                uint32 ipc_ready = 0;

                CAN_D( "[Send to CA72]\n" );
                // Format => Channel : CAN ID : Data
                ipc_ret = IPC_IsReady(0x03/*IPC_SVC_CH_CA72_NONSECURE*/, (uint32 *)&ipc_ready);
                if ((ipc_ready == 1UL) && (ipc_ret == 0))
                    IPC_SendPacket(0x03/*IPC_SVC_CH_CA72_NONSECURE*/, ucCh,(uint16)sRxMsg.mId,(uint8*)sRxMsg.mData, (uint16)sRxMsg.mDataLength);
                else
                    CAN_E( "[CAN DEMO] IPC is not ready\n" );

                CAN_D( "[Send to CA53]\n" );
                ipc_ready = 0;
                ipc_ret = IPC_IsReady(0x01/*IPC_SVC_CH_CA53_NONSECURE*/, (uint32 *)&ipc_ready);
                if ((ipc_ready == 1UL) && (ipc_ret == 0))
                    IPC_SendPacket(0x01/*IPC_SVC_CH_CA53_NONSECURE*/, ucCh,(uint16)sRxMsg.mId,(uint8*)sRxMsg.mData, (uint16)sRxMsg.mDataLength);
                else
                    CAN_E( "[CAN DEMO] IPC is not ready\n" );
#endif
            }
            else
            {
                 CAN_D(" [CAN DEMO] No message\n" );
                break;
            }
        }

        gReceiveFlag[ ucCh ] = 0;
    }
}

static void CAN_DemoSendReceive
(
    void
)
{
    uint8           ucCh1;
    uint8           ucCh2;
    uint8           ucMsgLength;
    uint8           ucTxMsgDlc;
    uint8           ucTxBufferIndex;
    uint32          uiTxMsgCnt;
    uint32          uiRxMsgNum;
    uint32          uiTxEvtMsgCnt;
    uint32          uiTimeout;
    CANMessage_t *  psTxMsg;
    CANMessage_t    sRxMsg;
    CANTxEvent_t    sTxEvtMsg;
    CANErrorType_t  result;

    /* the messages are sent to another channel */
    for( ucCh1 = 0U ; ucCh1 < CAN_CONTROLLER_NUMBER ; ucCh1++ )
    {
        ( void ) CAN_InitMessage( ucCh1 ); //clear remain messages

        for( uiTxMsgCnt = 0UL ; uiTxMsgCnt < CAN_MAX_TEST_MSG_NUM ; uiTxMsgCnt++ )
        {
            uiTimeout = 10;

            /* Fill data */
            for( ucTxMsgDlc = 0U ; ucTxMsgDlc < sTxMessageInfo[ uiTxMsgCnt ].mDataLength ; ucTxMsgDlc++ )
            {
                sTxMessageInfo[ uiTxMsgCnt ].mData[ ucTxMsgDlc ] = ucTxMsgDlc;
            }

            /* Send Tx message */
            psTxMsg = &sTxMessageInfo[ uiTxMsgCnt ];
            ( void ) CAN_SendMessage( ucCh1, psTxMsg, &ucTxBufferIndex );

            while( ( gCompletedFlag[ucCh1] == CAN_FLAG_FALSE ) && ( uiTimeout > 0UL ) )
            {
                CAN_PortingDelay( 1 );

                uiTimeout--;
            }

            /* Check send message */
            if( ( gCompletedFlag[ ucCh1 ] == CAN_FLAG_TRUE ) && ( uiTimeout != 0UL ) )
            {
                CAN_D( "[CAN DEMO] SEND     : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );

                uiTxEvtMsgCnt = CAN_CheckNewTxEvent( ucCh1 );
                if( 0UL < uiTxEvtMsgCnt )
                {
                    /* Get & Check Tx event message */
                    ( void ) SAL_MemSet( &sTxEvtMsg, 0, sizeof( CANTxEvent_t ) );

                    result = CAN_GetNewTxEvent( ucCh1, &sTxEvtMsg );

                    if( result == CAN_ERROR_NONE )
                    {
                        if( psTxMsg->mId == sTxEvtMsg.teId )
                        {
                            CAN_D( "[CAN DEMO] TX_EVENT : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
                        }
                        else
                        {
                            CAN_D( "[CAN DEMO] TX_EVENT : Fail(ID not match) - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
                        }
                    }
                    else
                    {
                        CAN_D( "[CAN DEMO] TX_EVENT : Fail to get Tx Event Message \r\n" );
                    }
                }

                gCompletedFlag[ucCh1] = CAN_FLAG_FALSE;
            }
            else
            {
                CAN_D( "[CAN DEMO] SEND        : Fail(Timeout) - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
            }

            /* Check receive message for other channel */
            for( ucCh2 = 0U ; ucCh2 < CAN_CONTROLLER_NUMBER ; ucCh2++ )
            {
                if( ucCh2 != ucCh1 )
                {
                    if( 0UL < gReceiveFlag[ ucCh2 ] )
                    {
                        uiRxMsgNum = CAN_CheckNewRxMessage( ucCh2 );

                        if( 0UL < uiRxMsgNum )
                        {
                            ( void ) CAN_GetNewRxMessage( ucCh2, &sRxMsg );

                            if( psTxMsg->mId == sRxMsg.mId )
                            {
                                if( sRxMsg.mDataLength == 0U )
                                {
                                    CAN_D( "[CAN DEMO] RECEIVE  : Fail(No data) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                }
                                else
                                {
                                    for( ucMsgLength = 0 ; ucMsgLength < sRxMsg.mDataLength ; ucMsgLength++ )
                                    {
                                        if( psTxMsg->mData[ ucMsgLength ] != sRxMsg.mData[ ucMsgLength ] )
                                        {
                                            CAN_D( "[CAN DEMO] RECEIVE  : Fail(Data not match) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                        }
                                    }
                                    CAN_D( "[CAN DEMO] RECEIVE  : Success - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                }
                            }
                            else
                            {
                                CAN_D( "[CAN DEMO] RECEIVE  : Fail(ID not match) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                            }
                        }

                        gReceiveFlag[ ucCh2 ] = 0;
                    }
                    else
                    {
                        CAN_D( "[CAN DEMO] RECEIVE  : Fail(Timeout) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                    }
                }
            }
            CAN_D( "\r\n" );
        }
    }
}

sint32 CAN_DemoInitialize
(
    void
)
{
    sint32          ret;
    CANErrorType_t  result;

#ifdef CAN_DEMO_RESPONSE_TEST
    uint8           ucCh; /* can response channel : 0 */
    uint8           ucTxMsgDlc;
    uint8           ucTxBufferIndex;
    CANMessage_t *  psMsg; /* can response message */

    ucCh            = 0;
    psMsg           = NULL_PTR;
#endif

    ret             = 0;

    CAN_D( "[CAN DEMO] CAN Initialize START!!\n" );

    ( void ) CAN_RegisterCallbackFunctionTx( &CAN_DemoCallbackTxEvent );

    ( void ) CAN_RegisterCallbackFunctionRx( &CAN_DemoCallbackRxEvent );

    ( void ) CAN_RegisterCallbackFunctionError( &CAN_DemoCallbackErrorEvent );

    result = CAN_Init();

    if(result == CAN_ERROR_NONE)
    {
        CAN_D( "[CAN DEMO] CAN Initialize SUCCESS\n" );

#ifdef CAN_DEMO_RESPONSE_TEST
        /* Fill data */
        for( ucTxMsgDlc = 0U ; ucTxMsgDlc < sTxPreMessageInfo.mDataLength ; ucTxMsgDlc++ )
        {
            sTxPreMessageInfo.mData[ ucTxMsgDlc ] = ucTxMsgDlc;
        }

        psMsg = &sTxPreMessageInfo;
        ( void ) CAN_SendMessage( ucCh, psMsg, &ucTxBufferIndex );
#endif

    }
    else
    {
        CAN_D( "[CAN DEMO] CAN Initialize FAIL\n" );

        result = CAN_ERROR_NOT_INIT;
    }

    CAN_D( "[CAN DEMO] CAN Initialize END!!\n" );

    if( result != CAN_ERROR_NONE ) {
        ret = -1;
    }

    return ret;
}

static void CAN_DemoUsage
(
    void
)
{
    mcu_printf( "=== USAGE INFO ===\n\n" );
    mcu_printf( "=== CMD LIST ===\n" );
    mcu_printf( " can receive start\n" );
    mcu_printf( " can receive stop\n" );
    mcu_printf( " can send\n" );
    mcu_printf( " can send receive \n" );
#if defined(EDUCATION_CAN_DEMO_EN)
    mcu_printf( " can info\n" );
    mcu_printf( " can set  [index] [value]\n" );
    mcu_printf( " can send [id] [length] [data(hex)]\n" );
    mcu_printf( " can send [id] [length] [data(hex)] repeat [ms]\n" );
    mcu_printf( " can stop\n" );
    mcu_printf( " can count [on|off]\n" );
#endif
    mcu_printf( "\r\n" );
    mcu_printf( "=== CMD INFORMATION ===\n" );
    mcu_printf( "receive start:            Start receiving message, received message is printed via log\n" );
    mcu_printf( "receive stop:             Stop receiving message.\n" );
    mcu_printf( "send:                     Send pre defined test message\n" );
    mcu_printf( "send receive:             Integrated test, All channel send and receive\n" );
#if defined(EDUCATION_CAN_DEMO_EN)
    mcu_printf( "can info:                                        Output meta information.\n" );
    mcu_printf( "can set  [index] [value]:                        Set the meta information.\n" );
    mcu_printf( "can send [id] [length] [data(hex)]:              Send message.\n" );
    mcu_printf( "can send [id] [length] [data(hex)] repeat [ms]:  Send message repeat.\n" );
    mcu_printf( "can stop:                                        Stop repeat send message.\n" );
    mcu_printf( "can count [on|off]:                              Change count output flag.\n" );
#endif
}

void CAN_DemoTest
(
    uint8                               ucArgc,
    void *                              pArgv[]
)
{
    const uint8 *   pucStr1;
    const uint8 *   pucStr2;
    sint32          ret;

    pucStr1         = NULL_PTR;
    pucStr2         = NULL_PTR;

    if( pArgv != NULL_PTR )
    {
        pucStr1 = ( const uint8 * ) pArgv[ 0 ];
        pucStr2 = ( const uint8 * ) pArgv[ 1 ];

        // CAN_D( "ucArgc %d\n", ucArgc );

        if( ucArgc == 1UL )
        {
            if( pucStr1 != NULL_PTR )
            {
                if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "send", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    CAN_DemoSend( 0 ); //send pre defined message from channel 0
                }
#if defined(EDUCATION_CAN_DEMO_EN)
                else if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "info", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    CAN_Edu_DemoInfo();
                }
                else if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "stop", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    uint8 i;
                    for ( i = 0; i < CAN_CONTROLLER_NUMBER; i++ )
                    {
                        gDemoData[ i ].control.isRepeat = FALSE;
                        gDemoData[ i ].control.repeatInterval = 0;
                        gDemoData[ i ].control.status = CAN_DEMO_SEND_STATUS_STOP;
                    }
                }
#endif
            }
            else
            {
                CAN_D( "Wrong argument\n" );

                CAN_DemoUsage();
            }
        }
        else if( ucArgc == 2UL )
        {
            if( ( pucStr1 != NULL_PTR ) && ( pucStr2 != NULL_PTR ) )
            {
                if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "receive", 7, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    if( ( SAL_StrNCmp( pucStr2, ( const uint8 * ) "start", 5, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        sTestInfo.tiRecv = TRUE;

                        CAN_D( "[CAN DEMO] CAN receive START\n" );
                    }
                    else if( ( SAL_StrNCmp( pucStr2, ( const uint8 * ) "stop", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        sTestInfo.tiRecv = FALSE;

                        CAN_D( "[CAN DEMO] CAN receive STOP\n" );
                    }
                    else
                    {
                        CAN_D( "[CAN DEMO] Wrong argument\n" );
                    }
                }
                else if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "send", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    if( ( SAL_StrNCmp( pucStr2, ( const uint8 * ) "receive", 7, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        CAN_DemoSendReceive(); //send and receive pre defined message from all channel to other channels
                    }
                    else
                    {
                        CAN_D( "Wrong argument\n" );

                        CAN_DemoUsage();
                    }
                }
                else if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "count", 5, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    if( ( SAL_StrNCmp( pucStr2, ( const uint8 * ) "on", 2, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        ;//cntFlag = TRUE;
                    }
                    else if( ( SAL_StrNCmp( pucStr2, ( const uint8 * ) "off", 3, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        ;//cntFlag = FALSE;
                    }
                    else
                    {
                        CAN_D( "Wrong argument\n" );

                        CAN_DemoUsage();
                    }
                }
                else
                {
                    CAN_D( "Wrong argument\n" );

                    CAN_DemoUsage();
                }
            }
        }
#if defined(EDUCATION_CAN_DEMO_EN)
        else if( ucArgc == 3UL )
        {
            if( pucStr1 != NULL_PTR )
            {
                if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "set", 3, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    int8 *cmdIndex = NULL_PTR;
                    int8 *cmdValue = NULL_PTR;

                    cmdIndex = ( int8 * ) pucStr2;
                    cmdValue = ( int8 * ) pArgv[ 2 ];

                    if ( (cmdIndex != NULL_PTR) && (cmdValue != NULL_PTR) )
                    {
                        uint32 len = 0;
                        uint8 index = 0;

                        // index
                        SAL_StrLength( cmdIndex , &len );
                        if (len > 1) {
                            CAN_D( "Wrong value\n" );
                            CAN_DemoUsage();
                            return;
                        }

                        index += ( cmdIndex[0] - '0' );

                        CAN_Edu_DemoSetInfo( index, cmdValue );
                    }
                    else
                    {
                        CAN_D( "Wrong argument\n" );
                        CAN_DemoUsage();
                    }
                }
            }
        }
        else if( ucArgc == 4UL )        // send [id] [length] [data]
        {
            if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "send", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
            {
                for (uint8 ch = 0; ch < CAN_CONTROLLER_NUMBER; ch++)
                {
                    if ( gNowMetaInfo.channel & CAN_DEMO_CHANNEL_VALUE_LIST[ ch ] )
                    {
                        CANDemoValue_t *canData = &gDemoData[ ch ].value;

                        int8 *cmdID     = NULL_PTR;
                        int8 *cmdLength = NULL_PTR;
                        uint8 *cmdHexData = NULL_PTR;

                        cmdID       = ( int8 * ) pucStr2;
                        cmdLength   = ( int8 * ) pArgv[ 2 ];
                        cmdHexData  = ( uint8 * ) pArgv[ 3 ];

                        if ( (cmdID != NULL_PTR) && (cmdLength != NULL_PTR) && (cmdHexData != NULL_PTR) )
                        {
                            uint32 i = 0;
                            uint32 len = 0;

                            // id
                            SAL_StrLength( cmdID , &len );
                            for( i = 0 ; i < len ; i++ )
                            {
                                canData->id *= 10;
                                canData->id += ( cmdID[ i ] - '0' );
                            }

                            // length
                            SAL_StrLength( cmdLength , &len );
                            for( i = 0 ; i < len ; i++ )
                            {
                                canData->length *= 10;
                                canData->length += ( cmdLength[ i ] - '0' );
                            }

                            // data
                            len = CAN_Edu_ConvHexStrToAscii( cmdHexData, canData->length, canData->data );
                            if (len == 255) {
                                CAN_D( "Wrong argument\n" );
                                CAN_DemoUsage();
                                return;
                            }

                            //// set status for send start
                            gDemoData[ ch ].control.status = CAN_DEMO_SEND_STATUS_START;
                        }
                        else
                        {
                            CAN_D( "Wrong argument\n" );
                            CAN_DemoUsage();
                        }
                    }
                }

            }

        }
        else if( ucArgc == 6UL )        // send [id] [length] [data] repeat [ms]
        {
            const uint8 *   pucStr5 = ( const uint8 * ) pArgv[ 4 ];
            if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "send", 4, &ret ) == SAL_RET_SUCCESS ) 
                && ( SAL_StrNCmp( pucStr5, ( const uint8 * ) "repeat", 6, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
            {
                for (uint8 ch = 0; ch < CAN_CONTROLLER_NUMBER; ch++)
                {
                    if ( gNowMetaInfo.channel & CAN_DEMO_CHANNEL_VALUE_LIST[ ch ] )
                    {
                        CANDemoValue_t *canData = &gDemoData[ ch ].value;

                        int8 *cmdID     = NULL_PTR;
                        int8 *cmdLength = NULL_PTR;
                        uint8 *cmdHexData = NULL_PTR;
                        int8 *cmdMillis = NULL_PTR;
                        cmdID       = ( int8 * ) pucStr2;
                        cmdLength   = ( int8 * ) pArgv[ 2 ];
                        cmdHexData  = ( uint8 * ) pArgv[ 3 ];
                        cmdMillis   = ( int8 * ) pArgv[ 5 ];

                        if ( (cmdID != NULL_PTR) && (cmdLength != NULL_PTR) && (cmdHexData != NULL_PTR) )
                        {
                            uint32 i = 0;
                            uint32 len = 0;

                            // id
                            SAL_StrLength( cmdID , &len );
                            for( i = 0 ; i < len ; i++ )
                            {
                                canData->id *= 10;
                                canData->id += ( cmdID[ i ] - '0' );
                            }

                            // length
                            SAL_StrLength( cmdLength , &len );
                            for( i = 0 ; i < len ; i++ )
                            {
                                canData->length *= 10;
                                canData->length += ( cmdLength[ i ] - '0' );
                            }

                            // data
                            len = CAN_Edu_ConvHexStrToAscii( cmdHexData, canData->length, canData->data );
                            if (len == 255) {
                                CAN_D( "Wrong argument\n" );
                                CAN_DemoUsage();
                                return;
                            }

                            // ms
                            SAL_StrLength( cmdMillis , &len );
                            for( i = 0 ; i < len ; i++ )
                            {
                                gDemoData[ ch ].control.repeatInterval *= 10;
                                gDemoData[ ch ].control.repeatInterval += ( cmdMillis[ i ] - '0' );
                            }
                            // repeat flag
                            gDemoData[ ch ].control.isRepeat = TRUE;

                            //// set status for send start
                            gDemoData[ ch ].control.status = CAN_DEMO_SEND_STATUS_START;
                        }
                        else
                        {
                            CAN_D( "Wrong argument\n" );
                            CAN_DemoUsage();
                        }
                    }
                }
            }
        }
#endif
        else
        {
            CAN_D( "Wrong argument\n" );

            CAN_DemoUsage();
        }
    }
}

#if defined(EDUCATION_CAN_DEMO_EN)
static void CAN_Edu_DemoInfo
(
    void
)
{
    mcu_printf( "====== CAN META INFO ======\n" );
    mcu_printf( " index\tmeta\t\tvalue\n" );
    mcu_printf( " [1]\tFD\t\t%d\n", gDemoData[0].meta.isFD );
    mcu_printf( " [2]\tChannel\t\t%d\n", gNowMetaInfo.channel );
    mcu_printf( " [3]\tBaudRate\t%d\n", gDemoData[0].meta.baudRate );
    mcu_printf( " [4]\tFast BaudRate\t%d\n", gDemoData[0].meta.fastBaudRate );
    mcu_printf( " [5]\tID Mask\t\t0x%03X\n", gDemoData[0].meta.idMask );
    mcu_printf( " [6]\tID Acceptance\t0x%03X\n", gDemoData[0].meta.idAcceptance );
}

static void CAN_Edu_DemoSetInfo
(
    uint8                               index,
    int8 *                              cmdValue
)
{
    uint32 i = 0;
    uint32 len = 0;
    int32 value = 0;

    // value
    SAL_StrLength( cmdValue , &len );
    if (index != 5 && index != 6)     // Except ID Mask, ID Acceptance (need hex)
    {
        for( i = 0 ; i < len ; i++ )
        {
            value *= 10;
            value += ( cmdValue[ i ] - '0' );
        }
    }

    switch (index)
    {
    case 1:     // FD           -> 0:CAN / 1:CAN-FD
    {
        if (value == 0 || value == 1)
        {
            gNowMetaInfo.isFD = value;
        }
        else
        {
            CAN_D( "Wrong value\n" );
            return;
        }
        break;
    }
    case 2:     // Channel     -> 1(0x01):A / 2(0x02):B / 4(0x04):C
    {
        if (value < 0x08)
        {
            gNowMetaInfo.channel = value;
        }
        else
        {
            CAN_D( "Wrong value\n" );
            return;
        }
        break;
    }
    case 3:     // BaudRate
    {
        gNowMetaInfo.baudRate = value;

        // same baudrate per channel
        for ( i = 0 ; i < CAN_CONTROLLER_NUMBER ; i++ )
        {
            gDemoData[ i ].meta.baudRate = value;

            // change bitrate
            switch ( gDemoData[ i ].meta.baudRate )
            {
                case 100:
                {
                    ArbitrationPhaseTimingPar[ i ].tpBRP            = 20;
                    ArbitrationPhaseTimingPar[ i ].tpPROP           = 1;
                    ArbitrationPhaseTimingPar[ i ].tpPhaseSeg1      = 15;
                    ArbitrationPhaseTimingPar[ i ].tpPhaseSeg2      = 3;
                    ArbitrationPhaseTimingPar[ i ].tpSJW            = 1;
                    ArbitrationPhaseTimingPar[ i ].tpTDC            = 0;
                    ArbitrationPhaseTimingPar[ i ].tpTDCOffset      = 0;
                    ArbitrationPhaseTimingPar[ i ].tpTDCFilterWindow = 0;
                    break;
                }
                case 200:
                {
                    ArbitrationPhaseTimingPar[ i ].tpBRP            = 10;
                    ArbitrationPhaseTimingPar[ i ].tpPROP           = 1;
                    ArbitrationPhaseTimingPar[ i ].tpPhaseSeg1      = 15;
                    ArbitrationPhaseTimingPar[ i ].tpPhaseSeg2      = 3;
                    ArbitrationPhaseTimingPar[ i ].tpSJW            = 1;
                    ArbitrationPhaseTimingPar[ i ].tpTDC            = 0;
                    ArbitrationPhaseTimingPar[ i ].tpTDCOffset      = 0;
                    ArbitrationPhaseTimingPar[ i ].tpTDCFilterWindow = 0;
                    break;
                }
                case 250:
                {
                    ArbitrationPhaseTimingPar[ i ].tpBRP            = 8;
                    ArbitrationPhaseTimingPar[ i ].tpPROP           = 1;
                    ArbitrationPhaseTimingPar[ i ].tpPhaseSeg1      = 15;
                    ArbitrationPhaseTimingPar[ i ].tpPhaseSeg2      = 3;
                    ArbitrationPhaseTimingPar[ i ].tpSJW            = 1;
                    ArbitrationPhaseTimingPar[ i ].tpTDC            = 0;
                    ArbitrationPhaseTimingPar[ i ].tpTDCOffset      = 0;
                    ArbitrationPhaseTimingPar[ i ].tpTDCFilterWindow = 0;
                    break;
                }
                case 500:
                {
                    ArbitrationPhaseTimingPar[ i ].tpBRP            = 1;
                    ArbitrationPhaseTimingPar[ i ].tpPROP           = 47;
                    ArbitrationPhaseTimingPar[ i ].tpPhaseSeg1      = 16;
                    ArbitrationPhaseTimingPar[ i ].tpPhaseSeg2      = 16;
                    ArbitrationPhaseTimingPar[ i ].tpSJW            = 16;
                    ArbitrationPhaseTimingPar[ i ].tpTDC            = 0;
                    ArbitrationPhaseTimingPar[ i ].tpTDCOffset      = 0;
                    ArbitrationPhaseTimingPar[ i ].tpTDCFilterWindow = 0;
                    break;
                }
            }

            CAN_SetControllerMode( i, CAN_MODE_RESET_CONTROLLER );
        }

        break;
    }
    case 4:     // Fast BaudRate
    {
        gNowMetaInfo.fastBaudRate = value;
        break;
    }
    case 5:     // ID Mask
    case 6:     // ID Acceptance
    {
        // check length
        if (len != 5 && len != 3)
        {
            CAN_D( "Wrong value\n" );
            return;
        }

        // remove '0x' in cmdValue
        if (len == 5)
        {
            if ( (cmdValue[0] != '0') || (cmdValue[1] != 'x') )
            {
                CAN_D( "Wrong value\n" );
                return;
            }

            for( i = 0 ; i < 3 ; i++ )
            {
                cmdValue[ i ] = cmdValue[ i + 2 ];
            }
            cmdValue[ 3 ] = '\0';
            len = 3;
        }

        // change string cmdValue to hex value
        for( i = 0 ; i < len ; i++ )
        {
            value <<= 4;

            if ( (cmdValue[ i ] >= '0') && (cmdValue[ i ] <= '9') )         { value += ( cmdValue[ i ] - '0' ); }
            else if ( (cmdValue[ i ] >= 'A') && (cmdValue[ i ] <= 'F') )    { value += ( cmdValue[ i ] - 'A' + 10 ); }
            else if ( (cmdValue[ i ] >= 'a') && (cmdValue[ i ] <= 'f') )    { value += ( cmdValue[ i ] - 'a' + 10 ); }
            else
            {
                CAN_D( "Wrong value\n" );
                return;
            }
        }

        if ( (value >= CAN_DEMO_ID_MIN) && (value <= CAN_DEMO_ID_MAX) )
        {
            if (index == 5)
            {
                gNowMetaInfo.idMask = value;
            }
            else
            {
                gNowMetaInfo.idAcceptance = value;
            }
        }
        else
        {
            CAN_D( "Wrong value\n" );
            return;
        }
        break;
    }
    default:
        CAN_D( "Wrong value\n" );
        return;
    }

    uint8 ch;
    for ( ch = 0 ; ch < CAN_CONTROLLER_NUMBER ; ch++ )
    {
        if ( gNowMetaInfo.channel & CAN_DEMO_CHANNEL_VALUE_LIST[ ch ] )
        {
            SAL_MemCopy( &gDemoData[ch].meta, &gNowMetaInfo, sizeof( CANDemoMeta_t ) );
            gDemoData[ch].meta.channel = ch;
        }
    }

    CAN_Edu_DemoInfo();
}

static void CAN_Edu_DemoSend
(
    uint8                                   ucCh
)
{
    uint8           canTxBufferIndex;
    uint32          canTimeout;
    uint32          canTxEvtMsgCnt;
    CANMessage_t    canTxMsg;
    CANTxEvent_t    canTxEvtMsg;
    CANErrorType_t  canErrCode;

    CANDemoData_t*  canData = &gDemoData[ ucCh ];

    if ( canData == NULL )
    {
        CAN_E( "Invalid channel %d\n", ucCh);
        return;
    }
    
    // check canID ((TODO) ExtendedID)
    if ( canData->value.id > CAN_DEMO_ID_MAX )
    {
        CAN_E( "CAN ID is out of range\n" );
        return;
    }

    // check data length
    if ( gNowMetaInfo.isFD == 1 )
    {
        if ( canData->value.length > CAN_DEMO_EXT_MSG_LEN_MAX )
        {
            CAN_E( "CAN FD data length %d is over %d\n", canData->value.length, CAN_DEMO_EXT_MSG_LEN_MAX );
            return;
        }
    }
    else
    {
        if ( canData->value.length > CAN_DEMO_STD_MSG_LEN_MAX )
        {
            CAN_E( "CAN data length %d is over %d\n", canData->value.length, CAN_DEMO_STD_MSG_LEN_MAX );
            return;
        }
    }

    ( void ) SAL_MemSet( &canTxMsg, 0, sizeof( CANMessage_t ) );

    // set canTxMsg
    // canTxMsg.mBufferType    = CAN_TX_BUFFER_TYPE_DBUFFER;
    canTxMsg.mBufferType    = CAN_TX_BUFFER_TYPE_QUEUE;
    // canTxMsg.mBufferIndex
    // canTxMsg.mErrorStateIndicator
    canTxMsg.mExtendedId = (canData->value.id > CAN_DEMO_ID_MAX) ? TRUE : FALSE;
    // canTxMsg.mRemoteTransmitRequest
    canTxMsg.mId = canData->value.id;
    canTxMsg.mFDFormat = gNowMetaInfo.isFD;
    // canTxMsg.mBitRateSwitching = gNowMetaInfo.baudRate;
    // canTxMsg.mMessageMarker
    // canTxMsg.mEventFIFOControl
    canTxMsg.mDataLength = canData->value.length;
    SAL_MemCopy( canTxMsg.mData, canData->value.data, canTxMsg.mDataLength );
    CAN_D( "canTxMsg => id: %d, isFE: %d, extID: %d, len: %d, data: %s", canTxMsg.mId, canTxMsg.mFDFormat, canTxMsg.mExtendedId, canTxMsg.mDataLength, canTxMsg.mData);

    canTimeout = canTxMsg.mDataLength + 10;

    // send
    CANErrorType_t ret = CAN_SendMessage( ucCh, &canTxMsg, &canTxBufferIndex );

    while ( ( gCompletedFlag[ucCh] == CAN_FLAG_FALSE ) && ( canTimeout > 0UL ) )
    {
        CAN_PortingDelay( 1 );

        canTimeout--;
    }
    CAN_D("\nret: %d \n", ret);

    if ( (canTxMsg.mDataLength > 1) ) {
        CAN_D("\nCAN_SEND ch: %d id: %d mDataLength:0x%02X (%d:%d:%d)\n", ucCh, canTxMsg.mId, (char)canTxMsg.mData[0], canTxMsg.mDataLength, gCompletedFlag[ucCh], canTimeout);

                uint8           ucMsgLength;
                CAN_D( "[SEND CAN DEMO]\n" );
                CAN_D( "< Channel %d SEND Message Information >\n", ucCh );
                CAN_D( "***********************************************************************************\n" );
                CAN_D( "[ID] : 0x%X, [DATA SIZE] : %d, [DATA] : \r\n", canTxMsg.mId, canTxMsg.mDataLength );
                for( ucMsgLength = 1U ; ucMsgLength < ( canTxMsg.mDataLength + 1U ) ; ucMsgLength++ )
                {
                    CAN_D( "0x%02X ", canTxMsg.mData[ ucMsgLength - 1U ] );

                    if( ( ucMsgLength % 16U ) == 0U ) {
                        CAN_D( "\n" );
                    }
                }
                CAN_D( "\n" );
                CAN_D( "***********************************************************************************\n" );
                CAN_D( "\n" );
        
    }
    else {
        CAN_D("\nCAN_SEND ch: %d id: %d mDataLength:%d (%d:%d)\n", ucCh, canTxMsg.mId, canTxMsg.mDataLength, gCompletedFlag[ucCh], canTimeout);
    }

    /* Check send message */
    if( ( gCompletedFlag[ucCh] == CAN_FLAG_TRUE ) && ( canTimeout != 0UL ) )
    {
        CAN_D( "Successful transmission.\n" );

        canTxEvtMsgCnt = CAN_CheckNewTxEvent( ucCh );
        if( 0UL < canTxEvtMsgCnt )
        {
            /* Get & Check Tx event message */
            ( void ) SAL_MemSet( &canTxEvtMsg, 0, sizeof( CANTxEvent_t ) );

            canErrCode = CAN_GetNewTxEvent( ucCh, &canTxEvtMsg );

            if( canErrCode == CAN_ERROR_NONE )
            {
                if( canTxMsg.mId == canTxEvtMsg.teId )
                {
                    CAN_D( "[CAN DEMO] TX_EVENT : Success - Tx channel : %d, Tx ID: 0x%X \r\n", ucCh, canTxMsg.mId );
                }
                else
                {
                    CAN_D( "[CAN DEMO] TX_EVENT : Fail(ID not match) - Tx channel : %d, Tx ID: 0x%X \r\n", ucCh, canTxMsg.mId );
                }
            }
            else
            {
                CAN_D( "[CAN DEMO] TX_EVENT : Fail to get Tx Event Message \r\n" );
            }
        }

        gCompletedFlag[ucCh] = CAN_FLAG_FALSE;
    }
}

uint32 CAN_Edu_ConvIntegerToStr
(
    uint32                              from,
    uint8 *                             to
)
{
    uint8 i;
    uint8 tmpStr[12];
    uint32 copyInteger;
    uint32 sLen;

    sLen = 0;
    copyInteger = from;

    if (copyInteger == 0)
    {
        to[0] = '0';
        to[1] = '\0';
        sLen = 1;
    }
    else
    {
        while (copyInteger > 0)
        {
            tmpStr[sLen++] = (copyInteger % 10) + '0';
            copyInteger = copyInteger / 10;
        }

        for ( i = 0; i < sLen; i++ )
        {
            to[ i ] = tmpStr[ sLen - i - 1 ];
        }
        to[ sLen ] = '\0';
    }

    return sLen;
}

uint32  CAN_Edu_ConvHexStrToAscii
(
    uint8 *                             src,
    uint8                               srcLen,
    uint8 *                             dest
)
{
    uint8 i;
    uint32 tLen;

    tLen = 0;
    srcLen *= 2;

    for ( i = 0; i < srcLen; i++ )
    {
        if ( src[ i ] >= '0' && src[ i ] <= '9' )
        {
            dest[ tLen ] = (src[ i ] - '0') << 4;
        }
        else if ( src[ i ] >= 'A' && src[ i ] <= 'F' )
        {
            dest[ tLen ] = (src[ i ] - 'A' + 10) << 4;
        }
        else if ( src[ i ] >= 'a' && src[ i ] <= 'f' )
        {
            dest[ tLen ] = (src[ i ] - 'a' + 10) << 4;
        }
        else
        {
            dest[ tLen ] = 0;
        }

        i++;
        if ( i > srcLen )
        {
            tLen = 255;     // invalid
            break;
        }

        if ( src[ i ] >= '0' && src[ i ] <= '9' )
        {
            dest[ tLen ] += (src[ i ] - '0');
        }
        else if ( src[ i ] >= 'A' && src[ i ] <= 'F' )
        {
            dest[ tLen ] += (src[ i ] - 'A' + 10);
        }
        else if ( src[ i ] >= 'a' && src[ i ] <= 'f' )
        {
            dest[ tLen ] += (src[ i ] - 'a' + 10);
        }
        else
        {
            dest[ tLen ] += 0;
        }

        tLen++;
    }

    return tLen;
}

uint32  CAN_Edu_ConvAsciiToHexStr
(
    uint8 *                             src,
    uint8                               srcLen,
    uint8 *                             dest
)
{
    uint8 i;
    uint32 tLen;

    tLen = 0;

    for ( i = 0; i < srcLen; i++ )
    {
        dest[ tLen++ ] = (src[ i ] >> 4) & 0x0F;
        dest[ tLen++ ] = src[ i ] & 0x0F;
    }

    for ( i = 0; i < tLen; i++ )
    {
        if ( dest[ i ] < 10 )
        {
            dest[ i ] += '0';
        }
        else
        {
            dest[ i ] += 'A' - 10;
        }
    }
    dest[ tLen ] = '\0';

    return tLen;
}

static void CAN_Edu_IpcCbFunc(uint16 uhwCmd, uint8 *pucData, uint16 uhwLength)
{
	if(s_ulxQueue != 0) {
		CANDemoValue_t sRxValue;
		sRxValue.id = uhwCmd;
		sRxValue.length = (uhwLength>CAN_DATA_LENGTH_SIZE)?CAN_DATA_LENGTH_SIZE:uhwLength;
		if(SAL_RET_SUCCESS == SAL_MemCopy(sRxValue.data, pucData, sRxValue.length))
		{
			SAL_QueuePut(s_ulxQueue, (void*)&sRxValue, sizeof(CANDemoValue_t), 0, SAL_OPT_NON_BLOCKING);
		}
	}
}

static void CAN_Edu_SetSendingData(uint32 uhwCmd, uint8 *pucData, uint32 uhwLength)
{        
    if (uhwCmd == (uint16)IPC_IPC_CMD_CA72_EDUCATION_CAN_DEMO_START)
    {
        if (uhwLength == 0)
        {
            CAN_E("%s:%d uhwLength invalid !!!\n", __func__, uhwLength);
            return;
        }

        uint8 ch_bitmask = (pucData[0] & 0xFF);
        uint8 tx_only_bitmask = (pucData[1] & 0xFF);
        uint16 can_id = (pucData[2] << 8) | pucData[3];

        CAN_D("\nch_bitmask: 0x%X, tx_only_bitmask: 0x%X, can_id: ox%X\n", ch_bitmask, tx_only_bitmask, can_id);

        for (uint8 ch = 0; ch < CAN_CONTROLLER_NUMBER; ch++)
        {
            if ((ch_bitmask & CAN_DEMO_CHANNEL_VALUE_LIST[ch]) == 0)
                continue;
            
            uint16 cmd_len = 4;
            CANDemoValue_t *canData = &gDemoData[ ch ].value;
            canData->id = can_id;

            canData->length = uhwLength - cmd_len; 
            SAL_MemCopy(canData->data, &pucData[cmd_len], canData->length);

            // test 
            uint8 ucMsgLength;
            CAN_D( "[CH] : 0x%x, [ID] : 0x%x, [DATA SIZE] : %d (%d), [DATA] : \r\n", ch, canData->id, canData->length, uhwLength );
            for( ucMsgLength = 0 ; ucMsgLength < canData->length ; ucMsgLength++ )
            {
                CAN_D( "0x%02X ", canData->data[ ucMsgLength ] );
                if( ucMsgLength && (( ucMsgLength % 16U ) == 0U) ) {
                    CAN_D( "\n" );
                }  // python3 IPC_Example.py snd --sndDataStr "123" --channel 2
            }
            CAN_D( "\n" );
            
            // set tx only channel
            if (tx_only_bitmask & CAN_DEMO_CHANNEL_VALUE_LIST[ch])   
                gNowMetaInfo.channel &= ~CAN_DEMO_CHANNEL_VALUE_LIST[ch];
            else
                gNowMetaInfo.channel |= CAN_DEMO_CHANNEL_VALUE_LIST[ch];

            CAN_D("gNowMetaInfo.channel : 0x%x \r\n", gNowMetaInfo.channel );

            // set status for send start
            gDemoData[ ch ].control.status = CAN_DEMO_SEND_STATUS_START;   
        }
    }
    else 
    {
        CAN_E("%s:%d uhwCmd invalid !!!\n", __func__, uhwLength);
    }
}

void CAN_Edu_DemoCreateApp
(
    void
)
{
    static uint32 uiCanDemoAppTaskID;
    static uint32 uiCanDemoAppTaskStk[CAN_DEMO_TASK_STK_SIZE];

    ( void ) SAL_TaskCreate( &uiCanDemoAppTaskID,
                   ( const uint8 * ) "Can Edu Demo Send Task",
                   ( SALTaskFunc ) &CAN_Edu_DemoSendTask,
                   ( uint32 * const ) &uiCanDemoAppTaskStk[0],
                   CAN_DEMO_TASK_STK_SIZE,
                   SAL_PRIO_CAN_DEMO,
                   NULL_PTR);
}

static boolean CAN_Edu_CheckCANDataToSend(uint32 *punMillsToWait)
{
    uint8 ucCh;
	uint32 ulDataLen = 0;
	CANDemoValue_t sqDemoMsg;
	boolean bDatafromR5 = false;

	if(s_ulxQueue != 0) {
		// Check data from R5 itself
		for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
		{
			 if(gDemoData[ ucCh ].control.status == CAN_DEMO_SEND_STATUS_START)
			{
				bDatafromR5 = true;
				break;
			}
		}

		// no data from R5, check IPC queue from A72.
		if (!bDatafromR5)
		{
			if(SAL_QueueGet(s_ulxQueue, &sqDemoMsg, &ulDataLen, 100, SAL_OPT_BLOCKING) == SAL_RET_FAILED)
			{
				return false;
			}

			if(ulDataLen > 0)
				CAN_Edu_SetSendingData(sqDemoMsg.id, sqDemoMsg.data, sqDemoMsg.length);
			else
				return false;

		}
	}
	else
	{
		CAN_E("No QUEUE... data from IPC will not work!!\n");
		if(!punMillsToWait)
			*punMillsToWait = 100;
		
		return true;
	}

	return true;
}

static void CAN_Edu_DemoSendTask
(
    void *                              pArg
)
{
    uint8 ucCh;
    uint32 hasRepeatIv;

    ( void ) pArg;

    // Register IPC callback function
    IPC_RegisterCbFunc(IPC_SVC_CH_CA72_NONSECURE, (uint8)TCC_IPC_CMD_CA72_EDUCATION_CAN_DEMO, (IPCCallback)&CAN_Edu_IpcCbFunc, NULL_PTR, NULL_PTR);
//    IPC_RegisterCbFunc(IPC_SVC_CH_CA53_NONSECURE, (uint8)TCC_IPC_CMD_CA53_EDUCATION_CAN_DEMO, (IPCCallback)&CAN_Edu_IpcCbFunc, NULL_PTR, NULL_PTR);
    // gDebugOption = gDebugOption | (DBG_TAG_BIT(5));
    // gDebugOption = (gDebugOption & ~(DBG_LEVEL_POSITION)) | DBG_LEVEL_ERROR;

    ( void ) SAL_MemSet( &gNowMetaInfo, 0, sizeof( CANDemoMeta_t ) );
    gNowMetaInfo.channel = CAN_DEMO_CHANNEL_VALUE_A | CAN_DEMO_CHANNEL_VALUE_B | CAN_DEMO_CHANNEL_VALUE_C;
    gNowMetaInfo.baudRate = 500;
    gNowMetaInfo.idAcceptance = CAN_DEMO_ID_MAX;

    ( void ) SAL_MemSet( gDemoData, 0, sizeof( CANDemoData_t ) * 3 );
    for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
    {
        gDemoData[ ucCh ].meta.channel = ucCh;
        gDemoData[ ucCh ].meta.baudRate = gNowMetaInfo.baudRate;
        gDemoData[ ucCh ].meta.idAcceptance = gNowMetaInfo.idAcceptance;
    }

    while( 1 )
    {
        hasRepeatIv = 0;

		uint32 unMillstoWait = 0;
		if(CAN_Edu_CheckCANDataToSend(&unMillstoWait) == false)
			continue;
		
        for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
        {
            //if ( gNowMetaInfo.channel & CAN_DEMO_CHANNEL_VALUE_LIST[ ucCh ] )
            {

                if ( gDemoData[ ucCh ].control.status == CAN_DEMO_SEND_STATUS_START )
                {
                    CAN_Edu_DemoSend( ucCh );

                    if ( gDemoData[ ucCh ].control.isRepeat == TRUE )
                    {
                        hasRepeatIv = gDemoData[ ucCh ].control.repeatInterval;
                    }
                    else
                    {
                        // nitialize canData value, statusi
                        SAL_MemSet( &gDemoData[ ucCh ].value, 0, sizeof( CANDemoValue_t ) );
                        SAL_MemSet( &gDemoData[ ucCh ].control, 0, sizeof( CANDemoControl_t ) );
                    }
                }
            }
        }

        if ( hasRepeatIv )
        {
            ( void ) SAL_TaskSleep( hasRepeatIv );
        }else if(unMillstoWait > 0)
       	{
			( void ) SAL_TaskSleep( unMillstoWait );
       	}
    }
}
#endif

void CAN_DemoCreateApp
(
    void
)
{
    static uint32 uiCanDemoAppTaskID;
    static uint32 uiCanDemoAppTaskStk[CAN_DEMO_TASK_STK_SIZE];

	char* qName = "canMsgQ";

    SALRetCode_t retCode = SAL_QueueCreate(&s_ulxQueue, (uint8*)qName, 100, (SALSize)sizeof(CANDemoValue_t));
	if(retCode == SAL_RET_FAILED)
	{
	    mcu_printf( "=== s_xQueue Queue was NULL ===\n\n" );
		s_ulxQueue = 0;
	}

    ( void ) SAL_TaskCreate( &uiCanDemoAppTaskID,
                   ( const uint8 * ) "Can Demo Task",
                   ( SALTaskFunc ) &CAN_DemoTask,
                   ( uint32 * const ) &uiCanDemoAppTaskStk[0],
                   CAN_DEMO_TASK_STK_SIZE,
                   SAL_PRIO_CAN_DEMO,
                   NULL_PTR);
}

static void CAN_DemoTask
(
    void *                              pArg
)
{
    uint8 ucCh;

    ( void ) pArg;

#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
    sTestInfo.tiRecv = TRUE; // dgist
#endif

#if defined(EDUCATION_CAN_DEMO_EN)
    sTestInfo.tiRecv = TRUE;
    mcu_printf("\nCAN EDUCATION Enabled\n");
#endif

    // gDebugOption = gDebugOption | (DBG_TAG_BIT(5));

    while( 1 )
    {
        if( sTestInfo.tiRecv == TRUE )
        {
            for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
            {
#if defined(EDUCATION_CAN_DEMO_EN)
                if ( gNowMetaInfo.channel & CAN_DEMO_CHANNEL_VALUE_LIST[ ucCh ] )
#endif     
                    CAN_DemoReceive( ucCh );
            }
        }

        ( void ) SAL_TaskSleep( 10 );
    }
}

#endif //#if (ACFG_APP_CAN_DEMO_EN == 1)

