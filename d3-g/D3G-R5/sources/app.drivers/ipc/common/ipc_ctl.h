/*
***************************************************************************************************
*
*   FileName : ipc_ctl.h
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

#ifndef IPC_CTL_H
#define IPC_CTL_H

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define MAX_READ_TIMEOUT                (50)      //ms
#define PING_DATA_SIZE                  (64)
#define PING_RESPONSE_SIGN              (0x5A)
#define MAX_IPC_RING_BUFF_SIZE          (2048)
#define MAX_IPC_FIFO_SIZE               (512U)
#define IPC_TASK_START_STK_SIZE         (256U)

typedef enum IPCStatus
{
    IPC_NULL                            = 0,
    IPC_INIT,
    IPC_OPENED,
    IPC_READY,                                 /*Buffer setting completed*/
    IPC_MAX_STATUS,
} IPCStatus_t;

typedef struct IPCHandler
{
    IPCStatus_t                         ihIpcStatus;
    IPCRingBuff_t                       ihReadRingBuffer;
    uint32                              ihIsWait;
    uint32                              ihTime;
    uint32                              ihMin;
    IPCCtlParam_t                       ihSetParam;
    IPCOption_t                         ihIpcOption;
    uint32                              feature;
} IPCHandler_t;

typedef struct IPCCtrTaskInfo
{
    uint32                              ipcControlQueueId;
    uint32                              ipcControlTaskId;
    uint32                              ipcControlTaskStk[IPC_TASK_START_STK_SIZE];
} IPCCtrTaskInfo_t;

extern IPCHandler_t                     gIPCHandler[IPC_SVC_CH_MAX];

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
void IPC_StructInit
(
    IPCSvcCh_t                          siCh,
    IPCHandler_t *                      pIpcHandler
);

IPCStatus_t IPC_GetStatus
(
    IPCSvcCh_t                          siCh
);

int32 IPC_Initialize
(
    IPCSvcCh_t                          siCh
);

void IPC_Release
(
    IPCSvcCh_t                          siCh
);

int32 IPC_Write
(
    IPCSvcCh_t                          siCh,
    const uint32 *                      puiCmd,
    const uint8 *                       pucBuff,
    const uint32                        uiSize
); //QAC

int32 IPC_Read
(
    IPCSvcCh_t                          siCh,
    uint8 *                             pucBuff,
    uint32                              uiSize,
    uint32                              uiFlag
);

int32 IPC_PingTest
(
    IPCSvcCh_t                          siCh,
    IPCPingInfo_t *                     pPingInfo
);

void IPC_Flush
(
    IPCSvcCh_t                          siCh
);
#endif /* _IPC_CTL_H_ */

