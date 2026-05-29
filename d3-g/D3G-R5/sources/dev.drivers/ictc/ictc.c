/*
***************************************************************************************************
*
*   FileName : ictc.c
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

#include "reg_phys.h"
#include "ictc.h"
#include "clock.h"
#include "clock_dev.h"
#include "gic.h"

/***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/

static uint32 timestamp_rising_edge[ICTC_TOTAL_CHANNEL_NUM][ICTC_TIMESTAMP_RECORD_MAXCNT];
static uint32 timestamp_falling_edge[ICTC_TOTAL_CHANNEL_NUM][ICTC_TIMESTAMP_RECORD_MAXCNT];
static uint32 r_tstmp_idx[ICTC_TOTAL_CHANNEL_NUM];
static uint32 f_tstmp_idx[ICTC_TOTAL_CHANNEL_NUM];

static ICTCCallBack_t user_priv[ICTC_TOTAL_CHANNEL_NUM];

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          ICTC_Init
*
* Initialize register map
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

void ICTC_Init
(
    void
)
{
    /*set peri clock*/
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC0, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC0);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC1, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC1);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC2, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC2);

    ICTC_D("\n ICTC ch0-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC0));
    ICTC_D("\n ICTC ch1-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC1));
    ICTC_D("\n ICTC ch2-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC2));
}

/*
***************************************************************************************************
*                                          ICTC_Deinit
*
* De-Initialize register map
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

void ICTC_Deinit
(
    void
)
{
    /*set peri clock*/
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC0);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC1);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC2);
}

