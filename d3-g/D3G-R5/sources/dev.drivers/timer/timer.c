/*
***************************************************************************************************
*
*   FileName : timer.c
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

#include "timer.h"

#include <reg_phys.h>
#include <bsp.h>
#include <gic.h>
#include <debug.h>

#include <clock.h>
#include <clock_dev.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#if (DEBUG_ENABLE)
#   include <debug.h>
#   define TMR_D(fmt, args...)          {LOGD(DBG_TAG_TIMER, fmt, ## args)}
#   define TMR_E(fmt, args...)          {LOGE(DBG_TAG_TIMER, fmt, ## args)}
#else
#   define TMR_D(fmt, args...)
#   define TMR_E(fmt, args...)
#endif


/* Register Map */
#define TMR_OP_EN_CFG                   (0x000UL)
#define TMR_MAIN_CNT_LVD                (0x004UL)
#define TMR_CMP_VALUE0                  (0x008UL)
#define TMR_CMP_VALUE1                  (0x00CUL)
#define TMR_PSCL_CNT                    (0x010UL)
#define TMR_MAIN_CNT                    (0x014UL)
#define TMR_IRQ_CTRL                    (0x018UL)

/* Configuration Value */
#define TMR_OP_EN_CFG_LDM0_ON           (1UL << 28UL)
#define TMR_OP_EN_CFG_LDM1_ON           (1UL << 29UL)
#define TMR_OP_EN_CFG_OPMODE_FREE_RUN   (0UL << 26UL)
#define TMR_OP_EN_CFG_OPMODE_ONE_SHOT   (1UL << 26UL)
#define TMR_OP_EN_CFG_LDZERO_OFFSET     (25UL)
#define TMR_OP_EN_CFG_CNT_EN            (1UL << 24UL)

/* 0: Reading this register to be cleared, 1: Writing a non-zero value to MASKED_IRQ_STATUS to be cleared */
#define TMR_IRQ_CLR_CTRL_WRITE          (1UL << 31UL)
#define TMR_IRQ_CLR_CTRL_READ           (0UL << 31UL)
#define TMR_IRQ_MASK_ALL                (0x1FUL)
#define TMR_IRQ_CTRL_IRQ_EN0            (1UL << 16UL)
#define TMR_IRQ_CTRL_IRQ_EN1            (2UL << 16UL)
#define TMR_IRQ_CTRL_IRQ_EN2            (4UL << 16UL)
#define TMR_IRQ_CTRL_IRQ_EN3            (8UL << 16UL)
#define TMR_IRQ_CTRL_IRQ_EN4            (16UL << 16UL)
#define TMR_IRQ_CTRL_IRQ_ALLEN          ((TMR_IRQ_CTRL_IRQ_EN0)         \
                                        | (TMR_IRQ_CTRL_IRQ_EN1)        \
                                        | (TMR_IRQ_CTRL_IRQ_EN2)        \
                                        | (TMR_IRQ_CTRL_IRQ_EN3)        \
                                        | (TMR_IRQ_CTRL_IRQ_EN4))

#define TMR_PRESCALE                    (11UL)
#define TMR_CLK_RATE                    ((12UL) * (1000UL) * (1000UL))

typedef struct TIMERResourceTable
{
    TIMERChannel_t                      rtChannel;
    boolean                             rtUsed;
    TIMERHandler                        rtHandler;
    void *                              rtArgs;
} TIMERResource_t;

/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/
static boolean          gTimerInitialized = FALSE;

static TIMERResource_t  gTimerRes[TIMER_CH_MAX];

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void TIMER_Handler
(
    void *                              pArgs
);

static uint32 TIMER_GetRate
(
    TIMERConfig_t *                     pCfg
);

/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/

static void TIMER_Handler
(
    void *                              pArgs
)
{
    TIMERResource_t *   timer;
    uint32              reg;

    timer   = (TIMERResource_t *)pArgs;

    if (timer != NULL_PTR)
    {
        reg = MCU_BSP_TIMER_BASE + ((uint32)timer->rtChannel * 0x100UL) + TMR_IRQ_CTRL;

        if (((SAL_ReadReg(reg) & TMR_IRQ_CTRL_IRQ_ALLEN) != 0UL) && (timer->rtUsed == TRUE))
        {
            (void)TIMER_InterruptClear(timer->rtChannel);

            if (timer->rtHandler != NULL_PTR)
            {
                timer->rtHandler(timer->rtChannel, timer->rtArgs);
            }
        }
    }
}

