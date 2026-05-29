/*
***************************************************************************************************
*
*   FileName : hsm_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : HSM TEST
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

#include <debug.h>
#include <stdlib.h>
#include <sal_internal.h>

#include "hsm_test.h"
#include "hsm_manager.h"

// clang-format off
/* Common */
static uint8 plain_data[32] = {
        0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08,
        0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08
};
static uint8 key[16] = {
        0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04
};

static uint8 iv[12] = {
        0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78
};
static uint8 aes_iv[16] = {
        0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78
};

/* AES Test */
static uint8 aes_ecb_cipher[32] = {
        0xC0, 0x3D, 0x4E, 0x0C, 0x09, 0x11, 0x34, 0x21, 0x04, 0x78, 0x3B, 0x6A, 0xB6, 0xD2, 0x90, 0x59,
        0xC0, 0x3D, 0x4E, 0x0C, 0x09, 0x11, 0x34, 0x21, 0x04, 0x78, 0x3B, 0x6A, 0xB6, 0xD2, 0x90, 0x59
};

static uint8 aes_ccm_cipher[32] = {
        0xee, 0xe5, 0x0b, 0x43, 0x98, 0x95, 0x27, 0x56, 0x57, 0xad, 0x0e, 0x3f, 0xb7, 0x79, 0x34, 0x15,
        0xa4, 0xc0, 0xda, 0xac, 0x50, 0xbf, 0xe4, 0x4e, 0xde, 0x2b, 0x72, 0xfd, 0x91, 0x4f, 0xde, 0xc3
};

static uint8 aes_gcm_cipher[32] = {
        0x16, 0x29, 0xb1, 0x8b, 0x2d, 0x06, 0x35, 0xe5, 0x48, 0x9d, 0xcd, 0xda, 0x52, 0xdb, 0xe9, 0xfd,
        0x46, 0xea, 0xc7, 0xf1, 0x47, 0x82, 0xe4, 0x83, 0x18, 0x69, 0x66, 0x97, 0x47, 0xd6, 0x7d, 0x47
};

static uint8 aes_ccm_tag[16] = {
        0x46, 0x41, 0xdd, 0x73, 0x09, 0x0b, 0x1f, 0xb8, 0xdb, 0xe4, 0x33, 0xd7, 0xcb, 0xed, 0xcf, 0x4b
};

static uint8 aes_gcm_tag[16] = {
        0x50, 0xda, 0xfe, 0xb3, 0x0f, 0xc1, 0x12, 0x08, 0xed, 0x5c, 0x13, 0x6d, 0x42, 0x4b, 0xb0, 0x35
};

static uint8 aes_aad[16] = {
        0x89, 0xAB, 0xCD, 0xEF, 0x89, 0xAB, 0xCD, 0xEF, 0x89, 0xAB, 0xCD, 0xEF, 0x89, 0xAB, 0xCD, 0xEF
};

static uint8 sm4_ecb_cipher[32] = {
        0x6D, 0x50, 0x86, 0x74, 0x95, 0xA4, 0xE5, 0x91, 0x62, 0x6E, 0xD5, 0x7A, 0x52, 0xE0, 0xD6, 0x83,
        0x6D, 0x50, 0x86, 0x74, 0x95, 0xA4, 0xE5, 0x91, 0x62, 0x6E, 0xD5, 0x7A, 0x52, 0xE0, 0xD6, 0x83
};

/* MAC Test */
static uint8 mac_key[32] = {
        0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04,
        0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04
};
static uint8 cmac_out[16] = {
        0xfb, 0x7c, 0xf6, 0x21, 0xd0, 0xc4, 0x86, 0x50, 0x6a, 0x1d, 0x4e, 0x29, 0x8a, 0xe5, 0xe0, 0xe3
};
static uint8 hmac_out[20] = {
        0x6c, 0x91, 0xbb, 0x89, 0x6f, 0xbb, 0x8b, 0x23, 0x5c, 0xd3, 0x42, 0xd5, 0x12, 0xa2, 0x6f, 0x4e,
        0xf6, 0x91, 0x94, 0x45
};
static uint8 sm3_hmac_out[32] = {
        0x1E, 0x33, 0xEE, 0x7C, 0xFD, 0x7B, 0x24, 0xE8, 0xFE, 0x43, 0xFC, 0x34, 0xED, 0xCA, 0xB5, 0x42,
        0xE4, 0x4D, 0x71, 0x49, 0xE8, 0xC7, 0x25, 0x87, 0xFB, 0xF5, 0x2D, 0x6D, 0xE0, 0xDC, 0x9E, 0x8C
};

