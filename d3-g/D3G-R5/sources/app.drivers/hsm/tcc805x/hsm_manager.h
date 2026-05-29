/*
***************************************************************************************************
*
*   FileName : hsm_manager.h
*
*   Copyright (c) Telechips Inc.
*
*   Description : HSM MANAGER
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

#ifndef HSM_MANAGER_HEADER
#define HSM_MANAGER_HEADER

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
// clang-format off
#define HSM_REQ_SB_VERIFY                   (0x00020014U)
#define HSM_REQ_RUN_AES                     (0x10010000U)
#define HSM_REQ_RUN_AES_BY_KT               (0x10020000U)
#define HSM_REQ_RUN_SM4                     (0x10030000U)
#define HSM_REQ_RUN_SM4_BY_KT               (0x10040000U)
#define HSM_REQ_VERIFY_CMAC                 (0x10110000U)
#define HSM_REQ_VERIFY_CMAC_BY_KT           (0x10120000U)
#define HSM_REQ_GEN_GMAC                    (0x10130000U)
#define HSM_REQ_GEN_GMAC_BY_KT              (0x10140000U)
#define HSM_REQ_GEN_HMAC                    (0x10150000U)
#define HSM_REQ_GEN_HMAC_BY_KT              (0x10160000U)
#define HSM_REQ_GEN_SM3_HMAC                (0x10170000U)
#define HSM_REQ_GEN_SM3_HMAC_BY_KT          (0x10180000U)
#define HSM_REQ_GEN_SHA                     (0x10210000U)
#define HSM_REQ_GEN_SM3                     (0x10220000U)
#define HSM_REQ_RUN_ECDSA_SIGN              (0x10310000U)
#define HSM_REQ_RUN_ECDSA_VERIFY            (0x10320000U)
#define HSM_REQ_RUN_RSASSA_PKCS_SIGN        (0x10550000U)
#define HSM_REQ_RUN_RSASSA_PKCS_VERIFY      (0x10560000U)
#define HSM_REQ_RUN_RSASSA_PSS_SIGN         (0x10570000U)
#define HSM_REQ_RUN_RSASSA_PSS_VERIFY       (0x10580000U)
#define HSM_REQ_GET_RNG                     (0x10610000U)
#define HSM_REQ_WRITE_OTP                   (0x10710000U)
#define HSM_REQ_WRITE_SNOR                  (0x10720000U)
#define HSM_REQ_SET_KEY_FROM_OTP            (0x10810000U)
#define HSM_REQ_SET_KEY_FROM_SNOR           (0x10820000U)
#define HSM_REQ_BLOCK_AP_SYSTEM             (0x10910000U)
#define HSM_REQ_GET_VER                     (0x20010000U)
// clang-format on

#define HSM_CMD_FIFO_SIZE (0x0008U)
#define HSM_MBOX_LOCATION_DATA (0x0400U)
#define HSM_MBOX_LOCATION_CMD (0x0000U)
#define HSM_MBOX_ID_HSM (0x4D5348) /* 0x4D5348 = "HSM" */

#define HSM_MBOX_CID_A72 (0x7200U)
#define HSM_MBOX_CID_A53 (0x5300U)
#define HSM_MBOX_CID_SC (0xD300U)
#define HSM_MBOX_CID_HSM (0xA000U)
#define HSM_MBOX_CID_R5 (0xFF00U)

#define HSM_MBOX_BSID_BL0 (0x0042U)
#define HSM_MBOX_BSID_BL1 (0x0043U)
#define HSM_MBOX_BSID_BL3 (0x0045U)
#define HSM_MBOX_BSID_KERNEL (0x0046U)

#define HSM_MBOX_HSM_CMD0 (HSM_MBOX_CID_R5 | HSM_MBOX_BSID_KERNEL)
#define HSM_RSASSA_PSS_OID_HASH (HSM_OID_SHA2_256)

#define HSM_MC_SUB_FW_IMAGE_ID (0x000003FFU)
#define HSM_MICOM_SRAM1_BASEADDR (0xC1000000U)

/* HSM key index */
#define HSM_A72_AES_KEY_INDEX (0x0000)
#define HSM_A72_MAC_KEY_INDEX (0x0001)
#define HSM_A53_AES_KEY_INDEX (0x0002)
#define HSM_A53_MAC_KEY_INDEX (0x0003)
#define HSM_R5_AES_KEY_INDEX (0x0004)
#define HSM_R5_MAC_KEY_INDEX (0x0005)

