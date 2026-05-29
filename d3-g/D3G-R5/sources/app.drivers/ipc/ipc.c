/*
***************************************************************************************************
*
*   FileName : ipc.c
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

#include <app_cfg.h>
#include <sal_com.h>
#include <sal_internal.h>

#include "ipc.h"
#include "ipc_typedef.h"
#include "ipc_api.h"
#include "debug.h"

void IPC_Create
(
    void
)
{
    IPC_CreateIPCParserTask();
}

void IPC_RegisterCbFunc
(
    IPCSvcCh_t                          siCh,
    uint8                               ucId,
    IPCCallback                         pFunc,
    void *                              pArg1,
    void *                              pArg2
)
{
    cbFunc[siCh][ucId].ipcCbFunc = pFunc;
    cbFunc[siCh][ucId].ipcArg1   = pArg1;
    cbFunc[siCh][ucId].ipcArg2   = pArg2;
}

int32 IPC_SendPacket
(
    IPCSvcCh_t                          siCh,
    uint16                              uiCmd1,
    uint16                              uiCmd2,
    const uint8 *                       pucData,
    uint16                              uiLength
)
{
    int32 ret;

    if (uiLength >= (((uint16) IPC_MAX_PACKET_SIZE) - ((uint16) IPC_PACKET_PREFIX_SIZE)))
    {
        ret = IPC_ERR_ARGUMENT;
    }
    else
    {
        ret = IPC_SendPacketWithIPCHeader(siCh, uiCmd1, uiCmd2, pucData, uiLength);
    }

    return ret;
}

int32 IPC_IsReady
(
    uint32                              uiCh,
    uint32 *                            uiIpcReady
)
{
    return IPC_Ioctl(uiCh, (uint32)IOCTL_IPC_ISREADY, (void *) uiIpcReady, NULL_PTR, NULL_PTR, NULL_PTR);
}

