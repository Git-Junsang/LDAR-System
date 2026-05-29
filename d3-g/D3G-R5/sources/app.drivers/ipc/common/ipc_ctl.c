/*

utint32                  support;***************************************************************************************************
*
*   FileName : ipc_ctl.c
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

#include <sal_com.h>
#include <sal_internal.h>
#include <gic.h>
#include "ipc.h"
#include "ipc_typedef.h"
#include "ipc_api.h"
#include "ipc_buffer.h"
#include "ipc_os.h"
#include "mbox.h"
#include "ipc_cmd.h"
#include "ipc_ctl.h"
#include "debug.h"

IPCMboxDev_t             dev[IPC_SVC_CH_MAX]  = {0, };
static IPCMboxMsgReg_t   recv[IPC_SVC_CH_MAX] = {0, };
IPCHandler_t             gIPCHandler[IPC_SVC_CH_MAX];
static uint8             gIPCRingBuffer[IPC_SVC_CH_MAX][MAX_IPC_RING_BUFF_SIZE];
static IPCCtrTaskInfo_t  gIPCCtrTaskInfo[IPC_SVC_CH_MAX];

static int32 IPC_WriteData
(
    IPCSvcCh_t                          siCh,
    const uint32 *                      puiCmd,
    const uint8 *                       pucBuff,
    const uint32                        uiSize
) //QAC
{
    int32 ret;

    ret = IPC_ERR_COMMON;

    if((puiCmd != NULL_PTR) && (uiSize <= MAX_IPC_FIFO_SIZE))
    {
        if(puiCmd[0] == ((uint32) IPC_APP_ID_A53MR))
        {
            ret = IPC_SendWrite(siCh, puiCmd, pucBuff, uiSize, 0);
        }
        else
        {
            ret = IPC_SendWrite(siCh, puiCmd, pucBuff, uiSize, gIPCHandler[siCh].ihIpcOption.isAck);
        }

        if(ret == 0)
        {
            ret = (int32) uiSize;
        }
        else
        {
            ret = IPC_ERR_TIMEOUT;
        }
    }
    return ret;
}

int32 IPC_Write
(
    IPCSvcCh_t                          siCh,
    const uint32 *                      puiCmd,
    const uint8 *                       pucBuff,
    const uint32                        uiSize
) //QAC
{
    int32 ret;

    ret = IPC_ERR_COMMON;

    if((puiCmd != NULL_PTR) && (uiSize > 0U))
    {
        ret = IPC_WriteData(siCh, puiCmd, pucBuff, uiSize);
    }

    return ret;
}

static int32 IPC_ReadData
(
    IPCSvcCh_t                          siCh,
    uint8 *                             pucBuff,
    uint32                              uiSize,
    uint32                              uiFlag
)
{
    int32  ret;
    uint32 dataSize;
    uint32 readSize;
    uint32 isReadAll;

    ret       = 0;
    dataSize  = 0;
    readSize  = 0;
    isReadAll = 0;

    if((pucBuff != NULL_PTR) && (uiSize > 0U))
    {
        IPC_CommonLock(siCh);
        gIPCHandler[siCh].ihTime = gIPCHandler[siCh].ihSetParam.vTime;
        gIPCHandler[siCh].ihMin  = gIPCHandler[siCh].ihSetParam.vMin;
        IPC_CommonUnlock(siCh);

        if(IPC_O_BLOCK != (uiFlag & IPC_O_BLOCK))
        {
            gIPCHandler[siCh].ihIsWait = 0;
        }
        else
        {
            if((gIPCHandler[siCh].ihTime == 0U) &&(gIPCHandler[siCh].ihMin == 0U))
            {
                isReadAll                = 1;
                gIPCHandler[siCh].ihTime = MAX_READ_TIMEOUT;
                gIPCHandler[siCh].ihMin  = uiSize;
            }
            else if(gIPCHandler[siCh].ihTime == 0U)
            {
                gIPCHandler[siCh].ihTime = MAX_READ_TIMEOUT;
            }
            else if(gIPCHandler[siCh].ihMin == 0U)
            {
                gIPCHandler[siCh].ihMin = 1;
            }
            else
            {
        	IPC_DBG("%s, ipc %d read mode ihTime[%d] ihMin[%d]\n", __func__, siCh, gIPCHandler[siCh].ihTime, gIPCHandler[siCh].ihMin);
            }

            gIPCHandler[siCh].ihIsWait = 1U;
        }

        IPC_RbufferLock(siCh);
        dataSize = (uint32) IPC_BufferDataAvailable(&gIPCHandler[siCh].ihReadRingBuffer);
        IPC_RbufferUnlock(siCh);

        if(dataSize < uiSize)
        {
            if(gIPCHandler[siCh].ihIsWait == 1U)
            {
                (void) IPC_ReadWaitEventTimeOut(siCh, gIPCHandler[siCh].ihTime * 100U);

                IPC_RbufferLock(siCh);
                dataSize = (uint32) IPC_BufferDataAvailable(&gIPCHandler[siCh].ihReadRingBuffer);
                IPC_RbufferUnlock(siCh);

                if((dataSize < gIPCHandler[siCh].ihMin) && (isReadAll == 0U))
                {
                    readSize = 0;
                }
                else
                {
                    if(dataSize <= uiSize)
                    {
                        readSize = dataSize;
                    }
                    else
                    {
                        readSize = uiSize;
                    }
                }
            }
            else
            {
                readSize = dataSize;
            }
        }
        else
        {
            readSize = uiSize;
        }

        if(readSize != 0U)
        {
            IPC_RbufferLock(siCh);
            ret = IPC_PopBuffer(&gIPCHandler[siCh].ihReadRingBuffer, pucBuff, readSize);
            IPC_RbufferUnlock(siCh);
            if(ret == IPC_BUFFER_OK)
            {
                ret = (int32) readSize;
            }
            else
            {
                ret = IPC_ERR_READ;
            }
        }
    }

    if(gIPCHandler[siCh].ihIsWait == 1U)
    {
        IPC_ReadClearEvent(siCh);
    }

    return ret;
}


int32 IPC_Read
(
    IPCSvcCh_t                          siCh,
    uint8 *                             pucBuff,
    uint32                              uiSize,
    uint32                              uiFlag
)
{
    int32 ret;

    ret = IPC_ERR_COMMON;

    if((pucBuff != NULL_PTR) && (uiSize > 0U))
    {
        IPC_ReadLock(siCh);
        ret = IPC_ReadData(siCh, pucBuff, uiSize, uiFlag);
        IPC_ReadUnlock(siCh);
    }

    return ret;
}

static IPCPingError_t IPC_PingError
(
    int32 *                             siError
)
{
    IPCPingError_t ret;

    if (siError != NULL_PTR)
    {
        if(*siError == IPC_ERR_TIMEOUT)
        {
            ret = IPC_PING_ERR_RESPOND;
        }
        else if (*siError == IPC_ERR_RECEIVER_NOT_SET)
        {
            ret = IPC_PING_ERR_RECEIVER_MBOX;
        }
        else if (*siError == IPC_ERR_RECEIVER_DOWN)
        {
            ret = IPC_PING_ERR_SEND;
        }
        else if (*siError == IPC_ERR_NOTREADY)
        {
            ret = IPC_PING_ERR_NOT_READY;
        }
        else
        {
            ret = IPC_PING_ERR_SENDER_MBOX;
        }

        *siError = IPC_ERR_NOTREADY;
    }
    else
    {
        ret = MAX_IPC_PING_ERR;
        IPC_ERR("%s siError is Null\n", __func__);
    }

    return ret;
}

int32 IPC_PingTest
(
    IPCSvcCh_t                          siCh,
    IPCPingInfo_t *                     pPingInfo
)
{
    int32           ret;
    int32           startTime;
    IPCHandler_t *  pIPCHandler;

    ret         = IPC_SUCCESS;
    startTime   = 0;
    pIPCHandler = &gIPCHandler[siCh];

    if(pPingInfo != NULL_PTR)
    {
        /* Check IPC */
        if(pIPCHandler->ihIpcStatus <= IPC_INIT)
        {
            pPingInfo->piPingResult = IPC_PING_ERR_INIT;
            ret = IPC_ERR_COMMON;
        }

        if(ret == IPC_SUCCESS)
        {
            startTime = IPC_GetMsec();
            IPC_WriteLock(siCh);
            ret = IPC_SendPing(siCh);

            if(ret == IPC_SUCCESS)
            {
                pPingInfo->piPingResult = IPC_PING_SUCCESS;
            }
            else
            {
                pPingInfo->piPingResult = IPC_PingError(&ret);
            }

            IPC_WriteUnlock(siCh);
            pPingInfo->piResponseTime = ((uint32) IPC_GetMsec()) - ((uint32) startTime); //msec
        }
    }

    return ret;
}

