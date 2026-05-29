/*
***************************************************************************************************
*
*   FileName : hsm_manager.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : HSM Mannger
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
#if 0
#define NDEBUG
#endif
#define TLOG_LEVEL (TLOG_DEBUG)
#include "hsm_log.h"

#include <app_cfg.h>
#include <sal_internal.h>
#include <string.h>
#include <stdlib.h>
#include <fwug.h>
#include <debug.h>

#include "hsm_manager.h"
#include "mbox.h"

/*
***************************************************************************************************
*                                            DEFINITIONS
***************************************************************************************************
*/
#define HSM_MAJOR (2)
#define HSM_MINOR (1)

static MBOXDvice_t *hsm_dev;

int32 HSM_Init(void)
{
    int8 devName[7] = {'H', 'S', 'M', 0x00, 0x00, 0x00, 0x00};

    if (hsm_dev == NULL) {
        DLOG("HSM Version:%d.%d\n", HSM_MAJOR, HSM_MINOR);
        hsm_dev = MBOX_DevOpen(MBOX_CH_HSM_NONSECURE, devName, 0);
        if (hsm_dev == NULL) {
            ELOG("Error dev_open\n");
            return HSM_ERR;
        }
    }

    return HSM_OK;
}

int32 HSM_VerifyFw(
    uint32 uiHeaderAddr, uint32 uiHeaderSize, uint32 uiImgAddr, uint32 uiImgSize, uint32 uiImgId)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = (HSM_MBOX_CID_R5 | HSM_MBOX_BSID_BL1); // multi channel number
    cmd[1] = HSM_REQ_SB_VERIFY; // cmd
    cmd[2] = uiHeaderAddr;
    cmd[3] = uiHeaderSize;
    cmd[4] = uiImgAddr;
    cmd[5] = uiImgSize;
    cmd[6] = uiImgId;
    cmd[7] = (uint32)HSM_MBOX_ID_HSM; // mbox id, 0x4D5348 = "HSM"

    ret = MBOX_DevSendSss(hsm_dev, cmd, data, 0);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    memset(data, 0, sizeof(uint32));

    ret = MBOX_DevRecvSss(hsm_dev, cmd, data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)cmd[2];
    return ret;
}

int32 HSM_SetKey(uint32 uiReq, uint32 uiAddr, uint32 uiDataSize, uint32 uiKeyIndex)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = uiAddr;
    idx++;
    data[idx] = uiDataSize;
    idx++;
    data[idx] = uiKeyIndex;
    idx++;

    cmd[4] = (idx * sizeof(uint32)); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    return ret;
}

