/*
***************************************************************************************************
*
*   FileName : HSMManagerA53.h
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

#ifndef _THSMMANAGER_H_
#define _THSMMANAGER_H_

#define TCCTHSM_CIPHER_SRC_MAX 300
#define TCCTHSM_CMAC_CHECKSUM 16
#define TCCTHSM_SHA256_CHECKSUM 32
#define TCCTHSM_RNG_MAX 32

#define TEEHSM_MAXKEYSLOT 32
#define DES_MAX_KEYSIZE 8
#define DEFAULT_IVSIZE 8
#define AES128_MAX_KEYSIZE 16
#define TEEHSM_MAXCMDBUFSIZE 64 * 1024 //512*1024

#define TEEHSM_MAX_IVSIZE 16
#define TEEHSM_SYMM_MAX_KEYSIZE 32
#define TEEHSM_OBJECT_ID_MAX_LEN 64
#define TEEHSM_ALLOC_MAX_KEYSIZE 3072
#define TCC_MBOX_MAX_MSG 512

/* key generate keyType */
#define TEETHSM_TYPE_AES 0xA0000010
#define TEETHSM_TYPE_DES 0xA0000011
#define TEETHSM_TYPE_DES3 0xA0000013
#define TEETHSM_TYPE_HMAC_MD5 0xA0000001
#define TEETHSM_TYPE_HMAC_SHA1 0xA0000002
#define TEETHSM_TYPE_HMAC_SHA224 0xA0000003
#define TEETHSM_TYPE_HMAC_SHA256 0xA0000004
#define TEETHSM_TYPE_HMAC_SHA384 0xA0000005
#define TEETHSM_TYPE_HMAC_SHA512 0xA0000006
#define TEETHSM_TYPE_RSA_PUBLIC_KEY 0xA0000030
#define TEETHSM_TYPE_RSA_KEYPAIR 0xA1000030
#define TEETHSM_TYPE_DSA_PUBLIC_KEY 0xA0000031
#define TEETHSM_TYPE_DSA_KEYPAIR 0xA1000031
#define TEETHSM_TYPE_DH_KEYPAIR 0xA1000032
#define TEETHSM_TYPE_ECDSA_PUBLIC_KEY 0xA0000041
#define TEETHSM_TYPE_ECDSA_KEYPAIR 0xA1000041
#define TEETHSM_TYPE_ECDH_PUBLIC_KEY 0xA0000042
#define TEETHSM_TYPE_ECDH_KEYPAIR 0xA1000042
#define TEETHSM_TYPE_GENERIC_SECRET 0xA0000000
#define TEETHSM_TYPE_CORRUPTED_OBJECT 0xA00000BE
#define TEETHSM_TYPE_DATA 0xA00000BF

