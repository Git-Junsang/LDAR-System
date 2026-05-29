/*
***************************************************************************************************
*
*   FileName : ipc_os.h
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

#ifndef IPC_OS_H
#define IPC_OS_H

#include <gic.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define TICK_PER_MS                     (1)

#define READ_BUF_CHECK_FLAGS            (0x0000000FUL)
#define READ_BUF_OVERFLOW_BIT           (0x00000001UL)
#define READ_BUF_CLOSE_BIT              (0x00000002UL)
#define READ_BUF_NO_INIT_BIT            (0x00000004UL)
#define READ_BUF_TIME_OUT_BIT           (0x00000008UL)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
int32 IPC_GetUsec
(
    void
);

int32 IPC_GetMsec
(
    void
);

void IPC_Mdelay
(
    uint32                              uiDly
);

void IPC_Udelay
(
    uint32                              uiDly
);

void IPC_CommonLock
(
    IPCSvcCh_t                          siCh
);

void IPC_CommonUnlock
(
    IPCSvcCh_t                          siCh
);

void IPC_MboxLock
(
    IPCSvcCh_t                          siCh
);

void IPC_MboxUnlock
(
    IPCSvcCh_t                          siCh
);

void IPC_WriteLock
(
    IPCSvcCh_t                          siCh
);

void IPC_WriteUnlock
(
    IPCSvcCh_t                          siCh
);

void IPC_ReadLock
(
    IPCSvcCh_t                          siCh
);

void IPC_ReadUnlock
(
    IPCSvcCh_t                          siCh
);

void IPC_RbufferLock
(
    IPCSvcCh_t                          siCh
);

void IPC_RbufferUnlock
(
    IPCSvcCh_t                          siCh
);

void IPC_WbufferLock
(
    IPCSvcCh_t                          siCh
);

void IPC_WbufferUnlock
(
    IPCSvcCh_t                          siCh
);

int32 IPC_CmdWaitEventTimeOut
(
    IPCSvcCh_t                          siCh,
    IPCCmdType_t                        cmdType,
    uint32                              uiTimeOut
);

void IPC_CmdWakePreSet
(
    IPCSvcCh_t                          siCh,
    IPCCmdType_t                        cmdType,
    uint32                              uiSeqID
);

void IPC_CmdWakeUp
(
    IPCSvcCh_t                          siCh,
    IPCCmdType_t                        cmdType,
    uint32                              uiSeqID,
    uint32                              uiNack
);

int32 IPC_ReadWaitEventTimeOut
(
    IPCSvcCh_t                          siCh,
    uint32                              uiTimeOut
);

void IPC_ReadClearEvent
(
    IPCSvcCh_t                          siCh
);

void IPC_ReadWakeUp
(
    IPCSvcCh_t                          siCh
);

int32 IPC_BufferOverFlowWaitEventTimeOut
(
    IPCSvcCh_t                          siCh,
    uint32                              uiTimeOut
);

void IPC_BufferOverFlowWakeUp
(
    IPCSvcCh_t                          siCh,
    uint32                              uiFlag
);

int32 IPC_RequestIrq
(
    uint32                              uiIrq,
    GICIsrFunc                          func
);

int32 IPC_FreeIrq
(
    uint32                              uiIrq
);

void IPC_OsResourceInit
(
    IPCSvcCh_t                          siCh
);

void IPC_OsResourceRelease
(
    IPCSvcCh_t                          siCh
);

#endif /* _IPC_OS_H_ */

