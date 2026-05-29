/*
***************************************************************************************************
*
*   FileName : key.c
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

#include <app_cfg.h>

#if (ACFG_APP_KEY_EN ==1)

#include "debug.h"
#include "key_adc.h"
#include "key_gpio.h"
#include "key_i2c.h"
#include "key.h"
#include "ipc.h"

/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/

#define KEY_SEND_TO_MULTIFUL_CORE
#define KEY_APP_DEBUG_ENABLE
#ifdef KEY_APP_DEBUG_ENABLE
#include "debug.h"
#   define KEY_APP_DEBUG(fmt, args...)  (mcu_printf(fmt, ## args))
#else
#   define KEY_APP_DEBUG(fmt, args...)  (do {} while(0))
#endif


#ifdef KEY_SEND_TO_MULTIFUL_CORE
typedef enum KEYClientType
{
    KEY_CLIENT_MAIN_APP                 = 0,
    KEY_CLIENT_SUB_APP,
    KEY_CLIENT_MAX

} KEYClientType_t;

typedef struct KEYAppType
{
    uint32                              ktClientKeyRequested[KEY_CLIENT_MAX];
    uint32                              ktClientKey[KEY_CLIENT_MAX][2];

} KEYAppType_t;

static KEYAppType_t gKeyApp;
#endif


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/


static void KEY_AppAdcTask
(
    void *                              pArgs
);

static void KEY_AppRotaryTask
(
    void *                              pArgs
);

static void KEY_AppIPCHandleForMainAP
(
    uint16                              uiCmd,
    uint8 *                             puData,
    uint16                              uiLength
);

#ifdef KEY_SEND_TO_MULTIFUL_CORE
static void KEY_AppIPCHandleForSubAP
(
    uint16                              uiCmd,
    uint8 *                             puData,
    uint16                              uiLength
);
static int32 KEY_AppBitFindAndCheckMasking
(
    KEYClientType_t                     ctType,
    uint8                               uiKey
);
static int32 KEY_AppIPCSendPacketMultiTarget
(
    uint8                               uiKey,
    uint16                              uiCmd2,
    const uint8*                        puData,
    uint16                              uiLength
);
#endif

static void KEY_AppRotaryTask
(
    void *                              pArgs
);

static void KEY_AppAdcTask
(
    void *                              pArgs
);

static void KEY_AppI2CTask
(
    void *                              pArgs
);


/*
***************************************************************************************************
*                                          KEY_AppIPCHandleForMainAP
*
*
* @param    uiCmd [in]
* @param    puData [in]
* @param    uiLength [in]
*
* Notes
*
***************************************************************************************************
*/
static void KEY_AppIPCHandleForMainAP(uint16 uiCmd, uint8 *puData, uint16 uiLength) // CS : Dangerous Function Cast
{
    mcu_printf("%s\n", __func__);
#ifdef KEY_SEND_TO_MULTIFUL_CORE
    if ((uiCmd == (uint16)0x3) &&(uiLength == (uint16)8) && (puData != ((void *)0)))
    {
        gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][0] = (uint32)puData[0];
        gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][0] |= (uint32)((uint32)puData[1]<<8);
        gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][0] |= (uint32)((uint32)puData[2]<<16);
        gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][0] |= (uint32)((uint32)puData[3]<<24);

        gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][1] = (uint32)puData[4];
        gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][1] |= (uint32)((uint32)puData[5]<<8);
        gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][1] |= (uint32)((uint32)puData[6]<<16);
        gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][1] |= (uint32)((uint32)puData[7]<<24);
        if((gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][0] != 0UL) || (gKeyApp.ktClientKey[KEY_CLIENT_MAIN_APP][1] != 0UL))
        {
            gKeyApp.ktClientKeyRequested[KEY_CLIENT_MAIN_APP] = 1;
        }
    }
#endif
}
#ifdef KEY_SEND_TO_MULTIFUL_CORE


