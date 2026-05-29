/*
***************************************************************************************************
*
*   FileName : ipc_buffer.h
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

#ifndef IPC_BUFFER_H
#define IPC_BUFFER_H

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define IPC_BUFFER_ERROR                (-1)
#define IPC_BUFFER_FULL                 (-2)
#define IPC_BUFFER_EMPTY                (-3)
#define IPC_BUFFER_INVALID_PARAM        (-4)
#define IPC_BUFFER_OK                   (0)

typedef struct IPCRingBuff
{
    uint32                              rbHead;     //read position
    uint32                              rbTail;     //write position
    uint32                              rbMaxBufferSize;
    uint8 *                             rbBuffer;
}IPCRingBuff_t;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
void IPC_BufferInit
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8 *                             pucBuff,
    uint32                              uiSize
);

void IPC_BufferFlush
(
    IPCRingBuff_t *                     pBufCtrl
);

void IPC_BufferFlushByte
(
    IPCRingBuff_t *                     pBufCtrl,
    uint32                              uiFlushSize
);

void IPC_BufferSetHead
(
    IPCRingBuff_t *                     pBufCtrl,
    int32                               siHead
);

void IPC_BufferSetTail
(
    IPCRingBuff_t *                     pBufCtrl,
    int32                               siTail
);

int32 IPC_PushOneByte
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8                               pucData
);

int32 IPC_PushOneByteOverWrite
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8                               pucData
);

int32 IPC_PopOneByte
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8*                              pucData
);

int32 IPC_BufferDataAvailable
(
    const IPCRingBuff_t *               pBufCtrl
);

uint32 IPC_BufferFreeSpace
(
    const IPCRingBuff_t *               pBufCtrl
);

int32 IPC_PushBuffer
(
    IPCRingBuff_t *                     pBufCtrl,
    const uint8 *                       pucBuffer,
    uint32                              uiSize
);

int32 IPC_PushBufferOverWrite
(
    IPCRingBuff_t *                     pBufCtrl,
    const uint8 *                       pucBuffer,
    uint32                              uiSize
);

int32 IPC_PopBuffer
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8 *                             pucBuffer,
    uint32                              uiSize
);

uint32 IPC_BufferGetHead
(
    const IPCRingBuff_t *               pBufCtrl
);

uint32 IPC_BufferGetTail
(
    const IPCRingBuff_t *               pBufCtrl
);

#endif /* _IPC_BUFFER_H_ */