void IPC_Flush
(
    IPCSvcCh_t                          siCh
)
{
    IPC_RbufferLock(siCh);
    IPC_BufferFlush(&gIPCHandler[siCh].ihReadRingBuffer);
    IPC_RbufferUnlock(siCh);
}

void IPC_StructInit
(
    IPCSvcCh_t                          siCh,
    IPCHandler_t *                      pIpcHandler
)
{
    (void) siCh;
    pIpcHandler->ihIpcStatus      = IPC_NULL;
    pIpcHandler->ihSetParam.vTime = 0;
    pIpcHandler->ihSetParam.vMin  = 0;
    pIpcHandler->ihTime           = 0;
    pIpcHandler->ihMin            = 0;
    pIpcHandler->ihIsWait         = 0;
}

static void IPC_SetStatus
(
    IPCSvcCh_t                          siCh,
    IPCStatus_t                         status,
    uint32                              flag
)
{
    gIPCHandler[siCh].ihIpcStatus = status;
    gIPCHandler[siCh].feature     = flag;
}

IPCStatus_t IPC_GetStatus
(
    IPCSvcCh_t                          siCh
)
{
    return gIPCHandler[siCh].ihIpcStatus;
}

static void IPC_SetBuffer
(
    IPCSvcCh_t                          siCh
)
{
    IPC_WbufferLock(siCh);
    IPC_BufferInit(&gIPCHandler[siCh].ihReadRingBuffer, (uint8*)&(gIPCRingBuffer[siCh]), MAX_IPC_RING_BUFF_SIZE);
    IPC_WbufferUnlock(siCh);
}