/*
***************************************************************************************************
*                                          KEY_AppIPCHandleForSubAP
*
*
* @param    uiCmd [in]
* @param    puData [in]
* @param    uiLength [in]
*
* Notes
*
***************************************************************************************************
*/
static void KEY_AppIPCHandleForSubAP(uint16 uiCmd, uint8 *puData, uint16 uiLength) // CS : Dangerous Function Cast
{
    mcu_printf("%s\n", __func__);
    if ((uiCmd == (uint16)0x3) &&(uiLength == (uint16)8) && (puData != ((void *)0)))
    {
        gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][0] = puData[0];
        gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][0] |= (uint32)((uint32)puData[1]<<8);
        gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][0] |= (uint32)((uint32)puData[2]<<16);
        gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][0] |= (uint32)((uint32)puData[3]<<24);

        gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][1] = puData[4];
        gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][1] |= (uint32)((uint32)puData[5]<<8);
        gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][1] |= (uint32)((uint32)puData[6]<<16);
        gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][1] |= (uint32)((uint32)puData[7]<<24);
        if((gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][0] != 0UL) || (gKeyApp.ktClientKey[KEY_CLIENT_SUB_APP][1] != 0UL))
        {
            gKeyApp.ktClientKeyRequested[KEY_CLIENT_SUB_APP] = 1;
        }
    }
}