/*
***************************************************************************************************
*                                          ICTC_GetIRQCtrlReg
*
* ICTC Interrupt Handler
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

static uint32 ICTC_GetIRQCtrlReg
(
    uint32                              uiChannel
)
{
    uint32  val;
    uint32  reg;

    val = 0;
    reg = ICTC_CHC0_BASE + ICTC_IRQ_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val = SAL_ReadReg(reg);
    }

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetPrevPeriodCnt
*
* ICTC get previous period Count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetPrevPeriodCnt
(
    uint32                              uiChannel
)
{
    uint32  val;
    uint32  reg;

#ifdef ICTC_SUB_ICTC_ENABLED
    uint32  sub_reg;
    uint32  sub_val;
#endif

    val     = 0;
    reg     = ICTC_CHC0_BASE + ICTC_CNT_PRE_PRD + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_CNT_PRE_PRD + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
    sub_val = 0;

#ifdef ICTC_SUB_ICTC_ENABLED
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val     = SAL_ReadReg(reg);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(CNT_PRE_PRD) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);
        }
    }

#else
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val = SAL_ReadReg(reg);
    }
#endif

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetPreDutyCnt
*
* ICTC get previous Duty Count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetPreDutyCnt
(
    uint32                              uiChannel
)
{
    uint32  val;
    uint32  reg;
    uint32  sub_reg;
    uint32  sub_val;

    val     = 0;
    reg     = ICTC_CHC0_BASE + ICTC_CNT_PRE_DUTY + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_CNT_PRE_DUTY + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
    sub_val = 0;

#ifdef ICTC_SUB_ICTC_ENABLED
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val     = SAL_ReadReg(reg);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(CNT_PRE_DUTY) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
    }
#else
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val = SAL_ReadReg(reg);
    }
#endif

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetCurEdgeCnt
*
* ICTC get current edge count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetCurEdgeCnt
(
    uint32                              uiChannel
)
{
    uint32  val;
    uint32  reg;
    uint32  sub_reg;
    uint32  sub_val;

    val     = 0;
    reg     = ICTC_CHC0_BASE + ICTC_CUR_EDGE_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_CUR_EDGE_CNT + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
    sub_val = 0;

#ifdef ICTC_SUB_ICTC_ENABLED
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val     = SAL_ReadReg(reg);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(CNT_PRE_DUTY) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
    }
#else
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val = SAL_ReadReg(reg);
    }
#endif

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetPrvEdgeCnt
*
* ICTC get previous edge count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetPrvEdgeCnt
(
    uint32                              uiChannel
)
{
    uint32  val;
    uint32  reg;
    uint32  sub_reg;
    uint32  sub_val;

    val     = 0;
    reg     = ICTC_CHC0_BASE + ICTC_PRV_EDGE_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_PRV_EDGE_CNT + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
    sub_val = 0;

#ifdef ICTC_SUB_ICTC_ENABLED
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val     = SAL_ReadReg(reg);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(CNT_PRE_DUTY) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
    }
#else
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        val = SAL_ReadReg(reg);
    }
#endif

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetREdgeTstmpCnt
*
* ICTC get rising edge timestamp count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetREdgeTstmpCnt
(
    uint32                              uiChannel
)
{
    uint32  val;
    uint32  reg;
    uint32  sub_reg;
    uint32  sub_val;

    val     = 0xFFFFFFFFUL;
    reg     = 0;
    sub_reg = 0;
    sub_val = 0;

#ifdef ICTC_SUB_ICTC_ENABLED
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_R_EDGE_TSTMP_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_reg = ICTC_CHC0_BASE + ICTC_R_EDGE_TSTMP_CNT + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
        val     = SAL_ReadReg(reg);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(R_EDGE_TSTMP_CNT) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
        else if(r_tstmp_idx[uiChannel] < ICTC_TIMESTAMP_RECORD_MAXCNT )
        {
            timestamp_rising_edge[uiChannel][r_tstmp_idx[uiChannel]]    = val;
            r_tstmp_idx[uiChannel]++;

            if(r_tstmp_idx[uiChannel] >= ICTC_TIMESTAMP_RECORD_MAXCNT )
            {
                r_tstmp_idx[uiChannel] = 0UL;
            }
        }
        else
        {
            ICTC_D("%s : do nothing.\n", __func__);
        }
    }
#else
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg = ICTC_CHC0_BASE + ICTC_R_EDGE_TSTMP_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        val = SAL_ReadReg(reg);

        if(r_tstmp_idx[uiChannel] < ICTC_TIMESTAMP_RECORD_MAXCNT )
        {
            timestamp_rising_edge[uiChannel][r_tstmp_idx[uiChannel]]    = val;
            r_tstmp_idx[uiChannel]++;

            if(r_tstmp_idx[uiChannel] >= ICTC_TIMESTAMP_RECORD_MAXCNT )
            {
                r_tstmp_idx[uiChannel] = 0UL;
            }
        }
    }
#endif

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetFEdgeTstmpCnt
*
* ICTC get falling edge timestamp count value
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetFEdgeTstmpCnt
(
    uint32                              uiChannel
)
{
    uint32  val;
    uint32  reg;
    uint32  sub_reg;
    uint32  sub_val;

    val     = 0xFFFFFFFFUL;
    reg     = 0;
    sub_reg = 0;
    sub_val = 0;

#ifdef ICTC_SUB_ICTC_ENABLED
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_F_EDGE_TSTMP_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_reg = ICTC_CHC0_BASE + ICTC_F_EDGE_TSTMP_CNT + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
        val     = SAL_ReadReg(reg);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(F_EDGE_TSTMP_CNT) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
        else if(f_tstmp_idx[uiChannel] < ICTC_TIMESTAMP_RECORD_MAXCNT )
        {
            timestamp_falling_edge[uiChannel][f_tstmp_idx[uiChannel]]   = val;
            f_tstmp_idx[uiChannel]++;

            if(f_tstmp_idx[uiChannel] >= (uint32)ICTC_TIMESTAMP_RECORD_MAXCNT )
            {
                f_tstmp_idx[uiChannel]  = 0UL;
            }
        }
        else
        {
            ICTC_D("%s : do nothing.\n", __func__);
        }
    }
#else
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg = ICTC_CHC0_BASE + ICTC_F_EDGE_TSTMP_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        val = SAL_ReadReg(reg);

        if(f_tstmp_idx[uiChannel] < ICTC_TIMESTAMP_RECORD_MAXCNT )
        {
            timestamp_falling_edge[uiChannel][f_tstmp_idx[uiChannel]]   = val;
            f_tstmp_idx[uiChannel]++;

            if(f_tstmp_idx[uiChannel] >= (uint32)ICTC_TIMESTAMP_RECORD_MAXCNT )
            {
                f_tstmp_idx[uiChannel]  = 0UL;
            }
        }
    }
#endif

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_ClearOpEnCtrlCounter
*
* ICTC Clear OP_EN_CTRL Counter Register
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
static void ICTC_ClearOpEnCtrlCounter
(
    uint32                              uiChannel,
    uint32                              uiCntValue
)
{
    uint32  reg;
    uint32  sub_reg;

    reg     = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        SAL_WriteReg( SAL_ReadReg(reg) | (uiCntValue), reg );
        SAL_WriteReg( SAL_ReadReg(reg) & ~(uiCntValue), reg );
#ifdef ICTC_SUB_ICTC_ENABLED
        SAL_WriteReg( SAL_ReadReg(sub_reg) | (uiCntValue), sub_reg );
        SAL_WriteReg( SAL_ReadReg(sub_reg) & ~(uiCntValue), sub_reg );
#endif
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_EnableCapture
*
* ICTC Enable Operation, Enable TCLK and ICTC EN
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
void ICTC_EnableCapture
(
    uint32                              uiChannel
)
{
    uint32  reg;
    uint32  sub_reg;

    reg     = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        /* enable TCLK_EN */
        SAL_WriteReg(SAL_ReadReg(reg) | ICTC_OPEN_CTRL_TCLK_EN, reg );
        /* enable ictc */
        SAL_WriteReg(SAL_ReadReg(reg) | ICTC_OPEN_CTRL_EN, reg );
