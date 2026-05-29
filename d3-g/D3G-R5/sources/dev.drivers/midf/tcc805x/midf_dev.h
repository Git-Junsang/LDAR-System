/*
***************************************************************************************************
*
*   FileName : midf_dev.h
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

#ifndef MIDF_DEV_HEADER
#define MIDF_DEV_HEADER

#define MIDF_FILTER_BASE                (0x1B93B000UL)
#define MIDF_FILTER_CFG_BASE            (0x1B939000UL)

//----------------------------------------------------------------------------------------------------------------------------
// Register structure
//----------------------------------------------------------------------------------------------------------------------------
#define MIDF_REGION_032KB               (0x0EUL)
#define MIDF_REGION_064KB               (0x0FUL)
#define MIDF_REGION_128KB               (0x10UL)
#define MIDF_REGION_256KB               (0x11UL)
#define MIDF_REGION_512KB               (0x12UL)
#define MIDF_REGION_001MB               (0x13UL)
#define MIDF_REGION_002MB               (0x14UL)
#define MIDF_REGION_004MB               (0x15UL)
#define MIDF_REGION_008MB               (0x16UL)
#define MIDF_REGION_016MB               (0x17UL)
#define MIDF_REGION_032MB               (0x18UL)
#define MIDF_REGION_064MB               (0x19UL)
#define MIDF_REGION_128MB               (0x1AUL)
#define MIDF_REGION_256MB               (0x1BUL)
#define MIDF_REGION_512MB               (0x1CUL)
#define MIDF_REGION_001GB               (0x1DUL)
#define MIDF_REGION_002GB               (0x1EUL)
#define MIDF_REGION_004GB               (0x1FUL)
#define MIDF_REGION_008GB               (0x20UL)

#define MIDF_SECURE_READ                (0x8UL)
#define MIDF_SECURE_WRITE               (0x4UL)

#define MIDF_NONSECURE_READ             (0x2UL)
#define MIDF_NONSECURE_WRITE            (0x1UL)

#define MIDF_REACT_OKAY                 (0x0UL<<0)
#define MIDF_REACT_DECERR               (0x1UL<<0)
#define MIDF_REACT_LOW                  (0x0UL<<1)
#define MIDF_REACT_HIGH                 (0x1UL<<1)

#define MIDF_SPECULATION_OFF           (1UL)
#define MIDF_SPECULATION_ON             (0UL)

#endif /* MIDF_DEV_HEADER */