/* Algorithm Identifiers */
#define TEETHSM_ALG_AES_ECB_NOPAD 0x10000010
#define TEETHSM_ALG_AES_CBC_NOPAD 0x10000110
#define TEETHSM_ALG_AES_CTR 0x10000210
#define TEETHSM_ALG_AES_CTS 0x10000310
#define TEETHSM_ALG_AES_XTS 0x10000410
#define TEETHSM_ALG_AES_CBC_MAC_NOPAD 0x30000110
#define TEETHSM_ALG_AES_CBC_MAC_PKCS5 0x30000510
#define TEETHSM_ALG_AES_CMAC 0x30000610
#define TEETHSM_ALG_AES_CCM 0x40000710
#define TEETHSM_ALG_AES_GCM 0x40000810
#define TEETHSM_ALG_DES_ECB_NOPAD 0x10000011
#define TEETHSM_ALG_DES_CBC_NOPAD 0x10000111
#define TEETHSM_ALG_DES_CBC_MAC_NOPAD 0x30000111
#define TEETHSM_ALG_DES_CBC_MAC_PKCS5 0x30000511
#define TEETHSM_ALG_DES3_ECB_NOPAD 0x10000013
#define TEETHSM_ALG_DES3_CBC_NOPAD 0x10000113
#define TEETHSM_ALG_DES3_CBC_MAC_NOPAD 0x30000113
#define TEETHSM_ALG_DES3_CBC_MAC_PKCS5 0x30000513
#define TEETHSM_ALG_RSASSA_PKCS1_V1_5_MD5 0x70001830
#define TEETHSM_ALG_RSASSA_PKCS1_V1_5_SHA1 0x70002830
#define TEETHSM_ALG_RSASSA_PKCS1_V1_5_SHA224 0x70003830
#define TEETHSM_ALG_RSASSA_PKCS1_V1_5_SHA256 0x70004830
#define TEETHSM_ALG_RSASSA_PKCS1_V1_5_SHA384 0x70005830
#define TEETHSM_ALG_RSASSA_PKCS1_V1_5_SHA512 0x70006830
#define TEETHSM_ALG_RSASSA_PKCS1_V1_5_MD5SHA1 0x7000F830
#define TEETHSM_ALG_RSASSA_PKCS1_PSS_MGF1_SHA1 0x70212930
#define TEETHSM_ALG_RSASSA_PKCS1_PSS_MGF1_SHA224 0x70313930
#define TEETHSM_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256 0x70414930
#define TEETHSM_ALG_RSASSA_PKCS1_PSS_MGF1_SHA384 0x70515930
#define TEETHSM_ALG_RSASSA_PKCS1_PSS_MGF1_SHA512 0x70616930
#define TEETHSM_ALG_RSASSA_PKCS1_PSS_MGF1_MD5 0x70111930
#define TEETHSM_ALG_RSAES_PKCS1_V1_5 0x60000130
#define TEETHSM_ALG_RSAES_PKCS1_OAEP_MGF1_SHA1 0x60210230
#define TEETHSM_ALG_RSAES_PKCS1_OAEP_MGF1_SHA224 0x60310230
#define TEETHSM_ALG_RSAES_PKCS1_OAEP_MGF1_SHA256 0x60410230
#define TEETHSM_ALG_RSAES_PKCS1_OAEP_MGF1_SHA384 0x60510230
#define TEETHSM_ALG_RSAES_PKCS1_OAEP_MGF1_SHA512 0x60610230
#define TEETHSM_ALG_RSAES_PKCS1_OAEP_MGF1_MD5 0x60110230
#define TEETHSM_ALG_RSA_NOPAD 0x60000030
#define TEETHSM_ALG_DSA_SHA1 0x70002131
#define TEETHSM_ALG_DSA_SHA224 0x70003131
#define TEETHSM_ALG_DSA_SHA256 0x70004131
#define TEETHSM_ALG_DH_DERIVE_SHARED_SECRET 0x80000032
#define TEETHSM_ALG_MD5 0x50000001
#define TEETHSM_ALG_SHA1 0x50000002
#define TEETHSM_ALG_SHA224 0x50000003
#define TEETHSM_ALG_SHA256 0x50000004
#define TEETHSM_ALG_SHA384 0x50000005
#define TEETHSM_ALG_SHA512 0x50000006
#define TEETHSM_ALG_MD5SHA1 0x5000000F
#define TEETHSM_ALG_HMAC_MD5 0x30000001
#define TEETHSM_ALG_HMAC_SHA1 0x30000002
#define TEETHSM_ALG_HMAC_SHA224 0x30000003
#define TEETHSM_ALG_HMAC_SHA256 0x30000004
#define TEETHSM_ALG_HMAC_SHA384 0x30000005
#define TEETHSM_ALG_HMAC_SHA512 0x30000006

#define TEETHSM_ALG_ECDSA_SHA1 0x70001042
#define TEETHSM_ALG_ECDSA_SHA224 0x70002042
#define TEETHSM_ALG_ECDSA_SHA256 0x70003042
#define TEETHSM_ALG_ECDSA_SHA384 0x70004042
#define TEETHSM_ALG_ECDSA_SHA512 0x70005042

#define TEETHSM_ALG_ECDSA_P192 0x70001041
#define TEETHSM_ALG_ECDSA_P224 0x70002041
#define TEETHSM_ALG_ECDSA_P256 0x70003041
#define TEETHSM_ALG_ECDSA_P384 0x70004041
#define TEETHSM_ALG_ECDSA_P521 0x70005041

