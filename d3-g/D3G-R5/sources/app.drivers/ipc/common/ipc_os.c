/*
***************************************************************************************************
*
*   FileName : ipc_os.c
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
*   merchantability, fitness for a particULar purpose or non-infringement of any patent, copyright
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
*   (of merchantability, fitness for a particULar purpose or non-infringement of any patent,
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
#include "ipc_typedef.h"
#include "ipc.h"
#include "ipc_api.h"
#include "ipc_os.h"

#define CTL_CMD_FLAG                    (0x00000001UL)
#define WRITE_CMD_FLAG                  (0x00000002UL)
#define READ_WAIT_FLAG                  (0x00000001UL)

typedef struct IPCAckInfo
{
    uint32                              aiSeqId;
    uint32                              aiNack;
} IPCAckInfo_t;

typedef struct IPCAckFlags
{
    uint32                              afEventFlagId;
    IPCAckInfo_t                        afWaitAck[MAX_CMD_TYPE];
} IPCAckFlags_t;

typedef struct IPCOsFlags
{
    IPCAckFlags_t                       ipcAckFlags;
    uint32                              ipcReadWaitFlagId;
    uint32                              ipcBufOverflowFlagId;
    uint32                              ipcCommonSemaId;
    uint32                              ipcMboxSemaId;
    uint32                              ipcReadSemaId;
    uint32                              ipcWriteSemaId;
    uint32                              ipcRbufSemaId;
    uint32                              ipcWbufSemaId;
} IPCOsFlags_t;

static IPCOsFlags_t ipcOSFlags[IPC_SVC_CH_MAX];

int32 IPC_GetUsec
(
    void
)
{
    /* uc-os is not supported usec */
    return 0;
}

int32 IPC_GetMsec
(
    void
)
{
    uint32 ret;

    (void) SAL_GetTickCount(&ret);

    return ((int32) ret) * TICK_PER_MS;
}

void IPC_Mdelay
(
    uint32                              uiDly
)
{
    (void) SAL_TaskSleep(uiDly);
}

void IPC_Udelay
(
    uint32                              uiDly
)
{
    (void) uiDly;
    /* to do*/
}

static void IPC_CommonLockInit
(
    IPCSvcCh_t                          siCh
)
{
    if(siCh == IPC_SVC_CH_CA53_NONSECURE)
    {
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcCommonSemaId), (const uint8 *) "CA53 Common Mutex", 1UL, SAL_OPT_BLOCKING);
    }

    if(siCh == IPC_SVC_CH_CA72_NONSECURE)
    {
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcCommonSemaId), (const uint8 *) "CA7 Common Mutex", 1UL, SAL_OPT_BLOCKING);
    }
}

void IPC_CommonLock
(
    IPCSvcCh_t                          siCh
)
{
    /*Acquire Mutex*/
    (void) SAL_SemaphoreWait(ipcOSFlags[siCh].ipcCommonSemaId, 0, SAL_OPT_BLOCKING);
}

void IPC_CommonUnlock
(
    IPCSvcCh_t                          siCh
)
{
    /*Release Mutex*/
    (void) SAL_SemaphoreRelease(ipcOSFlags[siCh].ipcCommonSemaId);
}

static void IPC_CommonLockDelete
(
    IPCSvcCh_t                          siCh
)
{
    (void)SAL_SemaphoreDelete(ipcOSFlags[siCh].ipcCommonSemaId);
}

