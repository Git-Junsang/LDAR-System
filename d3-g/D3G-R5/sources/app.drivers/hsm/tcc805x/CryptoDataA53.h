/*
***************************************************************************************************
*
*   FileName : CryptoDataA53.h
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

#ifndef _CRYPTODATA_H_
#define _CRYPTODATA_H_

/* AES Test */
extern unsigned char key_aes128[16];
extern unsigned char ECB_key[16];
extern unsigned char ECB_plain[16];
extern unsigned char ECB_cipher[16];
extern unsigned char AES_CBC_ucInitVector[16];
extern unsigned char AES_CBC_128KEY[16];
extern unsigned char AES_CBC_PLAINTEXT[16];
extern unsigned char AES_CBC_128KEYCIPHERTEXT[16];

/* DES Test */
extern unsigned char DES_ECB_KEY[8];
extern unsigned char DES_ECB_PLAINTEXT[24];
extern unsigned char DES_ECB_CIPHERTEXT[24];

/* TDES Test */
extern unsigned char TDES_CBC_ucInitVector[8];
extern unsigned char TDES_CBCKEY[24];
extern unsigned char TDES_CBCPLAINTEXT[32];
extern unsigned char TDES_3KEY_CBCCIPHERTEXT[32];

/* CMAC Test */
extern unsigned char msg_cmac[64];
extern unsigned char key_cmac[16];
extern unsigned char enckey_cmac[16];
extern unsigned char result_cmac[16];

/* SHA Test */
extern unsigned char msg_sha1[16];
extern unsigned char result_sha1[20];
extern unsigned char msg_sha256[46];
extern unsigned char result_sha256[32];

/* RSA Test */
extern unsigned char RSA_plain[28];
extern unsigned char RSA_1024bit_exponent_e[3];
extern unsigned char RSA_1024bit_Encryption[8 * 16];
extern unsigned char RSA_1024bit_pirvate_d[8 * 16];
extern unsigned char RSA_1024bit_modulus_n[8 * 16];
extern unsigned char gObjID[32];

/* ECDSA Signing Test */
extern unsigned char ECDSA_dig_P256[32];

#endif /* _CRYPTODATA_H_ */