/*
***************************************************************************************************
*                                          KEY_AppBitFindAndCheckMasking
*
*
* @param    ctType [in]
* @param    uiCmd2 [in]
* @param    uiKey [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static int32 KEY_AppBitFindAndCheckMasking(KEYClientType_t ctType, uint8 uiKey)
{
    int32 ret = -1;
    int32 bit_pos =-1;

    //ref https://wiki.telechips.com:8443/pages/viewpage.action?pageId=35211364, b. Key Data
    switch (uiKey)
    {
        case 0x01:
            bit_pos = 0;
            break;
        case 0x02:
            bit_pos = 1;
            break;
        case 0x03:
            bit_pos = 2;
            break;
        case 0x04:
            bit_pos = 3;
            break;
        case 0x05:
            bit_pos = 4;
            break;
        case 0x06:
            bit_pos = 5;
            break;
        case 0x11:
            bit_pos = 6;
            break;
        case 0x12:
            bit_pos = 7;
            break;
        case 0x13:
            bit_pos = 8;
            break;
        case 0x14:
            bit_pos = 9;
            break;
        case 0x15:
            bit_pos = 10;
            break;
        case 0x16:
            bit_pos = 11;
            break;
        case 0x17:
            bit_pos = 12;
            break;
        case 0x18:
            bit_pos = 13;
            break;
        case 0x19:
            bit_pos = 14;
            break;
        case 0x1a:
            bit_pos = 15;
            break;
        case 0x1b:
            bit_pos = 16;
            break;
        case 0x1c:
            bit_pos = 17;
            break;
        case 0x1d:
            bit_pos = 18;
            break;
        case 0x1e:
            bit_pos = 19;
            break;
        case 0x1f:
            bit_pos = 20;
            break;
        case 0x20:
            bit_pos = 21;
            break;
        case 0x21:
            bit_pos = 22;
            break;
        case 0x22:
            bit_pos = 23;
            break;
        case 0x23:
            bit_pos = 24;
            break;
        case 0x24:
            bit_pos = 25;
            break;
        case 0x25:
            bit_pos = 26;
            break;
        case 0x26:
            bit_pos = 27;
            break;
        case 0x27:
            bit_pos = 28;
            break;
        case 0x28:
            bit_pos = 29;
            break;
        case 0x29:
            bit_pos = 30;
            break;
        default:
            mcu_printf("Invalid Key Value \n");
            break;
    }

    if(bit_pos >= (int32)0)
    {
        if ( ((gKeyApp.ktClientKey[ctType][0]) & ((uint32)0x1<<(uint32)(bit_pos))) == 0UL)
        {
            //bit_pos = -1;
            return ret;
        }

        if ( ((gKeyApp.ktClientKey[ctType][1]) & ((uint32)0x1<<(uint32)(bit_pos))) == 0UL)
        {
            //bit_pos = -1;
            return ret;
        }
    }

    return bit_pos;
}

/*
***************************************************************************************************
*                                          KEY_AppIPCSendPacketMultiTarget
*
*
* @param    uiKey [in]
* @param    uiCmd2 [in]
* @param    puData [in]
* @param    uiLength [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static int32 KEY_AppIPCSendPacketMultiTarget(uint8 uiKey, uint16 uiCmd2, const uint8* puData, uint16 uiLength)
{
    int32 ipc_ret =-1; //QAC
    int32 bit_pos = -1;

    if ( gKeyApp.ktClientKeyRequested[KEY_CLIENT_MAIN_APP]== 1UL)
    {
        bit_pos = KEY_AppBitFindAndCheckMasking(KEY_CLIENT_MAIN_APP, uiKey);
        if (bit_pos >= 0)
        {
            if(puData != ((void *)0))
            {
            mcu_printf("%s, [KEY_IPC_SEND to MAIN CPU] ext_key(keyData[0]) : 0x%x\n", __func__, puData[0]);
            ipc_ret = IPC_SendPacket(IPC_SVC_CH_CA53_NONSECURE, (uint16)TCC_IPC_CMD_KEY, (uint16)uiCmd2, (const uint8*)puData, uiLength);
            }
        }
    }

    if ( gKeyApp.ktClientKeyRequested[KEY_CLIENT_SUB_APP]== 1UL)
    {
        bit_pos = KEY_AppBitFindAndCheckMasking(KEY_CLIENT_SUB_APP, uiKey);
        if (bit_pos >= 0)
        {
            if(puData != ((void *)0))
            {
            mcu_printf("%s, [KEY_IPC_SEND to SUB CPU] ext_key(keyData[0]) : 0x%x\n", __func__, puData[0]);
            ipc_ret = IPC_SendPacket(IPC_SVC_CH_CA72_NONSECURE, (uint16)TCC_IPC_CMD_KEY, (uint16)uiCmd2, (const uint8*)puData, uiLength);
            }
        }
    }

    return ipc_ret;
}
#endif



/*
***************************************************************************************************
*                                          KEY_AppRotaryTask
*
*
* @param    pArgs [in]
*
* Notes
*
***************************************************************************************************
*/
static void KEY_AppRotaryTask(void * pArgs)
{
    GPIORotaryKeyType_t rotary_value;
    uint8 ch = 0;
    int32 ipc_ret = 0;

    (void)pArgs;
    (void)KEY_GpioKeyOpen(0); 

    while (1) {
        ch = (uint8)KEY_ROTARY_CH1;
        rotary_value = KEY_GpioGetRotaryValue(ch);

        if (rotary_value != KEY_ROTARY_KEY_MAX)
        {
            mcu_printf("rotary_value 1 = %d \n",rotary_value);
            #ifdef KEY_SEND_TO_MULTIFUL_CORE
            ipc_ret = KEY_AppIPCSendPacketMultiTarget((uint8)rotary_value,  (uint16)TCC_IPC_CMD_KEY_ROTARY, (uint8*)&rotary_value, (uint16)1);
            #else
            ipc_ret = IPC_SendPacket(IPC_SVC_CH_CA53_NONSECURE, (uint16)TCC_IPC_CMD_KEY, (uint16)TCC_IPC_CMD_KEY_ROTARY, (uint8*)&rotary_value, (uint16)1);
            #endif
            mcu_printf("%s, [ROT_KEY_IPC_SEND] rotary_value_CH1 : %d, size : 1\n", __func__, rotary_value);
            rotary_value = KEY_ROTARY_KEY_MAX;
        }


        ch = (uint8)KEY_ROTARY_CH2;
        rotary_value = KEY_GpioGetRotaryValue(ch);

        if (rotary_value != KEY_ROTARY_KEY_MAX)
        {
            mcu_printf("rotary_value 2 = %d \n",rotary_value);
            #ifdef KEY_SEND_TO_MULTIFUL_CORE
            ipc_ret = KEY_AppIPCSendPacketMultiTarget((uint8)rotary_value,  (uint16)TCC_IPC_CMD_KEY_ROTARY, (uint8*)&rotary_value, (uint16)1);
            #else
            ipc_ret = IPC_SendPacket(IPC_SVC_CH_CA53_NONSECURE, (uint16)TCC_IPC_CMD_KEY, (uint16)TCC_IPC_CMD_KEY_ROTARY, (uint8*)&rotary_value, (uint16)1);
            #endif
            mcu_printf("%s, [ROT_KEY_IPC_SEND] rotary_value_CH2 : %d, size : 1, ret : %d\n", __func__, rotary_value, ipc_ret);
            rotary_value = KEY_ROTARY_KEY_MAX;
        }

        (void)SAL_TaskSleep(5);

    }


}

