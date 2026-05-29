/*
***************************************************************************************************
*
*   FileName : dse.c
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

#include <dse.h>
#include <gic.h>
#include <bsp.h>

#if (DEBUG_ENABLE)
    #include "debug.h"

    #define DSE_D(fmt,args...)          {LOGD(DBG_TAG_DSE, fmt, ## args)}
    #define DSE_E(fmt,args...)          {LOGE(DBG_TAG_DSE, fmt, ## args)}
#else
    #define DSE_D(fmt,args...)
    #define DSE_E(fmt,args...)
#endif

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

typedef struct DSEReg
{
    uint32                              rDSE_IRQ_STATUS;
    uint32                              rDSE_IRQ_MASK;
    uint32                              rDSE_IRQ_EN;
    uint32                              rDEF_SLV_CFG;
    uint32                              rDEF_SLV0_ADDR;
    uint32                              rDEF_SLV1_ADDR;
    uint32                              rDEF_SLV2_ADDR;
    uint32                              rDEF_SLV3_ADDR;
    uint32                              rDEF_SLV4_ADDR;
    uint32                              rDEF_SLV5_ADDR;
    uint32                              rDEF_SLV6_ADDR;
    uint32                              rDEF_SLV7_ADDR;
    uint32                              rDEF_SLV8_ADDR;
    uint32                              rDEF_RESERVED[3];
    uint32                              rDSE_IH_CFG_WR_PW;
    uint32                              rDSE_IH_CFG_WR_LOCK;
    uint32                              rSF_CHK_GRP_EN;
    uint32                              rSF_CHK_GRP_STS;
} DSEReg_t;

#define DSE_Cont                        ((volatile DSEReg_t         *)(0x1B938000UL))

#define DSE_PW                          (0x5AFEACE5UL)

/**************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/

uint32                                  gWait_irq   = 0UL;    /* using by unittest                */

static SALAddr                          gDseErrAddr = 0UL;    /* the address that caused the error*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void DSE_IRQ_Isr
(
    void *                               pArg
);

/*
***************************************************************************************************
*                                       DSE_WriteLock
*
* Default Slave Error Register Write Protection Enable/Disable
*
* @param    uiLock [in] Write Protection 1 : Enable, 0 : Disable
*
* @return   The result of Write Protection Function setting
*           SAL_RET_SUCCESS  Write Protection register setting succeed
*           SAL_RET_FAILED   Write Protection register setting fail
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t DSE_WriteLock
(
    uint32                              uiLock
)
{
    uint32       uiConfPw;
    SALRetCode_t ucRet;

    ucRet                       = (SALRetCode_t)SAL_RET_SUCCESS;
    DSE_Cont->rDSE_IH_CFG_WR_PW = DSE_PW;
    uiConfPw                    = DSE_Cont->rDSE_IH_CFG_WR_PW;

    if(uiConfPw == 1UL)
    {
        if(uiLock == 1UL)                        /* lock mode */
        {
            DSE_Cont->rDSE_IH_CFG_WR_LOCK = 1UL;
        }
        else
        {
            DSE_Cont->rDSE_IH_CFG_WR_LOCK = 0UL;
        }
    }
    else
    {
        ucRet = (SALRetCode_t)SAL_RET_FAILED;
        DSE_E("  Fail set password\n");
    }

    return ucRet;
}

/*
***************************************************************************************************
*                                       DSE_IRQ_Isr
*
* Default Slave Error Interrupt Service Routine.
*
* @param    pArg [in] none
*
* @return   none
*
* Notes     update error address to gDseErrAddr
*
***************************************************************************************************
*/

