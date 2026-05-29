/*
***************************************************************************************************
*
*   FileName : HSMManagerA53.c
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

//#define NDEBUG
#define TLOG_LEVEL TLOG_DEBUG
#include "tcc_hsm_log.h"

#include <app_cfg.h>
#include <string.h>
//#include <types.h>
#include <sal_internal.h>
#include <ipc.h>
//#include <osal.h>
//#include "HAL_API.h"
//#include "HAL_IPC.h"
//#include "HAL_devices.h"
#include "HSMManagerA53.h"
#if defined(TCC803x)
#include "fwug.h"
#endif
#include "debug.h"

//#include "tcc_ipc_protocol.h"

#define THSM_MAJOR 1
#define THSM_MINOR 9

#define IPC_APP_ID_HSM 6

#if (SERVICE_HSMMANAGER_EN == 1u)

/* This macro makes a command to communicate with A53.
 * THSM Magic number: 5, e.g. 0x5000 ~ 0x5FFF */
#define THSM_EVENT(magic_num, evt) (((magic_num & 0xF) << 12) | (evt & 0xFFF))

/* Command ID, TCCTHSM command range: 0x5000 ~ 0x5FFF */
// clang-format off
#define MAGIC_NUM                        (5)  // THSM magic number
#define TCCTHSM_EVT_INIT                  THSM_EVENT(MAGIC_NUM, 0x011) // Generic
#define TCCTHSM_EVT_FINALIZE              THSM_EVENT(MAGIC_NUM, 0x012) // Generic
#define TCCTHSM_EVT_GET_VERSION           THSM_EVENT(MAGIC_NUM, 0x013) // Generic
#define TCCTHSM_EVT_SET_MODE              THSM_EVENT(MAGIC_NUM, 0x014) // Generic
#define TCCTHSM_EVT_SET_KEY               THSM_EVENT(MAGIC_NUM, 0x015) // Generic
#define TCCTHSM_EVT_SET_KEY_FROM_STORAGE  THSM_EVENT(MAGIC_NUM, 0x016) // Generic
#define TCCTHSM_EVT_SET_KEY_FROM_OTP      THSM_EVENT(MAGIC_NUM, 0x017) // Generic
#define TCCTHSM_EVT_FREE_MODE             THSM_EVENT(MAGIC_NUM, 0x018) // Generic
#define TCCTHSM_EVT_SET_IV_SYMMETRIC      THSM_EVENT(MAGIC_NUM, 0x021) // Symmetric Cipher
#define TCCTHSM_EVT_RUN_CIPHER            THSM_EVENT(MAGIC_NUM, 0x022) // Symmetric Cipher
#define TCCTHSM_EVT_RUN_DIGEST            THSM_EVENT(MAGIC_NUM, 0x031) // Message Digest (SHA)
#define TCCTHSM_EVT_SET_IV_MAC            THSM_EVENT(MAGIC_NUM, 0x041) // Message Authentication Code (MAC)
#define TCCTHSM_EVT_COMPUTE_MAC           THSM_EVENT(MAGIC_NUM, 0x042) // Message Authentication Code (MAC)
#define TCCTHSM_EVT_COMPARE_MAC           THSM_EVENT(MAGIC_NUM, 0x043) // Message Authentication Code (MAC)
#define TCCTHSM_EVT_GET_RAND              THSM_EVENT(MAGIC_NUM, 0x051) // RNG
#define TCCTHSM_EVT_GEN_KEY_SS            THSM_EVENT(MAGIC_NUM, 0x061) // Key Generate
#define TCCTHSM_EVT_DEL_KEY_SS            THSM_EVENT(MAGIC_NUM, 0x062) // Key Generate
#define TCCTHSM_EVT_WRITE_KEY_SS          THSM_EVENT(MAGIC_NUM, 0x063) // Key Generate
#define TCCTHSM_EVT_WRITE_OTP             THSM_EVENT(MAGIC_NUM, 0x071) // OTP
#define TCCTHSM_EVT_WRITE_OTP_IMAGE       THSM_EVENT(MAGIC_NUM, 0x072) // OTP
#define TCCTHSM_EVT_ASYMMETRIC_ENC        THSM_EVENT(MAGIC_NUM, 0x081) // Asymmetric
#define TCCTHSM_EVT_ASYMMETRIC_DEC        THSM_EVENT(MAGIC_NUM, 0x082) // Asymmetric
#define TCCTHSM_EVT_ASYMMETRIC_SIGN       THSM_EVENT(MAGIC_NUM, 0x083) // Asymmetric
#define TCCTHSM_EVT_ASYMMETRIC_VERIFY     THSM_EVENT(MAGIC_NUM, 0x084) // Asymmetric

#define TCCTHSM_EVT_RUN_CIPHER_BY_DMA     THSM_EVENT(MAGIC_NUM, 0x091) // Symmetric Cipher
#define TCCTHSM_EVT_RUN_DIGEST_BY_DMA     THSM_EVENT(MAGIC_NUM, 0x092) // Message Digest (SHA)
#define TCCTHSM_EVT_COMPUTE_MAC_BY_DMA    THSM_EVENT(MAGIC_NUM, 0x093) // Message Authentication Code (MAC)
#define TCCTHSM_EVT_COMPARE_MAC_BY_DMA    THSM_EVENT(MAGIC_NUM, 0x094) // Message Authentication Code (MAC)
#define TCCTHSM_EVT_ASYMMETRIC_ENC_BY_DMA THSM_EVENT(MAGIC_NUM, 0x095) // Asymmetric
#define TCCTHSM_EVT_ASYMMETRIC_DEC_BY_DMA THSM_EVENT(MAGIC_NUM, 0x096) // Asymmetric
// clang-format on

#define TCCTHSM_CMD_FIFO_SIZE 8
#define TCCTHSM_CMD_RES_SIZE 4
#define TCCTHSM_MAX_EVENT_ATTEMPT_COUNT 10

#define TCCTHSM_TEST_ENABLE
//#define DMA_TEST_ENABLE

typedef uint32 HAL_HANDLE;

static HAL_HANDLE hHSM;
static uint32 eventHandler;
static struct tcc_hsm_mbox_data tcc_hsm_data;

static int TCCTHSM_WrtieMbox(
    HAL_HANDLE HALHandle, uint32 ulCommand, void *param1, void *param2, void *param3, void *param4)
{
    return SAL_RET_SUCCESS;
}