#ifdef ICTC_SUB_ICTC_ENABLED
        /* enable TCLK_EN */
        SAL_WriteReg(SAL_ReadReg(sub_reg) | ICTC_OPEN_CTRL_TCLK_EN, sub_reg );
        /* enable sub-ictc */
        SAL_WriteReg(SAL_ReadReg(sub_reg) | ICTC_OPEN_CTRL_EN, sub_reg );
#endif
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_DisableCapture
*
* ICTC Diable Operation, Clear TCLK and ICTC EN
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
void ICTC_DisableCapture
(
    uint32                              uiChannel
)
{
    uint32  reg;
    uint32  sub_reg;

    reg     = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        ICTC_ClearOpEnCtrlCounter(uiChannel,
                                  (ICTC_OPEN_CTRL_TSCNT_CLR  |
                                  ICTC_OPEN_CTRL_FLTCNT_CLR  |
                                  ICTC_OPEN_CTRL_TOCNT_CLR   |
                                  ICTC_OPEN_CTRL_EDEGCNT_CLR |
                                  ICTC_OPEN_CTRL_PDCNT_CLR));

        /* enable TCLK_EN */
        SAL_WriteReg(SAL_ReadReg(reg) & ~(ICTC_OPEN_CTRL_TCLK_EN), reg );
        /* enable ictc */
        SAL_WriteReg(SAL_ReadReg(reg) & ~(ICTC_OPEN_CTRL_EN), reg );
#ifdef ICTC_SUB_ICTC_ENABLED
        /* enable TCLK_EN */
        SAL_WriteReg(SAL_ReadReg(sub_reg) & ~(ICTC_OPEN_CTRL_TCLK_EN), sub_reg );
        /* enable sub-ictc */
        SAL_WriteReg(SAL_ReadReg(sub_reg) & ~(ICTC_OPEN_CTRL_EN), sub_reg );
#endif
    }
    return;
}

