/*
***************************************************************************************************
*
*   FileName : wdt_reg.h
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

#if !defined(WDT_REG_HEADER)
#define WDT_REG_HEADER

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
/* Register Map */
/* Register Map */
#define WDT_EN                          (0x00UL)
#define WDT_CLR                         (0x04UL)
#define WDT_IRQ_CNT                     (0x08UL)
#define WDT_RSR_CNT                     (0x0CUL)
#define WDT_SM_MODE                     (0x10UL)
#define WDT_WR_PW                       (0x14UL)

#define PMU_WDTCTRL                     (0xBCUL)

/* Configuration Value */
#define WDT_EN_ON                       (0x1UL)
#define WDT_EN_OFF                      (0x0UL)
#define WDT_CLR_APPLY                   (0x1UL)
#define WDT_SM_MODE_CONT_EN             (0x0UL << 3UL)
#define WDT_SM_MODE_CONT_DIS            (0x1UL << 3UL)

#define PMU_WDT_RSTEN                   (0x1UL << 7UL)
#define PMU_WDT_HSM_RSTEN               (0x1UL << 23UL)

/* Configuration Value */
#define WDT_PASSWORD                    (0x5AFEACE5UL)

#endif //WDT_REG_HEADER