static int TCCTHSM_ReadMbox(uint8 *buff, uint32 *size)
{
    SALRetCode_t err;
    uint32 flag = -1;
    int32 timeout = 1000;
    int32 i = 0;

    for (i = 0; i < TCCTHSM_MAX_EVENT_ATTEMPT_COUNT; i++) {
        err = SAL_EventGet(
            eventHandler, 1, timeout, (SAL_EVENT_OPT_SET_ANY | SAL_OPT_BLOCKING), &flag);

        if ((err == SAL_RET_SUCCESS)) 
        {
            if (flag == 0UL)
            {   // Waiting for becoming set
                continue;
            }
            else 
            {
                *size = tcc_hsm_data.readSize;
                memcpy((void *)buff, (const void *)tcc_hsm_data.buffer, *size);
                SAL_EventSet(eventHandler, 1, SAL_EVENT_OPT_CLR_ALL);
                if (i != 0) {
                    DLOG("Event timeout count = %d\n", i);
                }
                return SAL_RET_SUCCESS;
            }
        }
    }
    return SAL_RET_FAILED; /* time out */
}

void TCCTHSM_ReceiveMboxData(uint8 *buffer, uint32 size)
{
    if (buffer == NULL || size > 512) {
        ELOG("Wrong parameter! \n");
        return;
    }
    memcpy((void *)tcc_hsm_data.buffer, (const void *)buffer, size);
    tcc_hsm_data.readSize = size;
    SAL_EventSet(eventHandler, 1, SAL_EVENT_OPT_FLAG_SET);
    return;
}

int32 TCCTHSM_Open(void)
{
    if (hHSM == 0) {
        DLOG("THSM Version:%d.%d\n", THSM_MAJOR, THSM_MINOR);
        SAL_EventCreate(&eventHandler, (const uint8 *)"CA53 mbox read wait flag", 0);
        hHSM = ipc_ch_info[IPC_SVC_CH_CA53_NONSECURE].halHandle;
        if (hHSM == 0) {
            ELOG("Error HAL_Open\n");
            return TCCTHSM_ERR;
        }
        TCCTHSM_Init();
    }

    return TCCTHSM_OK;
}

int32 TCCTHSM_Close(void)
{
    if (hHSM != 0) {
        SAL_EventDelete(eventHandler);
    }

    return TCCTHSM_OK;
}

int32 TCCTHSM_Init(void)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;   // multibox channel number
    cmd[1] = TCCTHSM_EVT_INIT; // cmd
    cmd[2] = 0;                // dma_addr
    cmd[3] = DATA_MBOX;        // trans_type
    cmd[4] = sizeof(uint32);   // data length is 0, Only to avoid error on ipc_write */

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

int32 TCCTHSM_Finalize(void)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;       // multibox channel number
    cmd[1] = TCCTHSM_EVT_FINALIZE; // cmd
    cmd[2] = 0;                    // dma_addr
    cmd[3] = DATA_MBOX;            // trans_type
    cmd[4] = sizeof(uint32);       // data length is 0, Only to avoid error on ipc_write

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

int32 TCCTHSM_GetVersion(uint32 *major, uint32 *minor)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0, dstSize = 0;
    int32 ret = -1;

    if (major == NULL || minor == NULL) {
        ELOG("Error invalid param\n");
        return TCCTHSM_ERR_INVALID_PARAM;
    }

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;          // multi channel number
    cmd[1] = TCCTHSM_EVT_GET_VERSION; // cmd
    cmd[2] = 0;                       // dma_addr
    cmd[3] = DATA_MBOX;               // trans_type
    cmd[4] = sizeof(uint32);          // data length is 0, Only to avoid error on ipc_write

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        dstSize = data[1];
        if (dstSize == (sizeof(uint32) * 2)) {
            *major = data[2];
            *minor = data[3];
        }
    }

    DLOG("Version %d.%d\n", *major, *minor);

    return ret;
}

int32 TCCTHSM_SetMode(uint32 keyIndex, uint32 algorithm, uint32 opMode, uint32 keySize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;       // multibox channel number
    cmd[1] = TCCTHSM_EVT_SET_MODE; // cmd
    cmd[2] = 0;                    // dma_addr
    cmd[3] = DATA_MBOX;            // trans_type
    cmd[4] = sizeof(uint32) * 4;   // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = algorithm;
    data[2] = opMode;
    data[3] = keySize;

    dataSize = cmd[4];
    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];

    return ret;
}

int32 TCCTHSM_SetKey(
    uint32 keyIndex, uint32 *key1, uint32 key1Size, uint32 *key2, uint32 key2Size, uint32 *key3,
    uint32 key3Size)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                                        // multibox channel number
    cmd[1] = TCCTHSM_EVT_SET_KEY;                                   // cmd
    cmd[2] = 0;                                                     // dma_addr
    cmd[3] = DATA_MBOX;                                             // trans_type
    cmd[4] = (sizeof(uint32) * 4) + key1Size + key2Size + key3Size; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = key1Size;
    data[2] = key2Size;
    data[3] = key3Size;
    memcpy((void *)&data[4], (const void *)key1, key1Size);
    if (key2Size) {
        memcpy((void *)&data[4 + key1Size], (const void *)key2, key2Size);
    }
    if (key3Size) {
        memcpy((void *)&data[4 + key1Size + key2Size], (const void *)key3, key3Size);
    }

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];

    return ret;
}

int32 TCCTHSM_SetKeyFromStorage(uint32 keyIndex, int8 *objId, uint32 objIdLen)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[40] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                   // multibox channel number
    cmd[1] = TCCTHSM_EVT_SET_KEY_FROM_STORAGE; // cmd
    cmd[2] = 0;                                // dma_addr
    cmd[3] = DATA_MBOX;                        // trans_type
    cmd[4] = (sizeof(uint32) * 2) + objIdLen;  // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = objIdLen;
    memcpy((void *)&data[2], (const void *)objId, objIdLen);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];

    return ret;
}

int32 TCCTHSM_SetKeyFromOTP(uint32 keyIndex, uint32 otpAddr, uint32 keySize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[40] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;               // multibox channel number
    cmd[1] = TCCTHSM_EVT_SET_KEY_FROM_OTP; // cmd
    cmd[2] = 0;                            // dma_addr
    cmd[3] = DATA_MBOX;                    // trans_type
    cmd[4] = sizeof(uint32) * 3;           // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = otpAddr;
    data[2] = keySize;

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];

    return ret;
}

int32 TCCTHSM_FreeMode(uint32 keyIndex)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[10] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;        // multibox channel number
    cmd[1] = TCCTHSM_EVT_FREE_MODE; // cmd
    cmd[2] = 0;                     // dma_addr
    cmd[3] = DATA_MBOX;             // trans_type
    cmd[4] = sizeof(uint32);        // data_size

    /*Set data FIFO */
    data[0] = keyIndex;

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];

    return ret;
}

int32 TCCTHSM_SetIVSymmetric(uint32 keyIndex, uint32 *iv, uint32 ivSize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[40] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                // multibox channel number
    cmd[1] = TCCTHSM_EVT_SET_IV_SYMMETRIC;  // cmd
    cmd[2] = 0;                             // dma_addr
    cmd[3] = DATA_MBOX;                     // trans_type
    cmd[4] = (sizeof(uint32) * 2) + ivSize; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = ivSize;
    memcpy((void *)&data[2], (const void *)iv, ivSize);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];

    return ret;
}

