/*
***************************************************************************************************
*
*   FileName : key_adc.c
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

#include "key_adc.h"
#include "adc.h"
#include "gpio.h"
#include "pmio.h"
#include "key.h"
#include <bsp.h>



/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/


//#define ADCKEY_DEBUG
#ifdef ADCKEY_DEBUG
#   define ADCKEY_DBG(fmt, args...)     {LOGD(DBG_TAG_ADC, fmt, ## args)}
#else
#   define ADCKEY_DBG(fmt, args...)     {}
#endif


/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/

static KEYAdcKeyType_t ADCKey_Main;

static KEYKeyInfo_t tcc_key = {-1, 0, 0, 0, 0};

static struct KEYButton buttons0[5] = {
    {
        0x00,
        0x83,
        KEY_ENTER
    },
    {
        0x266,
        0x2EE,
        KEY_SCAN
    },
    {
        0x732,
        0x8CC,
        KEY_SETUP
    },
    {
        0x9E5,
        0xC18,
        KEY_MENU
    },
    {
        0xD4A,
        0xFA0,
        KEY_DISP
    },
};

static struct KEYButton buttons1[4] = {
    {
        0x6B,
        0x176,
        KEY_TRACKDOWN
    },
    {
        0x732,
        0x8CC,
        KEY_TRACKUP
    },
    {
        0x9E5,
        0xC18,
        KEY_FOLDERDOWN
    },
    {
        0xD4A,
        0xFA0,
        KEY_FOLDERUP
    },
};

static struct KEYButton buttons2[4] = {
    {
        0x00,
        0x83,
        KEY_POWER
    },
    {
        0x732,
        0x8CC,
        KEY_RADIO
    },
    {
        0x9E5,
        0xC18,
        KEY_MEDIA
    },
    {
        0xD4A,
        0xFA0,
        KEY_PHONE
    },
};

static struct KEYButton buttons3[6] = {
    {
        0x6B,
        0x83,
        KEY_1
    },
    {
        0x266,
        0x2EE,
        KEY_2
    },
    {
        0x52B,
        0x651,
        KEY_3
    },
    {
        0x732,
        0x8CC,
        KEY_4
    },
    {
        0x9E5,
        0xC18,
        KEY_5
    },
    {
        0xD4A,
        0xFA0,
        KEY_6
    },
};


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void KEY_AdcResetKeystatus
(
    void
);

static int32 KEY_AdcGetkeycodebyscancode
(
    int32 uiCh,
    uint32 uiAdcdata
);


/*
***************************************************************************************************
*                                          KEY_AdcResetKeystatus
*
*
* Notes
*
***************************************************************************************************
*/
static void KEY_AdcResetKeystatus (void)
{
    tcc_key.old_key = -1;
    tcc_key.key_pressed=(int32)KEY_ADC_RELEASED;
    tcc_key.key_status=(int32)KEY_ADC_RELEASED;
    tcc_key.key_chk_cnt=0;
    tcc_key.ch = 0;
}