/* HSM key address */
#define HSM_A72_AESKEY_ADDR (0x100)
#define HSM_A72_MACKEY_ADDR (0x110)
#define HSM_A53_AESKEY_ADDR (0x130)
#define HSM_A53_MACKEY_ADDR (0x140)
#define HSM_R5_AESKEY_ADDR (0x160)
#define HSM_R5_MACKEY_ADDR (0x170)

/* HSM Err code */
#define HSM_DATA_FIFO_OK (0U)
#define HSM_OK (0)
#define HSM_ERR (-1)
#define HSM_ERR_INVALID_PARAM (-2)
#define HSM_ERR_INVALID_STATE (-3)
#define HSM_ERR_INVALID_MEMORY (-4)
#define HSM_ERR_UNSUPPORT_FUNC (-5)
#define HSM_ERR_SOTB_CIPHER (-6)
#define HSM_ERR_OCCUPIED_RESOURCE (-7)

/* HSM OID HMAC */
#define HSM_OID_HMAC_SHA1_160 (0x00011100U)
#define HSM_OID_HMAC_SHA2_224 (0x00012200U)
#define HSM_OID_HMAC_SHA2_256 (0x00012300U)
#define HSM_OID_HMAC_SHA2_384 (0x00012400U)
#define HSM_OID_HMAC_SHA2_512 (0x00012500U)
#define HSM_OID_HMAC_SHA3_224 (0x00013200U)
#define HSM_OID_HMAC_SHA3_256 (0x00013300U)
#define HSM_OID_HMAC_SHA3_384 (0x00013400U)
#define HSM_OID_HMAC_SHA3_512 (0x00013500U)

/* HSM OID SHA/SM3 */
#define HSM_OID_SHA1_160 (0x00001100U)
#define HSM_OID_SHA2_224 (0x00002200U)
#define HSM_OID_SHA2_256 (0x00002300U)
#define HSM_OID_SHA2_384 (0x00002400U)
#define HSM_OID_SHA2_512 (0x00002500U)
#define HSM_OID_SHA3_224 (0x00003200U)
#define HSM_OID_SHA3_256 (0x00003300U)
#define HSM_OID_SHA3_384 (0x00003400U)
#define HSM_OID_SHA3_512 (0x00003500U)
#define HSM_OID_SM3_256 (0x01002300U)

/* HSM ECC Code */
#define HSM_OID_ECC_P256 (0x00000013U)
#define HSM_OID_ECC_P384 (0x00000014U)
#define HSM_OID_ECC_P521 (0x00000015U)
#define HSM_OID_ECC_BP256 (0x00000053U)
#define HSM_OID_ECC_BP384 (0x00000054U)
#define HSM_OID_ECC_BP512 (0x00000055U)
#define HSM_OID_SM2_256_SM3_256 (0x010023A3U)

/* HSM DMA type */
#define HSM_NONE_DMA (0)
#define HSM_DMA (1)

/* HSM ioctl cmd */
#define HSM_SET_KEY_FROM_OTP (0U)
#define HSM_SET_KEY_FROM_SNOR (1U)
#define HSM_RUN_AES (2U)
#define HSM_RUN_AES_BY_KT (3U)
#define HSM_RUN_SM4 (4U)
#define HSM_RUN_SM4_BY_KT (5U)
#define HSM_VERIFY_CMAC (6U)
#define HSM_VERIFY_CMAC_BY_KT (7U)
#define HSM_GEN_GMAC (8U)
#define HSM_GEN_GMAC_BY_KT (9U)
#define HSM_GEN_HMAC (10U)
#define HSM_GEN_HMAC_BY_KT (11U)
#define HSM_GEN_SM3_HMAC (12U)
#define HSM_GEN_SM3_HMAC_BY_KT (13U)
#define HSM_GEN_SHA (14U)
#define HSM_GEN_SM3 (15U)
#define HSM_RUN_ECDSA_SIGN (16U)
#define HSM_RUN_ECDSA_VERIFY (17U)
#define HSM_RUN_RSASSA_PKCS_SIGN (18U)
#define HSM_RUN_RSASSA_PKCS_VERIFY (19U)
#define HSM_RUN_RSASSA_PSS_SIGN (20U)
#define HSM_RUN_RSASSA_PSS_VERIFY (21U)
#define HSM_GET_RNG (22U)
#define HSM_WRITE_OTP (23U)
#define HSM_WRITE_SNOR (24U)
#define HSM_BLOCK_AP_SYSTEM (25U)
#define HSM_GET_VER (26U)
#define HSM_FULL (27U)
#define HSM_AGING (28U)
#define HSM_MAX (29U)