int32 TCCTHSM_RunCipher(
    uint32 keyIndex, uint8 *srcBuf, uint32 srcSize, uint8 *dstBuf, uint32 *dstSize, uint32 flag)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    if (srcSize > TCC_MBOX_MAX_MSG) {
        ELOG("Err Size(0x%x), Use TCCTHSM_RunCipherByDMA\n", srcSize);
        return TCCTHSM_ERR_INVALID_PARAM;
    }

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                 // multibox channel number
    cmd[1] = TCCTHSM_EVT_RUN_CIPHER;         // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = DATA_MBOX;                      // trans_type
    cmd[4] = (sizeof(uint32) * 3) + srcSize; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = flag;
    data[2] = srcSize;
    memcpy((void *)&data[3], (const void *)srcBuf, srcSize);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    memset(data, 0, dataSize);
    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *dstSize = data[1];
        if (*dstSize > 0) {
            memcpy((void *)dstBuf, (const void *)&data[2], *dstSize);
        } else {
            ELOG("Wrong dstSize(%d)\n", *dstSize);
            return TCCTHSM_ERR;
        }
    }

    return ret;
}

int32 TCCTHSM_RunCipherByDMA(
    uint32 keyIndex, uint32 srcBuf, uint32 srcSize, uint32 dstBuf, uint32 *dstSize, uint32 flag)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[32] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                // multibox channel number
    cmd[1] = TCCTHSM_EVT_RUN_CIPHER_BY_DMA; // cmd
    cmd[2] = 0;                             // dma_addr
    cmd[3] = DATA_MBOX;                     // trans_type
    cmd[4] = (sizeof(uint32) * 5);          // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = flag;
    data[2] = srcBuf;
    data[3] = srcSize;
    data[4] = dstBuf;

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *dstSize = data[1];
    }

    return ret;
}

int32 TCCTHSM_RunDigest(
    uint32 keyIndex, uint8 *chunk, uint32 chunkLen, uint8 *hash, uint32 *hashLen, uint32 flag)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    if (chunkLen > TCC_MBOX_MAX_MSG) {
        ELOG("Err Size(0x%x), Use TCCTHSM_RunDigest_ByDMA\n", chunkLen);
        return TCCTHSM_ERR_INVALID_PARAM;
    }

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                  // multibox channel number
    cmd[1] = TCCTHSM_EVT_RUN_DIGEST;          // cmd
    cmd[2] = 0;                               // dma_addr
    cmd[3] = DATA_MBOX;                       // trans_type
    cmd[4] = (sizeof(uint32) * 4) + chunkLen; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = flag;
    data[2] = chunkLen;
    data[3] = *hashLen;
    memcpy((void *)&data[4], (const void *)chunk, chunkLen);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *hashLen = data[1];
        if (*hashLen > 0) {
            memcpy((void *)hash, (const void *)&data[2], *hashLen);
        } else {
            ELOG("Wrong dstSize(%d)\n", *hashLen);
            return TCCTHSM_ERR;
        }
    }

    return ret;
}

int32 TCCTHSM_RunDigestByDMA(
    uint32 keyIndex, uint32 chunk, uint32 chunkLen, uint8 *hash, uint32 *hashLen, uint32 flag)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[32] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                // multibox channel number
    cmd[1] = TCCTHSM_EVT_RUN_DIGEST_BY_DMA; // cmd
    cmd[2] = 0;                             // dma_addr
    cmd[3] = DATA_MBOX;                     // trans_type
    cmd[4] = sizeof(uint32) * 5;            // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = flag;
    data[2] = chunk;
    data[3] = chunkLen;
    data[4] = *hashLen;

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *hashLen = data[1];
        if (*hashLen > 0) {
            memcpy((void *)hash, (const void *)&data[2], *hashLen);
        } else {
            ELOG("Wrong dstSize(%d)\n", *hashLen);
            return TCCTHSM_ERR;
        }
    }

    return ret;
}

int32 TCCTHSM_SetIVMAC(uint32 keyIndex, uint32 *iv, uint32 ivSize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[40] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                // multibox channel number
    cmd[1] = TCCTHSM_EVT_SET_IV_MAC;        // cmd
    cmd[2] = 0;                             // dma_addr
    cmd[3] = DATA_MBOX;                     // trans_type
    cmd[4] = (sizeof(uint32) * 2) + ivSize; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = ivSize;
    memcpy((void *)&data[2], (const void *)iv, ivSize);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

int32 TCCTHSM_ComputeMAC(
    uint32 keyIndex, uint8 *message, uint32 messageLen, uint8 *mac, uint32 *macLen, uint32 flag)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    if (messageLen > TCC_MBOX_MAX_MSG) {
        ELOG("Err Size(0x%x), Use TCCTHSM_ComputeMACByDMA\n", messageLen);
        return TCCTHSM_ERR_INVALID_PARAM;
    }

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                    // multibox channel number
    cmd[1] = TCCTHSM_EVT_COMPUTE_MAC;           // cmd
    cmd[2] = 0;                                 // dma_addr
    cmd[3] = DATA_MBOX;                         // trans_type
    cmd[4] = (sizeof(uint32) * 4) + messageLen; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = flag;
    data[2] = messageLen;
    data[3] = *macLen;
    memcpy((void *)&data[4], (const void *)message, messageLen);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *macLen = data[1];
        if (*macLen > 0) {
            memcpy((void *)mac, (const void *)&data[2], *macLen);
        } else {
            ELOG("Wrong dstSize(%d)\n", *macLen);
            return TCCTHSM_ERR;
        }
    }

    return ret;
}

int32 TCCTHSM_ComputeMACByDMA(
    uint32 keyIndex, uint32 message, uint32 messageLen, uint8 *mac, uint32 *macLen, uint32 flag)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[80] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                 // multibox channel number
    cmd[1] = TCCTHSM_EVT_COMPUTE_MAC_BY_DMA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = DATA_MBOX;                      // trans_type
    cmd[4] = (sizeof(uint32) * 5);           // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = flag;
    data[2] = message;
    data[3] = messageLen;
    data[4] = *macLen;

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *macLen = data[1];
        if (*macLen > 0) {
            memcpy((void *)mac, (const void *)&data[2], *macLen);
        } else {
            ELOG("Wrong dstSize(%d)\n", *macLen);
            return TCCTHSM_ERR;
        }
    }

    return ret;
}