/* RSASSA Test */
static uint8 mod_n[128] = {
    0xB6,0x74,0x4D,0x09, 0x45,0xCE,0x41,0xC7, 0xD1,0x6A,0x09,0xD8, 0xF2,0xE4,0xED,0x86,
    0xD8,0x6C,0x9F,0x95, 0xF4,0xEB,0x77,0x8F, 0x98,0x51,0x10,0x58, 0x54,0xAD,0xA5,0x18,
    0xAB,0xFE,0x6D,0x34, 0x21,0xD0,0x1D,0xA5, 0x91,0x69,0xDD,0x37, 0x7F,0x08,0x8B,0xEC,
    0xB3,0xF7,0xD6,0x7B, 0xFA,0xB9,0xF7,0x0E, 0x5E,0x87,0x9C,0xC4, 0xD2,0x77,0x7C,0xD1,
    0x4D,0x8A,0x24,0x1C, 0x31,0xC3,0x67,0x25, 0x71,0xF2,0x57,0xB9, 0xB2,0xE8,0xE6,0xB6,
    0xF3,0xA1,0x9A,0x10, 0xD4,0xA9,0xC9,0xB4, 0x83,0x46,0xA5,0x29, 0xC9,0xF1,0x03,0x87,
    0xCD,0xA4,0xEF,0x16, 0xBC,0x3E,0x1C,0x28, 0xAC,0x24,0x3E,0xEC, 0x40,0x2E,0x24,0x8C,
    0x58,0x88,0x3C,0x7B, 0x90,0x67,0xE6,0x93, 0x07,0xEA,0x54,0x5B, 0x0A,0x1F,0x30,0x33
};
static uint8 rsa_pri_key[128] = {
    0x44,0x17,0xF5,0xA4, 0x4F,0x4C,0x22,0x1A, 0xBC,0x1A,0x52,0x20, 0x88,0xCE,0x4D,0xA3,
    0xB4,0xE4,0x1D,0x4F, 0x2F,0x00,0x2E,0x38, 0x06,0x45,0xC2,0xCE, 0xE4,0x93,0x85,0xF9,
    0x46,0x3F,0x87,0x6C, 0x1C,0x6E,0x7A,0x92, 0x0F,0x4A,0x2E,0x48, 0x86,0xFD,0x9E,0x93,
    0x31,0xBB,0x7E,0x6D, 0xF8,0xEE,0x2C,0xB9, 0x4D,0x28,0x59,0xA6, 0xCF,0x51,0xCE,0xE0,
    0x00,0x45,0xF3,0x11, 0x9F,0xEF,0xD2,0x72, 0x05,0x21,0x2D,0x81, 0x7D,0xF7,0xB4,0x64,
    0xE2,0xE2,0xF1,0xF0, 0x25,0x48,0x3E,0x5F, 0xF8,0xCC,0x9E,0x09, 0x61,0xA3,0x8D,0x34,
    0x83,0x16,0x6D,0x53, 0xDC,0xE4,0x33,0x8B, 0x8E,0x32,0x0E,0x85, 0xE8,0xF0,0xE4,0xF7,
    0xDF,0x5B,0x8E,0x60, 0x8B,0x3E,0xB4,0xDE, 0xC3,0x45,0x73,0x61, 0xAD,0xC9,0x97,0xD9
};
static uint8 rsa_pub_key[4] = {
    0x00,0x01,0x00,0x01
};
static uint8 digest[32] = {
    0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1A,0x1B, 0x1C,0x1D,0x1E,0x1F
};

/* ECDSA Test */
static uint8 gzu08rsa_n[128] = {
    0xB6,0x74,0x4D,0x09, 0x45,0xCE,0x41,0xC7, 0xD1,0x6A,0x09,0xD8, 0xF2,0xE4,0xED,0x86,
    0xD8,0x6C,0x9F,0x95, 0xF4,0xEB,0x77,0x8F, 0x98,0x51,0x10,0x58, 0x54,0xAD,0xA5,0x18,
    0xAB,0xFE,0x6D,0x34, 0x21,0xD0,0x1D,0xA5, 0x91,0x69,0xDD,0x37, 0x7F,0x08,0x8B,0xEC,
    0xB3,0xF7,0xD6,0x7B, 0xFA,0xB9,0xF7,0x0E, 0x5E,0x87,0x9C,0xC4, 0xD2,0x77,0x7C,0xD1,
    0x4D,0x8A,0x24,0x1C, 0x31,0xC3,0x67,0x25, 0x71,0xF2,0x57,0xB9, 0xB2,0xE8,0xE6,0xB6,
    0xF3,0xA1,0x9A,0x10, 0xD4,0xA9,0xC9,0xB4, 0x83,0x46,0xA5,0x29, 0xC9,0xF1,0x03,0x87,
    0xCD,0xA4,0xEF,0x16, 0xBC,0x3E,0x1C,0x28, 0xAC,0x24,0x3E,0xEC, 0x40,0x2E,0x24,0x8C,
    0x58,0x88,0x3C,0x7B, 0x90,0x67,0xE6,0x93, 0x07,0xEA,0x54,0x5B, 0x0A,0x1F,0x30,0x33
};
static uint8 gzu08ecc_a_pub[64] = {
    0x80,0x90,0xFB,0xFD, 0x65,0x6E,0x74,0xB4, 0xBC,0x0C,0x6E,0xD6, 0x28,0x4A,0xA5,0x0E,
    0x8B,0x31,0x7A,0xBE, 0x6E,0xD8,0xC5,0xA1, 0x8C,0x18,0xDA,0x62, 0xB1,0x4F,0xB9,0x7E,
    0x98,0x64,0x5F,0xAF, 0x3A,0x87,0xB0,0xF8, 0x24,0xA7,0xE4,0x3E, 0x20,0xFC,0xCD,0x3C,
    0x3F,0xED,0xD4,0xD0, 0xAF,0x98,0xDA,0xD3, 0xC7,0x17,0xCB,0xA4, 0x20,0x56,0x97,0x1E
};
static uint8 gzu08_digest[32] = {
    0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1A,0x1B, 0x1C,0x1D,0x1E,0x1F
};

/* HASH Test */
static uint8 digest_sha1_160[20] = {
    0x45, 0x28, 0xde, 0xce, 0x6f, 0x68, 0x0e, 0xc6, 0x59, 0x0e, 0x36, 0x6f, 0x59, 0x63,
    0x43, 0x8d, 0xcf, 0xce, 0x34, 0x92
};
static uint8 digest_sm3_256[32] = {
    0x0A, 0x06, 0xFA, 0x67, 0x63, 0x29, 0x6B, 0xD7, 0x8C, 0x32, 0xB7, 0x4B, 0xCC, 0xEA,
    0x1F, 0x9C, 0x1E, 0x37, 0x2E, 0xD6, 0x3D, 0x67, 0x45, 0xBD, 0xB8, 0xE1, 0x55, 0x0C,
    0x38, 0xEB, 0xD8, 0x7A
};
static uint8 hash_message[32] = {
    0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06,
    0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08, 0x05, 0x06, 0x07, 0x08,
    0x05, 0x06, 0x07, 0x08
};
// clang-format on