static uint32 TIMER_GetRate
(
    TIMERConfig_t *                     pCfg
)
{
    uint32 retRate;

    retRate = 0;

    switch (pCfg->ctCounterMode)
    {
        case TIMER_COUNTER_MAIN :
        {
            retRate = (pCfg->ctMainValueUsec < 1000UL) ? (TMR_CLK_RATE * 2UL) : TMR_CLK_RATE;
            break;
        }

        case TIMER_COUNTER_COMP0 :
        {
            retRate = (pCfg->ctCmp0ValueUsec< 1000UL) ? (TMR_CLK_RATE * 2UL) : TMR_CLK_RATE;
            break;
        }

        case TIMER_COUNTER_COMP1 :
        {
            retRate = (pCfg->ctCmp1ValueUsec < 1000UL) ? (TMR_CLK_RATE * 2UL) : TMR_CLK_RATE;
            break;
        }

        case TIMER_COUNTER_SMALL_COMP :
        {
            // It need the smaller one.
            if (pCfg->ctCmp0ValueUsec > pCfg->ctCmp1ValueUsec)
            {
                retRate = (pCfg->ctCmp1ValueUsec < 1000UL) ? (TMR_CLK_RATE * 2UL) : TMR_CLK_RATE;
            }
            else
            {
                retRate = (pCfg->ctCmp0ValueUsec < 1000UL) ? (TMR_CLK_RATE * 2UL) : TMR_CLK_RATE;
            }

            break;
        }

        default :
        {
            TMR_E("unexpected code flow\n");
            break;
        }

    }

    return retRate;
}