int32 TCCTHSM_CompareMAC(
    uint32 keyIndex, uint8 *message, uint32 messageLen, uint8 *mac, uint32 macLen, uint32 flag)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[80] = {0};
    uint32 dataSize = 0, offset = 0, messageLenAlign = 0;
    int32 ret = -1;

    if ((messageLen + macLen) > TCC_MBOX_MAX_MSG) {
        ELOG("Err Size(0x%x), Use TCCTHSM_ComputeMACByDMA\n", messageLen);
        return TCCTHSM_ERR_INVALID_PARAM;
    }

    /* Set command FIFO */
    messageLenAlign = ((messageLen + 3) / sizeof(uint32)) * 4;
    cmd[0] = IPC_APP_ID_HSM;                                  // multibox channel number
    cmd[1] = TCCTHSM_EVT_COMPARE_MAC;                         // cmd
    cmd[2] = 0;                                               // dma_addr
    cmd[3] = DATA_MBOX;                                       // trans_type
    cmd[4] = (sizeof(uint32) * 4) + messageLenAlign + macLen; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = flag;
    data[2] = messageLen;
    data[3] = macLen;
    memcpy((void *)&data[4], (const void *)message, messageLen);
    offset = ((messageLen + 3) / sizeof(uint32));
    memcpy((void *)&data[4 + offset], (const void *)mac, macLen);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

int32 TCCTHSM_CompareMACByDMA(
    uint32 keyIndex, uint32 message, uint32 messageLen, uint8 *mac, uint32 macLen, uint32 flag)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[80] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                 // multibox channel number
    cmd[1] = TCCTHSM_EVT_COMPARE_MAC_BY_DMA; // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = DATA_MBOX;                      // trans_type
    cmd[4] = (sizeof(uint32) * 5) + macLen;  // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = flag;
    data[2] = message;
    data[3] = messageLen;
    data[4] = macLen;
    memcpy((void *)&data[5], (const void *)mac, macLen);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

int32 TCCTHSM_GetRand(uint32 *rng, uint32 rngSize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[32] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    if (rngSize > TCCTHSM_RNG_MAX) {
        ELOG(" Error rngSize(%d) is too big\n", rngSize);
        return TCCTHSM_ERR;
    }

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;       // multibox channel number
    cmd[1] = TCCTHSM_EVT_GET_RAND; // cmd
    cmd[2] = 0;                    // dma_addr
    cmd[3] = DATA_MBOX;            // trans_type
    cmd[4] = sizeof(uint32);       // data_size

    /* Set data FIFO */
    data[0] = rngSize;

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        rngSize = data[1];
        if (rngSize > 0) {
            memcpy((void *)rng, (const void *)&data[2], rngSize);
        } else {
            ELOG("Wrong dstSize(%d)\n", rngSize);
            return TCCTHSM_ERR;
        }
    }

    return ret;
}

int32 TCCTHSM_GenerateKeySS(int8 *objID, uint32 objIDLen, uint32 algorithm, uint32 keySize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[40] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                  // multibox channel number
    cmd[1] = TCCTHSM_EVT_GEN_KEY_SS;          // cmd
    cmd[2] = 0;                               // dma_addr
    cmd[3] = DATA_MBOX;                       // trans_type
    cmd[4] = (sizeof(uint32) * 3) + objIDLen; // data_size

    /* Set data FIFO */
    data[0] = algorithm;
    data[1] = keySize;
    data[2] = objIDLen;
    memcpy((void *)&data[3], (const void *)objID, objIDLen);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

int32 TCCTHSM_DeleteKeySS(int8 *objID, uint32 objIDLen)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[40] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;            // multibox channel number
    cmd[1] = TCCTHSM_EVT_DEL_KEY_SS;    // cmd
    cmd[2] = 0;                         // dma_addr
    cmd[3] = DATA_MBOX;                 // trans_type
    cmd[4] = sizeof(uint32) + objIDLen; // data_size

    /* Set data FIFO */
    data[0] = objIDLen;
    memcpy((void *)&data[1], (const void *)objID, objIDLen);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

int32 TCCTHSM_WriteKeySS(uint8 *objID, uint32 objIDLen, uint8 *buffer, uint32 bufferSize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[40] = {0};
    uint32 dataSize = 0, offset = 0, objIDLenAlign = 0;
    int32 ret = -1;

    /* Set command FIFO */
    objIDLenAlign = ((objIDLen + 3) / sizeof(uint32)) * 4;
    cmd[0] = IPC_APP_ID_HSM;                                    // multibox channel number
    cmd[1] = TCCTHSM_EVT_WRITE_KEY_SS;                          // cmd
    cmd[2] = 0;                                                 // dma_addr
    cmd[3] = DATA_MBOX;                                         // trans_type
    cmd[4] = (sizeof(uint32) * 2) + objIDLenAlign + bufferSize; // data_size

    /* Set data FIFO */
    data[0] = objIDLen;
    data[1] = bufferSize;
    memcpy((void *)&data[2], (const void *)objID, objIDLen);
    offset = ((objIDLen + 3) / sizeof(uint32));
    memcpy((void *)&data[2 + offset], (const void *)buffer, bufferSize);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

int32 TCCTHSM_WriteOTP(uint32 otpAddr, uint32 *otpBuf, uint32 otpSize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[32] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                 // multibox channel number
    cmd[1] = TCCTHSM_EVT_WRITE_OTP;          // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = DATA_MBOX;                      // trans_type
    cmd[4] = (sizeof(uint32) * 2) + otpSize; // data_size

    /* Set data FIFO */
    data[0] = otpAddr;
    data[1] = otpSize;
    memcpy((void *)&data[2], (const void *)otpBuf, otpSize);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];

    return ret;
}

int32 TCCTHSM_WriteOTPImage(uint32 otpAddr, uint32 otpSize)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[32] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;              // multibox channel number
    cmd[1] = TCCTHSM_EVT_WRITE_OTP_IMAGE; // cmd
    cmd[2] = 0;                           // dma_addr
    cmd[3] = DATA_MBOX;                   // trans_type
    cmd[4] = (sizeof(uint32) * 2);        // data_size

    /* Set data FIFO */
    data[0] = otpAddr;
    data[1] = otpSize;

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];

    return ret;
}

int32 TCCTHSM_AsymEncDec(
    uint32 keyIndex, uint8 *srcBuf, uint32 srcSize, uint8 *dstBuf, uint32 *dstSize, uint32 enc)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[128] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    if (srcSize > TCC_MBOX_MAX_MSG) {
        ELOG("Error Size(%d) is too big\n", srcSize);
        return TCCTHSM_ERR;
    }

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM; // multibox channel number
    if (enc == ENCRYPTION) { // cmd
        cmd[1] = TCCTHSM_EVT_ASYMMETRIC_ENC;
    } else
        cmd[1] = TCCTHSM_EVT_ASYMMETRIC_DEC;
    cmd[2] = 0;                              // dma_addr
    cmd[3] = DATA_MBOX;                      // trans_type
    cmd[4] = (sizeof(uint32) * 3) + srcSize; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = srcSize;
    data[2] = *dstSize;
    memcpy((void *)&data[3], (const void *)srcBuf, srcSize);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *dstSize = data[1];
        if (*dstSize > 0) {
            memcpy((void *)dstBuf, (const void *)&data[2], *dstSize);
        } else {
            ELOG("Wrong dstSize(%d)\n", *dstSize);
            return TCCTHSM_ERR;
        }
    }

    return ret;
}

