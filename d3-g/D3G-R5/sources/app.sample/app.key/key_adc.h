/*
***************************************************************************************************
*
*   FileName : key_adc.h
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

#ifndef ADCKEY_H
#define ADCKEY_H


#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#if defined(TCC803x)||defined(TCC805x) //<================================TCC803x
#   define TCC803X_KEY


    #if defined(TCC803X_KEY)
    #   define MAX_ADC_KEY                  (4) //TCC803x EVB (ADC0 ch0 ~ ch2)
    //#define MAX_ADC_KEY 3 //TCC803x EVB (ADC0 ch0 ~ ch2)
    #endif
#endif

/* Key definition*/
#define KEY_NULL                        (0x00)
#define KEY_1                           (0x01)
#define KEY_2                           (0x02)
#define KEY_3                           (0x03)
#define KEY_4                           (0x04)
#define KEY_5                           (0x05)
#define KEY_6                           (0x06)

#define KEY_ENTER                       (0x11)
#define KEY_SCAN                        (0x12)
#define KEY_SETUP                       (0x13)
#define KEY_MENU                        (0x14)
#define KEY_DISP                        (0x15)
#define KEY_TRACKDOWN                   (0x16)
#define KEY_TRACKUP                     (0x17)
#define KEY_FOLDERDOWN                  (0x18)
#define KEY_FOLDERUP                    (0x19)
#define KEY_POWER                       (0x1a)
#define KEY_RADIO                       (0x1b)
#define KEY_MEDIA                       (0x1c)
#define KEY_PHONE                       (0x1d) //KEY_HANDSFREE
#define KEY_VOLUMEUP                    (0x1e)
#define KEY_VOLUMEDOWN                  (0x1f)

#define KEY_MODE                        (0x20)
#define KEY_SEEKUP                      (0x21)
#define KEY_SEEKDOWN                    (0x22)
#define KEY_MUTE                        (0x23)
#define KEY_PHONEON                     (0x24)
#define KEY_PHONEOFF                    (0x25)

#define KEY_HOME                        (0x26)
#define KEY_RETURN                      (0x27)
#define KEY_PLAYPAUSE                   (0x28)
#define KEY_CARPLAY                     (0x29)

//Keys for Battleship keypad
#define KEY_BS_PHONE                    (0xA9)
#define KEY_BS_POWER                    (0x74)
#define KEY_BS_DMB                      (0xF9)
#define KEY_BS_MEDIA                    (0xE2)
#define KEY_BS_SETTING                  (0xFA)
#define KEY_BS_NAVI                     (0xFB)
#define KEY_BS_RADIO                    (0x181)



enum
{
    KEY_ADCKEY_READ                     = 1,
};

enum
{
    KEY_ADC_RELEASED                    =0,
    KEY_ADC_CHECK,
    KEY_ADC_PRESSED,
    KEY_ADC_PRESSING,
};

typedef struct KEYAdcKeyType
{
    uint32                              akADCKeyStatus;
    uint32                              akADCOpenOptions;
} KEYAdcKeyType_t;

typedef struct KEYButton
{
    uint32                              buSscancode;
    uint32                              buEscancode;
    uint16                              buVkcode;
} KEYButton_t;

typedef struct KEYKeyInfo
{
    int32                               old_key;
    int32                               ch;
    int32                               key_pressed;
    int32                               key_status;
    int32                               key_chk_cnt;
} KEYKeyInfo_t;


/*
***************************************************************************************************
*                                          KEY_AdcScan
*
*
* @param    piKeyVal [in]
* @param    piKPressed [in]
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_AdcScan
(
    int32 *                             piKeyVal,
    int32 *                             piKPressed
);


/*
***************************************************************************************************
*                                          KEY_AdcOpen
*
*
* @param    uiOptions [in]
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t KEY_AdcOpen
(
    uint32                              uiOptions
);


#endif /* __HAL_ADCKEY_H__ */