int32 HSM_RunAes(
    uint32 uiReq, uint32 uiObjId, uint8 *pucPubKey, uint32 uiKeySize, uint8 *pucIv, uint32 uiIvSize,
    uint8 *pucTag, uint32 uiTagSize, uint8 *pucAad, uint32 uiAadSize, uint8 *pucSrcAddr,
    uint32 uiSrcSize, uint8 *pucDstAddr, uint32 uiDstSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = uiObjId;
    idx++;
    data[idx] = HSM_NONE_DMA;
    idx++;
    data[idx] = uiKeySize;
    idx++;
    if ((pucPubKey != NULL) && (uiKeySize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucPubKey, uiKeySize);
        idx += (uiKeySize + 3U) / sizeof(uint32);
    }
    data[idx] = uiIvSize;
    idx++;
    if ((pucIv != NULL) && (uiIvSize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucIv, uiIvSize);
        idx += (uiIvSize + 3U) / sizeof(uint32);
    }
    if (uiReq == HSM_REQ_RUN_AES) {
        data[idx] = uiTagSize;
        idx++;
        data[idx] = uiAadSize;
        idx++;
        if (uiAadSize > 0U) {
            SAL_MemCopy((void *)&data[idx], (const void *)pucAad, uiAadSize);
            idx += (uiAadSize + 3U) / sizeof(uint32);
        }
    }
    data[idx] = uiSrcSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucSrcAddr, uiSrcSize);
    idx += (uiSrcSize + 3U) / sizeof(uint32);
    data[idx] = uiDstSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = uiDstSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x) idx(%d)\n", data[0], idx);
        return HSM_ERR;
    }
    if (data[1] == (uiDstSize + uiTagSize)) {
        SAL_MemCopy((void *)pucDstAddr, (const void *)&data[2], uiDstSize);
        if (uiReq == HSM_REQ_RUN_AES) {
            if (uiTagSize > 0U) {
                SAL_MemCopy(
                    (void *)pucTag, (const void *)&data[2U + (uiDstSize / sizeof(uint32))],
                    uiTagSize);
            }
        }
    } else {
        ELOG("wrong uiDstSize(%d)\n", data[1]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_RunAesByKt(
    uint32 uiReq, uint32 uiObjId, uint32 uiKeyIndex, uint8 *pucIv, uint32 uiIvSize, uint8 *ucTag,
    uint32 uiTagSize, uint8 *ucAad, uint32 uiAadSize, uint8 *pucSrcAddr, uint32 uiSrcSize,
    uint8 *pucDstAddr, uint32 uiDstSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = uiObjId;
    idx++;
    data[idx] = HSM_NONE_DMA;
    idx++;
    data[idx] = uiKeyIndex;
    idx++;

    data[idx] = uiIvSize;
    idx++;
    if ((pucIv != NULL) && (uiIvSize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucIv, uiIvSize);
        idx += (uiIvSize + 3U) / sizeof(uint32);
    }
    if (uiReq == HSM_REQ_RUN_AES_BY_KT) {
        data[idx] = uiTagSize;
        idx++;
        data[idx] = uiAadSize;
        idx++;
        if (uiAadSize > 0U) {
            SAL_MemCopy((void *)&data[idx], (const void *)ucAad, uiAadSize);
            idx += (uiAadSize + 3U) / sizeof(uint32);
        }
    }
    data[idx] = uiSrcSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucSrcAddr, uiSrcSize);
    idx += (uiSrcSize + 3U) / sizeof(uint32);
    data[idx] = uiDstSize;
    idx++;
    cmd[4] = idx * sizeof(uint32); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = uiDstSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(%d) idx(0x%x)\n", data[0], idx);
        return HSM_ERR;
    }

    if (data[1] == (uiDstSize + uiTagSize)) {
        SAL_MemCopy((void *)pucDstAddr, (const void *)&data[2], uiDstSize);
        if (uiReq == HSM_REQ_RUN_AES_BY_KT) {
            if (uiTagSize > 0U) {
                SAL_MemCopy(
                    (void *)ucTag, (const void *)&data[2U + (uiDstSize / sizeof(uint32))],
                    uiTagSize);
            }
        }
    } else {
        ELOG("wrong uiDstSize(%d)\n", data[1]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_GenMac(
    uint32 uiReq, uint32 uiObjId, uint8 *pucPubKey, uint32 uiKeySize, uint8 *pucSrcAddr,
    uint32 uiSrcSize, uint8 *pucMacAddr, uint32 uiMacSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    if ((uiReq == HSM_REQ_GEN_HMAC) || (uiReq == HSM_REQ_GEN_SM3_HMAC)) {
        data[idx] = uiObjId;
        idx++;
    }
    data[idx] = HSM_NONE_DMA;
    idx++;
    data[idx] = uiKeySize;
    idx++;
    if ((pucPubKey != NULL) && (uiKeySize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucPubKey, uiKeySize);
        idx += (uiKeySize + 3U) / sizeof(uint32);
    }

    data[idx] = uiSrcSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucSrcAddr, uiSrcSize);
    idx += (uiSrcSize + 3U) / sizeof(uint32);
    data[idx] = uiMacSize;
    idx++;
    if (uiReq == HSM_REQ_VERIFY_CMAC) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucMacAddr, uiMacSize);
        idx += (uiMacSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = uiMacSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(%d) idx(%d)\n", data[0], idx);
        return HSM_ERR;
    }

    if (uiReq != HSM_REQ_VERIFY_CMAC) {
        if (data[1] == uiMacSize) {
            SAL_MemCopy((void *)pucMacAddr, (const void *)&data[2], uiMacSize);
        } else {
            ELOG("wrong uiDstSize(%d)\n", data[1]);
            return HSM_ERR;
        }
    }

    return HSM_OK;
}

int32 HSM_GenMacByKt(
    uint32 uiReq, uint32 uiObjId, uint32 uiKeyIndex, uint8 *pucSrcAddr, uint32 uiSrcSize,
    uint8 *pucMacAddr, uint32 uiMacSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    if ((uiReq == HSM_REQ_GEN_HMAC_BY_KT) || (uiReq == HSM_REQ_GEN_SM3_HMAC_BY_KT)) {
        data[idx] = uiObjId;
        idx++;
    }
    data[idx] = HSM_NONE_DMA;
    idx++;
    data[idx] = uiKeyIndex;
    idx++;

    data[idx] = uiSrcSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucSrcAddr, uiSrcSize);
    idx += (uiSrcSize + 3U) / sizeof(uint32);
    data[idx] = uiMacSize;
    idx++;
    if (uiReq == HSM_REQ_VERIFY_CMAC_BY_KT) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucMacAddr, uiMacSize);
        idx += (uiMacSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = uiMacSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(%d) idx(%d)\n", data[0], idx);
        return HSM_ERR;
    }

    if (uiReq != HSM_REQ_VERIFY_CMAC_BY_KT) {
        if (data[1] == uiMacSize) {
            SAL_MemCopy((void *)pucMacAddr, (const void *)&data[2], uiMacSize);
        } else {
            ELOG("wrong uiDstSize(%d)\n", data[1]);
            return HSM_ERR;
        }
    }

    return HSM_OK;
}