/*
***************************************************************************************************
*                                          KEY_AppAdcTask
*
*
* @param    pArgs [in]
*
* Notes
*
***************************************************************************************************
*/
static void KEY_AppAdcTask(void * pArgs)
{
    //int32 prev_key = 0;//warn.. set but not used
    int32 ext_key = 0;
    int32 key_status = KEY_STAT_RELEASED;
    SALRetCode_t ret_key = SAL_RET_FAILED;
    uint8 keyData[2];
    int32 ipc_ret = 0;

    //  KeyAppAdcHalHandle = HAL_Open(HAL_ADC_KEY, 0);
    (void)pArgs;
    IPC_RegisterCbFunc(IPC_SVC_CH_CA53_NONSECURE, TCC_IPC_CMD_KEY, (IPCCallback)&KEY_AppIPCHandleForMainAP, (void *) 0, (void *) 0);//Msg(4:0313) Casting to different function pointer type. MISRA-C:2004 Rule 11.1; REFERENCE - ISO-6.3.4 Cast Operators
    #ifdef KEY_SEND_TO_MULTIFUL_CORE
    IPC_RegisterCbFunc(IPC_SVC_CH_CA72_NONSECURE, TCC_IPC_CMD_KEY, (IPCCallback)&KEY_AppIPCHandleForSubAP, (void *) 0, (void *) 0);//Msg(4:0313) Casting to different function pointer type. MISRA-C:2004 Rule 11.1; REFERENCE - ISO-6.3.4 Cast Operators
    #endif
    (void)KEY_AdcOpen(0);


    while (1)
    {

        ret_key = KEY_AdcScan(&ext_key, &key_status);
        if(ret_key == SAL_RET_SUCCESS)
        {

            mcu_printf("ext_key 1 = %d \n",ext_key);
            keyData[0] = ((uint8)ext_key & (uint8)0xFF);
            keyData[1] = ((uint8)key_status & (uint8)0xFF);

            if(keyData[1] == (uint8)0x00)
            {
                keyData[1] = (uint8)0x02;
            }

            #ifdef KEY_SEND_TO_MULTIFUL_CORE
            ipc_ret = KEY_AppIPCSendPacketMultiTarget(keyData[0],  (uint16)TCC_IPC_CMD_KEY_EXT, (uint8*)keyData, (uint16)2);
            #else
            ipc_ret = IPC_SendPacket(IPC_SVC_CH_CA53_NONSECURE, (uint16)TCC_IPC_CMD_KEY, (uint16)TCC_IPC_CMD_KEY_EXT, (uint8*)keyData, (uint16)2);
            #endif

            mcu_printf("%s, [ADC_KEY_IPC_SEND] ext_key(keyData[0]) : 0x%x, key_status(keyData[1]) : 0x%x, size : 2, ret : %d\n", __func__, keyData[0], keyData[1], ipc_ret);
            //prev_key = ext_key;//warn.. set but not used
        }
        //else
        {
            ;
        }

        (void)SAL_TaskSleep(10);
    }

}