static void IPC_InitStack
(
    IPCSvcCh_t                          siCh
)
{
    (void) SAL_MemSet((void*) gIPCCtrTaskInfo[siCh].ipcControlTaskStk, 0x00, IPC_TASK_START_STK_SIZE);
}

static void IPC_ControlTaskDel
(
    IPCSvcCh_t                          siCh
)
{
    (void) SAL_QueueDelete(gIPCCtrTaskInfo[siCh].ipcControlQueueId);
    (void) SAL_TaskDelete(gIPCCtrTaskInfo[siCh].ipcControlTaskId);
}

static void IPC_ReceiveCtlCmd
(
    IPCSvcCh_t                          siCh,
    const IPCMboxMsgReg_t  *            pMsg
)
{
    uint32        ack;
    uint32        temp;
    uint32        seqID;
    IPCCmdId_t    cmdID;
    IPCCmdId_t    srcIden;
    IPCCmdType_t  cmdType;

    if(pMsg != NULL_PTR)
    {
        seqID   = pMsg->mmReg[0];
        temp    = pMsg->mmReg[1] & CMD_ID_MASK;
        cmdID   = (IPCCmdId_t) temp;
        temp    = (pMsg->mmReg[1] & CMD_TYPE_MASK ) >> 16U;
        cmdType = (IPCCmdType_t) temp;
        srcIden = (IPCCmdId_t) pMsg->mmReg[2];
        ack     = gIPCHandler[siCh].ihIpcOption.isAck;

        switch((IPCCmdId_t) cmdID)
        {
            case IPC_OPEN :
            {
                IPC_SetStatus(siCh, IPC_READY, (uint32) pMsg->mmReg[2]);
                if(ack == IPC_O_ACK)
                {
                    (void) IPC_SendAck(siCh, seqID, IPC_ACK, CTL_CMD, pMsg->mmReg[1], IPC_FLAG_USENAMCK);
                }

                break;
            }

            case IPC_CLOSE :
            {
                gIPCHandler[siCh].ihIpcStatus = IPC_INIT;
                (void) SAL_MemSet((uint8*)&(gIPCRingBuffer[siCh]), 0, MAX_IPC_RING_BUFF_SIZE);
                IPC_BufferOverFlowWakeUp(siCh, READ_BUF_CLOSE_BIT);
                break;
            }

            case IPC_SEND_PING :
            {
                if(ack == IPC_O_ACK)
                {
                    (void) IPC_SendAck(siCh, seqID, IPC_ACK, CTL_CMD, pMsg->mmReg[1], 0U);
                }

                break;
            }

            case IPC_ACK :
            {
                if (ack == IPC_O_ACK)
                {
                    if (srcIden == IPC_OPEN)
                    {
                        (void) IPC_SetStatus(siCh, IPC_READY, (uint32) pMsg->mmReg[2]);
                    }
                    else
                    {
                        (void) IPC_CmdWakeUp(siCh, cmdType, pMsg->mmReg[0], 0U);
                    }
                }
                break;
            }
            case IPC_NACK :
            {
                if (ack == IPC_O_ACK)
                {
                   (void) IPC_CmdWakeUp(siCh, cmdType, pMsg->mmReg[0], 1U);
                }
                break;
            }
            default :
            {
        	    IPC_DBG("%s, ch[%d] cmdID[%d]\n", __func__, siCh, cmdID);
                break;
            }
        }
    }
}