static const int8 *hsm_cmd[] = {"0 set key from otp",
                                "1 set key from snor",
                                "2 run aes",
                                "3 run aes by kt",
                                "4 run sm4",
                                "5 run sm4 by kt",
                                "6 gen cmac",
                                "7 gen cmac by kt",
                                "8 gen gmac",
                                "9 gen gmac by kt",
                                "10 gen hmac",
                                "11 gen hmac by kt",
                                "12 gen sm3 hmac",
                                "13 gen sm3 hmac by kt",
                                "14 gen sha",
                                "15 gen sm3",
                                "16 run ecdsa sign",
                                "17 run ecdsa verify",
                                "18 run rsassa pkcs sign",
                                "19 run rsassa pkcs verify",
                                "20 run rsassa pss sign",
                                "21 run rsassa pss verify",
                                "22 gen random number",
                                "23 write otp",
                                "24 write snor",
                                "25 block AP system",
                                "26 get version",
                                "27 full test",
                                "28 aging(10,000)",
                                NULL};

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void HSM_PrintCmd(void)
{
    int32 i = 0;

    mcu_printf("\ncommand for hsm\n\n");

    for (i = 0; hsm_cmd[i] != NULL; i++) {
        mcu_printf("  %s\n", hsm_cmd[i]);
    }

    mcu_printf("\n");

    return;
}

static void HSM_HexDump(uint8 *pData, uint32 uiSize)
{
    uint8 ascii[17] = {0};
    uint32 i = 0;
    uint32 j = 0;
    ascii[16] = 0;
    for (i = 0; i < uiSize; ++i) {
        mcu_printf("%02X ", ((const uint8 *)pData)[i]);
        if ((((uint8 *)pData)[i] >= 0x20U) && (((uint8 *)pData)[i] <= 0x7EU)) {
            ascii[i % 16U] = ((const uint8 *)pData)[i];
        } else {
            ascii[i % 16U] = 0x2EU;
        }
        if ((((i + 1U) % 8U) == 0U) || ((i + 1U) == uiSize)) {
            mcu_printf(" ");
            if (((i + 1U) % 16U) == 0U) {
                mcu_printf("|  %s \n", ascii);
            } else if ((i + 1U) == uiSize) {
                ascii[(i + 1U) % 16U] = 0U;
                if (((i + 1U) % 16U) <= 8U) {
                    mcu_printf(" ");
                }
                for (j = ((i + 1U) % 16U); j < 16U; ++j) {
                    mcu_printf("   ");
                }
                mcu_printf("|  %s \n", ascii);
            } else {
            	mcu_printf("\n");
            }
        }
    }
    mcu_printf("\n");
}

static int32 HSM_SetKeyTest(uint32 uiCmd)
{
    int32 ret = HSM_ERR;
    uint32 req = 0;
    uint32 data_size = 0;
    uint32 aeskey_index = 0;
    uint32 mackey_index = 0;
    uint32 aeskey_addr = 0;
    uint32 mackey_addr = 0;

    if (uiCmd == HSM_SET_KEY_FROM_OTP) {
        req = HSM_REQ_SET_KEY_FROM_OTP;
    } else if (uiCmd == HSM_SET_KEY_FROM_SNOR) {
        req = HSM_REQ_SET_KEY_FROM_SNOR;
    } else {
        ELOG("Invalid uiCmd(%d)\n", uiCmd);
        return ret;
    }

    /* Set key information */
    aeskey_index = HSM_R5_AES_KEY_INDEX;
    mackey_index = HSM_R5_MAC_KEY_INDEX;
    aeskey_addr = HSM_R5_AESKEY_ADDR;
    mackey_addr = HSM_R5_MACKEY_ADDR;

    data_size = sizeof(key);
    ret = HSM_SetKey(req, aeskey_addr, data_size, aeskey_index);
    if (ret != HSM_OK) {
        ELOG("HSM_SetKeyTest Err(%d), addr=0x%x\n", ret, aeskey_addr);
        return ret;
    }

    data_size = sizeof(mac_key);
    ret = HSM_SetKey(req, mackey_addr, data_size, mackey_index);
    if (ret != HSM_OK) {
        ELOG("HSM_SetKey test fail(%d), addr=0x%x\n", ret, mackey_addr);
        return ret;
    }

    DLOG("HSM_SetKey(aeskey_addr=0x%x mackey_addr=0x%x) Success\n", aeskey_addr, mackey_addr);
    return ret;
}