int32 HSM_GenHash(
    uint32 uiReq, uint32 uiObjId, uint8 *pucSrcAddr, uint32 uiSrcSize, uint8 *pucDig,
    uint32 uiDigSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = uiObjId;
    idx++;
    data[idx] = HSM_NONE_DMA;
    idx++;

    data[idx] = uiSrcSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucSrcAddr, uiSrcSize);
    idx += (uiSrcSize + 3U) / sizeof(uint32);
    data[idx] = uiDigSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = uiDigSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(%d) idx(%d)\n", data[0], idx);
        return HSM_ERR;
    }

    if (data[1] == uiDigSize) {
        SAL_MemCopy((void *)pucDig, (const void *)&data[2], uiDigSize);
    } else {
        ELOG("wrong uiDstSize(%d)\n", data[1]);
        return HSM_ERR;
    }

    return HSM_OK;
}

int32 HSM_RunEcdsa(
    uint32 uiReq, uint32 uiObjId, uint8 *pucPubKey, uint32 uiKeySize, uint8 *pucDig,
    uint32 uiDigSize, uint8 *pucSig, uint32 uiSigSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = uiObjId;
    idx++;

    data[idx] = uiKeySize;
    idx++;
    if ((pucPubKey != NULL) && (uiKeySize > 0U)) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucPubKey, uiKeySize);
        idx += (uiKeySize + 3U) / sizeof(uint32);
    }

    data[idx] = uiDigSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucDig, uiDigSize);
    idx += (uiDigSize + 3U) / sizeof(uint32);

    data[idx] = uiSigSize;
    idx++;
    if (uiReq == HSM_REQ_RUN_ECDSA_VERIFY) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucSig, uiSigSize);
        idx += (uiSigSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (uiReq == HSM_REQ_RUN_ECDSA_VERIFY) {
        dataSize = sizeof(uint32);
    } else {
        dataSize = uiSigSize + (sizeof(uint32) * 2U);
    }
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error result value(0x%x) idx(%x)\n", data[0], idx);
        return HSM_ERR;
    }

    if (uiReq == HSM_REQ_RUN_ECDSA_SIGN) {
        if (data[1] == uiSigSize) {
            SAL_MemCopy((void *)pucSig, (const void *)&data[2], uiSigSize);
        } else {
            ELOG("wrong uiDstSize(%d)\n", data[1]);
            return HSM_ERR;
        }
    }

    return HSM_OK;
}

int32 HSM_RunRsa(
    uint32 uiReq, uint32 uiObjId, uint8 *pucModN, uint32 uiModNSize, uint8 *pucPubKey,
    uint32 uiKeySize, uint8 *pucDig, uint32 uiDigSize, uint8 *pucSig, uint32 uiSigSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = uiObjId;
    idx++;
    data[idx] = HSM_NONE_DMA;
    idx++;
    data[idx] = uiModNSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucModN, uiModNSize);
    idx += (uiModNSize + 3U) / sizeof(uint32);

    data[idx] = uiKeySize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucPubKey, uiKeySize);
    idx += (uiKeySize + 3U) / sizeof(uint32);

    data[idx] = uiDigSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucDig, uiDigSize);
    idx += (uiDigSize + 3U) / sizeof(uint32);

    data[idx] = uiSigSize;
    idx++;
    if ((uiReq == HSM_REQ_RUN_RSASSA_PKCS_VERIFY) || (uiReq == HSM_REQ_RUN_RSASSA_PSS_VERIFY)) {
        SAL_MemCopy((void *)&data[idx], (const void *)pucSig, uiSigSize);
        idx += (uiSigSize + 3U) / sizeof(uint32);
    }

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    if (uiReq == HSM_REQ_RUN_ECDSA_VERIFY) {
        dataSize = sizeof(uint32);
    } else {
        dataSize = uiDigSize + (sizeof(uint32) * 2U);
    }
    SAL_MemSet((void *)data, 0, dataSize);
    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);

    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    if (ret != HSM_OK) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return HSM_ERR;
    }
    if ((uiReq == HSM_REQ_RUN_RSASSA_PKCS_SIGN) || (uiReq == HSM_REQ_RUN_RSASSA_PSS_SIGN)) {
        if (data[1] == uiSigSize) {
            SAL_MemCopy((void *)pucSig, (const void *)&data[2], uiSigSize);
        } else {
            ELOG("wrong uiSigSize(%d)\n", data[1]);
            return HSM_ERR;
        }
    }

    return ret;
}