static void IPC_CreateMutex
(
    IPCSvcCh_t                          siCh
)
{
    if(siCh == IPC_SVC_CH_CA53_NONSECURE)
    {
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcMboxSemaId),  (const uint8 *) "CA53 Mbox Mutex",         1UL, SAL_OPT_BLOCKING);
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcReadSemaId),  (const uint8 *) "CA53 read func Mutex",    1UL, SAL_OPT_BLOCKING);
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcWriteSemaId), (const uint8 *) "CA53 write func Mutex",   1UL, SAL_OPT_BLOCKING);
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcRbufSemaId),  (const uint8 *) "CA53 read buffer Mutex",  1UL, SAL_OPT_BLOCKING);
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcWbufSemaId),  (const uint8 *) "CA53 write buffer Mutex", 1UL, SAL_OPT_BLOCKING);
    }

    if(siCh == IPC_SVC_CH_CA72_NONSECURE)
    {
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcMboxSemaId),  (const uint8 *) "CA7 Mbox Mutex",          1UL, SAL_OPT_BLOCKING);
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcReadSemaId),  (const uint8 *) "CA7 read func Mutex",     1UL, SAL_OPT_BLOCKING);
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcWriteSemaId), (const uint8 *) "CA7 write func Mutex",    1UL, SAL_OPT_BLOCKING);
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcRbufSemaId),  (const uint8 *) "CA7 read buffer Mutex",   1UL, SAL_OPT_BLOCKING);
        (void) SAL_SemaphoreCreate((uint32 *) &(ipcOSFlags[siCh].ipcWbufSemaId),  (const uint8 *) "CA7 write buffer  Mutex", 1UL, SAL_OPT_BLOCKING);
    }
}

static void IPC_DeleteMutex
(
    IPCSvcCh_t                          siCh
)
{
    (void) SAL_SemaphoreDelete(ipcOSFlags[siCh].ipcMboxSemaId);
    (void) SAL_SemaphoreDelete(ipcOSFlags[siCh].ipcReadSemaId);
    (void) SAL_SemaphoreDelete(ipcOSFlags[siCh].ipcWriteSemaId);
    (void) SAL_SemaphoreDelete(ipcOSFlags[siCh].ipcRbufSemaId);
    (void) SAL_SemaphoreDelete(ipcOSFlags[siCh].ipcWbufSemaId);
}

void IPC_MboxLock
(
    IPCSvcCh_t                          siCh
)
{
    /*Acquire Mutex*/
    (void) SAL_SemaphoreWait(ipcOSFlags[siCh].ipcMboxSemaId, 0, SAL_OPT_BLOCKING);
}

void IPC_MboxUnlock
(
    IPCSvcCh_t                          siCh
)
{
    /*Release Mutex*/
    (void) SAL_SemaphoreRelease(ipcOSFlags[siCh].ipcMboxSemaId);
}

void IPC_WriteLock
(
    IPCSvcCh_t                          siCh
)
{
    /*Acquire Mutex*/
    (void) SAL_SemaphoreWait(ipcOSFlags[siCh].ipcWriteSemaId, 0, SAL_OPT_BLOCKING);
}

void IPC_WriteUnlock
(
    IPCSvcCh_t                          siCh
)
{
    /*Release Mutex*/
    (void) SAL_SemaphoreRelease(ipcOSFlags[siCh].ipcWriteSemaId);
}

void IPC_ReadLock
(
    IPCSvcCh_t                          siCh
)
{
    /*Acquire Mutex*/
    (void) SAL_SemaphoreWait(ipcOSFlags[siCh].ipcReadSemaId, 0, SAL_OPT_BLOCKING);
}

void IPC_ReadUnlock
(
    IPCSvcCh_t                          siCh
)
{
    /*Release Mutex*/
    (void) SAL_SemaphoreRelease(ipcOSFlags[siCh].ipcReadSemaId);
}

void IPC_RbufferLock
(
    IPCSvcCh_t                          siCh
)
{
    /*Acquire Mutex*/
    (void) SAL_SemaphoreWait(ipcOSFlags[siCh].ipcRbufSemaId, 0, SAL_OPT_BLOCKING);
}

void IPC_RbufferUnlock
(
    IPCSvcCh_t                          siCh
)
{
    /*Release Mutex*/
    (void) SAL_SemaphoreRelease(ipcOSFlags[siCh].ipcRbufSemaId);
}

