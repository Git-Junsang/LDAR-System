/*
***************************************************************************************************
*
*   FileName : ipc_typedef.h
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

#ifndef IPC_TYPE_DEF_H
#define IPC_TYPE_DEF_H

#if (DEBUG_ENABLE)
#include "debug.h"
#define IPC_DBG(fmt, args...)           {LOGD(DBG_TAG_MBOX, fmt, ## args)}
#define IPC_ERR(fmt, args...)           {LOGE(DBG_TAG_MBOX, fmt, ## args)}
#else
#define IPC_DBG(fmt, args...)
#define IPC_ERR(fmt, args...)
#endif

#define IPC_Hw37                        (1LL << 37)
#define IPC_Hw36                        (1LL << 36)
#define IPC_Hw35                        (1LL << 35)
#define IPC_Hw34                        (1LL << 34)
#define IPC_Hw33                        (1LL << 33)
#define IPC_Hw32                        (1LL << 32)
#define IPC_Hw31                        (0x80000000U)
#define IPC_Hw30                        (0x40000000U)
#define IPC_Hw29                        (0x20000000U)
#define IPC_Hw28                        (0x10000000U)
#define IPC_Hw27                        (0x08000000U)
#define IPC_Hw26                        (0x04000000U)
#define IPC_Hw25                        (0x02000000U)
#define IPC_Hw24                        (0x01000000U)
#define IPC_Hw23                        (0x00800000U)
#define IPC_Hw22                        (0x00400000U)
#define IPC_Hw21                        (0x00200000U)
#define IPC_Hw20                        (0x00100000U)
#define IPC_Hw19                        (0x00080000U)
#define IPC_Hw18                        (0x00040000U)
#define IPC_Hw17                        (0x00020000U)
#define IPC_Hw16                        (0x00010000U)
#define IPC_Hw15                        (0x00008000U)
#define IPC_Hw14                        (0x00004000U)
#define IPC_Hw13                        (0x00002000U)
#define IPC_Hw12                        (0x00001000U)
#define IPC_Hw11                        (0x00000800U)
#define IPC_Hw10                        (0x00000400U)
#define IPC_Hw9                         (0x00000200U)
#define IPC_Hw8                         (0x00000100U)
#define IPC_Hw7                         (0x00000080U)
#define IPC_Hw6                         (0x00000040U)
#define IPC_Hw5                         (0x00000020U)
#define IPC_Hw4                         (0x00000010U)
#define IPC_Hw3                         (0x00000008U)
#define IPC_Hw2                         (0x00000004U)
#define IPC_Hw1                         (0x00000002U)
#define IPC_Hw0                         (0x00000001U)
#define IPC_HwZERO                      (0x00000000U)

typedef enum IPCCmdType
{
    CTL_CMD                             = 0x0000U,
    WRITE_CMD,
    MAX_CMD_TYPE,
} IPCCmdType_t;

typedef enum IPCCmdId
{
    /* control command */
    IPC_OPEN                            = 0x0001U,
    IPC_CLOSE,
    IPC_SEND_PING,
    IPC_WRITE,
    IPC_ACK,
    IPC_NACK,
    MAX_CMD_ID,
} IPCCmdId_t;
#endif /* _IPC_TYPE_DEF_H_ */