/*
***************************************************************************************************
*                                          TIMER_InterruptClear
*
* This function clears the interrupt signal that is occurred when meeting the conditions.
*
* @param    uiChannel [in] The specific channel for TIMER module
* @return
*
* Notes
*   It is called automatically with dealing with TIMER handler.
*   Therefore, user don't have chance to call this function.
*
***************************************************************************************************
*/
SALRetCode_t TIMER_InterruptClear
(
    TIMERChannel_t                      uiChannel
)
{
    uint32  reg;
    uint32  clr_ctl;

    reg     = MCU_BSP_TIMER_BASE + ((uint32)uiChannel * 0x100U) + TMR_IRQ_CTRL;
    clr_ctl = SAL_ReadReg(reg);

    if ((clr_ctl & TMR_IRQ_CLR_CTRL_WRITE) != 0U)
    {
        SAL_WriteReg(clr_ctl | TMR_IRQ_MASK_ALL, reg);
    }
    else    // TMR_IRQ_CLR_CTRL_READ
    {
        SAL_ReadReg(reg);
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          TIMER_Enable
*
* This function enables the specific TIMER module with TIMER_OP_FREERUN, TIMER_START_ZERO,
* TIMER_COUNTER_COMP0
*
* @param    uiChannel [in] The specific channel for TIMER module
* @param    uiUSec [in] The micro-second that user want to set for interrupt signal
* @param    fnHandler [in] The external handler which user needs to register
* @param    pArgs [in] The pointer of configuration structure table
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t TIMER_Enable
(
    TIMERChannel_t                      uiChannel,
    uint32                              uiUSec,
    TIMERHandler                        fnHandler,
    void *                              pArgs
)
{
    return TIMER_EnableWithMode(uiChannel, uiUSec, TIMER_OP_FREERUN, fnHandler, pArgs);
}

/*
***************************************************************************************************
*                                          TIMER_EnableWithMode
*
* This function enables the specific TIMER module with TIMER_START_ZERO, TIMER_COUNTER_COMP0.
*
* @param    uiChannel [in] The specific channel for TIMER module
* @param    uiUSec [in] The micro-second that user want to set for interrupt signal
* @param    uiOpMode [in] Operation mode, Free running or One-time running
* @param    fnHandler [in] The external handler which user needs to register
* @param    pArgs [in] The pointer of configuration structure table
* @return
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t TIMER_EnableWithMode
(
    TIMERChannel_t                      uiChannel,
    uint32                              uiUSec,
    TIMEROpMode_t                       uiOpMode,
    TIMERHandler                        fnHandler,
    void *                              pArgs
)
{
    TIMERConfig_t cfg;

    cfg.ctChannel       = uiChannel;
    cfg.ctStartMode     = TIMER_START_ZERO;
    cfg.ctOpMode        = uiOpMode;
    cfg.ctCounterMode   = TIMER_COUNTER_COMP0;
    cfg.ctMainValueUsec = 0;
    cfg.ctCmp0ValueUsec = uiUSec;
    cfg.ctCmp1ValueUsec = 0;
    cfg.fnHandler       = fnHandler;
    cfg.pArgs           = pArgs;

    return TIMER_EnableWithCfg(&cfg);
}

/*
***************************************************************************************************
*                                          TIMER_EnableWithCfg
*
* This function enables the specific TIMER module with configuration structure
*
* @param    pCfg [in] Configuration structure with TIMER module information
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t TIMER_EnableWithCfg
(
    TIMERConfig_t *                     pCfg
)
{
    uint32          clk_id;
    uint32          mainval;
    uint32          tmpval;
    uint32          cmpval0;
    uint32          cmpval1;
    uint32          reg;
    uint32          rate;
    uint32          rate_factor;
    SALRetCode_t    ret;
    uint32          reg_cfgval;
    uint32          reg_irqval;

    cmpval0     = 0x0UL;
    cmpval1     = 0x0UL;
    ret         = SAL_RET_SUCCESS;
    reg_cfgval  = 0x0UL;
    reg_irqval  = 0x0UL;

    // MISRA 14.7 : A function shall have a single point of exit at the end of the function
    if (gTimerInitialized == FALSE)
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_TMR,
            TIMER_API_ENABLE_CFG,
            TIMER_ERR_NOT_INITIALIZED,
            __FUNCTION__
        );
    }
    else if (pCfg == NULL_PTR)
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_TMR,
            TIMER_API_ENABLE_CFG,
            TIMER_ERR_INVALID_PARAM,
            __FUNCTION__
        );
    }
    else if (TIMER_CH_MAX <= pCfg->ctChannel)
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_TMR,
            TIMER_API_ENABLE_CFG,
            TIMER_ERR_OUTOF_RANGE_CHANNEL,
            __FUNCTION__
        );
    }
    else
    {
        rate = TIMER_GetRate(pCfg);

        if (rate == 0UL)
        {
            ret = SAL_DbgReportError
            (
                SAL_DRVID_TMR,
                TIMER_API_ENABLE_CFG,
                TIMER_ERR_NOT_SUPPORT_MODE,
                __FUNCTION__
            );
        }
        else
        {
            clk_id  = (uint32)CLOCK_PERI_TIMER0 + (uint32)pCfg->ctChannel;
            reg     = MCU_BSP_TIMER_BASE + ((uint32)pCfg->ctChannel * 0x100UL);

            (void)CLOCK_SetPeriRate((sint32)clk_id, rate);
            (void)CLOCK_EnablePeri((sint32)clk_id);

            if (CLOCK_GetPeriRate((sint32)clk_id) != rate)
            {
                ret = SAL_DbgReportError
                (
                    SAL_DRVID_TMR,
                    TIMER_API_ENABLE_CFG,
                    TIMER_ERR_SET_CLOCK,
                    __FUNCTION__
                );
            }
            else
            {
                reg_irqval  = TMR_IRQ_CTRL_IRQ_EN2;
                rate_factor = (rate / 1000UL) / ((TMR_PRESCALE + 1UL) * 1000UL);
                mainval     = (pCfg->ctMainValueUsec == 0UL) ? 0xFFFFFFFFUL : ((pCfg->ctMainValueUsec * rate_factor) - 1UL);

                switch (pCfg->ctCounterMode)
                {
                    case TIMER_COUNTER_COMP0 :
                    {
                        tmpval = (pCfg->ctCmp0ValueUsec * rate_factor) - 1UL;

                        if ((pCfg->ctStartMode == TIMER_START_MAINCNT)
                            && (((0xFFFFFFFFUL - tmpval) == 0xFFFFFFFFUL) || (mainval > (0xFFFFFFFFUL - tmpval))))
                        {
                            ret = SAL_DbgReportError
                            (
                                SAL_DRVID_TMR,
                                TIMER_API_ENABLE_CFG,
                                TIMER_ERR_OUTOF_COMPARATOR_0,
                                __FUNCTION__
                            );
                        }
                        else
                        {
                            cmpval0     = (pCfg->ctStartMode == TIMER_START_ZERO) ? tmpval : (mainval + tmpval);
                            cmpval1     = 0x0UL;
                            reg_cfgval  = TMR_OP_EN_CFG_LDM0_ON;
                            reg_irqval  |= TMR_IRQ_CTRL_IRQ_EN0;
                        }

                        break;
                    }

                    case TIMER_COUNTER_COMP1 :
                    {
                        tmpval = (pCfg->ctCmp1ValueUsec * rate_factor) - 1UL;

                        if ((pCfg->ctStartMode == TIMER_START_MAINCNT)
                            && (((0xFFFFFFFFUL - tmpval) == 0xFFFFFFFFUL) || (mainval > (0xFFFFFFFFUL - tmpval))))
                        {
                            ret = SAL_DbgReportError
                            (
                                SAL_DRVID_TMR,
                                TIMER_API_ENABLE_CFG,
                                TIMER_ERR_OUTOF_COMPARATOR_1,
                                __FUNCTION__
                            );
                        }
                        else
                        {
                            cmpval0     = 0x0UL;
                            cmpval1     = (pCfg->ctStartMode == TIMER_START_ZERO) ? tmpval : (mainval + tmpval);
                            reg_cfgval  = TMR_OP_EN_CFG_LDM1_ON;
                            reg_irqval  |= TMR_IRQ_CTRL_IRQ_EN1;
                        }

                        break;
                    }

                    case TIMER_COUNTER_SMALL_COMP :
                    {
                        tmpval      = (pCfg->ctCmp0ValueUsec * rate_factor) - 1UL;
                        cmpval0     = (pCfg->ctStartMode == TIMER_START_ZERO) ? tmpval : (mainval + tmpval);
                        tmpval      = (pCfg->ctCmp1ValueUsec * rate_factor) - 1UL;
                        cmpval1     = (pCfg->ctStartMode == TIMER_START_ZERO) ? tmpval : (mainval + tmpval);
                        reg_cfgval  = (TMR_OP_EN_CFG_LDM0_ON | TMR_OP_EN_CFG_LDM1_ON);
                        reg_irqval  |= (TMR_IRQ_CTRL_IRQ_EN0 | TMR_IRQ_CTRL_IRQ_EN1);
                        break;
                    }

                    default : //TIMER_COUNTER_MAIN
                    {
                        break;
                    }
                }

                if (ret == SAL_RET_SUCCESS)
                {
                    //reset main cnt load value
                    SAL_WriteReg(mainval, (uint32)(reg + TMR_MAIN_CNT_LVD));
                    SAL_WriteReg(cmpval0, (uint32)(reg + TMR_CMP_VALUE0));
                    SAL_WriteReg(cmpval1, (uint32)(reg + TMR_CMP_VALUE1));

                    reg_cfgval |= (TMR_PRESCALE
                                   | TMR_OP_EN_CFG_CNT_EN
                                   | ((uint32)pCfg->ctStartMode << TMR_OP_EN_CFG_LDZERO_OFFSET));

                    if (pCfg->ctOpMode == TIMER_OP_ONESHOT)
                    {
                        reg_cfgval |= TMR_OP_EN_CFG_OPMODE_ONE_SHOT;
                    }

                    SAL_WriteReg(reg_cfgval, (uint32)(reg + TMR_OP_EN_CFG));
                    SAL_WriteReg(SAL_ReadReg((uint32)(reg + TMR_IRQ_CTRL)) | reg_irqval, (uint32)(reg + TMR_IRQ_CTRL));
                    gTimerRes[pCfg->ctChannel].rtUsed       = TRUE;
                    gTimerRes[pCfg->ctChannel].rtHandler    = pCfg->fnHandler;
                    gTimerRes[pCfg->ctChannel].rtArgs       = pCfg->pArgs;

                    if (pCfg->ctChannel != TIMER_CH_0)    // timer 0 specially used by os timer
                    {
                        (void)GIC_IntVectSet
                        (
                            (uint32)GIC_TIMER_0 + (uint32)pCfg->ctChannel,
                            GIC_PRIORITY_NO_MEAN,
                            GIC_INT_TYPE_LEVEL_HIGH,
                            (GICIsrFunc)&TIMER_Handler,
                            (void*)&gTimerRes[pCfg->ctChannel]
                        );
                    }       // (ch != TIMER_CH_0)

                    TMR_D("Channel (%d) is enabled\n", (uint32)pCfg->ctChannel);
                }       // (ret == SAL_RET_SUCCESS)
            }       // (tcc_get_peri(clk_id) != rate)
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          TIMER_Disable
*
* This function disables the specific TIMER module.
*
* @param    uiChannel [in] The specific channel for TIMER module
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t TIMER_Disable
(
    TIMERChannel_t                      uiChannel
)
{
    SALRetCode_t    ret;
    uint32          reg;

    ret = SAL_RET_SUCCESS;
    reg = MCU_BSP_TIMER_BASE + ((uint32)uiChannel * 0x100UL);

    // MISRA 14.7 : A function shall have a single point of exit at the end of the function
    if (gTimerInitialized == FALSE)
    {
        ret = SAL_DbgReportError
        (
            SAL_DRVID_TMR,
            TIMER_API_DISABLE,
            TIMER_ERR_NOT_INITIALIZED,
            __FUNCTION__
        );
    }
    else
    {
        SAL_WriteReg((SAL_ReadReg((uint32)(reg + TMR_IRQ_CTRL)) & (~TMR_OP_EN_CFG_CNT_EN)), (uint32)(reg + TMR_OP_EN_CFG));
        SAL_WriteReg(0U, (uint32)(reg + TMR_IRQ_CTRL));

        gTimerRes[uiChannel].rtUsed     = FALSE;
        gTimerRes[uiChannel].rtHandler  = NULL;
        gTimerRes[uiChannel].rtArgs     = NULL_PTR;

        TMR_D("Channel (%d) is disabled\n", (uint32)uiChannel);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          TIMER_Init
*
* This function initializes clock and registers for all channels
*
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t TIMER_Init
(
    void
)
{
    uint32          reg;
    uint32          clk_rate;
    uint32          index;
    uint32          reg_val;
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_TIMER0, TMR_CLK_RATE);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_TIMER0);
    clk_rate    = CLOCK_GetPeriRate((sint32)CLOCK_PERI_TIMER0);

    // MISRA 14.7 : A function shall have a single point of exit at the end of the function
    if (clk_rate != TMR_CLK_RATE)
    {
        gTimerInitialized = FALSE;
        ret = SAL_DbgReportError
        (
            SAL_DRVID_TMR,
            TIMER_API_INIT,
            TIMER_ERR_SET_CLOCK,
            __FUNCTION__
        );

        // Uart is not ready to print sth so TMR_PASS/TMR_FAIL can't be showed off
    }
    else
    {
        for (index = 0 ; index < (uint32)TIMER_CH_MAX ; index++)
        {
            reg = MCU_BSP_TIMER_BASE + (index * 0x100UL);

            gTimerRes[index].rtChannel  = (TIMERChannel_t)index;
            gTimerRes[index].rtUsed     = FALSE;
            gTimerRes[index].rtHandler  = NULL_PTR;
            gTimerRes[index].rtArgs     = NULL_PTR;

            SAL_WriteReg(0x7FFFU, (uint32)(reg + TMR_OP_EN_CFG));
            SAL_WriteReg(0x0U,    (uint32)(reg + TMR_MAIN_CNT_LVD));
            SAL_WriteReg(0x0U,    (uint32)(reg + TMR_CMP_VALUE0));
            SAL_WriteReg(0x0U,    (uint32)(reg + TMR_CMP_VALUE1));

            reg_val = TMR_IRQ_CLR_CTRL_WRITE | TMR_IRQ_MASK_ALL;    // = TMR_IRQ_CLR_CTRL_READ | TMR_IRQ_MASK_ALL;

            SAL_WriteReg(reg_val, (uint32)(reg + TMR_IRQ_CTRL));
        }

        gTimerInitialized = TRUE;

        // Uart is not ready to print sth so TMR_PASS/TMR_FAIL can't be showed off
    }

    return ret;
}