/*receive read cmd : other prosseor write my read buffer */
static void IPC_ReceiveWriteCmd
(
    IPCSvcCh_t                          siCh,
    const IPCMboxMsgReg_t  *            pMsg
)
{
    int32         ret;
    uint32        readSize;
    uint32        seqID;
    uint32        temp;
    uint32        ack;
    IPCCmdId_t    cmdID;
    IPCCmdType_t  cmdType;

    ret      = IPC_BUFFER_ERROR;
    readSize = 0U;
    seqID    = 0U;

    if(pMsg != NULL_PTR)
    {
        seqID    = pMsg->mmReg[0];
        temp     = pMsg->mmReg[1] & CMD_ID_MASK;
        cmdID    = (IPCCmdId_t) temp;
        temp     = (pMsg->mmReg[1] & CMD_TYPE_MASK ) >> 16U;
        cmdType  = (IPCCmdType_t) temp;
        readSize = pMsg->mmReg[2];
        ack      = gIPCHandler[siCh].ihIpcOption.isAck;

        switch((IPCCmdId_t) cmdID)
        {
            case IPC_ACK :
            {
                if (ack == IPC_O_ACK)
                {
                    (void) IPC_CmdWakeUp(siCh, cmdType, pMsg->mmReg[0], 0U);
                }
                break;
            }
            case IPC_NACK :
            {
                if (ack == IPC_O_ACK)
                {
                   (void) IPC_CmdWakeUp(siCh, cmdType, pMsg->mmReg[0], 1U);
                }
                break;
            }
            default :
            {
                break;
            }
        }

        if((readSize > 0U) && (readSize <= MAX_IPC_FIFO_SIZE))
        {
            IPC_RbufferLock(siCh);

            ret = IPC_BufferFreeSpace(&gIPCHandler[siCh].ihReadRingBuffer);
            if((uint32) ret > readSize)
            {
                ret = IPC_PushBuffer(&gIPCHandler[siCh].ihReadRingBuffer, (const uint8 *) pMsg->mmDat, readSize);

                if (((gIPCHandler[siCh].feature & IPC_FLAG_USENAMCK) == IPC_FLAG_USENAMCK) && (ret < 0))
                {
                    if (ret == IPC_BUFFER_FULL)
                    {
                        (void) IPC_SendAck(siCh, seqID, IPC_NACK, WRITE_CMD, pMsg->mmReg[1], IPC_NACK_REASON_BUFFFUL);
                    }
                    else
                    {
                        (void) IPC_SendAck(siCh, seqID, IPC_NACK, WRITE_CMD, pMsg->mmReg[1], IPC_NACK_REASON_BUFFERR);
                    }
                }
                else
                {
                }
            }
            else
            {
                if ((gIPCHandler[siCh].feature & IPC_FLAG_USENAMCK) == IPC_FLAG_USENAMCK)
                {
                    (void) IPC_SendAck(siCh, seqID, IPC_NACK, WRITE_CMD, pMsg->mmReg[1], IPC_NACK_REASON_BUFFFUL);
                    ret = IPC_BUFFER_FULL; 
                }
                else
                {
                    IPC_BufferOverFlowWakeUp(siCh, READ_BUF_OVERFLOW_BIT);
                    ret = IPC_PushBufferOverWrite(&gIPCHandler[siCh].ihReadRingBuffer, (const uint8 *) pMsg->mmDat, readSize);

                    if (ret < 0)
                    {
                        ret = IPC_ERR_BUFFER;
                    }
                }
            }
            IPC_RbufferUnlock(siCh);
        }

        if(ret == IPC_BUFFER_OK)
        {
            if(gIPCHandler[siCh].ihIpcOption.isAck == IPC_O_ACK)
            {
                ret = IPC_SendAck(siCh, seqID, IPC_ACK, WRITE_CMD, pMsg->mmReg[1], 0U);
                if (ret < 0)
                {
                    ret = IPC_ERR_RECEIVER_DOWN;
                }
            }

            if(gIPCHandler[siCh].ihIsWait == 1U)
            {
                if(gIPCHandler[siCh].ihMin <= (uint32) IPC_BufferDataAvailable(&gIPCHandler[siCh].ihReadRingBuffer))
                {
                    IPC_ReadWakeUp(siCh);
                }
            }
        }
    }
}

