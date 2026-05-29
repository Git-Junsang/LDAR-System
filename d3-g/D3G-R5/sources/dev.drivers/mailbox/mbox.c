/*
***************************************************************************************************
*
*   FileName : mbox.c
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
#include <sal_com.h>
#include <sal_internal.h>
#include <mbox.h>
#include <mbox_phy.h>
#include <mbox_dev.h>
#include <debug.h>

static MBOXCallback_t mboxCallback;

static int32 MBOX_OsInit
(
    uint32 *                            uiLock
)
{
    int32 ret;

    ret = (int32) SAL_SemaphoreCreate(uiLock, (const uint8 *)"Mutex Created", 1UL, SAL_OPT_BLOCKING);

    if (ret != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_SemaphoreCreate fail [%d]\n", __func__, (sint32) ret);
    }

    return ret;
}

static int32 MBOX_WrLock
(
    uint32                              uiLock
)
{
    int32 ret;

    ret = (int32) SAL_SemaphoreWait(uiLock, 0, SAL_OPT_BLOCKING);

    if (ret != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_SemaphoreWait uiLock[%d] fail [%d]\n", __func__, (sint32) uiLock, (sint32) ret);
    }

    return ret;
}

static int32 MBOX_WrUnlk
(
    uint32                              uiLock
)
{
    int32 ret;

    ret = (int32) SAL_SemaphoreRelease(uiLock);

    if (ret != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_SemaphoreRelease uiLock[%d] fail [%d]\n", __func__, (sint32) uiLock, (sint32) ret);
    }

    return ret;
}

static int32 MBOX_RsInit
(
    uint32 *                            uiId
)
{
    int32 ret;

    ret = (int32) SAL_EventCreate(uiId, (const uint8 *)"signal event created", 0UL);

    if (ret != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_EventCreate uiId[%d] fail [%d]\n", __func__, ((uiId != ((void *)0)) ? ((sint32) *uiId) : -1), (sint32) ret);
    }

    return ret;
}

static int32 MBOX_RdWake
(
    uint32                              uiId
)
{
    int32 ret;

    ret = (int32) SAL_EventSet(uiId, 0x00000001UL, SAL_EVENT_OPT_FLAG_SET);

    if (ret != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_EventSet uiId[%d] fail [%d]\n", __func__, (sint32) uiId, (sint32) ret);
    }

    return ret;
}

static int32 MBOX_RdWait
(
    uint32                              uiId
)
{
    uint32  flag;
    int32   ret;
    int32   crl;
    int32   err;

    flag    = 0;
    crl     = MBOX_SUCCESS;
    err     = (int32) SAL_EventGet(uiId, 0x00000001UL, 0UL, (((uint32) SAL_EVENT_OPT_SET_ANY)|((uint32) SAL_OPT_BLOCKING)), &flag);

    if (err == (int32) SAL_RET_SUCCESS)
    {
        if (flag == 0UL)
        {
            MBOX_D("SAL_EventGet Timeout uiId[%d] [%d]\n", __func__, (sint32) uiId, (sint32) flag);
            ret = MBOX_ERR_TIMEOUT;
        }
        else
        {
            ret = MBOX_SUCCESS;
        }
    }
    else
    {
        MBOX_E("SAL_EventGet uiId[%d] Fail [%d]\n", __func__, (sint32) uiId, (sint32) flag);
    }

    crl = (int32) SAL_EventSet(uiId, 0x00000001UL, SAL_EVENT_OPT_CLR_ALL);

    if (crl != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_EventSet uiId[%d] fail [%d]\n", __func__, (sint32) uiId, (sint32) crl);
    }

    return ret;
}

static int32 MBOX_WsInit
(
    uint32 *                            uiId
)
{
    int32 ret;

    ret = (int32) SAL_EventCreate(uiId, (const uint8 *)"signal event created", 0UL);

    if (ret != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_EventCreate uiId[%d] fail [%d]\n", __func__, ((uiId != ((void *) 0)) ? ((sint32) *uiId) : -1), (sint32) ret);
    }

    return ret;
}

static int32 MBOX_WrWclr
(
    uint32                              uiId
)
{
    int32 ret;

    ret = (int32) SAL_EventSet(uiId, 0x00000001UL, SAL_EVENT_OPT_CLR_ALL);

    if (ret != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_EventSet uiId[%d] fail [%d]\n", __func__, (sint32) uiId, (sint32) ret);
    }

    return ret;
}

static int32 MBOX_WrWake
(
    uint32                              uiId
)
{
    int32 ret;

    ret = (int32) SAL_EventSet(uiId, 0x00000001UL, SAL_EVENT_OPT_FLAG_SET);

    if (ret != (int32) SAL_RET_SUCCESS)
    {
        MBOX_E("SAL_EventSet uiId[%d] fail [%d]\n", __func__, (sint32) uiId, (sint32) ret);
    }

    return ret;
}

static int32 MBOX_WrWait
(
    uint32                              uiId
)
{
    uint32  flag;
    int32   ret;
    int32   err;

    flag    = 0;
    err     = (int32) SAL_EventGet(uiId, 0x00000001UL, 3UL, (((uint32) SAL_EVENT_OPT_SET_ANY)|((uint32) SAL_OPT_BLOCKING)), &flag);

    if (err == (int32) SAL_RET_SUCCESS)
    {
        if (flag == 0UL)
        {
            MBOX_D("SAL_EventGet Timeout uiId[%d] [%d]\n", __func__, (sint32) uiId, (sint32) flag);
            ret = MBOX_ERR_TIMEOUT;
        }
        else
        {
            ret = MBOX_SUCCESS;
        }
    }
    else
    {
        MBOX_E("SAL_EventGet uiId[%d] Fail [%d]\n", __func__, (sint32) uiId, (sint32) flag);
    }

    return ret;
}

void MBOX_Init
(
    void
)
{
    mboxCallback.osInit = &MBOX_OsInit;     // Thread Safe For Writing.
    mboxCallback.wrLock = &MBOX_WrLock;     // Thread Safe For Writing.
    mboxCallback.wrUnlk = &MBOX_WrUnlk;     // Thread Safe For Writing.
    mboxCallback.rsInit = &MBOX_RsInit;     // Blocking Read  Depending on OS.
    mboxCallback.rdWake = &MBOX_RdWake;     // Blocking Read  Depending on OS.
    mboxCallback.rdWait = &MBOX_RdWait;     // Blocking Read  Depending on OS.
    mboxCallback.wsInit = &MBOX_WsInit;     // Blocking Write Depending on OS.
    mboxCallback.wrWake = &MBOX_WrWake;     // Blocking Write Depending on OS.
    mboxCallback.wrWclr = &MBOX_WrWclr;     // Blocking Write Depending on OS.
    mboxCallback.wrWait = &MBOX_WrWait;     // Blocking Write Depending on OS.

    if (MBOX_DevInit(MBOX_CH_CA53MP_SECURE, MBOX_INT_LEVEL_8, MBOX_MAX_MULTI_OPEN, &mboxCallback) != 0)
    {
        MBOX_E("mbox init fail for MBOX_CH_CA53\n");
    }

    if (MBOX_DevInit(MBOX_CH_CA53MP_NONSECURE, MBOX_INT_LEVEL_8, MBOX_MAX_MULTI_OPEN, &mboxCallback) != 0)
    {
        MBOX_E("mbox init fail for MBOX_CH_CA53_NS\n");
    }

    if (MBOX_DevInit(MBOX_CH_CA72MP_SECURE, MBOX_INT_LEVEL_8, MBOX_MAX_MULTI_OPEN, &mboxCallback) != 0)
    {
        MBOX_E("mbox init fail for MBOX_CH_CA07\n");
    }

    if (MBOX_DevInit(MBOX_CH_CA72MP_NONSECURE, MBOX_INT_LEVEL_8, MBOX_MAX_MULTI_OPEN, &mboxCallback) != 0)
    {
        MBOX_E("mbox init fail for MBOX_CH_CA07_NS\n");
    }
#if defined(MCU_BSP_SOC_REV_TCC805x_ES)
    if (MBOX_DevInit(MBOX_CH_CM4_NONSECURE, MBOX_INT_LEVEL_8, MBOX_MAX_MULTI_OPEN, &mboxCallback) != 0)
    {
        MBOX_E("mbox init fail for MBOX_CH_CM04_NS\n");
    }
#endif

    if (MBOX_DevInit(MBOX_CH_HSM_NONSECURE, MBOX_INT_LEVEL_8, MBOX_MAX_MULTI_OPEN, &mboxCallback) != 0)
    {
        MBOX_E("mbox init fail for MBOX_CH_CM04_NS\n");
    }
}

