/*
***************************************************************************************************
*
*   FileName : pmio_test.h
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

#ifndef  PMIO_TEST_HEAEDER
#define  PMIO_TEST_HEAEDER

#include "pmio_dev.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define PMIO_TEST_VA_REG_NUM            (35UL)
#define PMIO_TEST_VA_BASE_ADDR          (PMIO_VA_BASE_ADDR)
#define PMIO_TEST_VA_BACKUP_RAM         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x000UL))
#define PMIO_TEST_VA_PMGPIO_DAT         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x800UL))
#define PMIO_TEST_VA_PMGPIO_EN          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x804UL))
#define PMIO_TEST_VA_PMGPIO_FS          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x808UL))
#define PMIO_TEST_VA_PMGPIO_IEN         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x80CUL))
#define PMIO_TEST_VA_PMGPIO_PE          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x810UL))
#define PMIO_TEST_VA_PMGPIO_PS          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x814UL))
#define PMIO_TEST_VA_PMGPIO_CD0         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x818UL))
#define PMIO_TEST_VA_PMGPIO_CD1         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x81CUL))
#define PMIO_TEST_VA_PMGPIO_EE0         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x820UL))
#define PMIO_TEST_VA_PMGPIO_EE1         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x824UL))
#define PMIO_TEST_VA_PMGPIO_CTL         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x828UL))
#define PMIO_TEST_VA_PMGPIO_DI          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x82CUL))
#define PMIO_TEST_VA_PMGPIO_STR         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x830UL))
#define PMIO_TEST_VA_PMGPIO_STF         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x834UL))
#define PMIO_TEST_VA_PMGPIO_POL         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x838UL))
#define PMIO_TEST_VA_PMGPIO_PROT        ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x83CUL))
#define PMIO_TEST_VA_PCTLIO_DAT         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x840UL))
#define PMIO_TEST_VA_PCTLIO_EN          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x844UL))
#define PMIO_TEST_VA_PCTLIO_FS          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x848UL))
#define PMIO_TEST_VA_PCTLIO_IEN         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x84CUL))
#define PMIO_TEST_VA_PCTLIO_PE          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x850UL))
#define PMIO_TEST_VA_PCTLIO_PS          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x854UL))
#define PMIO_TEST_VA_PCTLIO_CD0         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x858UL))
#define PMIO_TEST_VA_PCTLIO_CD1         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x85CUL))
#define PMIO_TEST_VA_PMGPIO_APB         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0x900UL))
#define PMIO_TEST_VA_GP_K_FDAT          ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA00UL))
#define PMIO_TEST_VA_GP_K_IRQST         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA10UL))
#define PMIO_TEST_VA_GP_K_IRQEN         ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA14UL))
#define PMIO_TEST_VA_GP_K_IRQPOL        ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA18UL))
#define PMIO_TEST_VA_GP_K_IRQTM0        ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA1CUL))
#define PMIO_TEST_VA_GP_K_IRQTM1        ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA20UL))
#define PMIO_TEST_VA_GP_K_FCK           ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA24UL))
#define PMIO_TEST_VA_GP_K_FBP           ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA28UL))
#define PMIO_TEST_VA_GP_K_CTL           ((uint32)(PMIO_TEST_VA_BASE_ADDR) | (uint32)(0xA2CUL))

typedef struct PMIO_TEST_REG_STRUCT
{
    uint8                         prtName[20];
    uint8                         prtSize;
    uint32                        prtAddr;

}PmioTestRegStruct_t;

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
uint32 PMIO_TEST_GetByAddress
(
    uint32 uiAddr
);
uint32 PMIO_TEST_GetByName
(
    uint8* pucName
);
void PMIO_TEST_SetByAddress
(
    uint32 uiAddr,
    uint32 uiVa
);
void PMIO_TEST_SetByName
(
    uint8* pucName,
    uint32 uiRegVa
);
void PMIO_TEST_PrintStatus
(
    void
);

void PMIO_TEST_Run
(
    uint32 uiNum
);

#endif /* _PMIO_TEST_HEAEDER_*/