/*
***************************************************************************************************
*                                          ICTC_SetOpEnCtrlCounter
*
* ICTC set OP_EN_CTRL Counter Register
* @param    channel number 0 ~ 2.
* @param    counter valuie
* Notes
*
***************************************************************************************************
*/
void ICTC_SetOpEnCtrlCounter
(
    uint32                              uiChannel,
    uint32                              uiCntEnValue
)
{
    uint32  reg;
    uint32  sub_reg;

    reg     = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        SAL_WriteReg(uiCntEnValue, reg);
#ifdef ICTC_SUB_ICTC_ENABLED
        SAL_WriteReg(uiCntEnValue, sub_reg );
#endif
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_SetOpModeCtrlReg
*
* ICTC set OP_MODE_CTRL Register
* @param    channel number 0 ~ 2.
* @param    OP_MODE Value

* Notes
*
***************************************************************************************************
*/
SALRetCode_t ICTC_SetOpModeCtrlReg
(
    uint32                              uiChannel,
    uint32                              uiModeValue
)
{
    SALRetCode_t    ret;
    uint32          f_in_sel_val;
    uint32          reg;
    uint32          sub_reg;

    ret             = SAL_RET_SUCCESS;
    f_in_sel_val    = (uiModeValue & ICTC_OPMODE_CTRL_F_IN_SEL_BIT_MASK);
    reg             = ICTC_CHC0_BASE + ICTC_OP_MODE_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg         = ICTC_CHC0_BASE + ICTC_OP_MODE_CTRL + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    /* check f_in_sel range */
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        if((f_in_sel_val < 242UL) || (244UL <= f_in_sel_val))
        {
            SAL_WriteReg(ICTC_OPMODE_CTRL_RESET_VALUE, reg );
            SAL_WriteReg(SAL_ReadReg(reg) | uiModeValue, reg );
#ifdef ICTC_SUB_ICTC_ENABLED
            SAL_WriteReg(ICTC_OPMODE_CTRL_RESET_VALUE, sub_reg);
            SAL_WriteReg(uiModeValue, sub_reg );
#endif
        }
        else
        {
            ICTC_Err("\n ICTC invalid f_in_sel value \n");
            ret = (SALRetCode_t)SAL_RET_FAILED;
        }
    }
    else
    {
        ICTC_Err("\n ICTC invalid uiChannel value \n");
        ret = (SALRetCode_t)SAL_RET_FAILED;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          ICTC_ClearIRQCTRLReg
*
* ICTC Clear IRQ_CTRL Register
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
static void ICTC_ClearIRQCTRLReg
(
    uint32                              uiChannel
)
{
    uint32  irq_en;
    uint32  reg;
    uint32  sub_reg;

    irq_en  = 0;
    reg     = ICTC_CHC0_BASE + ICTC_IRQ_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_IRQ_CTRL + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        /*read IRQ_EN 31:24*/
        irq_en  = ((uint32)SAL_ReadReg(reg) >> ICTC_IRQ_CTRL_IRQEN_OFFSET);
        SAL_WriteReg(SAL_ReadReg(reg) | ((uint32)(0xFFUL) << ICTC_IRQ_CTRL_IRQCLR_OFFSET), reg);
        SAL_WriteReg(0x00000000UL, reg);
        SAL_WriteReg((irq_en << ICTC_IRQ_CTRL_IRQEN_OFFSET), reg);
#ifdef ICTC_SUB_ICTC_ENABLED
        irq_en = ((uint32)SAL_ReadReg(sub_reg) >> ICTC_IRQ_CTRL_IRQEN_OFFSET);
        SAL_WriteReg(SAL_ReadReg(sub_reg) | ((uint32)(0xFFUL) << ICTC_IRQ_CTRL_IRQCLR_OFFSET), sub_reg);
        SAL_WriteReg(0x00000000UL, sub_reg);
        SAL_WriteReg((irq_en << ICTC_IRQ_CTRL_IRQEN_OFFSET), sub_reg);
#endif
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_IrqHandler
*
* ICTC Interrupt Handler
* @param    user data argument.
* Notes
*
***************************************************************************************************
*/
static void ICTC_IrqHandler
(
    void *                              pArg
)
{
    ICTCCallBack_t *    pdata;
    uint32              channel;
    uint32              val;

    pdata   = (ICTCCallBack_t *)NULL_PTR;
    channel = 0UL;
    val     = 0UL;

    if (pArg != NULL_PTR)
    {
        pdata   = (ICTCCallBack_t*)pArg;
        channel = pdata->cbChannel;
        val     = ICTC_GetIRQCtrlReg(channel);

        if((val & ICTC_IRQ_CTRL_FEDGE_NOTI_IRQ) == ICTC_IRQ_CTRL_FEDGE_NOTI_IRQ)
        {
            (void)ICTC_GetFEdgeTstmpCnt(channel);
        }

        if((val & ICTC_IRQ_CTRL_REDGE_NOTI_IRQ) == ICTC_IRQ_CTRL_REDGE_NOTI_IRQ)
        {
            (void)ICTC_GetREdgeTstmpCnt(channel);
        }

        if((val & ICTC_IRQ_CTRL_DT_ERR_IRQ) == ICTC_IRQ_CTRL_DT_ERR_IRQ)
        {
            if (pdata->cbCallBackFunc != (ICTCCallback)NULL)
            {
                pdata->cbCallBackFunc(channel, ICTC_GetPrevPeriodCnt(channel), ICTC_GetPreDutyCnt(channel));
            }

        }

        /* clear interrupt */
        ICTC_ClearIRQCTRLReg(channel);
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_SetIRQCtrlReg
*
* ICTC set IRQ_CTRL Register
* @param    channel number 0 ~ 2.
* @param    IRQ_EN
* Notes
*
***************************************************************************************************
*/
void ICTC_SetIRQCtrlReg
(
    uint32                              uiChannel,
    uint32                              uiIRQValue
)
{
    uint32  reg;
    uint32  sub_reg;

    reg     = ICTC_CHC0_BASE + ICTC_IRQ_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_IRQ_CTRL + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        /* clear interrupt flag */
        SAL_WriteReg(SAL_ReadReg(reg) | ((uint32)(0xFFUL) << ICTC_IRQ_CTRL_IRQCLR_OFFSET), reg);
        /* reset 0x00 */
        SAL_WriteReg(0x00000000UL, reg);
        /* IRQ_EN */
        SAL_WriteReg( (uiIRQValue << ICTC_IRQ_CTRL_IRQEN_OFFSET), reg);
#ifdef ICTC_SUB_ICTC_ENABLED
        /* clear interrupt flag */
        SAL_WriteReg(SAL_ReadReg(sub_reg) | ((uint32)(0xFFUL) << ICTC_IRQ_CTRL_IRQCLR_OFFSET), sub_reg);
        /* reset 0x00 */
        SAL_WriteReg(0x00000000UL, sub_reg);
        /* IRQ_EN */
        SAL_WriteReg( (uiIRQValue << ICTC_IRQ_CTRL_IRQEN_OFFSET), sub_reg);

#endif
        switch(uiChannel)
        {
            case 0:
            {
                (void)GIC_IntVectSet((uint32)GIC_IC_TC0, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_IC_TC0);
                break;
            }

            case 1:
            {
                (void)GIC_IntVectSet((uint32)GIC_IC_TC1, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_IC_TC1);
                break;
            }

            case 2:
            {
                (void)GIC_IntVectSet((uint32)GIC_IC_TC2, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_IC_TC2);
                break;
            }

            default:
            {
                // error
                break;
            }
        }
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_SetCallBackFunc
*
* ICTC set CallbackFunction
* @param    channel number 0 ~ 2.
* @param    CallBack Function
* Notes
*
***************************************************************************************************
*/
void ICTC_SetCallBackFunc
(
    uint32                              uiChannel,
    ICTCCallback                        pCallbackFunc
)
{
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        user_priv[uiChannel].cbChannel      = uiChannel;
        user_priv[uiChannel].cbCallBackFunc = pCallbackFunc;
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_SetTimeoutValue
*
* ICTC set Time-Out Value Register
* @param    channel number 0 ~ 2.
* @param    timeout value.
* Notes
*
***************************************************************************************************
*/
void ICTC_SetTimeoutValue
(
    uint32                              uiChannel,
    uint32                              uiTimeoutValue
)
{
    uint32  reg;
    uint32  sub_reg;

    reg     = ICTC_CHC0_BASE + ICTC_TIME_OUT + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_reg = ICTC_CHC0_BASE + ICTC_TIME_OUT + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        SAL_WriteReg( (0x0FFFFFFFUL & uiTimeoutValue), reg );
#ifdef ICTC_SUB_ICTC_ENABLED
        SAL_WriteReg( (0x0FFFFFFFUL & uiTimeoutValue), sub_reg );
#endif
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_SetEdgeMatchingValue
*
* ICTC set Edge Matching Value, R_EDGE, F_EDGE, EDGE_COUNTER
* @param    channel number 0 ~ 2.
* @param    rising edge matching value.
* @param    falling edge matching value.
* @param    edge counter matching value.
* @return
*
* Notes
*
***************************************************************************************************
*/
void ICTC_SetEdgeMatchingValue
(
    uint32                              uiChannel,
    uint32                              uiRisingEdgeMat,
    uint32                              uiFallingEdgeMat,
    uint32                              uiEdgeCounterMat
)
{
    uint32  r_edge_reg;
    uint32  f_edge_reg;
    uint32  edge_cnt_reg;
    uint32  sub_r_edge_reg;
    uint32  sub_f_edge_reg;
    uint32  sub_edge_cnt_reg;

    r_edge_reg          = ICTC_CHC0_BASE + ICTC_R_EDGE + (uiChannel * ICTC_CHANNEL_OFFSET);
    f_edge_reg          = ICTC_CHC0_BASE + ICTC_F_EDGE + (uiChannel * ICTC_CHANNEL_OFFSET);
    edge_cnt_reg        = ICTC_CHC0_BASE + ICTC_EDGE_CNT_MAT + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_r_edge_reg      = ICTC_CHC0_BASE + ICTC_R_EDGE + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
    sub_f_edge_reg      = ICTC_CHC0_BASE + ICTC_F_EDGE + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
    sub_edge_cnt_reg    = ICTC_CHC0_BASE + ICTC_EDGE_CNT_MAT + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        SAL_WriteReg((0x0FFFFFFFUL & uiRisingEdgeMat), r_edge_reg );
        SAL_WriteReg((0x0FFFFFFFUL & uiFallingEdgeMat), f_edge_reg );
        SAL_WriteReg((0x0000FFFFUL & uiEdgeCounterMat), edge_cnt_reg );

#ifdef ICTC_SUB_ICTC_ENABLED
        SAL_WriteReg((0x0FFFFFFFUL & uiRisingEdgeMat), sub_r_edge_reg );
        SAL_WriteReg((0x0FFFFFFFUL & uiFallingEdgeMat), sub_f_edge_reg );
        SAL_WriteReg((0x0000FFFFUL & uiEdgeCounterMat), sub_edge_cnt_reg );
#endif
    }

    return;
}

/*
***************************************************************************************************
*                                          ICTC_SetCompareRoundValue
*
* ICTC set Period/Duty compare round value configuration register
* @param    channel number 0 ~ 2.
* @param    period compare round value.
* @param    duty compare round value.
* Notes
*
***************************************************************************************************
*/
void ICTC_SetCompareRoundValue
(
    uint32                              uiChannel,
    uint32                              uiPeriodCompareRound,
    uint32                              uiDutyCompareRound
)
{
    uint32  prd_cmp_reg;
    uint32  duty_cmp_reg;
    uint32  sub_prd_cmp_reg;
    uint32  sub_duty_cmp_reg;

    prd_cmp_reg         = ICTC_CHC0_BASE + ICTC_PRD_CMP_RND + (uiChannel * ICTC_CHANNEL_OFFSET);
    duty_cmp_reg        = ICTC_CHC0_BASE + ICTC_DUTY_CMP_RND + (uiChannel * ICTC_CHANNEL_OFFSET);
    sub_prd_cmp_reg     = ICTC_CHC0_BASE + ICTC_PRD_CMP_RND + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);
    sub_duty_cmp_reg    = ICTC_CHC0_BASE + ICTC_DUTY_CMP_RND + ((uiChannel + ICTC_REDUNDANCY_MODUDLE_OFFSET) * ICTC_CHANNEL_OFFSET);

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        SAL_WriteReg((0x0FFFFFFFUL & uiPeriodCompareRound), prd_cmp_reg);
        SAL_WriteReg((0x0FFFFFFFUL & uiDutyCompareRound), duty_cmp_reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        SAL_WriteReg((0x0FFFFFFFUL & uiPeriodCompareRound), sub_prd_cmp_reg);
        SAL_WriteReg((0x0FFFFFFFUL & uiDutyCompareRound), sub_duty_cmp_reg);
#endif
    }

    return;
}