typedef enum {
    TEETHSM_MODE_ENCRYPT = 0,
    TEETHSM_MODE_DECRYPT = 1,
    TEETHSM_MODE_SIGN = 2,
    TEETHSM_MODE_VERIFY = 3,
    TEETHSM_MODE_MAC = 4,
    TEETHSM_MODE_DIGEST = 5,
    TEETHSM_MODE_DERIVE = 6
} TEETHSMOperationMode;

typedef enum _TCCTHSM_ERROR {
    TCCTHSM_OK = 0,
    TCCTHSM_ERR = -1,
    TCCTHSM_ERR_INVALID_PARAM = -2,
    TCCTHSM_ERR_INVALID_STATE = -3,
    TCCTHSM_ERR_INVALID_MEMORY = -4,
    TCCTHSM_ERR_UNSUPPORT_FUNC = -5,
    TCCTHSM_ERR_SOTB_CIPHER = -6,
    TCCTHSM_ERR_OCCUPIED_RESOURCE = -7,
} TCCTHSM_ERROR;

typedef enum {
    NONE = 0,
    DVB_CSA2 = 1,
    DVB_CSA3 = 2,
    AES_128 = 3,
    DES = 4,
    TDES_128 = 5,
    Multi2 = 6
} CIPHER_ALGO;

typedef enum _CIPHER_OP_MODE {
    ECB = 0,
    CBC = 1,
    Assigned_For_CA_Provider1 = 2,
    Assigned_For_CA_Provider2 = 3,
    CTR_128 = 4,
    CTR_64 = 5
} CIPHER_OP_MODE;

typedef enum _RESDUAL_FOR_AES {
    No_Residual_Block_Processing_For_AES = 0,
    CBC_CS2_For_AES = 1,
    SCTE52_For_AES = 2,
    CTR = 3,
} RESDUAL_FOR_AES;

typedef enum _SMSG {
    No_Solitary_Block_Processing = 0,
    Solitary_Block_handling_Using_IV1 = 1,
    Solitary_Block_handling_Using_IV2 = 2,
    CTR_FOR_AES = 3,
} SMSG;

enum _TEETHSM_ACCMULATES_FLAG
{
    ACCMU_FLAG_NONE = 0,
    ACCMU_FLAG_FIRST = 1,
    ACCMU_FLAG_LAST = 2,
};

typedef enum _CIPHER_KEY_SIZE { KEY_SIZE_64 = 8, KEY_SIZE_128 = 16 } CIPHER_KEY_SIZE;

typedef enum _CIPHER_KEY_MODE {
    TCKL = 0,
    CPU_Key = 1,
} CIPHER_KEY_MODE;

typedef enum _CIPHER_KEY_TYPE {
    CORE_Key = 0,
    Multi2_System_Key = 1,
    CMAC_Key = 2,
} CIPHER_KEY_TYPE;

typedef enum _CIPHER_PAD_TYPE {
    PAD_NONE = 0,
    PAD_PKCS7 = 1,
} CIPHER_PAD_TYPE;

typedef enum _CIPHER_RUN_MODE {
    DECRYPTION = 0,
    ENCRYPTION = 1,
} CIPHER_RUN_MODE;

typedef enum _CMAC_FLAG {
    CMAC_FLAG_NONE = 0,
    CMAC_FLAG_FIRST = 1,
    CMAC_FLAG_LAST = 2,
} CMAC_FLAG;

typedef enum _SHA256_FLAG {
    SHA256_FLAG_NONE = 0,
    SHA256_FLAG_FIRST = 1,
    SHA256_FLAG_LAST = 2,
} SHA256_FLAG;

typedef enum _MAILBOX_TYPE {
    DATA_MBOX = 0,
    DMA = 1,
} MAILBOX_TYPE;

struct tcc_hsm_mbox_data
{
    uint8 buffer[TCC_MBOX_MAX_MSG];
    uint32 readSize;
};

// clang-format off
extern void TCCTHSM_ReceiveMboxData(uint8* buffer, uint32 size);
extern int32 TCCTHSM_Open(void);
extern int32 TCCTHSM_Close(void);

