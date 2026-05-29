/*
***************************************************************************************************
*
*   FileName : key_gpio.h
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

#ifndef GPIOKEY_H
#define GPIOKEY_H

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define KEY_ROTARY_CH1                  (0)
#define KEY_ROTARY_CH2                  (1)
#define KEY_ROTARY_CH_MAX               (2)


typedef struct GPIOKeyType
{
    uint32                              uiGPIOKeyStatus;
    uint32                              uiGPIOOpenOptions;
} GPIOKeyType_t;



typedef enum GPIOMode
{
    KEY_INPUT_MODE                      = 0,
    KEY_OUTPUT_MODE
} GPIOMode_t;

typedef enum GPIORotaryKeyType
{
    KEY_ROTARY_KEY_CTRL_UP              = 1,
    KEY_ROTARY_KEY_CTRL_DOWN,
    KEY_ROTARY_KEY_VOL_UP,
    KEY_ROTARY_KEY_VOL_DOWN,
    KEY_ROTARY_KEY_MAX
} GPIORotaryKeyType_t;

typedef enum GPIORotaryType
{
    KEY_ROTARY_CTRL_UPDOWN              = 1,
    KEY_ROTARY_VOL_UPDOWN,
    KEY_ROTARY_MAX
} GPIORotaryType_t;


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          KEY_GpioRotaryInitialize
*
* @param    uiOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t KEY_GpioKeyOpen
(
    uint32                              uiOptions
);


/*
***************************************************************************************************
*                                          KEY_GpioGetRotaryValue
*
* @param    uiChannel [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
GPIORotaryKeyType_t KEY_GpioGetRotaryValue
(
    uint8                               uiChannel
);


#endif /* _GPIOKEY_H_ */