static void DSE_IRQ_Isr
(
    void *                              pArg
)
{
    uint32 uiDseSts;

    (void)pArg;
    uiDseSts = DSE_Cont->rDSE_IRQ_STATUS;

    if(uiDseSts == DSE_INTERCON)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV0_ADDR;
    }
    else if(uiDseSts == DSE_AP_MBOX)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV1_ADDR;
    }
    else if(uiDseSts == DSE_SYSPERI)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV2_ADDR;
    }
    else if(uiDseSts == DSE_UART)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV3_ADDR;
    }
    else if(uiDseSts == DSE_PDM)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV4_ADDR;
    }
    else if(uiDseSts == DSE_MC_MBOX)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV5_ADDR;
    }
    else if(uiDseSts == DSE_CAN)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV6_ADDR;
    }
    else if(uiDseSts == DSE_GPSB)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV7_ADDR;
    }
    else if(uiDseSts == DSE_ISOLATE)
    {
        gDseErrAddr = DSE_Cont->rDEF_SLV8_ADDR;
    }
    else
    {
        DSE_E("unexpected code flow\n");
    }

    DSE_D("##### Default Slave Error occurred(IRQ 0x%x), addr x%x\n", uiDseSts, gDseErrAddr);
    (void)DSE_WriteLock(0UL);
    DSE_Cont->rDSE_IRQ_STATUS   = uiDseSts;
    (void)DSE_WriteLock(1UL);
    gWait_irq = 0UL;

    return;
}

/*
***************************************************************************************************
*                                       DSE_Deinit
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void DSE_Deinit
(
    void
)
{
    (void)GIC_IntVectSet
    (
        GIC_DEFAULT_SLV_ERR,
        GIC_PRIORITY_NO_MEAN,
        GIC_INT_TYPE_LEVEL_HIGH,
        NULL,
        NULL
    );
    (void)GIC_IntSrcDis(GIC_DEFAULT_SLV_ERR);
    DSE_Cont->rDSE_IRQ_MASK = 0UL;
    DSE_Cont->rDSE_IRQ_EN   = 0UL;

    return;
}

/*
***************************************************************************************************
*                                       DSE_Init
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t DSE_Init
(
    uint32                              uiModeFlag
)
{
    uint32       uiSlvCfg;
    uint32       uiMode;
    SALRetCode_t ucRet;

    uiSlvCfg = 0UL;
    uiMode   = 0UL;
    ucRet    = (SALRetCode_t)SAL_RET_FAILED;

    (void)GIC_IntVectSet
    (
        GIC_DEFAULT_SLV_ERR,
        GIC_PRIORITY_NO_MEAN,
        GIC_INT_TYPE_EDGE_RISING,
        (GICIsrFunc)&DSE_IRQ_Isr,
        NULL
    );
    (void)GIC_IntSrcEn(GIC_DEFAULT_SLV_ERR);
    ucRet   = DSE_WriteLock(0UL);

    if(ucRet == SAL_RET_SUCCESS)
    {
        DSE_Cont->rDSE_IRQ_MASK = 0UL;
        DSE_Cont->rDSE_IRQ_EN   = 1UL;

        uiMode = (uiModeFlag & DSE_MODE_MASK);

        if(uiMode == DSE_RES_MODE)
        {
            uiSlvCfg = (uiModeFlag & DES_SEL_MASK);
        }
        else if(uiMode == DSE_INT_MODE)
        {
            uiSlvCfg = ((uiModeFlag & DES_SEL_MASK)<<16UL);
        }
        else if(uiMode == DSE_RES_INT_MODE)
        {
            uiSlvCfg = ((uiModeFlag & DES_SEL_MASK) | ((uiModeFlag & DES_SEL_MASK)<<16UL));
        }
        else
        {
            uiSlvCfg = 0UL;
        }

        DSE_Cont->rDEF_SLV_CFG      = uiSlvCfg;
        DSE_Cont->rDSE_IH_CFG_WR_PW = DSE_PW;
        DSE_Cont->rSF_CHK_GRP_EN    = DSE_SM_SF_CHK_GRP_SEL;
    }

    ucRet = DSE_WriteLock(1UL);

    return ucRet;
}


/*
***************************************************************************************************
*                                       DSE_GetGrpSts
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   rSF_CHK_GRP_STS : Soft Fault Check Group Status Register value
*
* Notes
*
***************************************************************************************************
*/
uint32 DSE_GetGrpSts
(
    void
)
{
    uint32 uiRet;

    uiRet = DSE_Cont->rSF_CHK_GRP_STS;

    return uiRet;
}