/* HSM AES type */
#define HSM_AES_ENCRYPT (0x00000000U)
#define HSM_AES_DECRYPT (0x01000000U)
#define HSM_AES_ECB_128 (0x00100008U)
#define HSM_AES_ECB_192 (0x00180008U)
#define HSM_AES_ECB_256 (0x00200008U)
#define HSM_AES_CBC_128 (0x00100108U)
#define HSM_AES_CBC_192 (0x00180108U)
#define HSM_AES_CBC_256 (0x00200108U)
#define HSM_AES_CTR_128 (0x00100208U)
#define HSM_AES_CTR_192 (0x00180208U)
#define HSM_AES_CTR_256 (0x00200208U)
#define HSM_AES_XTS_128 (0x00100308U)
#define HSM_AES_XTS_256 (0x00200308U)
#define HSM_AES_CCM_128 (0x00101008U)
#define HSM_AES_CCM_192 (0x00181008U)
#define HSM_AES_CCM_256 (0x00201008U)
#define HSM_AES_GCM_128 (0x00101108U)
#define HSM_AES_GCM_192 (0x00181108U)
#define HSM_AES_GCM_256 (0x00201108U)

/* HSM OID SM4 */
#define HSM_OID_SM4_ENCRYPT (0x00000000U)
#define HSM_OID_SM4_DECRYPT (0x01000000U)
#define HSM_OID_SM4_ECB_128 (0x00100008U)
#define HSM_OID_SM4_CBC_128 (0x00100108U)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
int32 HSM_Init(void);
int32 HSM_VerifyFw(
    uint32 uiHeaderAddr, uint32 uiHeaderSize, uint32 uiImgAddr, uint32 uiImgSize, uint32 uiImgId);
int32 HSM_SetKey(uint32 uiReq, uint32 uiAddr, uint32 uiDataSize, uint32 uiKeyIndex);
int32 HSM_RunAes(
    uint32 uiReq, uint32 uiObjId, uint8 *pucPubKey, uint32 uiKeySize, uint8 *pucIv, uint32 uiIvSize,
    uint8 *pucTag, uint32 uiTagSize, uint8 *pucAad, uint32 uiAadSize, uint8 *pucSrcAddr,
    uint32 uiSrcSize, uint8 *pucDstAddr, uint32 uiDstSize);
int32 HSM_RunAesByKt(
    uint32 uiReq, uint32 uiObjId, uint32 uiKeyIndex, uint8 *pucIv, uint32 uiIvSize, uint8 *ucTag,
    uint32 uiTagSize, uint8 *ucAad, uint32 uiAadSize, uint8 *pucSrcAddr, uint32 uiSrcSize,
    uint8 *pucDstAddr, uint32 uiDstSize);
int32 HSM_GenMac(
    uint32 uiReq, uint32 uiObjId, uint8 *pucPubKey, uint32 uiKeySize, uint8 *pucSrcAddr,
    uint32 uiSrcSize, uint8 *pucMacAddr, uint32 uiMacSize);
int32 HSM_GenMacByKt(
    uint32 uiReq, uint32 uiObjId, uint32 uiKeyIndex, uint8 *pucSrcAddr, uint32 uiSrcSize,
    uint8 *pucMacAddr, uint32 uiMacSize);
int32 HSM_GenHash(
    uint32 uiReq, uint32 uiObjId, uint8 *pucSrcAddr, uint32 uiSrcSize, uint8 *pucDig,
    uint32 uiDigSize);
int32 HSM_RunEcdsa(
    uint32 uiReq, uint32 uiObjId, uint8 *pucPubKey, uint32 uiKeySize, uint8 *pucDig,
    uint32 uiDigSize, uint8 *pucSig, uint32 uiSigSize);
int32 HSM_RunRsa(
    uint32 uiReq, uint32 uiObjId, uint8 *pucModN, uint32 uiModNSize, uint8 *pucPubKey,
    uint32 uiKeySize, uint8 *pucDig, uint32 uiDigSize, uint8 *pucSig, uint32 uiSigSize);
int32 HSM_Write(uint32 uiReq, uint32 uiAddr, uint8 *pucBuf, uint32 uiBufSize);
int32 HSM_GetRand(uint32 uiReq, uint32 *pucRng, uint32 uiRngSize);
int32 HSM_BlockApSystem(uint32 uiReq);
int32 HSM_GetVersion(uint32 uiReq, uint32 *uiX, uint32 *uiY, uint32 *uiZ);

#endif /* HSM_MANAGER_HEADER */