void IPC_WbufferLock
(
    IPCSvcCh_t                          siCh
)
{
    /*Acquire Mutex*/
    (void) SAL_SemaphoreWait(ipcOSFlags[siCh].ipcWbufSemaId, 0, SAL_OPT_BLOCKING);
}

void IPC_WbufferUnlock
(
    IPCSvcCh_t                          siCh
)
{
    /*Release Mutex*/
    (void) SAL_SemaphoreRelease(ipcOSFlags[siCh].ipcWbufSemaId);
}

static void IPC_CmdEventCreate
(
    IPCSvcCh_t                          siCh
)
{
    uint32 cmdId;

    if(siCh == IPC_SVC_CH_CA53_NONSECURE)
    {
        (void) SAL_EventCreate((uint32 *)&(ipcOSFlags[siCh].ipcAckFlags.afEventFlagId), (const uint8 *)"CA53 command ack flag group", 0);
    }

    if(siCh == IPC_SVC_CH_CA72_NONSECURE)
    {
        (void) SAL_EventCreate((uint32 *)&(ipcOSFlags[siCh].ipcAckFlags.afEventFlagId), (const uint8 *)"CA7 command ack flag group", 0);
    }

    for(cmdId = (uint32) CTL_CMD ; cmdId < (uint32) MAX_CMD_TYPE ; cmdId++)
    {
        ipcOSFlags[siCh].ipcAckFlags.afWaitAck[cmdId].aiSeqId = 0xFFFFFFFFU;
    }
}

static void IPC_CmdEventDelete
(
    IPCSvcCh_t                          siCh
)
{
    (void) SAL_EventDelete(ipcOSFlags[siCh].ipcAckFlags.afEventFlagId);
}

int32 IPC_CmdWaitEventTimeOut
(
    IPCSvcCh_t                          siCh,
    IPCCmdType_t                        cmdType,
    uint32                              uiTimeOut
)
{
    int32        ret;
    uint32       retFlag;
    uint32       waitFlag;
    SALRetCode_t err;

    retFlag = 0UL;

    if ((siCh < IPC_SVC_CH_MAX) && (cmdType < MAX_CMD_TYPE)) //QAC : Dereference of an invalid pointer value
    {
        switch(cmdType)
        {
            case CTL_CMD :
            {
                waitFlag = CTL_CMD_FLAG;
                break;
            }

            case WRITE_CMD :
            {
                waitFlag = WRITE_CMD_FLAG;
                break;
            }

            default :
            {
                waitFlag = (uint32) 0x00;
                break;
            }
        }

        err = SAL_EventGet
        (
            ipcOSFlags[siCh].ipcAckFlags.afEventFlagId,
            waitFlag,
            uiTimeOut,
            ((uint32) SAL_EVENT_OPT_SET_ANY | (uint32) SAL_EVENT_OPT_CONSUME | (uint32) SAL_OPT_BLOCKING),
            &retFlag
        );

        if((err == SAL_RET_SUCCESS))
        {
            if (retFlag == 0U)
            {
                ret = IPC_ERR_TIMEOUT;  /* time out */
            }
            else
            {
                if (ipcOSFlags[siCh].ipcAckFlags.afWaitAck[cmdType].aiNack == 0U)
                {
                    ret = IPC_SUCCESS;
                }
                else
                {
                    ret = IPC_ERR_RECV_NACK; 
                }
            }
        }
        else
        {
            ret = IPC_ERR_COMMON;
        }
    }
    else
    {
        ret = IPC_ERR_ARGUMENT;
    }

    return ret;
}