int32 TCCTHSM_AsymEncDecByDMA(
    uint32 keyIndex, uint32 srcBuf, uint32 srcSize, uint32 dstBuf, uint32 *dstSize, uint32 enc)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[32] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM; // multibox channel number
    if (enc == ENCRYPTION) { // cmd
        cmd[1] = TCCTHSM_EVT_ASYMMETRIC_ENC_BY_DMA;
    } else
        cmd[1] = TCCTHSM_EVT_ASYMMETRIC_DEC_BY_DMA;
    cmd[2] = 0;                    // dma_addr
    cmd[3] = DATA_MBOX;            // trans_type
    cmd[4] = (sizeof(uint32) * 4); // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = srcBuf;
    data[2] = srcSize;
    data[3] = dstBuf;
    data[4] = *dstSize;

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *dstSize = data[1];
    }

    return ret;
}

int32 TCCTHSM_AsymSignDigest(
    uint32 keyIndex, uint8 *dig, uint32 digSize, uint8 *sig, uint32 *sigLen)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[80] = {0};
    uint32 dataSize = 0;
    int32 ret = -1;

    if (digSize > TCC_MBOX_MAX_MSG) {
        ELOG(" Error Size(%d) is too big\n", digSize);
        return TCCTHSM_ERR;
    }

    /* Set command FIFO */
    cmd[0] = IPC_APP_ID_HSM;                 // multibox channel number
    cmd[1] = TCCTHSM_EVT_ASYMMETRIC_SIGN;    // cmd
    cmd[2] = 0;                              // dma_addr
    cmd[3] = DATA_MBOX;                      // trans_type
    cmd[4] = (sizeof(uint32) * 3) + digSize; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = digSize;
    data[2] = *sigLen;
    memcpy((void *)&data[3], (const void *)dig, digSize);
    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    if (ret == TCCTHSM_OK) {
        *sigLen = data[1];
        if (*sigLen > 0) {
            memcpy((void *)sig, (const void *)&data[2], *sigLen);
        } else {
            ELOG("Wrong sig len(%d)\n", *sigLen);
            return TCCTHSM_ERR;
        }
    }

    return ret;
}