static void IPC_ControlTaskA72NonSecure
(
    void *                              p_arg
)
{
    int32         ret;
    uint32        temp;
    IPCCmdType_t  cmdType;

    (void) p_arg;
    ret  = -1;

    while(1)
    {
        (void) SAL_MemSet(recv[MBOX_CH_CA72MP_NONSECURE].mmReg, 0x00, MAX_MBOX_CMD_FIFO_CNT);
        (void) SAL_MemSet(recv[MBOX_CH_CA72MP_NONSECURE].mmDat, 0x00, MAX_MBOX_DATA_FIFO_CNT);
        ret = MBOX_DevRecv(dev[MBOX_CH_CA72MP_NONSECURE].mbDev, recv[MBOX_CH_CA72MP_NONSECURE].mmReg, recv[MBOX_CH_CA72MP_NONSECURE].mmDat);

        if(ret >= 0)
        {
            temp    = (recv[MBOX_CH_CA72MP_NONSECURE].mmReg[1] & CMD_TYPE_MASK ) >> 16U;
            cmdType = (IPCCmdType_t) temp;

            switch((IPCCmdType_t) cmdType)
            {
                case CTL_CMD :
                {
                    IPC_ReceiveCtlCmd(IPC_SVC_CH_CA72_NONSECURE, (const IPCMboxMsgReg_t *) &recv[MBOX_CH_CA72MP_NONSECURE]);
                    break;
                }

                case WRITE_CMD :
                {
                    IPC_ReceiveWriteCmd(IPC_SVC_CH_CA72_NONSECURE, (const IPCMboxMsgReg_t *) &recv[MBOX_CH_CA72MP_NONSECURE]);
                    break;
                }

                default:
                {
                    IPC_DBG("%s, cmdType %d\n", __func__, cmdType);
                    break;
                }
            }
        }
    }
}