void IPC_CmdWakePreSet
(
    IPCSvcCh_t                          siCh,
    IPCCmdType_t                        cmdType,
    uint32                              uiSeqID
)
{
    uint32 waitFlag;

    if ((siCh < IPC_SVC_CH_MAX) && (cmdType < MAX_CMD_TYPE)) //QAC : Dereference of an invalid pointer value
    {
        switch(cmdType)
        {
            case CTL_CMD :
            {
                waitFlag = CTL_CMD_FLAG;
                break;
            }

            case WRITE_CMD :
            {
                waitFlag = WRITE_CMD_FLAG;
                break;
            }

            default :
            {
                waitFlag = (uint32) 0x00;
                break;
            }
        }

        (void)SAL_EventSet(ipcOSFlags[siCh].ipcAckFlags.afEventFlagId, waitFlag, SAL_EVENT_OPT_CLR_ALL); // checking point the meaning of SAL_EVENT_OPT_CLR_ALL
        ipcOSFlags[siCh].ipcAckFlags.afWaitAck[cmdType].aiSeqId = uiSeqID;
        ipcOSFlags[siCh].ipcAckFlags.afWaitAck[cmdType].aiNack = 0U;
    }
}

void IPC_CmdWakeUp
(
    IPCSvcCh_t                          siCh,
    IPCCmdType_t                        cmdType,
    uint32                              uiSeqID,
    uint32                              uiNack
)
{
    uint32 waitFlag;

    if ((siCh < IPC_SVC_CH_MAX) && (cmdType < MAX_CMD_TYPE)) //QAC : Dereference of an invalid pointer value
    {
        if(ipcOSFlags[siCh].ipcAckFlags.afWaitAck[cmdType].aiSeqId == uiSeqID)
        {
            switch(cmdType)
            {
                case CTL_CMD :
                {
                    waitFlag = CTL_CMD_FLAG;
                    break;
                }

                case WRITE_CMD :
                {
                    waitFlag = WRITE_CMD_FLAG;
                    break;
                }

                default :
                {
                    waitFlag = (uint32) 0x00;
                    break;
                }
            }

            ipcOSFlags[siCh].ipcAckFlags.afWaitAck[cmdType].aiNack = uiNack;
            (void) SAL_EventSet(ipcOSFlags[siCh].ipcAckFlags.afEventFlagId, waitFlag, SAL_EVENT_OPT_FLAG_SET);
        }
        else
        {
            ;/* Receive unknown ACK */
        }
    }

}

static void IPC_ReadEventCreate
(
    IPCSvcCh_t                          siCh
)
{
    if(siCh == IPC_SVC_CH_CA53_NONSECURE)
    {
        (void) SAL_EventCreate((uint32 *)&(ipcOSFlags[siCh].ipcReadWaitFlagId), (const uint8 *) "CA53 read wait flag group", 0);
    }

    if(siCh == IPC_SVC_CH_CA72_NONSECURE)
    {
        (void) SAL_EventCreate((uint32 *)&(ipcOSFlags[siCh].ipcReadWaitFlagId), (const uint8 *) "CA7 read wait flag group", 0);
    }
}

static void IPC_ReadEventDelete
(
    IPCSvcCh_t                          siCh
)
{
    (void) SAL_EventDelete(ipcOSFlags[siCh].ipcReadWaitFlagId);
}

int32 IPC_ReadWaitEventTimeOut
(
    IPCSvcCh_t                          siCh,
    uint32                              uiTimeOut
)
{
    int32        ret;
    SALRetCode_t err;
    uint32       retFlag;

    retFlag = 0UL;

    err = SAL_EventGet(ipcOSFlags[siCh].ipcReadWaitFlagId, READ_WAIT_FLAG, uiTimeOut, ((uint32) SAL_EVENT_OPT_SET_ANY| (uint32) SAL_OPT_BLOCKING), &retFlag);

    if((err == SAL_RET_SUCCESS))
    {
        if (retFlag == 0U)
        {
            ret = IPC_ERR_TIMEOUT;  /* time out */
        }
        else
        {
            ret = IPC_SUCCESS;
        }
    }
    else
    {
        ret = IPC_ERR_COMMON;
    }

    return ret;
}

void IPC_ReadClearEvent
(
    IPCSvcCh_t                          siCh
)
{
    (void) SAL_EventSet(ipcOSFlags[siCh].ipcReadWaitFlagId, READ_WAIT_FLAG, SAL_EVENT_OPT_CLR_ALL);
}

