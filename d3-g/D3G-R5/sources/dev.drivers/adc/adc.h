/*
***************************************************************************************************
*
*   FileName : adc.h
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

#ifndef MCU_BSU_ADC_HEADER
#define MCU_BSU_ADC_HEADER

#include <sal_internal.h>

#if (DEBUG_ENABLE)
    #include "debug.h"

    #define ADC_D(fmt,args...)          {LOGD(DBG_TAG_ADC, fmt, ## args)}
    #define ADC_E(fmt,args...)          {LOGE(DBG_TAG_ADC, fmt, ## args)}
#else
    #define ADC_D(fmt,args...)          {}
    #define ADC_E(fmt,args...)          {}
#endif

/*
***************************************************************************************************
*                                            GLOBAL DEFINITIONS
***************************************************************************************************
*/

typedef enum ADCChannel
{
    ADC_CHANNEL_0                       = 0,
    ADC_CHANNEL_1                       = 1,
    ADC_CHANNEL_2                       = 2,
    ADC_CHANNEL_3                       = 3,
    ADC_CHANNEL_4                       = 4,
    ADC_CHANNEL_5                       = 5,
    ADC_CHANNEL_6                       = 6,
    ADC_CHANNEL_7                       = 7,
    ADC_CHANNEL_8                       = 8,
    ADC_CHANNEL_9                       = 9,
    ADC_CHANNEL_10                      = 10,
    ADC_CHANNEL_11                      = 11,
    ADC_CHANNEL_MAX                     = 12
} ADCChannel_t;

typedef enum ADCMode
{
    ADC_MODE_NORMAL                     = 0,
    ADC_MODE_IRQ                        = 1,
    ADC_MODE_DMA                        = 2
} ADCMode_t;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          ADC_Init
*
* @param    iType [in]
*
* Notes
*
***************************************************************************************************
*/

void ADC_Init
(
    uint8                               Type
);

/*
***************************************************************************************************
*                                          ADC_Read
*
* @param    iChannel [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

uint32 ADC_Read
(
    ADCChannel_t                        Channel
);

/*
***************************************************************************************************
*                                          ADC_AutoScan
*
* @return
*
* Notes
*
***************************************************************************************************
*/

uint32* ADC_AutoScan
(
    void
);

/*
***************************************************************************************************
*                                          ADC_DeInit
*
* @param    iType [in]
*
* Notes
*
***************************************************************************************************
*/

void ADC_DeInit
(
    void
);

#endif /* __MCU_BSU_ADC_HEADER__ */