/*
***************************************************************************************************
*                                          KEY_AdcGetkeycodebyscancode
*
*
* @param    uiCh [in]
* @param    uiAdcdata [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static int32 KEY_AdcGetkeycodebyscancode(int32 uiCh, uint32 uiAdcdata)
{
    uint32 i;
    int32 key = -1;
    uint32 data=0;

    data = (uiAdcdata & 0x00000FFFUL);

    switch(uiCh)
    {
        case 0: //ch 2 //MC_FRT_ADKEY1
            for (i = 0; i < (sizeof(buttons0)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons0[i].buSscancode) && (data <= buttons0[i].buEscancode))
                {
                    key = (int32)buttons0[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", uiCh, key);
                    break;
                }
            }
            break;
        #if defined(TCC803X_KEY)
        case 1: //ch 3 //MC_FRT_ADKEY2
            for (i = 0; i < (sizeof(buttons1)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons1[i].buSscancode) && (data <= buttons1[i].buEscancode))
                {
                    key = (int32)buttons1[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", uiCh, key);
                    break;
                }
            }
            break;
        #endif
        #if defined(TCC803X_KEY)
        case 2: //ch 4 //MC_FRT_ADKEY3
            for (i = 0; i < (sizeof(buttons2)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons2[i].buSscancode) && (data <= buttons2[i].buEscancode))
                {
                    key = (int32)buttons2[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", uiCh, key);
                    break;
                }
            }
            break;
        #endif
        #if defined(TCC803X_KEY)
        case 3: //ch 5 //MC_FRT_ADKEY4
            for (i = 0; i < (sizeof(buttons3)/sizeof(KEYButton_t)); i++) {
                if ((data >= buttons3[i].buSscancode) && (data <= buttons3[i].buEscancode))
                {
                    key = (int32)buttons3[i].buVkcode;
                    ADCKEY_DBG("adc key (%d) : 0x%x\n", uiCh, key);
                    break;
                }
            }
            break;
        #endif
        default :
            ADCKEY_DBG("%s : Wrong parameter - %d\n", __func__, uiCh);
            break;
    }

    return key;
}


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
SALRetCode_t KEY_AdcOpen(uint32 uiOptions)
{
    SALRetCode_t lRet;

#if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
    static uint32 adc_key_pad_type = KEY_MANY_PUSHKEY;
#endif
    lRet = SAL_RET_SUCCESS;

    ADCKey_Main.akADCKeyStatus = KEY_DEVICE_ST_NO_OPEN; //CS : Useless Assignment - Modify initial value

    if (uiOptions  != 0UL)
    {
        lRet = SAL_RET_FAILED;
    }
    else
    {
        ADCKey_Main.akADCOpenOptions  = uiOptions;
        ADCKey_Main.akADCKeyStatus = KEY_DEVICE_ST_OPENED;
        // open driver
    }
#if (TCC_EVM_BD_VERSION >= TCC803x_BD_VER_0_1)
    //check key type

    PMIO_SetGPK( PMIO_GPK(5) );

    (void)GPIO_Config(GPIO_GPK((uint32)5), GPIO_FUNC((uint32)0)|GPIO_INPUT|GPIO_INPUTBUF_EN);
    adc_key_pad_type = GPIO_Get(GPIO_GPK((uint32)5));

    ADCKEY_DBG("OPEN : adc_key_pad_type = %d\n",adc_key_pad_type);

    if(adc_key_pad_type == (uint32)KEY_ONLY_ROTARY)
    {
        (void)SAL_MemSet(&buttons0[0], 0x00, sizeof(KEYButton_t));
        buttons0[0].buSscancode = 0x9E5;
        buttons0[0].buEscancode = 0xC18;
        buttons0[0].buVkcode = KEY_POWER;

        (void)SAL_MemSet(&buttons1[0], 0x00, sizeof(KEYButton_t));
        buttons1[1].buSscancode = 0x732;
        buttons1[1].buEscancode = 0x8CC;
        buttons1[1].buVkcode = KEY_MODE;
        buttons1[2].buSscancode = 0x9E5;
        buttons1[2].buEscancode = 0xC18;
        buttons1[2].buVkcode = KEY_5;
        buttons1[3].buSscancode = 0xD4A;
        buttons1[3].buEscancode = 0xFA0;
        buttons1[3].buVkcode = KEY_6;
        (void)SAL_MemSet(&buttons2[0], 0x00, sizeof(KEYButton_t)); //QAC
        buttons2[0].buSscancode = 0x00;
        buttons2[0].buEscancode = 0x83;
        buttons2[0].buVkcode = KEY_ENTER;
    }
 #endif
    return lRet;
}


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
SALRetCode_t KEY_AdcScan(int32 * piKeyVal, int32 * piKPressed)
{
    uint32 read_buf[MAX_ADC_KEY]={0}; //CS : Uninitialized Variable
#ifdef TCC803X_KEY
    uint32 *dev_read_buf = NULL;
#else
    uint32 *pbuf = &read_buf[0];
    int32 key_mask=0;
#endif
    SALRetCode_t ret_key = SAL_RET_FAILED;
    int32 key = -1;
    int32 i;
    int32 key_checked = 0;

    if((piKeyVal == ((void *)0)) || (piKPressed == ((void *)0)))
    {
        return ret_key;
    }

    dev_read_buf = ADC_AutoScan();

    for (i=0; i<MAX_ADC_KEY; i++)
    {
        switch (i)
        {
            case 0:
                read_buf[i]= (dev_read_buf[ADC_CHANNEL_0]&0xfffUL);
                break;
            case 1:
                read_buf[i]= (dev_read_buf[ADC_CHANNEL_1]&0xfffUL);
                break;
            case 2:
                read_buf[i]= (dev_read_buf[ADC_CHANNEL_2]&0xfffUL);
                break;
            case 3:
                read_buf[i]= (dev_read_buf[ADC_CHANNEL_10]&0xfffUL);
                break;
            default:
				ADCKEY_DBG("INVALID ADC KEY \n");
                break;
        }
    }


    for (i=0; i<MAX_ADC_KEY; i++) {
        key = KEY_AdcGetkeycodebyscancode(i, read_buf[i]);
        if (key > 0)
        {
            if((tcc_key.key_pressed == (int32)KEY_ADC_RELEASED)  && (tcc_key.old_key != key)) //first pressed
            {
                ADCKEY_DBG("Key info[%d] : [Pressed]old_key = %d / cur_key=%d\n", i, tcc_key.old_key, key);
                tcc_key.old_key = key;
                tcc_key.ch = i;
                tcc_key.key_pressed = (int32)KEY_ADC_PRESSED;
                tcc_key.key_status = (int32)KEY_ADC_CHECK; // to verify
                tcc_key.key_chk_cnt = 0;
                //*key_val = tcc_key.old_key[i];
                *piKPressed = 0;
                ret_key = SAL_RET_FAILED;
            }
            else if ((tcc_key.key_pressed == (int32)KEY_ADC_PRESSED)  && (tcc_key.old_key == key)) //keep pressed
            {
                if (tcc_key.key_status == (int32)KEY_ADC_CHECK)
                {
                    if(tcc_key.key_chk_cnt > 5)
                    {
                        ADCKEY_DBG("Key info[%d] : [chk]ch = %d / old_key = %d / cur_key=%d\n", i, tcc_key.ch, tcc_key.old_key, key);
                        tcc_key.key_status = (int32)KEY_ADC_PRESSED;
                        tcc_key.key_chk_cnt = 0;
                        *piKeyVal = tcc_key.old_key;
                        *piKPressed = 1;
                        ret_key = SAL_RET_SUCCESS;
                        key_checked = 1;
                        //break;
                    }
                    else
                    {
                        tcc_key.key_chk_cnt++;
                    }
                }
                else {
                    tcc_key.key_status = (int32)KEY_ADC_PRESSING;
                    *piKeyVal = tcc_key.old_key;
                    *piKPressed = 1;
                    ret_key = SAL_RET_FAILED;
                }
                ADCKEY_DBG("Key info[%d] : [Pressing]old_key = %d / cur_key=%d\n", i, tcc_key.old_key, key);
            }
            else
            {
                if((tcc_key.key_status == (int32)KEY_ADC_PRESSED) || (tcc_key.key_status == (int32)KEY_ADC_PRESSING))
                {
                    ADCKEY_DBG("Key info[%d] : [Released_1]ch = %d / old_key = %d / cur_key=%d\n", i, tcc_key.ch, tcc_key.old_key, key);
                    tcc_key.key_pressed = (int32)KEY_ADC_RELEASED;
                    tcc_key.key_status = (int32)KEY_ADC_RELEASED;
                    tcc_key.key_chk_cnt = 0;
                    *piKeyVal = tcc_key.old_key;
                    *piKPressed = 0;
                    ret_key = SAL_RET_SUCCESS;
                    key_checked = 1;
                    //break;
                }
                else {
                    ADCKEY_DBG("Key info[%d](No pressed key) : [Released_1]ch = %d / old_key = %d / cur_key=%d\n", i, tcc_key.ch, tcc_key.old_key, key);
                    tcc_key.old_key = -1;
                    tcc_key.key_pressed = (int32)KEY_ADC_RELEASED;
                    tcc_key.key_status = (int32)KEY_ADC_RELEASED;
                    key_checked = 0;
                }
            }
            ADCKEY_DBG("Key info[%d] : ch = %d / old_key = %d / cur_key=%d / pressed=%d\n", i, tcc_key.ch, tcc_key.old_key, key, tcc_key.key_pressed);
        }
        else
        {
            if(tcc_key.key_status == NULL)
            {
                KEY_AdcResetKeystatus();
                ret_key = SAL_RET_FAILED;
            }
            else if((tcc_key.key_status > (int32)KEY_ADC_CHECK) && (tcc_key.old_key > (int32)0) && (tcc_key.ch == i)) //released
            {
                ADCKEY_DBG("Key info[%d] : [Released_2]ch = %d /old_key = %d / cur_key=%d\n", i, tcc_key.ch, tcc_key.old_key, key);
                *piKeyVal = tcc_key.old_key;
                *piKPressed = 0;
                KEY_AdcResetKeystatus();
                ret_key = SAL_RET_SUCCESS;
                key_checked = 1;
                //break;
            }
            else
            {
                ret_key = SAL_RET_FAILED; //none key
            }
        }

        if (key_checked == 1) //stop scaning key
        {
            key_checked = 0; //key detected
            break;
        }
        else //keep scaning key
        {
            ; //none key
        }
    }


    return ret_key;
}