void IPC_ReadWakeUp
(
    IPCSvcCh_t                          siCh
)
{
    (void) SAL_EventSet(ipcOSFlags[siCh].ipcReadWaitFlagId, READ_WAIT_FLAG, SAL_EVENT_OPT_FLAG_SET);
}

static void IPC_BufferOverFlowEventCreate
(
    IPCSvcCh_t                          siCh
)
{
    if(siCh == IPC_SVC_CH_CA53_NONSECURE)
    {
        (void) SAL_EventCreate((uint32 *)&(ipcOSFlags[siCh].ipcBufOverflowFlagId), (const uint8 *) "CA53 buffer overflow flag group", 0);
    }

    if(siCh == IPC_SVC_CH_CA72_NONSECURE)
    {
        (void) SAL_EventCreate((uint32 *)&(ipcOSFlags[siCh].ipcBufOverflowFlagId), (const uint8 *) "CA7 buffer overflow flag group", 0);
    }
}

static void IPC_BufferOverFlowEventDelete
(
    IPCSvcCh_t                          siCh
)
{
    (void)SAL_EventDelete(ipcOSFlags[siCh].ipcBufOverflowFlagId);
}

int32 IPC_BufferOverFlowWaitEventTimeOut
(
    IPCSvcCh_t                          siCh,
    uint32                              uiTimeOut
)
{
    int32        ret;
    SALRetCode_t err;
    uint32       retFlag;

    ret = 0;
    retFlag = 0UL;

    (void) SAL_EventSet(ipcOSFlags[siCh].ipcBufOverflowFlagId, READ_BUF_CHECK_FLAGS, SAL_EVENT_OPT_CLR_ALL);

    err = SAL_EventGet(ipcOSFlags[siCh].ipcBufOverflowFlagId, READ_BUF_CHECK_FLAGS, uiTimeOut,
                      ((uint32) SAL_EVENT_OPT_SET_ANY| (uint32) SAL_EVENT_OPT_CONSUME| (uint32) SAL_OPT_BLOCKING), &retFlag);

    if((err == SAL_RET_SUCCESS))
    {
        if (retFlag == 0U)
        {
            ret = IPC_ERR_TIMEOUT;  /* time out */
        }
        else
        {
            ret = (int32) retFlag;
        }
    }
    else
    {
        ret = IPC_ERR_COMMON;
    }

    return ret;
}

void IPC_BufferOverFlowWakeUp
(
    IPCSvcCh_t                          siCh,
    uint32                              uiFlag
)
{
    (void) SAL_EventSet(ipcOSFlags[siCh].ipcBufOverflowFlagId, uiFlag, SAL_EVENT_OPT_FLAG_SET);
}

int32 IPC_RequestIrq
(
    uint32                              uiIrq,
    GICIsrFunc                          func
)
{
    /* register mbox rx interrupt handler   */

    (void) GIC_IntVectSet(uiIrq, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_LOW, func, NULL_PTR);
    (void) GIC_IntSrcEn(uiIrq);

    return 0;
}

int32 IPC_FreeIrq
(
    uint32                              uiIrq
)
{
    (void) GIC_IntSrcDis(uiIrq);

    return  0;
}

void IPC_OsResourceInit
(
    IPCSvcCh_t                          siCh
)
{
    IPC_CommonLockInit(siCh);
    IPC_CreateMutex(siCh);
    IPC_CmdEventCreate(siCh);
    IPC_ReadEventCreate(siCh);
    IPC_BufferOverFlowEventCreate(siCh);
}

void IPC_OsResourceRelease
(
    IPCSvcCh_t                          siCh
)
{
    IPC_CommonLockDelete(siCh);
    IPC_DeleteMutex(siCh);
    IPC_CmdEventDelete(siCh);
    IPC_ReadEventDelete(siCh);
    IPC_BufferOverFlowWakeUp(siCh, READ_BUF_CLOSE_BIT);
    IPC_BufferOverFlowEventDelete(siCh);
}

