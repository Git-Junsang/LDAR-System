/*
***************************************************************************************************
*
*   FileName : gdma_dev.h
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

#ifndef GDMA_DEV_HEADER_H
#define GDMA_DEV_HEADER_H

#define GDMA_BASE_ADDRESS               (0x1B600000UL)

#define GDMA_INTSR                      (0x00UL)
#define GDMA_ITCSR                      (0x04UL)
#define GDMA_ITCCR                      (0x08UL)
#define GDMA_IESR                       (0x0CUL)
#define GDMA_IECR                       (0x10UL)
#define GDMA_RITCSR                     (0x14UL)
#define GDMA_REISR                      (0x18UL)
#define GDMA_ECR                        (0x1CUL)
#define GDMA_SBRR                       (0x20UL)
#define GDMA_SSRR                       (0x24UL)
#define GDMA_SLBRR                      (0x28UL)
#define GDMA_SLSRR                      (0x2CUL)
#define GDMA_CR                         (0x30UL)
#define GDMA_SR                         (0x34UL)

#define DMA_CH_SRC_ADDR(x)              ((0x100UL + ((x) * 0x20UL)))
#define DMA_CH_DST_ADDR(x)              ((0x104UL + ((x) * 0x20UL)))
#define DMA_CH_LLI(x)                   ((0x108UL + ((x) * 0x20UL)))
#define DMA_CH_CON(x)                   ((0x10CUL + ((x) * 0x20UL)))
#define DMA_CH_CONFIG(x)                ((0x110UL + ((x) * 0x20UL)))

#define DMA_MAX_XFER_SIZE               (0xFFFUL)

#endif  /* GDMA_DEV_HEADER_H */