static int32 HSM_RunAesTest(uint32 uiCmd)
{
    uint32 obj_id = 0;
    uint32 key_size = 0;
    uint32 src_size = 0;
    uint32 dst_size = 0;
    uint32 req = 0;
    uint32 tag_size = 0;
    uint32 aad_size = 0;
    uint32 iv_size = 0;
    uint8 dst[32] = {0};
    uint8 tag[32] = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    switch (uiCmd) {
    case HSM_RUN_AES:
        /* AES ECB Test */
        req = HSM_REQ_RUN_AES;
        obj_id = HSM_AES_ECB_128 | HSM_AES_ENCRYPT;
        dst_size = sizeof(aes_ecb_cipher);
        src_size = sizeof(plain_data);
        key_size = sizeof(key);
        iv_size = sizeof(aes_iv);
        ret = HSM_RunAes(
            req, obj_id, key, key_size, aes_iv, iv_size, NULL, 0, NULL, 0, plain_data, src_size,
            dst, dst_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunAes test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)aes_ecb_cipher, (const void *)dst, (SALSize)dst_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump((uint8 *)aes_ecb_cipher, dst_size);
            HSM_HexDump((uint8 *)dst, dst_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }

        /* AES CCM Test */
        req = HSM_REQ_RUN_AES;
        obj_id = HSM_AES_CCM_128 | HSM_AES_ENCRYPT;
        dst_size = sizeof(aes_ccm_cipher);
        src_size = sizeof(plain_data);
        key_size = sizeof(key);
        iv_size = sizeof(iv);
        tag_size = sizeof(aes_ccm_tag);
        aad_size = sizeof(aes_aad);
        ret = HSM_RunAes(
            req, obj_id, key, key_size, iv, iv_size, tag, tag_size, aes_aad, aad_size, plain_data,
            src_size, dst, dst_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunAes test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)aes_ccm_cipher, (const void *)dst, dst_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(aes_ccm_cipher, dst_size);
            HSM_HexDump(dst, dst_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)tag, (const void *)aes_ccm_tag, tag_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(tag, tag_size);
            HSM_HexDump(aes_ccm_tag, tag_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }

        /* AES GCM Test */
        req = HSM_REQ_RUN_AES;
        obj_id = HSM_AES_GCM_128 | HSM_AES_ENCRYPT;
        dst_size = sizeof(aes_gcm_cipher);
        src_size = sizeof(plain_data);
        key_size = sizeof(key);
        iv_size = sizeof(iv);
        tag_size = sizeof(aes_gcm_tag);
        aad_size = sizeof(aes_aad);
        ret = HSM_RunAes(
            req, obj_id, key, key_size, iv, iv_size, tag, tag_size, aes_aad, aad_size, plain_data,
            src_size, dst, dst_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunAes test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)aes_gcm_cipher, (const void *)dst, dst_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(aes_gcm_cipher, dst_size);
            HSM_HexDump(dst, dst_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)tag, (const void *)aes_gcm_tag, tag_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(tag, tag_size);
            HSM_HexDump(aes_gcm_tag, tag_size);
            ELOG("Wrong tag_data, size=%d\n", tag_size);
            return HSM_ERR;
        }

        DLOG("RUN_AES Success(%d)\n", ret);
        break;

    case HSM_RUN_SM4:
        /* SM4 ECB */
        req = HSM_REQ_RUN_SM4;
        obj_id = (HSM_OID_SM4_ECB_128 | HSM_OID_SM4_ENCRYPT);
        dst_size = sizeof(sm4_ecb_cipher);
        src_size = sizeof(plain_data);
        key_size = sizeof(key);
        ret = HSM_RunAes(
            req, obj_id, key, key_size, NULL, 0, NULL, 0, NULL, 0, plain_data, src_size, dst,
            dst_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunAes test fail(%d)\n", ret);
            return ret;
        }
        sal_ret = SAL_MemCmp((const void *)sm4_ecb_cipher, (const void *)dst, dst_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(sm4_ecb_cipher, dst_size);
            HSM_HexDump(dst, dst_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("RUN_SM4 Success(%d)\n", ret);
        break;

    default:
        ELOG("Wrong uiCmd type(%d)\n", uiCmd);
        break;
    }

    return ret;
}

static int32 HSM_RunAesByKtTest(uint32 uiCmd)
{
    uint32 obj_id = 0;
    uint32 src_size = 0;
    uint32 dst_size = 0;
    uint32 iv_size = 0;
    uint32 req = 0;
    uint32 key_index = 0;
    uint32 tag_size = 0;
    uint32 aad_size = 0;
    uint8 dst[32] = {0};
    uint8 tag[32] = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    switch (uiCmd) {
    case HSM_RUN_AES_BY_KT:
        /* AES ECB */
        req = HSM_REQ_RUN_AES_BY_KT;
        obj_id = HSM_AES_ECB_128 | HSM_AES_ENCRYPT;
        dst_size = sizeof(aes_ccm_cipher);
        src_size = sizeof(plain_data);
        iv_size = sizeof(aes_iv);
        key_index = HSM_R5_AES_KEY_INDEX;

        ret = HSM_RunAesByKt(
            req, obj_id, key_index, aes_iv, iv_size, NULL, 0, NULL, 0, plain_data, src_size, dst,
            dst_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunAes test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)aes_ecb_cipher, (const void *)dst, dst_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(aes_ecb_cipher, dst_size);
            HSM_HexDump(dst, dst_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }

        /* AES CCM */
        req = HSM_REQ_RUN_AES_BY_KT;
        obj_id = HSM_AES_CCM_128 | HSM_AES_ENCRYPT;
        dst_size = sizeof(aes_ccm_cipher);
        src_size = sizeof(plain_data);
        iv_size = sizeof(iv);
        tag_size = sizeof(aes_ccm_tag);
        aad_size = sizeof(aes_aad);
        key_index = HSM_R5_AES_KEY_INDEX;

        ret = HSM_RunAesByKt(
            req, obj_id, key_index, iv, iv_size, tag, tag_size, aes_aad, aad_size, plain_data,
            src_size, dst, dst_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunAes test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)aes_ccm_cipher, (const void *)dst, dst_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(aes_ccm_cipher, dst_size);
            HSM_HexDump(dst, dst_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)tag, (const void *)aes_ccm_tag, tag_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(tag, tag_size);
            HSM_HexDump(aes_ccm_tag, tag_size);
            ELOG("Wrong tag_data(%d)\n", ret);
            return HSM_ERR;
        }

        /* AES GCM */
        req = HSM_REQ_RUN_AES_BY_KT;
        obj_id = HSM_AES_GCM_128 | HSM_AES_ENCRYPT;
        dst_size = sizeof(aes_gcm_cipher);
        src_size = sizeof(plain_data);
        iv_size = sizeof(iv);
        tag_size = sizeof(aes_gcm_tag);
        aad_size = sizeof(aes_aad);
        key_index = HSM_R5_AES_KEY_INDEX;

        ret = HSM_RunAesByKt(
            req, obj_id, key_index, iv, iv_size, tag, tag_size, aes_aad, aad_size, plain_data,
            src_size, dst, dst_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunAes test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)aes_gcm_cipher, (const void *)dst, dst_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(aes_gcm_cipher, dst_size);
            HSM_HexDump(dst, dst_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)tag, (const void *)aes_gcm_tag, tag_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(tag, tag_size);
            HSM_HexDump(aes_gcm_tag, tag_size);
            ELOG("Wrong tag_data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("RUN_AES_BY_KT Success(%d)\n", ret);
        break;

    case HSM_RUN_SM4_BY_KT:
        /* SM4 ECB */
        req = HSM_REQ_RUN_SM4_BY_KT;
        obj_id = (HSM_OID_SM4_ECB_128 | HSM_OID_SM4_ENCRYPT);
        dst_size = sizeof(sm4_ecb_cipher);
        src_size = sizeof(plain_data);
        key_index = HSM_R5_AES_KEY_INDEX;

        ret = HSM_RunAesByKt(
            req, obj_id, key_index, NULL, 0, NULL, 0, NULL, 0, plain_data, src_size, dst, dst_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunAes test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)sm4_ecb_cipher, (const void *)dst, dst_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(sm4_ecb_cipher, dst_size);
            HSM_HexDump(dst, dst_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("RUN_SM4_BY_KT Success(%d)\n", ret);
        break;

    default:
        ELOG("Wrong uiCmd type(%d)\n", uiCmd);
        break;
    }

    return ret;
}

static int32 HSM_GenMacTest(uint32 uiCmd)
{
    uint32 obj_id = 0;
    uint32 key_size = 0;
    uint32 src_size = 0;
    uint32 mac_size = 0;
    uint32 req = 0;
    uint8 mac[32] = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    switch (uiCmd) {
    case HSM_VERIFY_CMAC:
        req = HSM_REQ_VERIFY_CMAC;
        obj_id = 0;
        src_size = sizeof(plain_data);
        key_size = sizeof(key);
        mac_size = sizeof(cmac_out);
        ret = HSM_GenMac(req, obj_id, key, key_size, plain_data, src_size, cmac_out, mac_size);
        if (ret != HSM_OK) {
            ELOG("hsm_gen_cmac test fail(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("VERIFY_CMAC Success(%d)\n", ret);
        break;

    case HSM_GEN_GMAC:
        ELOG("Not yet supported(%d)\n", uiCmd);
        break;

    case HSM_GEN_HMAC:
        req = HSM_REQ_GEN_HMAC;
        obj_id = HSM_OID_HMAC_SHA1_160;
        src_size = sizeof(plain_data);
        key_size = sizeof(mac_key);
        mac_size = 20;
        ret = HSM_GenMac(req, obj_id, mac_key, key_size, plain_data, src_size, mac, mac_size);
        if (ret != HSM_OK) {
            ELOG("hsm_gen_hmac test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)hmac_out, (const void *)mac, mac_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(hmac_out, mac_size);
            HSM_HexDump(mac, mac_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("TCCHSM_GEN_HMAC Success(%d)\n", ret);
        break;

    case HSM_GEN_SM3_HMAC:
        req = HSM_REQ_GEN_SM3_HMAC;
        obj_id = 0;
        src_size = sizeof(plain_data);
        key_size = sizeof(mac_key);
        mac_size = 32;
        ret = HSM_GenMac(req, obj_id, mac_key, key_size, plain_data, src_size, mac, mac_size);
        if (ret != HSM_OK) {
            ELOG("hsm_gen_sm3_hmac test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)sm3_hmac_out, (const void *)mac, mac_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(sm3_hmac_out, mac_size);
            HSM_HexDump(mac, mac_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("TCCHSM_GEN_SM3_HMAC Success(%d)\n", ret);
        break;

    default:
        ELOG("Wrong uiCmd type(%d)\n", uiCmd);
        break;
    }

    return ret;
}

static int32 HSM_GenMacByKtTest(uint32 uiCmd)
{
    uint32 obj_id = 0;
    uint32 key_index = 0;
    uint32 src_size = 0;
    uint32 mac_size = 0;
    uint32 req = 0;
    uint8 mac[32] = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    switch (uiCmd) {
    case HSM_VERIFY_CMAC_BY_KT:
        req = HSM_REQ_VERIFY_CMAC_BY_KT;
        obj_id = 0;
        src_size = sizeof(plain_data);
        key_index = HSM_R5_AES_KEY_INDEX;
        mac_size = sizeof(cmac_out);
        ret = HSM_GenMacByKt(req, obj_id, key_index, plain_data, src_size, cmac_out, mac_size);
        if (ret != HSM_OK) {
            ELOG("hsm_gen_cmac test fail(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("VERIFY_CMAC_BY_KT Success(%d)\n", ret);
        break;

    case HSM_GEN_GMAC_BY_KT:
        ELOG("Not yet supported(%d)\n", uiCmd);
        break;

    case HSM_GEN_HMAC_BY_KT:
        req = HSM_REQ_GEN_HMAC_BY_KT;
        obj_id = HSM_OID_HMAC_SHA1_160;
        src_size = sizeof(plain_data);
        key_index = HSM_R5_MAC_KEY_INDEX;
        mac_size = 20;
        ret = HSM_GenMacByKt(req, obj_id, key_index, plain_data, src_size, mac, mac_size);
        if (ret != HSM_OK) {
            ELOG("hsm_gen_hmac test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)hmac_out, (const void *)mac, mac_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(hmac_out, mac_size);
            HSM_HexDump(mac, mac_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("TCCHSM_GEN_HMAC_BY_KT Success(%d)\n", ret);
        break;

    case HSM_GEN_SM3_HMAC_BY_KT:
        req = HSM_REQ_GEN_SM3_HMAC_BY_KT;
        obj_id = 0;
        src_size = sizeof(plain_data);
        key_index = HSM_R5_MAC_KEY_INDEX;
        mac_size = 32;
        ret = HSM_GenMacByKt(req, obj_id, key_index, plain_data, src_size, mac, mac_size);
        if (ret != HSM_OK) {
            ELOG("hsm_gen_sm3_hmac test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)sm3_hmac_out, (const void *)mac, mac_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(sm3_hmac_out, mac_size);
            HSM_HexDump(mac, mac_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("TCCHSM_GEN_SM3_HMAC_BY_KT Success(%d)\n", ret);
        break;

    default:
        ELOG("Wrong uiCmd type(%d)\n", uiCmd);
        break;
    }

    return ret;
}

static int32 HSM_GenHashTest(uint32 uiCmd)
{
    uint32 obj_id = 0;
    uint32 src_size = 0;
    uint32 dig_size = 0;
    uint32 req = 0;
    uint8 dig[32] = {0};
    int32 ret = HSM_ERR;
    SALRetCode_t sal_ret = SAL_RET_FAILED;

    switch (uiCmd) {
    case HSM_GEN_SHA:
        req = HSM_REQ_GEN_SHA;
        obj_id = HSM_OID_SHA1_160;
        dig_size = 20;
        src_size = sizeof(hash_message);
        ret = HSM_GenHash(req, obj_id, hash_message, src_size, dig, dig_size);
        if (ret != HSM_OK) {
            ELOG("HSM_GenHash test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)digest_sha1_160, (const void *)dig, dig_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(digest_sha1_160, dig_size);
            HSM_HexDump(dig, dig_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("TCCHSM_GEN_SHA Success(%d)\n", ret);
        break;

    case HSM_GEN_SM3:
        req = HSM_REQ_GEN_SM3;
        obj_id = HSM_OID_SM3_256;
        dig_size = 32;
        src_size = sizeof(hash_message);
        ret = HSM_GenHash(req, obj_id, hash_message, src_size, dig, dig_size);
        if (ret != HSM_OK) {
            ELOG("HSM_GenHash test fail(%d)\n", ret);
            return HSM_ERR;
        }
        sal_ret = SAL_MemCmp((const void *)digest_sm3_256, (const void *)dig, dig_size, &ret);
        if ((sal_ret != SAL_RET_SUCCESS) || (ret != 0)) {
            HSM_HexDump(digest_sm3_256, dig_size);
            HSM_HexDump(dig, dig_size);
            ELOG("Wrong cipher data(%d)\n", ret);
            return HSM_ERR;
        }
        DLOG("TCCHSM_GEN_SHA Success(%d)\n", ret);
        break;

    default:
        ELOG("Wrong uiCmd type(%d)\n", uiCmd);
        break;
    }

    return ret;
}

static int32 HSM_RunEcdsaTest(void)
{
    uint32 obj_id = 0;
    uint32 sig_size = 0;
    uint32 key_size = 0;
    uint32 dig_size = 0;
    uint32 req = 0;
    uint8 sig[64] = {0};
    uint8 pri_key[32] = {0};
    int32 ret = HSM_ERR;

    /* Signing */
    req = HSM_REQ_RUN_ECDSA_SIGN;
    obj_id = HSM_OID_ECC_BP256;
    (void) SAL_MemCopy((void *)pri_key, (const void *)gzu08rsa_n, (SALSize)sizeof(pri_key));
    pri_key[0] &= 0x7FU;
    key_size = sizeof(pri_key);
    dig_size = sizeof(gzu08_digest);
    sig_size = 64;
    ret = HSM_RunEcdsa(req, obj_id, pri_key, key_size, gzu08_digest, dig_size, sig, sig_size);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa verify test fail(%d)\n", ret);
        return ret;
    }
    /* Verify */
    req = HSM_REQ_RUN_ECDSA_VERIFY;
    obj_id = HSM_OID_ECC_BP256;
    key_size = sizeof(gzu08ecc_a_pub);
    dig_size = sizeof(gzu08_digest);
    sig_size = sizeof(sig);
    ret =
        HSM_RunEcdsa(req, obj_id, gzu08ecc_a_pub, key_size, gzu08_digest, dig_size, sig, sig_size);
    if (ret != HSM_OK) {
        ELOG("HSM_RunEcdsa verify test fail(%d)\n", ret);
        return ret;
    }
    DLOG("ecdsa signing success(%d)\n", ret);

    return ret;
}

static int32 HSM_RunRsaTest(uint32 uiCmd)
{
    int32 ret = HSM_ERR;
    uint32 obj_id = 0;
    uint32 req = 0;
    uint32 prikey_size = 0;
    uint32 dig_size = 0;
    uint32 sig_size = 0;
    uint32 key_size = 0;
    uint32 mod_n_size = 0;
    uint8 sig[128] = {0};

    switch (uiCmd) {
    case HSM_RUN_RSASSA_PKCS_SIGN:
    case HSM_RUN_RSASSA_PKCS_VERIFY:
        /* Signing */
        req = HSM_REQ_RUN_RSASSA_PKCS_SIGN;
        obj_id = 0;
        mod_n_size = sizeof(mod_n);
        prikey_size = sizeof(rsa_pri_key);
        dig_size = sizeof(digest);
        sig_size = sizeof(sig);
        ret = HSM_RunRsa(
            req, obj_id, mod_n, mod_n_size, rsa_pri_key, prikey_size, digest, dig_size, sig,
            sig_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunRsa signing test fail(%d)\n", ret);
            return ret;
        }

        /* Verify */
        req = HSM_REQ_RUN_RSASSA_PKCS_VERIFY;
        obj_id = HSM_OID_ECC_BP256;
        key_size = sizeof(rsa_pub_key);
        dig_size = sizeof(digest);
        sig_size = sizeof(sig);
        ret = HSM_RunRsa(
            req, obj_id, mod_n, mod_n_size, rsa_pub_key, key_size, digest, dig_size, sig, sig_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunEcdsa verify test fail(%d)\n", ret);
            return ret;
        }

        DLOG("rsassa pkcs signing and verifing success(%d)\n", ret);
        break;

    case HSM_RUN_RSASSA_PSS_SIGN:
    case HSM_RUN_RSASSA_PSS_VERIFY:
        req = HSM_REQ_RUN_RSASSA_PSS_SIGN;
        obj_id = HSM_RSASSA_PSS_OID_HASH;
        mod_n_size = sizeof(mod_n);
        prikey_size = sizeof(rsa_pri_key);
        dig_size = sizeof(digest);
        sig_size = sizeof(sig);
        ret = HSM_RunRsa(
            req, obj_id, mod_n, mod_n_size, rsa_pri_key, prikey_size, digest, dig_size, sig,
            sig_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunRsa signing test fail(%d)\n", ret);
            return ret;
        }

        /* Verify */
        req = HSM_REQ_RUN_RSASSA_PSS_VERIFY;
        obj_id = HSM_RSASSA_PSS_OID_HASH;
        key_size = sizeof(rsa_pub_key);
        dig_size = sizeof(digest);
        sig_size = sizeof(sig);
        ret = HSM_RunRsa(
            req, obj_id, mod_n, mod_n_size, rsa_pub_key, key_size, digest, dig_size, sig, sig_size);
        if (ret != HSM_OK) {
            ELOG("HSM_RunEcdsa verify test fail(%d)\n", ret);
            return ret;
        }

        DLOG("rsassa pss signing and verifing success(%d)\n", ret);
        break;
    default:
        ELOG("Invalid uiCmd(%d)\n", uiCmd);
        break;
    }

    return ret;
}

static int32 HSM_GetRngTest(void)
{
    uint32 rng_size = 0;
    uint32 req = 0;
    int32 ret = HSM_ERR;
    uint32 rand_num[4] = {0};

    req = HSM_REQ_GET_RNG;
    rng_size = sizeof(rand_num);
    ret = HSM_GetRand(req, (uint32 *)rand_num, rng_size);
    if (ret != HSM_OK) {
        ELOG("HSM_GetRand test fail(%d)\n", ret);
        return ret;
    }

    DLOG("%x %x %x %x\n", rand_num[0], rand_num[1], rand_num[2], rand_num[3]);

    return ret;
}

static int32 HSM_WriteOtpTest(void)
{
    uint32 aeskey_addr = 0;
    uint32 mackey_addr = 0;
    uint32 data_size = 0;
    int32 ret = HSM_ERR;

    // write aes test key
    aeskey_addr = HSM_R5_AESKEY_ADDR;
    data_size = sizeof(key);

    ret = HSM_Write(HSM_REQ_WRITE_OTP, aeskey_addr, key, data_size);
    if (ret != HSM_OK) {
        ELOG("HSM_Write_test fail(%d) addr=0x%x\n", ret, aeskey_addr);
        return ret;
    }

    // write mac test key
    mackey_addr = HSM_R5_MACKEY_ADDR;
    data_size = sizeof(mac_key);

    ret = HSM_Write(HSM_REQ_WRITE_OTP, mackey_addr, mac_key, data_size);
    if (ret != HSM_OK) {
        ELOG("HSM_Write_test fail(%d) addr=0x%x\n", ret, mackey_addr);
        return ret;
    }

    DLOG("HSM_Write_otp(aeskey_addr=0x%x mackey_addr=0x%x) Success\n", aeskey_addr, mackey_addr);
    return ret;
}

static int32 HSM_WriteSnorTest(void)
{
    uint32 aeskey_addr = 0;
    uint32 mackey_addr = 0;
    uint32 data_size = 0;
    int32 ret = HSM_ERR;

    // write aes test key
    aeskey_addr = HSM_R5_AESKEY_ADDR;
    data_size = sizeof(key);

    ret = HSM_Write(HSM_REQ_WRITE_SNOR, aeskey_addr, key, data_size);
    if (ret != HSM_OK) {
        ELOG("HSM_Write_test fail(%d)\n", ret);
        return ret;
    }

    // write mac test key
    mackey_addr = HSM_R5_MACKEY_ADDR;
    data_size = sizeof(mac_key);

    ret = HSM_Write(HSM_REQ_WRITE_SNOR, mackey_addr, mac_key, data_size);
    if (ret != HSM_OK) {
        ELOG("HSM_Write_test fail(%d)\n", ret);
        return ret;
    }

    DLOG("HSM_Write_snor(aeskey_addr=0x%x mackey_addr=0x%x) Success\n", aeskey_addr, mackey_addr);

    return ret;
}

static int32 HSM_BlockApSystemTest(void)
{
    uint32 req = 0;
    int32 ret = HSM_ERR;

    req = HSM_REQ_BLOCK_AP_SYSTEM;
    ret = HSM_BlockApSystem(req);
    if (ret != HSM_OK) {
        ELOG("HSM_Block_Ap_System_test fail(%d)\n", ret);
        return ret;
    }

    DLOG("HSM_Block_Ap_System Success\n");

    return ret;
}

static int32 HSM_GetVersionTest(void)
{
    int32 ret = HSM_ERR;
    uint32 req = 0;
    uint32 x = 0;
    uint32 y = 0;
    uint32 z = 0;

    req = HSM_REQ_GET_VER;
    ret = HSM_GetVersion(req, &x, &y, &z);
    if (ret != HSM_OK) {
        ELOG("HSM_GetVersion Err(%d)\n", ret);
        return ret;
    }

    return ret;
}
static int32 HSM_FullTest(int32 iCnt)
{
    int32 i = 0;
    int32 ret = HSM_ERR;

    for (i = 0; i < iCnt; i++) {
        ret = HSM_SetKeyTest(HSM_SET_KEY_FROM_SNOR);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_RunAesTest(HSM_RUN_AES);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_RunAesTest(HSM_RUN_SM4);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_RunAesByKtTest(HSM_RUN_AES_BY_KT);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_RunAesByKtTest(HSM_RUN_SM4_BY_KT);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GenMacTest(HSM_VERIFY_CMAC);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GenMacTest(HSM_GEN_HMAC);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GenMacTest(HSM_GEN_SM3_HMAC);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GenMacByKtTest(HSM_VERIFY_CMAC_BY_KT);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GenMacByKtTest(HSM_GEN_HMAC_BY_KT);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GenMacByKtTest(HSM_GEN_SM3_HMAC_BY_KT);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GenHashTest(HSM_GEN_SHA);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GenHashTest(HSM_GEN_SM3);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_RunEcdsaTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }

        ret = HSM_RunRsaTest(HSM_RUN_RSASSA_PKCS_SIGN);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_RunRsaTest(HSM_RUN_RSASSA_PKCS_VERIFY);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_RunRsaTest(HSM_RUN_RSASSA_PSS_SIGN);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_RunRsaTest(HSM_RUN_RSASSA_PSS_VERIFY);
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GetRngTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
        ret = HSM_GetVersionTest();
        if (ret != HSM_OK) {
            ELOG(" Fail test(%d, %d)\n", i, iCnt);
            return ret;
        }
    }

    DLOG("Success test(%d, %d)\n", i, iCnt);

    return ret;
}

int32 HSM_Test(void *pCommand)
{
    int32 ret = HSM_ERR;
    int32 cmd = 0;

    cmd = atol((const int8 *)pCommand);

    DLOG(" uiCmd:%d\n", cmd);
    ret = HSM_Init();
    if(ret != HSM_OK){
    	ELOG("HSM_Init Fail(%d)\n", ret);
    }

    switch (cmd) {
    case HSM_SET_KEY_FROM_OTP:
    case HSM_SET_KEY_FROM_SNOR:
        ret = HSM_SetKeyTest((uint32)cmd);
        break;

    case HSM_RUN_AES:
    case HSM_RUN_SM4:
        ret = HSM_RunAesTest((uint32)cmd);
        break;

    case HSM_RUN_AES_BY_KT:
    case HSM_RUN_SM4_BY_KT:
        ret = HSM_RunAesByKtTest((uint32)cmd);
        break;

    case HSM_VERIFY_CMAC:
    case HSM_GEN_GMAC:
    case HSM_GEN_HMAC:
    case HSM_GEN_SM3_HMAC:
        ret = HSM_GenMacTest((uint32)cmd);
        break;

    case HSM_VERIFY_CMAC_BY_KT:
    case HSM_GEN_GMAC_BY_KT:
    case HSM_GEN_HMAC_BY_KT:
    case HSM_GEN_SM3_HMAC_BY_KT:
        ret = HSM_GenMacByKtTest((uint32)cmd);
        break;

    case HSM_GEN_SHA:
    case HSM_GEN_SM3:
        ret = HSM_GenHashTest((uint32)cmd);
        break;

    case HSM_RUN_ECDSA_SIGN:
    case HSM_RUN_ECDSA_VERIFY:
        ret = HSM_RunEcdsaTest();
        break;

    case HSM_RUN_RSASSA_PKCS_SIGN:
    case HSM_RUN_RSASSA_PKCS_VERIFY:
    case HSM_RUN_RSASSA_PSS_SIGN:
    case HSM_RUN_RSASSA_PSS_VERIFY:
        ret = HSM_RunRsaTest((uint32)cmd);
        break;

    case HSM_WRITE_OTP:
        ret = HSM_WriteOtpTest();
        break;

    case HSM_WRITE_SNOR:
        ret = HSM_WriteSnorTest();
        break;

    case HSM_GET_RNG:
        ret = HSM_GetRngTest();
        break;

    case HSM_BLOCK_AP_SYSTEM:
        ret = HSM_BlockApSystemTest();
        break;
		
    case HSM_GET_VER:
        ret = HSM_GetVersionTest();
        break;

    case HSM_FULL:
    	ret = HSM_FullTest(1);
        break;

    case HSM_AGING:
    	ret = HSM_FullTest(10000);
        break;

    default:
        ELOG("unknown command(%d)\n", cmd);
        HSM_PrintCmd();
        break;
    }

    return ret;
}