int32 HSM_Write(uint32 uiReq, uint32 uiAddr, uint8 *pucBuf, uint32 uiBufSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = HSM_ERR;

    /* Set cmd FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    /* Set data FIFO */
    data[idx] = uiAddr;
    idx++;
    data[idx] = uiBufSize;
    idx++;
    SAL_MemCopy((void *)&data[idx], (const void *)pucBuf, uiBufSize);
    idx += (uiBufSize + 3U) / sizeof(uint32);

    cmd[4] = idx * sizeof(uint32); // data size

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    return ret;
}

int32 HSM_GetRand(uint32 uiReq, uint32 *pucRng, uint32 uiRngSize)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    uint32 idx = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    data[idx] = HSM_NONE_DMA;
    idx++;
    data[idx] = uiRngSize;
    idx++;

    cmd[4] = idx * sizeof(uint32); // data size
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, idx);
    if (ret != HSM_OK) {
        ELOG("Error MBOX_DevSendSss(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = uiRngSize + (sizeof(uint32) * 2U);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    if (ret != HSM_OK) {
        ELOG("Error: 0x%x\n", ret);
    }

    if (data[1] == uiRngSize) {
        SAL_MemCopy((void *)pucRng, (const void *)&data[2], uiRngSize);
    } else {
        ELOG("wrong uiDstSize(%d)\n", data[1]);
        return HSM_ERR;
    }

    return ret;
}

int32 HSM_BlockApSystem(uint32 uiReq)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[4] = dataSize;                        // data length is 0,
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = (uint32)HSM_MBOX_ID_HSM;        // mbox id, 0x4D5348 = "HSM"

    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, dataSize);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = sizeof(uint32);
    SAL_MemSet((void *)data, 0, dataSize);

    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);
    if (ret < 0) {
        ELOG("Error MBOX_DevRecvSss(%d)\n", ret);
        return HSM_ERR;
    }

    ret = (int32)data[0];
    return ret;

}

int32 HSM_GetVersion(uint32 uiReq, uint32 *uiX, uint32 *uiY, uint32 *uiZ)
{
    uint32 cmd[HSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    if ((uiX == NULL) || (uiY == NULL) || (uiZ == NULL)) {
        ELOG("Error invalid param\n");
        return HSM_ERR_INVALID_PARAM;
    }

    /* Set command FIFO */
    cmd[0] = HSM_MBOX_HSM_CMD0;              // multi channel number
    cmd[1] = uiReq | HSM_MBOX_LOCATION_DATA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = HSM_NONE_DMA;                   // trans_type
    cmd[4] = 0;                              // data length is 0,
    cmd[5] = 0;                              // Not used
    cmd[6] = 0;                              // Not used
    cmd[7] = 0x4D5348;                       // mbox id, 0x4D5348 = "HSM"

    dataSize = cmd[4];
    ret = MBOX_DevSendSss(hsm_dev, cmd, (uint32 *)data, dataSize);
    if (ret != HSM_OK) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return HSM_ERR;
    }

    dataSize = (sizeof(uint32) * 5U);
    SAL_MemSet((void *)data, 0, dataSize);
    ret = MBOX_DevRecvSss(hsm_dev, cmd, (uint32 *)data);

    if (ret < 0) {
        ELOG("Error recv_sss (%d)\n", ret);
        return HSM_ERR;
    }

    if (data[0] != HSM_DATA_FIFO_OK) {
        ELOG("Error: 0x%x\n", data[0]);
    }

    dataSize = data[1];
    if (dataSize == (sizeof(uint32) * 3U)) {
        *uiX = data[2];
        *uiY = data[3];
        *uiZ = data[4];
    } else {
        ELOG("wrong rdata size(%d)\n", dataSize);
        return HSM_ERR;
    }

    DLOG("Version %d.%d.%d\n", *uiX, *uiY, *uiZ);

    return HSM_OK;
}

