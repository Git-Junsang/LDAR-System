/*
***************************************************************************************************
*
*   FileName : ipc_buffer.c
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
#include "ipc_typedef.h"
#include "ipc.h"
#include "ipc_api.h"
#include "ipc_buffer.h"

void IPC_BufferInit
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8 *                             pucBuff,
    uint32                              uiSize
)
{
    pBufCtrl->rbHead          = 0U;
    pBufCtrl->rbTail          = 0U;
    pBufCtrl->rbBuffer        = pucBuff;
    pBufCtrl->rbMaxBufferSize = uiSize;
}

int32 IPC_PushOneByte
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8                               pucData
)
{
    int32  ret;
    uint32 temp;

    ret = IPC_BUFFER_ERROR;

    if (pBufCtrl != NULL_PTR)
    {
        temp = pBufCtrl->rbTail;
        temp++;
        temp %= pBufCtrl->rbMaxBufferSize;

        if (temp == pBufCtrl->rbHead)
        {
            ret = IPC_BUFFER_FULL;
        }
        else
        {
            pBufCtrl->rbBuffer[pBufCtrl->rbTail] = pucData;
            pBufCtrl->rbTail                     = (uint32)temp;
            ret = IPC_BUFFER_OK;
        }
    }
    else
    {
        ret = IPC_BUFFER_INVALID_PARAM;
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }

    return ret;
}

int32 IPC_PushOneByteOverWrite
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8                               pucData
)
{
    int32  ret;
    uint32 temp;

    if (pBufCtrl != NULL_PTR)
    {
        temp = pBufCtrl->rbTail;
        temp++;
        temp %= pBufCtrl->rbMaxBufferSize;

        if (temp == pBufCtrl->rbHead)
        {
            pBufCtrl->rbHead++;
            pBufCtrl->rbHead = pBufCtrl->rbHead % pBufCtrl->rbMaxBufferSize;
            pBufCtrl->rbBuffer[pBufCtrl->rbTail] = pucData;
            pBufCtrl->rbTail                     = (uint32)temp;
            ret = IPC_BUFFER_OK;
        }
        else
        {
            pBufCtrl->rbBuffer[pBufCtrl->rbTail] = pucData;
            pBufCtrl->rbTail                     = (uint32)temp;
            ret = IPC_BUFFER_OK;
        }
    }
    else
    {
        ret = IPC_BUFFER_INVALID_PARAM;
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }

    return ret;
}

int32 IPC_PopOneByte
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8 *                             pucData
)
{
    int32  ret;
    uint32 temp;

    if ((pBufCtrl != NULL_PTR) && (pucData != NULL_PTR))
    {
        if (pBufCtrl->rbTail == pBufCtrl->rbHead)
        {
            ret = IPC_BUFFER_EMPTY;
        }
        else
        {
            temp = pBufCtrl->rbHead;
            temp++;
            temp     %= pBufCtrl->rbMaxBufferSize;
            *pucData = pBufCtrl->rbBuffer[pBufCtrl->rbHead];
            pBufCtrl->rbHead = temp;
            ret = IPC_BUFFER_OK;
        }
    }
    else
    {
        ret = IPC_BUFFER_INVALID_PARAM;
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }

    return ret;
}

void IPC_BufferFlush
(
    IPCRingBuff_t *                     pBufCtrl
)
{
    if (pBufCtrl != NULL_PTR)
    {
        pBufCtrl->rbHead = 0;
        pBufCtrl->rbTail = 0;
    }
    else
    {
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }
}

void IPC_BufferFlushByte
(
    IPCRingBuff_t *                     pBufCtrl,
    uint32                              uiFlushSize
)
{
    uint32 temp;

    if (pBufCtrl != NULL_PTR)
    {
        if (pBufCtrl->rbTail < pBufCtrl->rbHead)
        {
            temp = pBufCtrl->rbHead + uiFlushSize;

            if(temp< pBufCtrl->rbMaxBufferSize)
            {
                pBufCtrl->rbHead = temp;
            }
            else
            {
                temp %= pBufCtrl->rbMaxBufferSize;

                if(pBufCtrl->rbTail <= temp)
                {
                    pBufCtrl->rbHead = pBufCtrl->rbTail;
                }
                else
                {
                    pBufCtrl->rbHead = temp;
                }
            }
        }
        else
        {
            temp = pBufCtrl->rbHead + uiFlushSize;

            if(pBufCtrl->rbTail <= temp)
            {
                pBufCtrl->rbHead = pBufCtrl->rbTail;
            }
            else
            {
                pBufCtrl->rbHead = temp;
            }
        }
    }
    else
    {
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }
}

int32 IPC_BufferDataAvailable
(
    const IPCRingBuff_t *               pBufCtrl
)
{
    int32 iRet;
    int32 iRead;
    int32 iWrite;

    iRet   = 0;
    iRead  = 0;
    iWrite = 0;

    if (pBufCtrl != NULL_PTR)
    {
        iRead  = (int32) pBufCtrl->rbHead;
        iWrite = (int32) pBufCtrl->rbTail;

        if (iWrite >= iRead)
        {
            // The read pointer is before the write pointer in the bufer.
            iRet = iWrite - iRead;
        }
        else
        {
            // The write pointer is before the read pointer in the buffer.
            iRet = (int32)pBufCtrl->rbMaxBufferSize - (iRead - iWrite);
        }
    }
    else
    {
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }

    return iRet;
}

uint32 IPC_BufferFreeSpace
(
    const IPCRingBuff_t *               pBufCtrl
)
{
    int32 iRet;
    int32 iRead;
    int32 iWrite;

    iRet   = 0;
    iRead  = 0;
    iWrite = 0;

    if (pBufCtrl != NULL_PTR)
    {
        iRead = (int32)pBufCtrl->rbHead;
        iWrite = (int32)pBufCtrl->rbTail;

        if (iWrite < iRead)
        {
            // The write pointer is before the read pointer in the buffer.
            iRet = iRead - iWrite - 1;
        }
        else
        {
            // The read pointer is before the write pointer in the bufer.
            iRet = (int32) pBufCtrl->rbMaxBufferSize - (iWrite - iRead) - 1;
        }
    }
    else
    {
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }

    return ((uint32) iRet);
}

void IPC_BufferSetHead
(
    IPCRingBuff_t *                     pBufCtrl,
    int32                               siHead
)
{
    if (pBufCtrl != NULL_PTR)
    {
        pBufCtrl->rbHead = (uint32) siHead;
    }
    else
    {
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }
}

void IPC_BufferSetTail
(
    IPCRingBuff_t *                     pBufCtrl,
    int32                               siTail
)
{
    if (pBufCtrl != NULL_PTR)
    {
        pBufCtrl->rbTail = (uint32) siTail;
    }
    else
    {
        IPC_ERR("%s Buffer Pointer is NULL\n", __func__);
    }
}

uint32 IPC_BufferGetHead
(
    const IPCRingBuff_t *               pBufCtrl
)
{
    return ((pBufCtrl != NULL_PTR) ? pBufCtrl->rbHead : 0U);
}

uint32 IPC_BufferGetTail
(
    const IPCRingBuff_t *               pBufCtrl
)
{
    return ((pBufCtrl != NULL_PTR) ? pBufCtrl->rbTail : 0U);
}

int32 IPC_PushBuffer
(
    IPCRingBuff_t *                     pBufCtrl,
    const uint8 *                       pucBuffer,
    uint32                              uiSize
)
{
    int32  ret;
    uint32 freeSpace;
    uint32 remainSize;
    uint32 continuousSize;

    ret             = IPC_BUFFER_ERROR;
    freeSpace       = 0;
    continuousSize  = 0;
    freeSpace       = IPC_BufferFreeSpace(pBufCtrl);

    if(freeSpace >= uiSize)
    {
        continuousSize = pBufCtrl->rbMaxBufferSize - pBufCtrl->rbTail;

        if(continuousSize > (uint32) uiSize)
        {
            if (SAL_MemCopy((void *)&pBufCtrl->rbBuffer[pBufCtrl->rbTail], (const void *)pucBuffer, (uint32)uiSize) == SAL_RET_SUCCESS)
            {
                pBufCtrl->rbTail += uiSize;
                pBufCtrl->rbTail = pBufCtrl->rbTail % pBufCtrl->rbMaxBufferSize;
                ret = IPC_BUFFER_OK;
            }
            else
            {
                ret = IPC_BUFFER_ERROR;
            }
        }
        else
        {
            if (SAL_MemCopy((void *)&pBufCtrl->rbBuffer[pBufCtrl->rbTail], (const void *)pucBuffer, (uint32)continuousSize) == SAL_RET_SUCCESS)
            {
                remainSize = uiSize - continuousSize;
                if (SAL_MemCopy((void *)&pBufCtrl->rbBuffer[0], (const void *)&pucBuffer[continuousSize], (uint32)remainSize) == SAL_RET_SUCCESS)
                {
                    pBufCtrl->rbTail = remainSize;
                    pBufCtrl->rbTail = pBufCtrl->rbTail % pBufCtrl->rbMaxBufferSize;
                    ret = IPC_BUFFER_OK;
                }
                else
                {
                    ret = IPC_BUFFER_ERROR;
                }
            }
            else
            {
                ret = IPC_BUFFER_ERROR;
            }
        }
    }
    else
    {
        ret = IPC_BUFFER_FULL;
    }

    return ret;
}

int32 IPC_PushBufferOverWrite
(
    IPCRingBuff_t *                     pBufCtrl,
    const uint8 *                       pucBuffer,
    uint32                              uiSize
)
{
    int32  ret;
    uint32 remainSize;
    uint32 continuousSize;

    ret= IPC_BUFFER_ERROR;

    if(pBufCtrl->rbMaxBufferSize >= uiSize)
    {
        continuousSize = pBufCtrl->rbMaxBufferSize - pBufCtrl->rbTail;

        if(continuousSize > uiSize)
        {
            (void ) SAL_MemCopy((void *)&pBufCtrl->rbBuffer[pBufCtrl->rbTail], (const void *)pucBuffer, (uint32)uiSize);
            pBufCtrl->rbTail += uiSize;
            pBufCtrl->rbTail = pBufCtrl->rbTail % pBufCtrl->rbMaxBufferSize;
            pBufCtrl->rbHead = (pBufCtrl->rbTail + 1U) % pBufCtrl->rbMaxBufferSize;
            ret = (int32) uiSize;
        }
        else
        {
            (void) SAL_MemCopy((void *)&pBufCtrl->rbBuffer[pBufCtrl->rbTail], (const void *)pucBuffer, continuousSize);
            remainSize = uiSize - continuousSize;
            (void) SAL_MemCopy((void *)&pBufCtrl->rbBuffer[0], (const void *)&pucBuffer[continuousSize], (uint32)remainSize);
            pBufCtrl->rbTail = remainSize;
            pBufCtrl->rbTail = pBufCtrl->rbTail % pBufCtrl->rbMaxBufferSize;
            pBufCtrl->rbHead = (pBufCtrl->rbTail + 1U) % pBufCtrl->rbMaxBufferSize;
            ret = (int32) uiSize;
        }
    }
    return ret;
}

int32 IPC_PopBuffer
(
    IPCRingBuff_t *                     pBufCtrl,
    uint8 *                             pucBuffer,
    uint32                              uiSize
)
{
    int32  ret;
    uint32 dataSize;
    uint32 remainSize;
    uint32 continuousSize;

    ret            = IPC_BUFFER_ERROR;
    dataSize       = 0;
    continuousSize = 0;

    if (pBufCtrl->rbTail == pBufCtrl->rbHead)
    {
        ret = IPC_BUFFER_EMPTY;
    }
    else
    {
        dataSize = (uint32) IPC_BufferDataAvailable(pBufCtrl);

        if(dataSize >= uiSize)
        {
            continuousSize = pBufCtrl->rbMaxBufferSize - pBufCtrl->rbHead;
            ret = IPC_BUFFER_OK;

            if(continuousSize > uiSize)
            {
                (void) SAL_MemCopy((void *)pucBuffer, (const void *)&pBufCtrl->rbBuffer[pBufCtrl->rbHead],uiSize );
                pBufCtrl->rbHead += uiSize;
                pBufCtrl->rbHead = pBufCtrl->rbHead % pBufCtrl->rbMaxBufferSize;
            }
            else
            {
                (void) SAL_MemCopy((void *)pucBuffer, (const void *)&pBufCtrl->rbBuffer[pBufCtrl->rbHead],continuousSize );
                remainSize = uiSize - continuousSize;
                (void) SAL_MemCopy((void *)&pucBuffer[continuousSize], (const void *)&pBufCtrl->rbBuffer[0],remainSize );
                pBufCtrl->rbHead= remainSize;
                pBufCtrl->rbHead = pBufCtrl->rbHead % pBufCtrl->rbMaxBufferSize;
            }
        }
    }
    return ret;
}