int32 TCCTHSM_AsymVerifyDigest(
    uint32 keyIndex, uint8 *dig, uint32 digSize, uint8 *sig, uint32 sigLen)
{
    uint32 cmd[TCCTHSM_CMD_FIFO_SIZE] = {0};
    uint32 data[160] = {0};
    uint32 dataSize = 0, offset = 0, digSizeAlign;
    int32 ret = -1;

    if ((digSize + sigLen) > TCC_MBOX_MAX_MSG) {
        ELOG("Error Size is too big %d %d\n", digSize, sigLen);
        return TCCTHSM_ERR;
    }

    /* Set command FIFO */
    digSizeAlign = ((digSize + 3) / sizeof(uint32)) * 4;
    cmd[0] = IPC_APP_ID_HSM;                               // multibox channel number
    cmd[1] = TCCTHSM_EVT_ASYMMETRIC_VERIFY;                // cmd
    cmd[2] = 0;                                            // dma_addr
    cmd[3] = DATA_MBOX;                                    // trans_type
    cmd[4] = (sizeof(uint32) * 3) + digSizeAlign + sigLen; // data_size

    /* Set data FIFO */
    data[0] = keyIndex;
    data[1] = digSize;
    data[2] = sigLen;
    memcpy((void *)&data[3], (const void *)dig, digSize);
    offset = ((digSize + 3) / sizeof(uint32));
    memcpy((void *)&data[3 + offset], (const void *)sig, sigLen);

    dataSize = cmd[4];

    ret = TCCTHSM_WrtieMbox(
        hHSM, IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&data[0], (void *)&dataSize, 0);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Write(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = TCCTHSM_ReadMbox((uint8 *)data, &dataSize);
    if (ret != SAL_RET_SUCCESS) {
        ELOG("Error HAL_Read(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    ret = data[0];
    return ret;
}

#ifdef TCCTHSM_TEST_ENABLE
#include "CryptoDataA53.h"

#define MICOM_SRAM1_BASEADDR 0xC1000000
#define SRC_OFFSET 0x00000000
#define DST_OFFSET 0x00008000

#ifdef DMA_TEST_ENABLE
uint32 srcPhy = MICOM_SRAM1_BASEADDR + SRC_OFFSET;
uint32 dstPhy = MICOM_SRAM1_BASEADDR + DST_OFFSET;
#endif

static void HexDump(const void *data, size_t size)
{
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        mcu_printf("%02X ", ((unsigned char *)data)[i]);
        if (((unsigned char *)data)[i] >= ' ' && ((unsigned char *)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char *)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            mcu_printf(" ");
            if ((i + 1) % 16 == 0) {
                mcu_printf("|  %s \n", ascii);
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    mcu_printf(" ");
                }
                for (j = (i + 1) % 16; j < 16; ++j) {
                    mcu_printf("   ");
                }
                mcu_printf("|  %s \n", ascii);
            }
        }
    }
    mcu_printf("\n");
}

int32 TCCTHSM_AES_Test(void)
{
    int32 ret = 0;
    uint32 algorithm = 0, mode = 0, keySize = 0, ivSize = 0, inputSize = 0, outputSize = 0;
    uint32 keyIndex = 0;
    uint8 dstBuf[16] = {0};
    /* Set AES-128 ECB Mode Operation */
    algorithm = TEETHSM_ALG_AES_ECB_NOPAD;
    mode = TEETHSM_MODE_ENCRYPT;
    keySize = AES128_MAX_KEYSIZE;
    ivSize = 0;
    inputSize = sizeof(ECB_plain);

    /* Run Cipther */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKey(keyIndex, (uint32 *)ECB_key, keySize, NULL, 0, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetIVSymmetric(keyIndex, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)ECB_plain, inputSize);
    TCCTHSM_RunCipherByDMA(
        keyIndex, srcPhy, inputSize, dstPhy, &outputSize, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
    if (memcmp((const void *)dstPhy, (const void *)ECB_cipher, outputSize) != 0) {
        DLOG("Expected data(0x%x) \n", outputSize);
        HexDump(ECB_cipher, outputSize);
        DLOG("received data \n");
        HexDump((uint8 *)dstPhy, outputSize);
        DLOG("Fail AES128 ECB Encryption \n");
        return TCCTHSM_ERR;
    }
#else
    TCCTHSM_RunCipher(
        keyIndex, ECB_plain, inputSize, dstBuf, &outputSize, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
    if (memcmp((const void *)dstBuf, (const void *)ECB_cipher, outputSize) != 0) {
        DLOG("Expected data(0x%x) \n", outputSize);
        HexDump(ECB_cipher, outputSize);
        DLOG("received data \n");
        HexDump(dstBuf, outputSize);
        DLOG("Fail AES128 ECB Encryption \n");
        return TCCTHSM_ERR;
    }
#endif

    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    /* Set AES-128 CBC Mode Operation */
    algorithm = TEETHSM_ALG_AES_CBC_NOPAD;
    mode = TEETHSM_MODE_DECRYPT;
    keySize = AES128_MAX_KEYSIZE;
    ivSize = 16;
    inputSize = sizeof(AES_CBC_128KEYCIPHERTEXT);

    /* Run Cipther */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKey(keyIndex, (uint32 *)AES_CBC_128KEY, keySize, NULL, 0, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetIVSymmetric(keyIndex, (uint32 *)AES_CBC_ucInitVector, ivSize))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)AES_CBC_128KEYCIPHERTEXT, inputSize);
    TCCTHSM_RunCipherByDMA(
        keyIndex, srcPhy, inputSize, dstPhy, &outputSize, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
    if (memcmp((const void *)dstPhy, (const void *)AES_CBC_PLAINTEXT, outputSize) != 0) {
        DLOG("Expected data(0x%x) \n", outputSize);
        HexDump(AES_CBC_PLAINTEXT, outputSize);
        DLOG("received data \n");
        HexDump((uint8 *)dstPhy, outputSize);
        ELOG("Fail AES128 CBC Decryption \n");
        return TCCTHSM_ERR;
    }
#else
    TCCTHSM_RunCipher(
        keyIndex, AES_CBC_128KEYCIPHERTEXT, inputSize, dstBuf, &outputSize,
        ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
    if (memcmp((const void *)dstBuf, (const void *)AES_CBC_PLAINTEXT, outputSize) != 0) {
        DLOG("Expected data(0x%x) \n", outputSize);
        HexDump(AES_CBC_PLAINTEXT, outputSize);
        DLOG("received data \n");
        HexDump(dstBuf, outputSize);
        ELOG("Fail AES128 CBC Decryption \n");
        return TCCTHSM_ERR;
    }
#endif

    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    DLOG("Success AES test\n");
    return TCCTHSM_OK;
}

int32 TCCTHSM_DES_Test(void)
{
    int32 ret = 0;
    uint32 algorithm = 0, mode = 0, keySize = 0, ivSize = 0, inputSize = 0, outputSize = 0;
    uint32 keyIndex = 0;
    uint8 dstBuf[24] = {0};

    /* Set DES ECB Mode Operation */
    algorithm = TEETHSM_ALG_DES_ECB_NOPAD;
    mode = TEETHSM_MODE_ENCRYPT;
    keySize = DES_MAX_KEYSIZE;
    ivSize = 0;
    inputSize = sizeof(DES_ECB_PLAINTEXT);

    /* Run Cipther */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKey(keyIndex, (uint32 *)DES_ECB_KEY, keySize, NULL, 0, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetIVSymmetric(keyIndex, NULL, ivSize))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)DES_ECB_PLAINTEXT, inputSize);
    TCCTHSM_RunCipherByDMA(
        keyIndex, srcPhy, inputSize, dstPhy, &outputSize, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
    if (memcmp((const void *)dstPhy, (const void *)DES_ECB_CIPHERTEXT, outputSize) != 0) {
        DLOG("Expected data(0x%x) \n", outputSize);
        HexDump((uint8 *)DES_ECB_CIPHERTEXT, outputSize);
        DLOG("received data \n");
        HexDump((uint8 *)dstPhy, outputSize);
        ELOG("Fail DES Encryption \n");
        return TCCTHSM_ERR;
    }
#else
    TCCTHSM_RunCipher(
        keyIndex, DES_ECB_PLAINTEXT, inputSize, dstBuf, &outputSize,
        ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
    if (memcmp((const void *)dstBuf, (const void *)DES_ECB_CIPHERTEXT, outputSize) != 0) {
        DLOG("Expected data(0x%x) \n", outputSize);
        HexDump(DES_ECB_CIPHERTEXT, outputSize);
        DLOG("received data \n");
        HexDump(dstBuf, outputSize);
        ELOG("Fail DES Encryption \n");
        return TCCTHSM_ERR;
    }
#endif
    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    DLOG("Success DES test\n");
    return TCCTHSM_OK;
}

int32 TCCTHSM_TDES_Test(void)
{
    int32 ret = 0;
    uint32 algorithm = 0, mode = 0, keySize = 0, ivSize = 0, inputSize = 0, outputSize = 0;
    uint32 keyIndex = 0;
    uint8 dstBuf[32] = {0};

    /* Set TDES ECB Mode Operation */
    algorithm = TEETHSM_ALG_DES3_CBC_NOPAD;
    mode = TEETHSM_MODE_DECRYPT;
    keySize = 24;
    ivSize = 8;
    inputSize = sizeof(TDES_3KEY_CBCCIPHERTEXT);

    /* Run Cipther */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKey(keyIndex, (uint32 *)TDES_CBCKEY, keySize, NULL, 0, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetIVSymmetric(keyIndex, (uint32 *)TDES_CBC_ucInitVector, ivSize))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)TDES_3KEY_CBCCIPHERTEXT, inputSize);
    TCCTHSM_RunCipherByDMA(
        keyIndex, srcPhy, inputSize, dstPhy, &outputSize, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
    if (memcmp((const void *)dstPhy, (const void *)TDES_CBCPLAINTEXT, outputSize) != 0) {
        DLOG("Expected data(0x%x) \n", outputSize);
        HexDump(TDES_CBCPLAINTEXT, outputSize);
        DLOG("received data \n");
        HexDump((uint8 *)dstPhy, outputSize);
        ELOG("Fail TDES Encryption \n");
        return TCCTHSM_ERR;
    }
#else
    TCCTHSM_RunCipher(
        keyIndex, TDES_3KEY_CBCCIPHERTEXT, inputSize, dstBuf, &outputSize,
        ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
    if (memcmp((const void *)dstBuf, (const void *)TDES_CBCPLAINTEXT, outputSize) != 0) {
        DLOG("Expected data(0x%x) \n", outputSize);
        HexDump(TDES_CBCPLAINTEXT, outputSize);
        DLOG("received data \n");
        HexDump(dstBuf, outputSize);
        ELOG("Fail TDES Encryption \n");
        return TCCTHSM_ERR;
    }
#endif
    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    DLOG("Success TDES test\n");
    return TCCTHSM_OK;
}

int32 TCCTHSM_SHA_Test(void)
{
    int32 ret = 0;
    uint32 algorithm = 0, mode = 0, keySize = 0, inputSize = 0;
    uint32 keyIndex = 0, hashLen = 0;
    uint8 dstBuf[48] = {0};

    /* Set SHA1 Mode Operation */
    algorithm = TEETHSM_ALG_SHA1;
    mode = TEETHSM_MODE_DIGEST;
    keySize = 0;
    inputSize = sizeof(msg_sha1);
    hashLen = sizeof(result_sha1);

    /* Run SHA1 */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)msg_sha1, inputSize);
    ret = TCCTHSM_RunDigestByDMA(
        keyIndex, srcPhy, inputSize, dstBuf, &hashLen, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
#else
    ret = TCCTHSM_RunDigest(
        keyIndex, msg_sha1, inputSize, dstBuf, &hashLen, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
#endif
    if (ret == TCCTHSM_OK) {
        if (memcmp((const void *)dstBuf, (const void *)result_sha1, hashLen) != 0) {
            DLOG("Expected data(0x%x) \n", hashLen);
            HexDump(result_sha1, hashLen);
            DLOG("received data \n");
            HexDump(dstBuf, hashLen);
            ELOG("Fail SHA256 \n");
            return TCCTHSM_ERR;
        }
    } else {
        ELOG("TCCTHSM_RunDigest Err(%d) hashLen=0x%x\n", ret, hashLen);
        return TCCTHSM_ERR;
    }

    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    /* Set SHA256 Mode Operation */
    algorithm = TEETHSM_ALG_SHA256;
    mode = TEETHSM_MODE_DIGEST;
    keySize = 0;
    inputSize = sizeof(msg_sha256);
    hashLen = sizeof(result_sha256);

    /* Run SHA256 */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)msg_sha256, inputSize);
    ret = TCCTHSM_RunDigestByDMA(
        keyIndex, srcPhy, inputSize, dstBuf, &hashLen, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
#else
    ret = TCCTHSM_RunDigest(
        keyIndex, msg_sha256, inputSize, dstBuf, &hashLen, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
#endif
    if (ret == TCCTHSM_OK) {
        if (memcmp((const void *)dstBuf, (const void *)result_sha256, hashLen) != 0) {
            DLOG("Expected data(0x%x) \n", hashLen);
            HexDump(result_sha256, hashLen);
            DLOG("received data \n");
            HexDump(dstBuf, hashLen);
            ELOG("Fail SHA256 \n");
            return TCCTHSM_ERR;
        }
    } else {
        ELOG("TCCTHSM_RunDigest Err(%d) hashLen=0x%x\n", ret, hashLen);
        return TCCTHSM_ERR;
    }

    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    DLOG("Success SHA test\n");
    return TCCTHSM_OK;
}

int32 TCCTHSM_Rand_Test(void)
{
    uint32 rng[4] = {0};
    uint32 rngSize = 16;

    TCCTHSM_GetRand(rng, rngSize);

    DLOG("%x %x %x %x\n", rng[0], rng[1], rng[2], rng[3]);

    return TCCTHSM_OK;
}

int32 TCCTHSM_MAC_Test(void)
{
    int32 ret = 0, algorithm = 0, mode = 0, keySize = 0;
    uint32 inputSize = 0, keyIndex = 0, macLen = 0;
    uint8 dstBuf[64] = {0};

    /* Set CMAC Mode Operation */
    algorithm = TEETHSM_ALG_AES_CMAC;
    mode = TEETHSM_MODE_MAC;
    keySize = AES128_MAX_KEYSIZE;
    inputSize = sizeof(msg_cmac);
    macLen = 16;

    /* Run CMAC */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKey(keyIndex, (uint32 *)key_cmac, keySize, NULL, 0, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetIVMAC(keyIndex, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)msg_cmac, inputSize);
    ret = TCCTHSM_ComputeMACByDMA(
        keyIndex, srcPhy, inputSize, dstBuf, &macLen, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
#else
    ret = TCCTHSM_ComputeMAC(
        keyIndex, msg_cmac, inputSize, dstBuf, &macLen, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
#endif
    if (ret == TCCTHSM_OK) {
        if (memcmp((const void *)dstBuf, (const void *)result_cmac, macLen) != 0) {
            DLOG("Expected data(0x%x) \n", macLen);
            HexDump(result_cmac, macLen);
            DLOG("received data \n");
            HexDump(dstBuf, macLen);
            ELOG("Fail CMAC Test \n");
            return TCCTHSM_ERR;
        }
    } else {
        ELOG("TCCTHSM_ComputeMAC Err(0x%x) mac_len=0x%x\n", ret, macLen);
        return TCCTHSM_ERR;
    }

    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    /* Run TCCTHSM_CompareMAC */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKey(keyIndex, (uint32 *)key_cmac, keySize, NULL, 0, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetIVMAC(keyIndex, NULL, 0))) {
        ELOG("TCCHSM_SetKey Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)msg_cmac, inputSize);
    ret = TCCTHSM_CompareMACByDMA(
        keyIndex, srcPhy, inputSize, dstBuf, macLen, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
#else
    ret = TCCTHSM_CompareMAC(
        keyIndex, msg_cmac, inputSize, dstBuf, macLen, ACCMU_FLAG_FIRST | ACCMU_FLAG_LAST);
#endif
    if (ret != TCCTHSM_OK) {
        ELOG("TCCTHSM_CompareMAC Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    DLOG("Success CMAC test\n");
    return TCCTHSM_OK;
}

int32 TCCTHSM_RSA_Test(void)
{
    int32 ret = -1, algorithm = 0, mode = 0, keySize = 0;
    uint32 inputSize = 0, keyIndex = 0;
    uint32 outLen = 0, objIDLen = 0;
    uint8 dstBuf[64] = {0};
    uint8 dstOutBuf[64] = {0};

    keySize = 8 * 8;
    algorithm = TEETHSM_ALG_RSAES_PKCS1_V1_5;
    objIDLen = strlen((const int8 *)gObjID);
    if ((ret = TCCTHSM_GenerateKeySS((int8 *)gObjID, objIDLen, algorithm, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    /* Set RSA Enc Operation */
    algorithm = TEETHSM_ALG_RSAES_PKCS1_V1_5; // TEETHSM_ALG_RSA_NOPAD
    mode = TEETHSM_MODE_ENCRYPT;
    keySize = 8 * 8; // 8*16;
    inputSize = sizeof(RSA_plain);
    outLen = sizeof(dstBuf);

    /* Run Encryption RSA */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKeyFromStorage(keyIndex, (int8 *)gObjID, objIDLen))) {
        ELOG("SetKeyFromStorage Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    memcpy((void *)srcPhy, (const void *)RSA_plain, inputSize);
    if (TCCTHSM_AsymEncDecByDMA(keyIndex, srcPhy, inputSize, dstPhy, &outLen, ENCRYPTION)) {
        ELOG("Run RSA Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#else
    if ((ret = TCCTHSM_AsymEncDec(keyIndex, RSA_plain, inputSize, dstBuf, &outLen, ENCRYPTION))) {
        ELOG("Run RSA Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#endif
    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    /* Set RSA Dec Operation */
    algorithm = TEETHSM_ALG_RSAES_PKCS1_V1_5; // TEETHSM_ALG_RSA_NOPAD
    mode = TEETHSM_MODE_DECRYPT;
    keySize = 8 * 8; // 8*16;
    inputSize = outLen;

    /* Run Decryption RSA */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKeyFromStorage(keyIndex, (int8 *)gObjID, objIDLen))) {
        ELOG("SetKeyFromStorage Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
#ifdef DMA_TEST_ENABLE
    if (TCCTHSM_AsymEncDecByDMA(keyIndex, dstPhy, inputSize, srcPhy, &outLen, DECRYPTION)) {
        ELOG("Run RSA Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if (memcmp((const void *)srcPhy, (const void *)dstPhy, outLen) != 0) {
        DLOG("Expected data(0x%x) \n", outLen);
        HexDump((uint8 *)srcPhy, outLen);
        DLOG("received data \n");
        HexDump((uint8 *)dstPhy, outLen);
        ELOG("Fail Run RSA \n");
        return TCCTHSM_ERR;
    }
#else
    if ((ret = TCCTHSM_AsymEncDec(keyIndex, dstBuf, inputSize, dstOutBuf, &outLen, DECRYPTION))) {
        ELOG("Run RSA Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if (memcmp((const void *)dstOutBuf, (const void *)RSA_plain, outLen) != 0) {
        DLOG("Expected data(0x%x) \n", outLen);
        HexDump(RSA_plain, outLen);
        DLOG("received data \n");
        HexDump(dstOutBuf, outLen);
        ELOG("Fail Run RSA \n");
        return TCCTHSM_ERR;
    }
#endif
    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_DeleteKeySS((int8 *)gObjID, objIDLen))) {
        ELOG("DeleteKeySS Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    DLOG("Success RSA test\n");
    return TCCTHSM_OK;
}

int32 TCCTHSM_OTP_Test(void)
{
    uint32 otpAddr = 0x00000004;
    uint32 otpSize = 16;

    TCCTHSM_WriteOTP(otpAddr, (uint32 *)key_aes128, otpSize);

    return TCCTHSM_OK;
}

int32 TCCTHSM_ECDSA_Test(void)
{
    int32 ret = -1, algorithm = 0, mode = 0, keySize = 0;
    uint32 digSize = 0, keyIndex = 0;
    uint32 objIDLen = 0, sigLen = 0;
    uint8 dig[32] = {0}, sig[64] = {0};

    algorithm = TEETHSM_ALG_ECDSA_P256;
    keySize = 256 / 8; // 256bit(P256) / 8 = 32byte
    objIDLen = strlen((const int8 *)gObjID);
    if ((ret = TCCTHSM_GenerateKeySS((int8 *)gObjID, objIDLen, algorithm, keySize))) {
        ELOG("TCCTHSM_GenerateKeySS Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    /* Set ECDSA Operation */
    mode = TEETHSM_MODE_SIGN;
    digSize = sizeof(ECDSA_dig_P256); // 256bit / 8 = 32byte
    sigLen = sizeof(sig);

    memcpy((void *)dig, (const void *)ECDSA_dig_P256, digSize);

    /* Run ECDSA Signing */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKeyFromStorage(keyIndex, (int8 *)gObjID, objIDLen))) {
        ELOG("SetKeyFromStorage Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_AsymSignDigest(keyIndex, dig, digSize, sig, &sigLen))) {
        ELOG("Run RSA Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    /* Set ECDSA Operation */
    mode = TEETHSM_MODE_VERIFY;

    /* Run Decryption RSA */
    if ((ret = TCCTHSM_SetMode(keyIndex, algorithm, mode, keySize))) {
        ELOG("TCCTHSM_SetMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_SetKeyFromStorage(keyIndex, (int8 *)gObjID, objIDLen))) {
        ELOG("SetKeyFromStorage Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_AsymVerifyDigest(keyIndex, dig, digSize, sig, sigLen))) {
        ELOG("Run ECDSA Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    if ((ret = TCCTHSM_FreeMode(keyIndex))) {
        ELOG("TCCTHSM_FreeMode Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }
    if ((ret = TCCTHSM_DeleteKeySS((int8 *)gObjID, objIDLen))) {
        ELOG("DeleteKeySS Err(%d)\n", ret);
        return TCCTHSM_ERR;
    }

    DLOG("Success ECDSA test\n");
    return TCCTHSM_OK;
}

void TCCTHSM_Full_Test(int cnt)
{
    int i;
    int32 ret = TCCTHSM_OK;

    for (i = 0; i < cnt; i++) {
        ret |= TCCTHSM_AES_Test();
        ret |= TCCTHSM_DES_Test();
        ret |= TCCTHSM_TDES_Test();
        ret |= TCCTHSM_SHA_Test();
        ret |= TCCTHSM_Rand_Test();
        ret |= TCCTHSM_MAC_Test();
        ret |= TCCTHSM_RSA_Test();
        ret |= TCCTHSM_ECDSA_Test();
        if (ret != TCCTHSM_OK) {
            DLOG(" Fail test(%d, %d)\n", i, cnt);
            return;
        }
    }

    DLOG("Success test(%d, %d)\n", i, cnt);

    return;
}

int32 TCCTHSM_Test(uint8 *cmd)
{
    sint32 ret;

    TCCTHSM_Open();

    DLOG(" cmd:%s\n", cmd);

    if ((SAL_StrCmp(cmd, (const uint8 *)"aes", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_AES_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"des", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_DES_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"tdes", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_TDES_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"sha", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_SHA_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"rand", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_Rand_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"mac", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_MAC_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"rsa", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_RSA_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"otp", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_OTP_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"ecdsa", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_ECDSA_Test();
    } else if (SAL_StrCmp(cmd, (const uint8 *)"full", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_Full_Test(1);
    } else if (SAL_StrCmp(cmd, (const uint8 *)"aging", &ret) == SAL_RET_SUCCESS) && (ret == 0)) {
        TCCTHSM_Full_Test(10000);
    } else {
        DLOG("Error invalid command! \n");
        DLOG("aes  \n");
        DLOG("des  \n");
        DLOG("tdes  \n");
        DLOG("sha  \n");
        DLOG("rand  \n");
        DLOG("mac  \n");
        DLOG("rsa  \n");
        DLOG("ecdsa  \n");
        DLOG("full  \n");
        DLOG("aging  \n");
    }
    return TCCTHSM_OK;
}

#else // #ifdef TCCTHSM_TEST_ENABLE
int32 TCCTHSM_Test(uint8 *cmd)
{
    DLOG("%s is not supported\n", cmd);
    return TCCTHSM_ERR_UNSUPPORT_FUNC;
}
#endif

#else // #if (SERVICE_HSMMANAGER_EN == 1u)
void TCCTHSM_ReceiveMboxData(uint8* buffer, uint32 size)
{
    return;
}
#endif