/*
***************************************************************************************************
*                                          KEY_AppI2CTask
*
*
* @param    pArgs [in]
*
* Notes
*
***************************************************************************************************
*/
static void KEY_AppI2CTask(void * pArgs)
{

    int32 ext_key = 0;
    int32 ret_key;
    int32 key_status = KEY_STAT_RELEASED;
    uint8 keyData[2];
    int32 ipc_ret = 0;

    (void)pArgs;
    (void)KEY_I2cOpen();

    while (1)
    {
        ret_key = KEY_I2cScan(&ext_key, &key_status);
        if(ret_key == (int32)SAL_RET_SUCCESS)
        {
            //mcu_printf("ext_key 1 = %d \n",ext_key);
            keyData[0] = ((uint8)ext_key & (uint8)0xFF);
            keyData[1] = ((uint8)key_status & (uint8)0xFF);
            if(keyData[1] == (uint8)0x00)
            {
                keyData[1] = (uint8)0x02;
            }
            #ifdef KEY_SEND_TO_MULTIFUL_CORE
            ipc_ret = KEY_AppIPCSendPacketMultiTarget(keyData[0],  (uint16)TCC_IPC_CMD_KEY_EXT, (uint8*)keyData, 2);
            #else
            ipc_ret = IPC_SendPacket(IPC_SVC_CH_CA53_NONSECURE, (uint16)TCC_IPC_CMD_KEY, (uint16)TCC_IPC_CMD_KEY_EXT, (uint8*)keyData, 2);
            #endif
            //mcu_printf("%s, [I2C_KEY_IPC_SEND] ext_key(keyData[0]) : 0x%x, key_status(keyData[1]) : 0x%x, size : 2, ret : %d\n", __func__, keyData[0], keyData[1], ipc_ret);
			(void)ipc_ret; //fix compile warning [set but not used]
        }
        else
        {
            ;
        }


        (void)SAL_TaskSleep(5);
    }
}





/*
***************************************************************************************************
*                                          KEY_AppCreate
*
*
*
* Notes
*
***************************************************************************************************
*/
void KEY_AppCreate(void)
{
    static uint32 KeyAppAdcTaskID;
    static uint32 KeyAppAdcTaskStk[KEY_APP_TASK_STK_SIZE];
    static uint32 KeyAppRotaryTaskID;
    static uint32 KeyAppRotaryTaskStk[KEY_APP_TASK_STK_SIZE];
    static uint32 KeyAppI2CTaskID;
    static uint32 KeyAppI2CTaskStk[KEY_APP_TASK_STK_SIZE];

#ifdef KEY_SEND_TO_MULTIFUL_CORE
    (void) SAL_MemSet((void*)&gKeyApp, 0x00, sizeof(KEYAppType_t));
#endif
    (void) SAL_TaskCreate(&KeyAppAdcTaskID, (const uint8 *)"KeyAppAdcTask", (SALTaskFunc)&KEY_AppAdcTask,
                        (void * const)&KeyAppAdcTaskStk[0], KEY_APP_TASK_STK_SIZE, SAL_PRIO_KEY_APP, (void *) 0);
    //CS : Empty if Statement - Remove redundant if statement.

    (void) SAL_TaskCreate(&KeyAppRotaryTaskID, (const uint8 *)"KeyAppRotaryTask", (SALTaskFunc)&KEY_AppRotaryTask,
                        (void * const)&KeyAppRotaryTaskStk[0], KEY_APP_TASK_STK_SIZE, SAL_PRIO_KEY_APP, (void *) 0);
    //CS : Empty if Statement - Remove redundant if statement.
    (void) SAL_TaskCreate(&KeyAppI2CTaskID, (const uint8 *)"KeyAppI2CTask", (SALTaskFunc)&KEY_AppI2CTask,
                        (void * const)&KeyAppI2CTaskStk[0], KEY_APP_TASK_STK_SIZE, SAL_PRIO_KEY_APP, (void *) 0);
}
#endif