extern int32 TCCTHSM_Init(void);
extern int32 TCCTHSM_Finalize(void);
extern int32 TCCTHSM_GetVersion(uint32* major, uint32* minor);
extern int32 TCCTHSM_SetMode(uint32 keyIndex, uint32 algorithm, uint32 opMode, uint32 keySize);
extern int32 TCCTHSM_SetKey(uint32 keyIndex, uint32 *key1, uint32 key1Size, uint32 *key2, uint32 key2Size, uint32 *key3, uint32 key3Size);
extern int32 TCCTHSM_SetKeyFromStorage(uint32 keyIndex, int8 *objId, uint32 objIdLen);
extern int32 TCCTHSM_SetKeyFromOTP(uint32 keyIndex, uint32 otpAddr, uint32 keySize);
extern int32 TCCTHSM_FreeMode(uint32 keyIndex);

extern int32 TCCTHSM_SetIVSymmetric(uint32 keyIndex, uint32 *iv, uint32 ivSize);
extern int32 TCCTHSM_RunCipher(uint32 keyIndex, uint8 *srcBuf, uint32 srcSize, uint8 *dstBuf, uint32 *dstSize, uint32 flag);
extern int32 TCCTHSM_RunCipherByDMA(uint32 keyIndex, uint32 srcBuf, uint32 srcSize, uint32 dstBuf, uint32 *dstSize, uint32 flag);
extern int32 TCCTHSM_RunDigest(uint32 keyIndex, uint8 *chunk, uint32 chunkLen, uint8 *hash, uint32 *hashLen, uint32 flag);
extern int32 TCCTHSM_RunDigestByDMA(uint32 keyIndex, uint32 chunk, uint32 chunkLen, uint8 *hash, uint32 *hashLen, uint32 flag);

extern int32 TCCTHSM_SetIVMAC(uint32 keyIndex, uint32 *iv, uint32 ivSize);
extern int32 TCCTHSM_ComputeMAC(uint32 keyIndex, uint8 *message, uint32 messageLen, uint8 *mac, uint32 *macLen, uint32 flag);
extern int32 TCCTHSM_ComputeMACByDMA(uint32 keyIndex, uint32 message, uint32 messageLen, uint8 *mac, uint32 *macLen, uint32 flag);
extern int32 TCCTHSM_CompareMAC(uint32 keyIndex, uint8 *message, uint32 messageLen, uint8 *mac, uint32 macLen, uint32 flag);
extern int32 TCCTHSM_CompareMACByDMA(uint32 keyIndex, uint32 message, uint32 messageLen, uint8 *mac, uint32 macLen, uint32 flag);
extern int32 TCCTHSM_GetRand(uint32 *rng, uint32 rngSize);
extern int32 TCCTHSM_GenerateKeySS(int8 *objID, uint32 objIDLen, uint32 algorithm, uint32 keySize);
extern int32 TCCTHSM_DeleteKeySS(int8 *objID, uint32 objIDLen);
extern int32 TCCTHSM_WriteKeySS(uint8 *objID, uint32 objIDLen, uint8 *buffer, uint32 bufferSize);

extern int32 TCCTHSM_WriteOTP(uint32 otpAddr, uint32 *otpBuf, uint32 otpSize);
extern int32 TCCTHSM_WriteOTPImage(uint32 otpAddr, uint32 otpSize);

extern int32 TCCTHSM_AsymEncDec(uint32 keyIndex, uint8 *srcBuf, uint32 srcSize, uint8 *dstBuf, uint32 *dstSize, uint32 enc);
extern int32 TCCTHSM_AsymEncDecByDMA(uint32 keyIndex, uint32 srcBuf, uint32 srcSize, uint32 dstBuf, uint32 *dstSize, uint32 enc);
extern int32 TCCTHSM_AsymSignDigest(uint32 keyIndex, uint8 *dig, uint32 digSize, uint8 *sig, uint32 *sigLen);
extern int32 TCCTHSM_AsymVerifyDigest(uint32 keyIndex, uint8 *dig, uint32 digSize, uint8 *sig, uint32 sigLen);
// clang-format on

#endif /* _THSMMANAGER_H_ */

