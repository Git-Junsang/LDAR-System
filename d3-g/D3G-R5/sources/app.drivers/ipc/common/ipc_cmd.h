/*
***************************************************************************************************
*
*   FileName : ipc_cmd.h
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

#ifndef IPC_CMD_H
#define IPC_CMD_H

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define CMD_TYPE_MASK                   (0xFFFF0000U)
#define CMD_ID_MASK                     (0xFFFFU)

#define MAX_MBOX_DATA_IDX               (2U)
#define MAX_MBOX_CMD_FIFO_CNT           (8U)
#define MAX_MBOX_DATA_FIFO_CNT          (128U)

typedef struct IPCMboxMsgReg
{
    uint32                              mmReg[MAX_MBOX_CMD_FIFO_CNT];
    uint32                              mmDat[MAX_MBOX_DATA_FIFO_CNT];
} IPCMboxMsgReg_t;

typedef struct MBOXDev
{
    MBOXDvice_t *                       mbDev;
} IPCMboxDev_t;

extern IPCMboxDev_t dev[IPC_SVC_CH_MAX];

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
int32 IPC_SendOpen
(
    IPCSvcCh_t                          siCh
);

int32 IPC_SendClose
(
    IPCSvcCh_t                          siCh
);

int32 IPC_SendWrite
(
    IPCSvcCh_t                          siCh,
    const uint32 *                      puiCmd,
    const uint8 *                       pucBuff,
    const uint32                        uiSize,
    uint32                              uiIsAck
); //QAC

int32 IPC_SendPing
(
    IPCSvcCh_t                          siCh
);

int32 IPC_SendAck
(
    IPCSvcCh_t                          siCh,
    uint32                              uiSeqID,
    uint32                              uiCmdID,
    IPCCmdType_t                        cmdType,
    uint32                              uiSourcCmd,
    uint32                              feature
);

#endif /* _IPC_CMD_H_ */