static void IPC_ControlTaskA53NonSecure
(
    void *                              p_arg
)
{
    int32         ret;
    uint32        temp;
    IPCCmdType_t  cmdType;

    (void) p_arg;
    ret = -1;

    while(1)
    {
        (void) SAL_MemSet(recv[MBOX_CH_CA53MP_NONSECURE].mmReg, 0x00, MAX_MBOX_CMD_FIFO_CNT);
        (void) SAL_MemSet(recv[MBOX_CH_CA53MP_NONSECURE].mmDat, 0x00, MAX_MBOX_DATA_FIFO_CNT);
        ret = MBOX_DevRecv(dev[MBOX_CH_CA53MP_NONSECURE].mbDev, recv[MBOX_CH_CA53MP_NONSECURE].mmReg, recv[MBOX_CH_CA53MP_NONSECURE].mmDat);

        if(ret >= 0)
        {
            temp    = (recv[MBOX_CH_CA53MP_NONSECURE].mmReg[1] & CMD_TYPE_MASK ) >> 16U;
            cmdType = (IPCCmdType_t) temp;

            switch((IPCCmdType_t) cmdType)
            {
                case CTL_CMD :
                {
                    IPC_ReceiveCtlCmd(IPC_SVC_CH_CA53_NONSECURE, (const IPCMboxMsgReg_t *) &recv[MBOX_CH_CA53MP_NONSECURE]);
                    break;
                }

                case WRITE_CMD :
                {
                    IPC_ReceiveWriteCmd(IPC_SVC_CH_CA53_NONSECURE, (const IPCMboxMsgReg_t *) &recv[MBOX_CH_CA53MP_NONSECURE]);
                    break;
                }

                default :
                {
                    IPC_DBG("%s, cmdType %d\n", __func__, cmdType);
                    break;
                }
            }
        }
    }
}

static int32 IPC_ControlTaskCreate
(
    IPCSvcCh_t                          siCh
)
{
    int32 ret;

    ret = IPC_SUCCESS;
    IPC_InitStack(siCh);

    if(siCh == IPC_SVC_CH_CA53_NONSECURE)
    {
        ret = (int32) SAL_TaskCreate
        (
            &(gIPCCtrTaskInfo[siCh].ipcControlTaskId),
            (const uint8 *) "CA53 control task",
            (SALTaskFunc) &IPC_ControlTaskA72NonSecure,
            (uint32 * const)&(gIPCCtrTaskInfo[siCh].ipcControlTaskStk[0]),
            IPC_TASK_START_STK_SIZE, SAL_PRIO_IPC_CONTROL, NULL_PTR
        );
    }

    if(siCh == IPC_SVC_CH_CA72_NONSECURE)
    {
        ret = (int32) SAL_TaskCreate
        (
            &(gIPCCtrTaskInfo[siCh].ipcControlTaskId),
            (const uint8 *) "CA7 control task",
            (SALTaskFunc) &IPC_ControlTaskA53NonSecure,
            (uint32 * const) &(gIPCCtrTaskInfo[siCh].ipcControlTaskStk[0]),
            IPC_TASK_START_STK_SIZE,
            SAL_PRIO_IPC_CONTROL, NULL_PTR
        );
    }

    if(ret != ((int32) SAL_RET_SUCCESS))
    {
        ret = IPC_ERR_COMMON;
    }

    return ret;

}

int32 IPC_Initialize
(
    IPCSvcCh_t                          siCh
)
{
    int32 ret;
    int8  devName[7];

    ret        = IPC_SUCCESS;
    devName[0] = 'I';
    devName[1] = 'P';
    devName[2] = 'C';
    devName[3] = '\0';
    devName[4] = '\0';
    devName[5] = '\0';
    devName[6] = '\0';

    gIPCHandler[siCh].ihIpcStatus = IPC_INIT;
    dev[siCh].mbDev = MBOX_DevOpen((MBOXCh_t) siCh, devName, 0);

    IPC_OsResourceInit(siCh);
    IPC_SetBuffer(siCh);

    ret = IPC_ControlTaskCreate(siCh);

    if(ret == IPC_SUCCESS)
    {
        IPC_DBG("%s, ipc %d initialized\n", __func__, siCh);
        (void) IPC_SendOpen(siCh);
        gIPCHandler[siCh].ihIpcStatus = IPC_OPENED;
    }

    return ret;
}

void IPC_Release
(
    IPCSvcCh_t                          siCh
)
{
    IPC_ReadWakeUp(siCh);           /* wake up pending thread */

    (void) IPC_SendClose(siCh);
    IPC_OsResourceRelease(siCh);
    IPC_ControlTaskDel(siCh);
}

