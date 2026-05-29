/*
***************************************************************************************************
*
*   FileName : ipc_cmd.c
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
#include <gic.h>
#include <sal_internal.h>
#include "ipc_typedef.h"
#include "ipc.h"
#include "ipc_api.h"
#include "ipc_os.h"
#include "mbox.h"
#include "ipc_cmd.h"

#define OPEN_MBOX_TIMEOUT               (5)   //ms
#define MBOX_TIMEOUT                    (100) //ms
#define ACK_TIMEOUT                     (500) //ms

static IPCMboxMsgReg_t sendMsg[IPC_SVC_CH_MAX];

static uint32 IPC_GetSequentialId
(
    IPCSvcCh_t                          siCh
)
{
    static uint32 sendSeqId[IPC_SVC_CH_MAX] = {0, };

    IPC_CommonLock(siCh);
    sendSeqId[siCh]++;

    if(sendSeqId[siCh] == 0xFFFFFFFFU)
    {
        sendSeqId[siCh] = 1;
    }

    IPC_CommonUnlock(siCh);

    return sendSeqId[siCh];
}

int32 IPC_SendOpen
(
    IPCSvcCh_t                          siCh
)
{
    int32  ret;
    uint32 oppinit;

    sendMsg[siCh].mmReg[0] = IPC_GetSequentialId(siCh);
    sendMsg[siCh].mmReg[1] = (((uint32) CTL_CMD) << 16U) | ((uint32) IPC_OPEN);
    sendMsg[siCh].mmReg[2] = IPC_FLAG_USENAMCK;
    sendMsg[siCh].mmReg[3] = 0U;
    sendMsg[siCh].mmReg[4] = 0U;

    ret = MBOX_DevIoctl(dev[siCh].mbDev, MBOX_IO_CMD_GET_OPP_STS, &oppinit);

    if (ret == MBOX_SUCCESS)
    {
        if (oppinit == 0x01U)
        {
            ret = MBOX_DevSendCmd(dev[siCh].mbDev, sendMsg[siCh].mmReg);
        }
        else
        {
            ret = MBOX_ERR_OPP_INIT;
        }
    }
    else
    {
        ret = MBOX_ERR_IOCTL;
    }

    return ret;
}

int32 IPC_SendClose
(
    IPCSvcCh_t                          siCh
)
{
    int32 ret;

    sendMsg[siCh].mmReg[0] = IPC_GetSequentialId(siCh);
    sendMsg[siCh].mmReg[1] = ((uint32) CTL_CMD<< 16U) | ((uint32) IPC_CLOSE);
    sendMsg[siCh].mmReg[2] = 0;
    sendMsg[siCh].mmReg[3] = 0;
    sendMsg[siCh].mmReg[4] = 0;

    ret = MBOX_DevSendCmd(dev[siCh].mbDev, sendMsg[siCh].mmReg);

    return ret;
}

int32 IPC_SendWrite
(
    IPCSvcCh_t                          siCh,
    const uint32 *                      puiCmd,
    const uint8 *                       pucBuff,
    const uint32                        uiSize,
    uint32                              uiIsAck
) //QAC
{
    int32  ret;
    uint32 len;

    sendMsg[siCh].mmReg[0] = IPC_GetSequentialId(siCh);
    sendMsg[siCh].mmReg[1] = *(puiCmd+1);
    sendMsg[siCh].mmReg[2] = *(puiCmd+2);
    sendMsg[siCh].mmReg[3] = *(puiCmd+3);
    sendMsg[siCh].mmReg[4] = *(puiCmd+4);
    sendMsg[siCh].mmReg[5] = *(puiCmd+5);

    if(uiIsAck == IPC_O_ACK)
    {
        IPC_CmdWakePreSet(siCh, WRITE_CMD, sendMsg[siCh].mmReg[0]);
    }

    len = ((uiSize % 4U) == 0U) ? (uiSize / 4U) : ((uiSize / 4U) + 1U);

    if ((len > 0U) && (len < MAX_MBOX_DATA_FIFO_CNT))
    {
        (void) SAL_MemCopy(sendMsg[siCh].mmDat, pucBuff, uiSize);
        ret = MBOX_DevSend(dev[siCh].mbDev, sendMsg[siCh].mmReg, sendMsg[siCh].mmDat, len);

        if((uiIsAck == IPC_O_ACK) && (ret == IPC_SUCCESS))
        {
            ret = IPC_CmdWaitEventTimeOut(siCh, WRITE_CMD,ACK_TIMEOUT);
        }
    }
    else
    {
        ret = IPC_ERR_INVALID_DL;
    }

    return ret;
}

int32 IPC_SendPing
(
    IPCSvcCh_t                          siCh
)
{
    int32 ret;

    sendMsg[siCh].mmReg[0] = IPC_GetSequentialId(siCh);
    sendMsg[siCh].mmReg[1] = ((uint32) CTL_CMD << 16U) | ((uint32) IPC_SEND_PING);
    sendMsg[siCh].mmReg[2] = 0;
    sendMsg[siCh].mmReg[3] = 0;
    sendMsg[siCh].mmReg[4] = 0;

    IPC_CmdWakePreSet(siCh, CTL_CMD, sendMsg[siCh].mmReg[0]);
    ret = MBOX_DevSendCmd(dev[siCh].mbDev, sendMsg[siCh].mmReg);

    if(ret == IPC_SUCCESS)
    {
        ret = IPC_CmdWaitEventTimeOut(siCh, CTL_CMD, ACK_TIMEOUT);

    }

    return ret;
}

int32 IPC_SendAck
(
    IPCSvcCh_t                          siCh,
    uint32                              uiSeqID,
    uint32                              uiCmdID,
    IPCCmdType_t                        cmdType,
    uint32                              uiSourcCmd,
    uint32                              feature
)
{
    int32 ret;

    sendMsg[siCh].mmReg[0] = uiSeqID;
    sendMsg[siCh].mmReg[1] = ((uint32) cmdType << 16U) | ((uint32) uiCmdID);
    sendMsg[siCh].mmReg[2] = uiSourcCmd;
    sendMsg[siCh].mmReg[3] = feature;
    sendMsg[siCh].mmReg[4] = 0;
    sendMsg[siCh].mmReg[5] = 0;

    ret = MBOX_DevSendCmd(dev[siCh].mbDev, sendMsg[siCh].mmReg);

    return ret;
}

