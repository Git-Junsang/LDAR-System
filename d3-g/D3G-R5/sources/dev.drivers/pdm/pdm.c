/*
***************************************************************************************************
*
*   FileName : pdm.c
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

#include <bsp.h>
#include "pdm.h"
#include "fmu.h"

/**************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/


static PDMChannelHandler_t              PDMHandler[PDM_TOTAL_CHANNELS];
static PMMISRData_t                     PDMIsrData[PDM_TOTAL_CHANNELS];

static boolean                          g_safety_enabled = FALSE;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void PDM_Delay1u
(
    uint32                              uiN
);

static SALRetCode_t PMM_SetDutyMarginValue
(
    uint32                              uiChannel,
    uint32                              uiNoiseFilterValue,
    uint32                              uiDutyMarginValue
);

static SALRetCode_t PMM_SetTimeoutValue
(
    uint32                              uiChannel,
    uint32                              uiTimeoutValue
);

/************************************************************************************************/
/*                                          PDM_Delay1u                                         */
/*                                                                                              */
/* Delay.                                                                                       */
/*                                                                                              */
/* @param                                                                                       */
/* @return                                                                                      */
/*                                                                                              */
/* Notes                                                                                        */
/*                                                                                              */
/************************************************************************************************/

static void PDM_Delay1u
(
    uint32                              uiN
)
{
    uint32  i;

    for (i = 0; i < uiN; i++)
    {
         BSP_NOP_DELAY();
    }
}

/*
***************************************************************************************************
*                                          PDM_CheckChannelValidation
*
* Check channel number, pdm module has 12channel  0 ~ 11.
*
* @param    channel number 0 ~ 11.
* @return
*
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t PDM_CheckChannelValidation
(
    uint32                              uiChannel
)
{
    SALRetCode_t ret;

    if(uiChannel < (uint32)PDM_CH_MAX)
    {
        ret = (SALRetCode_t)SAL_RET_SUCCESS;
    }
    else
    {
        PDM_Err("Invalid channel number. \n", __func__);
        ret = (SALRetCode_t)SAL_RET_FAILED;
    }
    return ret;
}

/******************************************************************************
 *  FMU ISR
******************************************************************************/
#ifdef PDM_SAFETY_FEATURE_ENABLED
/******************************************************************************
 * PDM Mornitoring for Safety Features, PMM
 ******************************************************************************/

/*
***************************************************************************************************
*                                           PMM_SetDutyMarginValue
*
* set noise filter value and duty margine value.

* @param    channel number 0 ~ 11.
* @param    noise filter value
* @param    duty margin value
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t PMM_SetDutyMarginValue
(
    uint32                              uiChannel,
    uint32                              uiNoiseFilterValue,
    uint32                              uiDutyMarginValue
)
{
    uint32          val;
    uint32          reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val         = 0;
    reg         = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        reg = PMM_BASE +
              (module_id * PMM_MODULE_OFFSET) +
              PMM_GetPMMDutyMarginReg(channel_id);

        val = (uiNoiseFilterValue << PMM_FLT_VAL_OFFSET) |
              (uiDutyMarginValue << PMM_DUTY_MARGIN_OFFSET);

        SAL_WriteReg(val,  reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                           PMM_SetTimeoutValue
*
* set pmm time out value, if the output of pdm is not generated for a certain period of time
* a fault has occured
* @param    channel number 0 ~ 11.
* @param    time-out value
* @return
* Notes     this value is counted on a divided pdm operating clock basis.
*
***************************************************************************************************
*/

static SALRetCode_t PMM_SetTimeoutValue
(
    uint32                              uiChannel,
    uint32                              uiTimeoutValue)
{
    uint32          reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    reg         = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        reg = PMM_BASE + (module_id * PMM_MODULE_OFFSET)+PMM_GetPMMTimeoutValueReg(channel_id);
        SAL_WriteReg(uiTimeoutValue,  reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                           PMM_EnableMonitoring
*
* Enable PDM Monitoring Module for Safety
*
* @param    channel number 0 ~ 11.
* @param    input connection selection.
* @param    enable commission error check.
* @return
* Notes It is for safety feature.
*
***************************************************************************************************
*/

static SALRetCode_t PMM_EnableMonitoring
(
    uint32                              uiChannel,
    uint32                              uiConnectionSelect,
    uint32                              uiCommissionErrorCheck
)
{
    uint32          pdmen_reg;
    uint32          ctrl_reg;
    uint32          val;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    pdmen_reg   = 0;
    ctrl_reg    = 0;
    val         = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    /* determine whether external mode or internal mode */
    if(ret == SAL_RET_SUCCESS)
    {
        pdmen_reg = PMM_BASE + PMM_PDMEN_REG_OFFSET + (module_id * PMM_MODULE_OFFSET);
        ctrl_reg  = PMM_BASE + PMM_CTRL_REG_OFFSET + (module_id * PMM_MODULE_OFFSET);

        val = SAL_ReadReg(ctrl_reg);
        /* clear cnect bit */
        val &= ~(0x1UL << PMM_GetPMMCtrlPmmCNECTReg(channel_id));
        /* clear cm en bit */
        val &= ~(0x1UL << PMM_GetPMMCtrlPmmCMENReg(channel_id));

        /* set cnect bit */
        val |= ((uiConnectionSelect > 0UL) ? ((uint32)PMM_ON) : ((uint32)PMM_OFF)) <<
               (uint32)PMM_GetPMMCtrlPmmCNECTReg(channel_id);

        /* set cm, commission error check enable bit */
        val |= ((uiCommissionErrorCheck > 0UL) ? ((uint32)PMM_ON) : ((uint32)PMM_OFF)) <<
               (uint32)PMM_GetPMMCtrlPmmCMENReg(channel_id);

               /* set CNECT and CM_EN */
        SAL_WriteReg(val , ctrl_reg);

        /* monitoring module start sequence */
        /* pmm_ctrl pdm monitoring module enable register */
        SAL_WriteReg(SAL_ReadReg(ctrl_reg) | ((uint32)0x1U<<(PMM_GetPMMCtrlPmmEnReg(channel_id))),
                      ctrl_reg);

        /* pmm_pdmen pmm operation enable register */
        SAL_WriteReg(SAL_ReadReg(pdmen_reg) | ((uint32)0x1U<<(PMM_GetPDMENEnableReg(channel_id))),
                      pdmen_reg);

        /* pmm_ctrl pmm check triggering register */
        SAL_WriteReg(SAL_ReadReg(ctrl_reg) | ((uint32)0x1U<<(PMM_GetPMMCtrlPmmTRIGReg(channel_id))),
                      ctrl_reg);

        PDMHandler[uiChannel].chPMMEnable = PMM_ON;
    }

    return ret;
}

/*
***************************************************************************************************
*                                           PMM_DisableMonitoring
*
* Disable PDM Monitoring Module.
*
* @param    channel number 0 ~ 11.
* @return
* Notes It is for safety feature.
*
***************************************************************************************************
*/
static SALRetCode_t PMM_DisableMonitoring
(
    uint32                              uiChannel
)
{
    uint32          pdmen_reg;
    uint32          ctrl_reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    pdmen_reg   = 0;
    ctrl_reg    = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        ctrl_reg  = PMM_BASE + PMM_CTRL_REG_OFFSET + (module_id * PMM_MODULE_OFFSET);
        /* clear pmm_ctrl pdm monitoring module enable register */
        SAL_WriteReg(SAL_ReadReg(ctrl_reg) & ~(0x1UL<<(PMM_GetPMMCtrlPmmEnReg(channel_id))),
                      ctrl_reg);

        pdmen_reg = PMM_BASE + PMM_PDMEN_REG_OFFSET + (module_id * PMM_MODULE_OFFSET);
        /* clear pmm_pdmen pmm operation enable register */
        SAL_WriteReg(SAL_ReadReg(pdmen_reg) & ~(0x1UL<<(PMM_GetPDMENEnableReg(channel_id))),
                      pdmen_reg);

        PDMHandler[uiChannel].chPMMEnable = PMM_OFF;
    }

    return ret;
}

/*
***************************************************************************************************
*                                           PMM_SetConfig
*
* Set Configuration value for PMM
*
* @param    channel number 0 ~ 11.
* @return
* Notes     every config value should be same as PDM's.
*
***************************************************************************************************
*/
static SALRetCode_t PMM_SetConfig
(
    uint32                              uiChannel
)
{
    uint32          val;
    uint32          op_mode_reg;
    uint32          out_ptn_1_reg;
    uint32          out_ptn_2_reg;
    uint32          out_ptn_3_reg;
    uint32          out_ptn_4_reg;
    uint32          max_cnt_reg;

    uint32          pstn_1_reg; /* low position */
    uint32          pstn_2_reg; /* high position */
    uint32          pstn_3_reg; /* low position */
    uint32          pstn_4_reg; /* high position */

    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val             = 0;
    op_mode_reg     = 0;
    out_ptn_1_reg   = 0;
    out_ptn_2_reg   = 0;
    out_ptn_3_reg   = 0;
    out_ptn_4_reg   = 0;
    max_cnt_reg     = 0;

    pstn_1_reg = 0; /* low position */
    pstn_2_reg = 0; /* high position */
    pstn_3_reg = 0; /* low position */
    pstn_4_reg = 0; /* high position */

    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        (void)PMM_SetDutyMarginValue(uiChannel, 0x0U, PDMHandler[uiChannel].chPMMDutyMargin);
        (void)PMM_SetTimeoutValue(uiChannel, PDMHandler[uiChannel].chPMMTimeoutValue);

        op_mode_reg = PMM_BASE + PDM_OP_MODE_REG_OFFSET + (module_id * PMM_MODULE_OFFSET);

        val = SAL_ReadReg(op_mode_reg);
        /* operation mode */
        val = (val & ~(0xFUL << PDM_GetOPModeOperationModeReg(channel_id))) |
              (uint32)(PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode <<
              PDM_GetOPModeOperationModeReg(channel_id));

        /* clear signal inverse */
        val = ((val & ~(0x1UL << PDM_GetOPModeSignalInverseReg(channel_id))) |
              ((uint32)(PDMHandler[uiChannel].chModeCfgInfo.mcInversedSignal <<
              PDM_GetOPModeSignalInverseReg(channel_id))));

        /* clear output signal in idle state only for phase mode 1/2 */
        val = ((val & ~(0x1UL << PDM_GetOPModeOuputIdleReg(channel_id))) |
              ((uint32)(PDMHandler[uiChannel].chOutSignalInIdle <<
              PDM_GetOPModeOuputIdleReg(channel_id))));

        /* input clock divide */
        val = ((val & ~(0x3UL << PDM_GetOPModeClockDivideReg(channel_id))) |
              (uint32)(PDMHandler[uiChannel].chModeCfgInfo.mcClockDivide <<
              PDM_GetOPModeClockDivideReg(channel_id)));

        SAL_WriteReg(val, op_mode_reg);

        if(PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode == PDM_OUTPUT_MODE_PHASE_1)
        {
            pstn_1_reg = PMM_BASE +
                         (module_id * PMM_MODULE_OFFSET) +
                         PDM_GetPSTN1Reg(channel_id);

            pstn_2_reg = PMM_BASE +
                         (module_id * PMM_MODULE_OFFSET) +
                         PDM_GetPSTN2Reg(channel_id);

            /* low count setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcPosition1,
                          pstn_1_reg);

            /* high count setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcPosition2,
                          pstn_2_reg);
        }
        else if(PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode == PDM_OUTPUT_MODE_PHASE_2)
        {
            pstn_1_reg = PMM_BASE +
                         (module_id * PMM_MODULE_OFFSET) +
                         PDM_GetPSTN1Reg(channel_id);

            pstn_2_reg = PMM_BASE +
                         (module_id * PMM_MODULE_OFFSET) +
                         PDM_GetPSTN2Reg(channel_id);

            pstn_3_reg = PMM_BASE +
                         (module_id * PMM_MODULE_OFFSET) +
                         PDM_GetPSTN3Reg(channel_id);

            pstn_4_reg = PMM_BASE +
                         (module_id * PMM_MODULE_OFFSET) +
                         PDM_GetPSTN4Reg(channel_id);

            /* low 1 count setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcPosition1,
                          pstn_1_reg);

            /* high 1 count setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcPosition2,
                          pstn_2_reg);

            /* low 2 count setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcPosition3,
                          pstn_3_reg);

            /* high 2 count setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcPosition4,
                          pstn_4_reg);

        }
        else if(PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode == PDM_OUTPUT_MODE_REGISTER_1)
        {
            out_ptn_1_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN1Reg(channel_id);

            /* output pattern1 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1,
                          out_ptn_1_reg);
        }
        else if(PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode == PDM_OUTPUT_MODE_REGISTER_2)
        {
            out_ptn_1_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN1Reg(channel_id);

            out_ptn_2_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN2Reg(channel_id);

            out_ptn_3_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN3Reg(channel_id);

            out_ptn_4_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN4Reg(channel_id);

            /* output pattern1 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1,
                          out_ptn_1_reg);
            /* output pattern2 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern2,
                          out_ptn_2_reg);
            /* output pattern3 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern3,
                          out_ptn_3_reg);
            /* output pattern4 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern4,
                          out_ptn_4_reg);
        }
        else if(PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode == PDM_OUTPUT_MODE_REGISTER_2)
        {
            out_ptn_1_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN1Reg(channel_id);

            out_ptn_2_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN2Reg(channel_id);

            out_ptn_3_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN3Reg(channel_id);

            out_ptn_4_reg = PMM_BASE +
                            (module_id * PMM_MODULE_OFFSET) +
                            PDM_GetOutPTN4Reg(channel_id);

            max_cnt_reg   = PMM_BASE + PDM_MAX_CNT_REG_OFFSET  + (module_id * PMM_MODULE_OFFSET);

            /* output pattern1 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1,
                          out_ptn_1_reg);
            /* output pattern2 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern2,
                          out_ptn_2_reg);
            /* output pattern3 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern3,
                          out_ptn_3_reg);
            /* output pattern4 setting */
            SAL_WriteReg(PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern4,
                          out_ptn_4_reg);

            /* output data maximum count register, MAX_CNT, 7bits */
            val = SAL_ReadReg(max_cnt_reg);

            val = (val & ~(0xFFUL << PDM_GetMaxCountValueReg(channel_id))) |
                  (uint32)(PDMHandler[uiChannel].chModeCfgInfo.mcMaxCount <<
                  PDM_GetMaxCountValueReg(channel_id));

            SAL_WriteReg(val, max_cnt_reg);
        }
        else
        {
            ret = (SALRetCode_t)SAL_RET_FAILED;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                           PMM_PDM0IrqHandler
*
* PMM PDM0 ISR handler
*
* @param    user data argument
* @return
* Notes
*
***************************************************************************************************
*/

static void PMM_PDM0IrqHandler
(
    void *                              pArg
)
{
    uint32  reg;
    uint32  val;

    reg = PMM0_BASE + PMM_FAULT_STS_REG_OFFSET;
    val = 0UL;

    (void)pArg;
    val = SAL_ReadReg(reg);
    (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_PDM0);

    if((val & PMM_FAULT_STS_CH_A_MASK) != (uint32)NULL)
    {
        PDMHandler[0].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_B_MASK) != (uint32)NULL)
    {
        PDMHandler[1].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_C_MASK) != (uint32)NULL)
    {
        PDMHandler[2].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_D_MASK) != (uint32)NULL)
    {
        PDMHandler[3].chPMMErrChannel = TRUE;
    }

    //PDM_Err("\n FMU IRQ : fault status 0x%x \n", val);

}

/*
***************************************************************************************************
*                                           PMM_PDM1IrqHandler
*
* PMM PDM1 ISR handler
*
* @param    user data argument
* @return
* Notes
*
***************************************************************************************************
*/
static void PMM_PDM1IrqHandler
(
    void *                              pArg
)
{
    uint32  reg;
    uint32  val;

    reg = PMM1_BASE + PMM_FAULT_STS_REG_OFFSET;
    val = 0UL;

    (void)pArg;
    val = SAL_ReadReg(reg);
    (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_PDM1);

    if((val & PMM_FAULT_STS_CH_A_MASK) != (uint32)NULL)
    {
        PDMHandler[4].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_B_MASK) != (uint32)NULL)
    {
        PDMHandler[5].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_C_MASK) != (uint32)NULL)
    {
        PDMHandler[6].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_D_MASK) != (uint32)NULL)
    {
        PDMHandler[7].chPMMErrChannel = TRUE;
    }

    //PDM_Err("\n FMU IRQ : fault status 0x%x \n", val);

}

/*
***************************************************************************************************
*                                           PMM_PDM2IrqHandler
*
* PMM PDM2 ISR handler
*
* @param    user data argument
* @return
* Notes
*
***************************************************************************************************
*/
static void PMM_PDM2IrqHandler
(
    void *                              pArg
)
{
    uint32  reg;
    uint32  val;

    reg = PMM2_BASE + PMM_FAULT_STS_REG_OFFSET;
    val = 0UL;

    (void)pArg;
    val = SAL_ReadReg(reg);
    (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_PDM2);

    if((val & PMM_FAULT_STS_CH_A_MASK) != (uint32)NULL)
    {
        PDMHandler[8].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_B_MASK) != (uint32)NULL)
    {
        PDMHandler[9].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_C_MASK) != (uint32)NULL)
    {
        PDMHandler[10].chPMMErrChannel = TRUE;
    }

    if((val & PMM_FAULT_STS_CH_D_MASK) != (uint32)NULL)
    {
        PDMHandler[11].chPMMErrChannel = TRUE;
    }

    //PDM_Err("\n FMU IRQ : fault status 0x%x \n", val);

}

#endif

/*
***************************************************************************************************
*                                          PDM_EnableClock
*
* Enable PDM peripheral clock.
* Notes
*
***************************************************************************************************
*/
void PDM_EnableClock
(
    void
)
{
    /*set peri clock*/
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_PWM0, PDM_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_PWM0);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_PWM1, PDM_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_PWM1);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_PWM2, PDM_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_PWM2);

    PDM_Delay1u(1000UL);

    PDM_D("\n PDM ch0-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_PWM0));
    PDM_D("\n PDM ch1-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_PWM1));
    PDM_D("\n PDM ch2-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_PWM2));
}

/*
***************************************************************************************************
*                                          PDM_DisableClock
*
* Disable PDM peripheral clock.
* Notes
*
***************************************************************************************************
*/
void PDM_DisableClock
(
    void
)
{
    /*set peri clock*/
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_PWM0);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_PWM1);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_PWM2);
}

/*
***************************************************************************************************
*                                          PDM_Init
*
* Initialize variable for register map and setup FMU for error Handling.
*
* Notes
*
***************************************************************************************************
*/
void PDM_Init
(
    void
)
{
    uint32  uiChannel;

    for(uiChannel = 0UL ; uiChannel < PDM_CH_MAX ; uiChannel++)
    {
        /* Initialize pdm handle */
        PDMHandler[uiChannel].chModuleId                     = uiChannel / PDM_TOTAL_CH_PER_MODULE;
        PDMHandler[uiChannel].chChannelId                    = uiChannel % PDM_TOTAL_CH_PER_MODULE;
        PDMHandler[uiChannel].chEnable                       = PDM_OFF;
        PDMHandler[uiChannel].chIdleState                    = PDM_OFF;
        PDMHandler[uiChannel].chOutSignalInIdle              = PDM_OFF;
        PDMHandler[uiChannel].chIdleState                    = PDM_OFF;

        PDMHandler[uiChannel].chModeCfgInfo.mcPortNumber     = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode  = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcClockDivide    = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcLoopCount      = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcInversedSignal = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcPeriodNanoSec1 = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcDutyNanoSec1   = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcPeriodNanoSec2 = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcDutyNanoSec2   = PDM_OFF;
        PDMHandler[uiChannel].chModeCfgInfo.mcPosition1      = 0x0;
        PDMHandler[uiChannel].chModeCfgInfo.mcPosition2      = 0x0;
        PDMHandler[uiChannel].chModeCfgInfo.mcPosition3      = 0x0;
        PDMHandler[uiChannel].chModeCfgInfo.mcPosition4      = 0x0;

        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1    = 0x0;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern2    = 0x0;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern3    = 0x0;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern4    = 0x0;
        PDMHandler[uiChannel].chModeCfgInfo.mcMaxCount       = 0x0;

#ifdef PDM_SAFETY_FEATURE_ENABLED
        PDMHandler[uiChannel].chPMMEnable            = PMM_ON;
        PDMHandler[uiChannel].chPMMInputCnect        = PMM_OFF;
        PDMHandler[uiChannel].chPMMFaultStatus       = 0x0UL;
        PDMHandler[uiChannel].chPMMTimeoutValue      = 0xFFFFFFFFUL;
        PDMHandler[uiChannel].chPMMDutyMargin        = 0xFFUL;
        PDMHandler[uiChannel].chPMMErrChannel        = 0xFFUL;
#endif
    }

    PDM_EnableClock();

#ifdef PDM_SAFETY_FEATURE_ENABLED

    if(g_safety_enabled == FALSE)
    {
        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PDM0, (FMUSeverityLevelType_t)FMU_SVL_HIGH,
                                  (FMUIntFnctPtr)&PMM_PDM0IrqHandler,
                                   NULL_PTR);

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PDM1, (FMUSeverityLevelType_t)FMU_SVL_HIGH,
                                  (FMUIntFnctPtr)&PMM_PDM1IrqHandler,
                                  NULL_PTR);

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PDM2, (FMUSeverityLevelType_t)FMU_SVL_HIGH,
                                  (FMUIntFnctPtr)&PMM_PDM2IrqHandler,
                                  NULL_PTR);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_PDM0);
        (void)FMU_Set((FMUFaultid_t)FMU_ID_PDM1);
        (void)FMU_Set((FMUFaultid_t)FMU_ID_PDM2);

        g_safety_enabled = TRUE;
    }

#endif

    return;
}

/*
***************************************************************************************************
*                                          PDM_Deinit
*
* De-initialize PDM descriptor.
*
* Notes
*
***************************************************************************************************
*/

void PDM_Deinit
(
    void
)
{
    PDM_DisableClock();
    (void)SAL_MemSet(PDMHandler, 0, sizeof(PDMHandler)*PDM_TOTAL_CHANNELS);

#ifdef PDM_SAFETY_FEATURE_ENABLED
    (void)SAL_MemSet(PDMIsrData, 0, sizeof(PDMIsrData)*PDM_TOTAL_CHANNELS);
#endif

}

/*
***************************************************************************************************
*                                          PDM_SetPortSelection
*
* Set Port Selection Register
*
* @param    channel number 0 ~ 11.
* @param    port number.
* @return
*
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t PDM_SetPortSelection
(
    uint32                              uiChannel,
    uint32                              uiPortNum
)
{
    uint32          val;
    uint32          reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val         = 0;
    reg         = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        PDMHandler[uiChannel].chModeCfgInfo.mcPortNumber = uiPortNum;
        reg = PDM_PCFG_BASE + PDM_GetPortCfgSelectionReg(module_id);
        val = SAL_ReadReg(reg);
        val &= ~((0xFFUL) << PDM_GetPortCfgSelectionShift(channel_id));
        val |= uiPortNum << PDM_GetPortCfgSelectionShift(channel_id);
        SAL_WriteReg(val, reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_WaitForIdleStatus
*
* Wait for IDLE status
*
* @param    channel number 0 ~ 11.
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t PDM_WaitForIdleStatus
(
    uint32                              uiChannel
)
{
    SALRetCode_t    ret;
    uint32          delay_cnt;

    ret         = PDM_CheckChannelValidation(uiChannel);
    delay_cnt   = 0x3FFFFUL;

    if(ret == SAL_RET_SUCCESS)
    {
        while(delay_cnt > 0UL) /* QAC */
        {
            delay_cnt--;

            if(PDM_GetChannelStatus(uiChannel) == 0x0UL)
            {
                break;
            }
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_SetOutputLoopCount
*
* Set PDM Ouput Loop Count
*
* @param    channel number 0 ~ 11.
* @param    if repetition_loop value is '0' PDM-X operate infinite value mode, defult '0'
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t PDM_SetOutputLoopCount
(
    uint32                              uiChannel,
    uint32                              uiLooCount
)
{
    uint32          val;
    uint32          step_reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val         = 0;
    step_reg    = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        PDMHandler[uiChannel].chModeCfgInfo.mcLoopCount = uiLooCount;
        step_reg    = PDM_BASE + PDM_OP_STEP_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);
        val = SAL_ReadReg(step_reg);
        val = val & ~(0xFUL << (PDM_GetOPStepLoopCountReg(channel_id)));
        val = val |(uiLooCount << (PDM_GetOPStepLoopCountReg(channel_id)));
        SAL_WriteReg(val, step_reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                           PDM_SetOutputInverse
*
* Enable output signal Inverse

* @param    channel number 0 ~ 11.
* @param    0: normal , 1: inverse
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t PDM_SetOutputInverse
(
    uint32                              uiChannel,
    uint32                              uiOutputInverse
)
{
    uint32          val;
    uint32          op_mode_reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val         = 0;
    op_mode_reg = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id  = PDMHandler[uiChannel].chModuleId;
    channel_id = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        op_mode_reg = PDM_BASE + PDM_OP_MODE_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);
        val         = SAL_ReadReg(op_mode_reg);
        PDMHandler[uiChannel].chModeCfgInfo.mcInversedSignal = uiOutputInverse;

        /*output_signal, INV_X is set, output signal inversed*/
        val = ((val & ~(0x1UL << PDM_GetOPModeSignalInverseReg(channel_id))) |
               (uiOutputInverse << PDM_GetOPModeSignalInverseReg(channel_id)));

        SAL_WriteReg(val, op_mode_reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_SetRegisterOutMode1
*
* Set register mode 1 pattern value and clock divide
*
* @param    channel number 0 ~ 11.
* @param    clock divide value.
* @param    pattern value, 32bits.
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t PDM_SetRegisterOutMode1
(
    uint32                              uiChannel,
    uint32                              uiClockkDivide,
    uint32                              uiPatternValue
)
{
    uint32          val;
    uint32          op_mode_reg;
    uint32          out_ptn_1_reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val             = 0;
    op_mode_reg     = 0;
    out_ptn_1_reg   = 0;
    ret             = PDM_CheckChannelValidation(uiChannel);
    module_id       = PDMHandler[uiChannel].chModuleId;
    channel_id      = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        op_mode_reg     = PDM_BASE + PDM_OP_MODE_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);
        out_ptn_1_reg   = PDM_BASE + (module_id * PDM_MODULE_OFFSET) + PDM_GetOutPTN1Reg(channel_id);
        val             = SAL_ReadReg(op_mode_reg);

        if((((val >> PDM_GetOPModeOperationModeReg(channel_id)) & 0xFU) != PDM_OUTPUT_MODE_REGISTER_1) &&
            (PDMHandler[uiChannel].chEnable == PDM_ON))
        {
            (void)PDM_Disable(uiChannel, PMM_ON);
            (void)PDM_WaitForIdleStatus(uiChannel);
        }

        PDMHandler[uiChannel].chModeCfgInfo.mcClockDivide = uiClockkDivide;
        PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode = PDM_OUTPUT_MODE_REGISTER_1;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1 = uiPatternValue;

        /* operation mode */
        val = (val & ~((uint32)0xFU << PDM_GetOPModeOperationModeReg(channel_id))) |
              ((uint32)PDM_OUTPUT_MODE_REGISTER_1 << PDM_GetOPModeOperationModeReg(channel_id));

        /* clk divide */
        val = ((val & ~((uint32)0x3U << PDM_GetOPModeClockDivideReg(channel_id))) |
              (uiClockkDivide << PDM_GetOPModeClockDivideReg(channel_id)));

        SAL_WriteReg(val, op_mode_reg);

        /* output pattern register */
        SAL_WriteReg(uiPatternValue, out_ptn_1_reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_SetRegisterOutMode2
*
* Set register mode 2 pattern value and clock divide
*
* @param    channel number 0 ~ 11.
* @param    clock divide value.
* @param    pattern1 value, 32bits.
* @param    pattern2 value, 32bits.
* @param    pattern3 value, 32bits.
* @param    pattern4 value, 32bits.
* @return
*
* Notes set register mode, use 4register, 4*32bits in each PDM clock cycles,
        PDM generates PDM output signal by reading all bits of 1 register.
*
***************************************************************************************************
*/
static SALRetCode_t PDM_SetRegisterOutMode2
(
    uint32                              uiChannel,
    uint32                              uiClockkDivide,
    uint32                              uiPatternValue1,
    uint32                              uiPatternValue2,
    uint32                              uiPatternValue3,
    uint32                              uiPatternValue4
)
{
    uint32          val;
    uint32          op_mode_reg;
    uint32          out_ptn_1_reg;
    uint32          out_ptn_2_reg;
    uint32          out_ptn_3_reg;
    uint32          out_ptn_4_reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val             = 0;
    op_mode_reg     = 0;
    out_ptn_1_reg   = 0;
    out_ptn_2_reg   = 0;
    out_ptn_3_reg   = 0;
    out_ptn_4_reg   = 0;

    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        op_mode_reg = PDM_BASE + PDM_OP_MODE_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);

        out_ptn_1_reg = PDM_BASE +
                        (module_id * PDM_MODULE_OFFSET) +
                        PDM_GetOutPTN1Reg(channel_id);

        out_ptn_2_reg = PDM_BASE +
                        (module_id * PDM_MODULE_OFFSET) +
                        PDM_GetOutPTN2Reg(channel_id);

        out_ptn_3_reg = PDM_BASE +
                        (module_id * PDM_MODULE_OFFSET) +
                        PDM_GetOutPTN3Reg(channel_id);

        out_ptn_4_reg = PDM_BASE +
                        (module_id * PDM_MODULE_OFFSET) +
                        PDM_GetOutPTN4Reg(channel_id);

        val = SAL_ReadReg(op_mode_reg);

        if((((val >> PDM_GetOPModeOperationModeReg(channel_id)) & 0xFU) != PDM_OUTPUT_MODE_REGISTER_2) &&
             (PDMHandler[uiChannel].chEnable == PDM_ON))
        {
            (void)PDM_Disable(uiChannel, PMM_ON);
            (void)PDM_WaitForIdleStatus(uiChannel);
        }

        PDMHandler[uiChannel].chModeCfgInfo.mcClockDivide   = uiClockkDivide;
        PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode = PDM_OUTPUT_MODE_REGISTER_2;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1   = uiPatternValue1;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1   = uiPatternValue2;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1   = uiPatternValue3;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1   = uiPatternValue4;


        /* operation mode */
        val = (val & ~((uint32)0xFU << PDM_GetOPModeOperationModeReg(channel_id))) |
              ((uint32)PDM_OUTPUT_MODE_REGISTER_2 << PDM_GetOPModeOperationModeReg(channel_id));

        /* clk divide */
        val = ((val & ~((uint32)0x3U << PDM_GetOPModeClockDivideReg(channel_id))) |
              (uiClockkDivide << PDM_GetOPModeClockDivideReg(channel_id)));

        SAL_WriteReg(val, op_mode_reg);

        /* output pattern register */
        SAL_WriteReg(uiPatternValue1, out_ptn_1_reg);
        SAL_WriteReg(uiPatternValue2, out_ptn_2_reg);
        SAL_WriteReg(uiPatternValue3, out_ptn_3_reg);
        SAL_WriteReg(uiPatternValue4, out_ptn_4_reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_SetRegisterOutMode3
*
* Set register mode 3 pattern value and clock divide and MAX value
*
* @param    channel number 0 ~ 11.
* @param    clock divide value.
* @param    pattern1 value, 32bits.
* @param    pattern2 value, 32bits.
* @param    pattern3 value, 32bits.
* @param    pattern4 value, 32bits.
* @param    MAX value, 1 ~ 127
* @return
*
* Notes set register mode, use 4register, 4*32bits in each PDM clock cycles,
        PDM generates PDM output signal by reading all bits of 1 register.
*
***************************************************************************************************
*/
static SALRetCode_t PDM_SetRegisterOutMode3
(
    uint32                              uiChannel,
    uint32                              uiClockkDivide,
    uint32                              uiPatternValue1,
    uint32                              uiPatternValue2,
    uint32                              uiPatternValue3,
    uint32                              uiPatternValue4,
    uint32                              uiMaxValue
)
{
    uint32          val;
    uint32          op_mode_reg;
    uint32          out_ptn_1_reg;
    uint32          out_ptn_2_reg;
    uint32          out_ptn_3_reg;
    uint32          out_ptn_4_reg;
    uint32          max_cnt_reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val             = 0;
    op_mode_reg     = 0;
    out_ptn_1_reg   = 0;
    out_ptn_2_reg   = 0;
    out_ptn_3_reg   = 0;
    out_ptn_4_reg   = 0;
    max_cnt_reg     = 0;

    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        op_mode_reg = PDM_BASE + PDM_OP_MODE_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);

        out_ptn_1_reg = PDM_BASE +
                        (module_id * PDM_MODULE_OFFSET) +
                        PDM_GetOutPTN1Reg(channel_id);

        out_ptn_2_reg = PDM_BASE +
                        (module_id * PDM_MODULE_OFFSET) +
                        PDM_GetOutPTN2Reg(channel_id);

        out_ptn_3_reg = PDM_BASE +
                        (module_id * PDM_MODULE_OFFSET) +
                        PDM_GetOutPTN3Reg(channel_id);

        out_ptn_4_reg = PDM_BASE +
                        (module_id * PDM_MODULE_OFFSET) +
                        PDM_GetOutPTN4Reg(channel_id);

        max_cnt_reg = PDM_BASE + PDM_MAX_CNT_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);

        val = SAL_ReadReg(op_mode_reg);

        if((((val >> PDM_GetOPModeOperationModeReg(channel_id)) & 0xFU) != PDM_OUTPUT_MODE_REGISTER_3) &&
             (PDMHandler[uiChannel].chEnable == PDM_ON))
        {
            (void)PDM_Disable(uiChannel, PMM_ON);
            (void)PDM_WaitForIdleStatus(uiChannel);
        }

        PDMHandler[uiChannel].chModeCfgInfo.mcClockDivide   = uiClockkDivide;
        PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode = PDM_OUTPUT_MODE_REGISTER_3;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1   = uiPatternValue1;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1   = uiPatternValue2;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1   = uiPatternValue3;
        PDMHandler[uiChannel].chModeCfgInfo.mcOutPattern1   = uiPatternValue4;
        PDMHandler[uiChannel].chModeCfgInfo.mcMaxCount      = uiMaxValue;

        /* operation mode */
        val = (val & ~((uint32)0xFU << PDM_GetOPModeOperationModeReg(channel_id))) |
              ((uint32)PDM_OUTPUT_MODE_REGISTER_2 << PDM_GetOPModeOperationModeReg(channel_id));

        /* clk divide */
        val = ((val & ~((uint32)0x3U << PDM_GetOPModeClockDivideReg(channel_id))) |
               (uiClockkDivide << PDM_GetOPModeClockDivideReg(channel_id)));

        SAL_WriteReg(val, op_mode_reg);

        /* output pattern register */
        SAL_WriteReg(uiPatternValue1, out_ptn_1_reg);
        SAL_WriteReg(uiPatternValue2, out_ptn_2_reg);
        SAL_WriteReg(uiPatternValue3, out_ptn_3_reg);
        SAL_WriteReg(uiPatternValue4, out_ptn_4_reg);

        /* output data maximum count register, MAX_CNT, 7bits */
        val = SAL_ReadReg(max_cnt_reg);

        val = (val & ~((uint32)0xFFU << PDM_GetMaxCountValueReg(channel_id))) |
              (uiMaxValue << PDM_GetMaxCountValueReg(channel_id));

        SAL_WriteReg(val, max_cnt_reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_SetPhaseMode1
*
* set phase mode 1, PSTN_1 and PSTN_2
*
* @param    channel number 0 ~ 11.
* @param    PSTN_1 value.
* @param    PSTN_2 value.
* @return
*
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t PDM_SetPhaseMode1
(
    uint32                              uiChannel,
    uint32                              uiClockkDivide,
    uint32                              uiPositionValue1,
    uint32                              uiPositionValue2
)
{
    uint32          val;
    uint32          op_mode_reg;
    uint32          pstn_1_reg; /* low position */
    uint32          pstn_2_reg; /* high position */

    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    val         = 0;
    op_mode_reg = 0;
    pstn_1_reg  = 0;
    pstn_2_reg  = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;


    if(ret == SAL_RET_SUCCESS)
    {
        op_mode_reg = PDM_BASE + PDM_OP_MODE_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);
        pstn_1_reg  = PDM_BASE + (module_id * PDM_MODULE_OFFSET) + PDM_GetPSTN1Reg(channel_id);
        pstn_2_reg  = PDM_BASE + (module_id * PDM_MODULE_OFFSET) + PDM_GetPSTN2Reg(channel_id);
        val         = SAL_ReadReg(op_mode_reg);

        if((((val >> PDM_GetOPModeOperationModeReg(channel_id)) & 0xFU) != PDM_OUTPUT_MODE_PHASE_1) &&
             (PDMHandler[uiChannel].chEnable == PDM_ON))
        {
            (void)PDM_Disable(uiChannel, PMM_ON);
            (void)PDM_WaitForIdleStatus(uiChannel);
        }

        PDMHandler[uiChannel].chModeCfgInfo.mcClockDivide   = uiClockkDivide;
        PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode = PDM_OUTPUT_MODE_PHASE_1;
        PDMHandler[uiChannel].chModeCfgInfo.mcPosition1     = uiPositionValue1;
        PDMHandler[uiChannel].chModeCfgInfo.mcPosition2     = uiPositionValue2;

        /* operation mode phase 1 */
        val = (val & ~((uint32)0xFU << PDM_GetOPModeOperationModeReg(channel_id))) |
              ((uint32)PDM_OUTPUT_MODE_PHASE_1 << PDM_GetOPModeOperationModeReg(channel_id));

        SAL_WriteReg(val, op_mode_reg);

        /* divide clock,  default value by 2 */
        val = SAL_ReadReg(op_mode_reg);
        val = ((val & ~((uint32)0x3U << PDM_GetOPModeClockDivideReg(channel_id))) |
               (uiClockkDivide << PDM_GetOPModeClockDivideReg(channel_id)));

        SAL_WriteReg(val, op_mode_reg);

        /* pahse mode position register */
        SAL_WriteReg((uint32) uiPositionValue1, pstn_1_reg);
        SAL_WriteReg((uint32) uiPositionValue2, pstn_2_reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_SetPhaseMode2
*
* set phase mode 1, PSTN_1, PSTN_2, PSTN_3 and PSTN_4
*
* @param    channel number 0 ~ 11.
* @param    PSTN_1 value.
* @param    PSTN_2 value.
* @param    PSTN_3 value.
* @param    PSTN_4 value.
* @return
*
* Notes
*
***************************************************************************************************
*/
 static SALRetCode_t PDM_SetPhaseMode2
(
    uint32                              uiChannel,
    uint32                              uiClockkDivide,
    uint32                              uiPositionValue1,
    uint32                              uiPositionValue2,
    uint32                              uiPositionValue3,
    uint32                              uiPositionValue4
)
{
     uint32         val;
     uint32         op_mode_reg;
     uint32         pstn_1_reg; /* low position */
     uint32         pstn_2_reg; /* high position */
     uint32         pstn_3_reg; /* low position-2*/
     uint32         pstn_4_reg; /* high position-2 */
     SALRetCode_t   ret;
     uint32         module_id;
     uint32         channel_id;

     val            = 0;
     op_mode_reg    = 0;
     pstn_1_reg     = 0;
     pstn_2_reg     = 0;
     pstn_3_reg     = 0;
     pstn_4_reg     = 0;

     ret        = PDM_CheckChannelValidation(uiChannel);
     module_id  = PDMHandler[uiChannel].chModuleId;
     channel_id = PDMHandler[uiChannel].chChannelId;

     if(ret == SAL_RET_SUCCESS)
     {
         op_mode_reg = PDM_BASE + PDM_OP_MODE_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);
         pstn_1_reg  = PDM_BASE + (module_id * PDM_MODULE_OFFSET) + PDM_GetPSTN1Reg(channel_id);
         pstn_2_reg  = PDM_BASE + (module_id * PDM_MODULE_OFFSET) + PDM_GetPSTN2Reg(channel_id);
         pstn_3_reg  = PDM_BASE + (module_id * PDM_MODULE_OFFSET) + PDM_GetPSTN3Reg(channel_id);
         pstn_4_reg  = PDM_BASE + (module_id * PDM_MODULE_OFFSET) + PDM_GetPSTN4Reg(channel_id);
         val         = SAL_ReadReg(op_mode_reg);

         if((((val >> PDM_GetOPModeOperationModeReg(channel_id)) & 0xFU) != PDM_OUTPUT_MODE_PHASE_2) &&
            (PDMHandler[uiChannel].chEnable == PDM_ON))
         {
             (void)PDM_Disable(uiChannel, PMM_ON);
             (void)PDM_WaitForIdleStatus(uiChannel);
         }

         PDMHandler[uiChannel].chModeCfgInfo.mcClockDivide      = uiClockkDivide;
         PDMHandler[uiChannel].chModeCfgInfo.mcOperationMode    = PDM_OUTPUT_MODE_PHASE_2;
         PDMHandler[uiChannel].chModeCfgInfo.mcPosition1        = uiPositionValue1;
         PDMHandler[uiChannel].chModeCfgInfo.mcPosition2        = uiPositionValue2;
         PDMHandler[uiChannel].chModeCfgInfo.mcPosition3        = uiPositionValue3;
         PDMHandler[uiChannel].chModeCfgInfo.mcPosition4        = uiPositionValue4;

         /* operation mode phase 2 */
         val = (val & ~((uint32)0xFU << PDM_GetOPModeOperationModeReg(channel_id))) |
               ((uint32)PDM_OUTPUT_MODE_PHASE_2 << PDM_GetOPModeOperationModeReg(channel_id));
         SAL_WriteReg(val, op_mode_reg);

         /* divide by 2 : default value */
         val = SAL_ReadReg(op_mode_reg);
         val = ((val & ~((uint32)0x3U << PDM_GetOPModeClockDivideReg(channel_id))) |
               (uiClockkDivide << PDM_GetOPModeClockDivideReg(channel_id)));
         SAL_WriteReg(val, op_mode_reg);

         /* pahse mode position register */
         SAL_WriteReg((uint32) uiPositionValue1, pstn_1_reg);
         SAL_WriteReg((uint32) uiPositionValue2, pstn_2_reg);
         SAL_WriteReg((uint32) uiPositionValue3, pstn_3_reg);
         SAL_WriteReg((uint32) uiPositionValue4, pstn_4_reg);
     }

     return ret;
}

/*
***************************************************************************************************
*                                          PDM_GetChannelStatus
*
* Get channel status
*
* @param    channel number 0 ~ 11.
* @return   return value 1 means PDMn-X is busy status
*
* Notes
*
***************************************************************************************************
*/
uint32 PDM_GetChannelStatus
(
    uint32                              uiChannel
)
{
    uint32  val;
    uint32  reg;
    uint32  ret;
    uint32  module_id;
    uint32  channel_id;

    val         = 0;
    reg         = 0;
    ret         = 0xFFFFFFFFUL;
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(PDM_CheckChannelValidation(uiChannel) == SAL_RET_SUCCESS)
    {
        reg = PDM_BASE + PDM_STATUS_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);
        val = SAL_ReadReg(reg);
        ret = ((val >> PDM_GetBusyStatusReg(channel_id)) & 0x1UL);
    }

    return ret;
}

/*
***************************************************************************************************
*                                           PDM_SetDutyAndPeriod
*
* calculate duty and period for phase mode 1 & 2

* @param    channel number 0 ~ 11.
* @param    duty1 ns
* @param    period1 ns
* @param    duty1 ns
* @param    period2 ns
* @return
*
* Notes     if period2 is '0', pdm mode will be phase mode1, otherwise phase mode2
*           nano second scale.
*
***************************************************************************************************
*/
static SALRetCode_t PDM_SetDutyAndPeriod
(
    uint32                              uiChannel,
    uint32                              uiDutyNanoSec1,
    uint32                              uiPeriodNanoSec1,
    uint32                              uiDutyNanoSec2,
    uint32                              uiPeriodNanoSec2
)
{
    SALRetCode_t    ret;
    uint32          clock_divide;
    uint32          clk_freq;
    uint32          clk_period_ns;
    uint32          total_cnt;
    uint32          divide;
    uint32          total_period_ns;
    uint32          phase_mode_type; /* phase mode 1 or phase mode 2 */
    uint32          period1_cnt;
    uint32          period2_cnt;
    uint32          cal_duty1;
    uint32          cal_period1;
    uint32          cal_duty2;
    uint32          cal_period2;
    uint32          position1;
    uint32          position2;
    uint32          position3;
    uint32          position4;

    ret             = PDM_CheckChannelValidation(uiChannel);
    clock_divide    = 0;
    clk_freq        = 1UL;
    clk_period_ns   = 0;
    total_cnt       = 0;
    divide          = 0;
    total_period_ns = 0;
    phase_mode_type = 0;
    period1_cnt     = 0;
    period2_cnt     = 0;
    cal_duty1       = 0;
    cal_period1     = 0;
    cal_duty2       = 0;
    cal_period2     = 0;
    position1       = 0;
    position2       = 0;
    position3       = 0;
    position4       = 0;

    if (ret == SAL_RET_SUCCESS)
    {
        /*check dt and prd*/
        if(uiPeriodNanoSec1 == 0UL)
        {
            PDM_Err("invalid period_ns \n");
            ret = SAL_RET_FAILED;
        }

        if((uiDutyNanoSec1 == 0UL) || (uiDutyNanoSec1 > uiPeriodNanoSec1) || (uiDutyNanoSec2 > uiPeriodNanoSec2))
        {
            PDM_Err("invalid duty_ns \n");
            ret = SAL_RET_FAILED;
        }

        /*check peripheral clock for PDM*/
        clk_freq = CLOCK_GetPeriRate((sint32)CLOCK_PERI_PWM0);

        if(clk_freq == 0UL)
        {
            PDM_Err("invalid peripheral clock \n");
            ret = SAL_RET_FAILED;
        }

        if(ret == SAL_RET_SUCCESS)
        {
            /*chekc phase mode type, phase mode 1 or phase mode 2*/
            if(uiPeriodNanoSec2 == 0UL)
            {
                phase_mode_type = PDM_OUTPUT_MODE_PHASE_1;
            }
            else
            {
                phase_mode_type = PDM_OUTPUT_MODE_PHASE_2;
            }

            /* calculate 1 cycle period, total period time */
            total_period_ns = uiPeriodNanoSec1 + uiPeriodNanoSec2;

            /*convert to nano-sec scale, period = 1sec / freq.*/
            clk_period_ns = (1000UL * 1000UL * 1000UL) / clk_freq;

            /*input clock divide*/
            while (clock_divide != PDM_DIVID_MAX)
            {
                /*input clock divide, default DIV_2 and 4, 8, 16, k is 0, 1, 3 */
                clk_period_ns = clk_period_ns * (2UL);

                /*get total count */
                total_cnt = (total_period_ns / clk_period_ns);

                if(phase_mode_type == PDM_OUTPUT_MODE_PHASE_1)
                {
                    /*output count value in phase mode : pstn1+2, pstn2+2*/
                    if (total_cnt >= 4UL)
                    {
                        total_cnt = total_cnt - 4UL;
                    }
                    else
                    {
                        total_cnt = 0UL;
                    }

                    if ((total_cnt <= 1UL) || (total_cnt <= 0xFFFFUL))
                    {
                        break;
                    }

                    clock_divide++;
                }
                else
                {
                    /*output count value in phase mode : pstn1+2, pstn2+2 pstn3+2, pstn4+2*/
                    if (total_cnt >= 8UL)
                    {
                        total_cnt = total_cnt - 8UL;
                    }
                    else
                    {
                        total_cnt = 0UL;
                    }

                    if ((total_cnt <= 1UL) || (total_cnt <= 0xFFFFUL))
                    {
                        break;
                    }

                    clock_divide++;
                }
            }

            if(phase_mode_type == PDM_OUTPUT_MODE_PHASE_1)
            {
                /* prevent over flow 32bit */
                for (divide = 1UL ; divide < 0xFFFFUL ; divide++)
                {
                    /* 0xFFFFFFFFU > total_cnt * duty_ns / divide */
                    if ((0xFFFFFFFFUL / uiDutyNanoSec1) > (total_cnt / divide))
                    {
                        break;
                    }
                }

                cal_duty1   = uiDutyNanoSec1 / divide;
                cal_period1 = uiPeriodNanoSec1 / divide;

                /* High */
                position2 = (total_cnt * (cal_duty1)) / (cal_period1);

                /* Low */
                position1 = total_cnt - position2;

                (void)PDM_SetPhaseMode1(uiChannel, clock_divide, position1, position2);
            }
            else
            {
                for (divide = 1UL ; divide < 0xFFFFUL ; divide++)
                {
                    /* 0xFFFFFFFFU > total_cnt * period_ns1 / divide */
                    if ((0xFFFFFFFFUL  / uiPeriodNanoSec1) > (total_cnt / divide))
                    {
                        break;
                    }
                }

                /* get duty 1 and period 1*/
                cal_duty1   = uiDutyNanoSec1 / divide;
                cal_period1 = uiPeriodNanoSec1 / divide;

                /* get duty 2 and period 2*/
                cal_duty2   = uiDutyNanoSec2 / divide;
                cal_period2 = uiPeriodNanoSec2 / divide;

                period1_cnt = (total_cnt* cal_period1) / (cal_period1 + cal_period2);
                period2_cnt = total_cnt - period1_cnt;

                /*High 1*/
                position2   = (period1_cnt * (cal_duty1)) / (cal_period1);

                /*Low 1*/
                position1   = period1_cnt - position2;

                /*High 2*/
                position4   = (period2_cnt * (cal_duty2)) / (cal_period2);

                /*LOW 2*/
                position3   = period2_cnt - position4;

                (void)PDM_SetPhaseMode2(uiChannel,
                                  clock_divide,
                                  position1,
                                  position2,
                                  position3,
                                  position4);
            }
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_Enable
*
* Enable pdm module, setup OP_EN register, and Trigger output signal
*
* @param    channel number 0 ~ 11.
* @param    Enable PMM monitoring.
* @return
*
* Notes To operate PDM correctly MUST
        set all configuration and enable first
        set VUP to 1 to announce the end of register update
        set TRIG to 1, PDM output will be generated correctly.
*
***************************************************************************************************
*/
SALRetCode_t PDM_Enable
(
    uint32                              uiChannel,
    uint32                              uiMonitoring
)
{
    uint32          reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    reg         = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        reg = PDM_BASE + PDM_OP_EN_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);

#ifdef PDM_SAFETY_FEATURE_ENABLED
        if(uiMonitoring == PMM_ON)
        {
            /*If in Busy state*/
            if(PDM_GetChannelStatus(uiChannel) == 0x1U)
            {
                (void)PDM_Disable(uiChannel, (uint32)PMM_ON);
                (void)PDM_WaitForIdleStatus(uiChannel);
            }

            (void)PMM_SetConfig(uiChannel);
            /* connection : internal and disable commssion error check  */
            (void)PMM_EnableMonitoring(uiChannel, (uint32)PDM_CONNECT_INTERNAL, (uint32)0);
        }
#endif

        SAL_WriteReg(SAL_ReadReg(reg) | ((uint32)0x1U << (PDM_GetOPENEnalbeReg(channel_id))) , reg);

        /* announce update PDM-A/B/C/D */
        SAL_WriteReg(SAL_ReadReg(reg) | ((uint32)0x1U << (PDM_GetOPENValueUpReg(channel_id))) , reg);

        /* <At least 1 Cycle delay required based on 'Peripheral_Clock/DIV_x'
          (DIV_x was set to 'PDMn_BASE + 0x08' in order 1)> */

        PDM_Delay1u(10000UL);

        /* triggering output generation PDM-A/B/C/D */
        SAL_WriteReg(SAL_ReadReg(reg) | ((uint32)0x1U << (PDM_GetOPENTrigReg(channel_id))), reg);
        PDMHandler[uiChannel].chEnable = PDM_ON;
    }

    return ret; //success
}

/*
***************************************************************************************************
*                                          PDM_Disable
*
* Disable pdm module
*
* @param    channel number 0 ~ 11.
* @param    Disable PMM monitoring.
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t PDM_Disable
(
    uint32                              uiChannel,
    uint32                              uiMonitoring
)
{
    uint32          reg;
    SALRetCode_t    ret;
    uint32          module_id;
    uint32          channel_id;

    reg         = 0;
    ret         = PDM_CheckChannelValidation(uiChannel);
    module_id   = PDMHandler[uiChannel].chModuleId;
    channel_id  = PDMHandler[uiChannel].chChannelId;

    if(ret == SAL_RET_SUCCESS)
    {
        reg = PDM_BASE + PDM_OP_EN_REG_OFFSET + (module_id * PDM_MODULE_OFFSET);

#ifdef PDM_SAFETY_FEATURE_ENABLED
        if(uiMonitoring == PMM_ON)
        {
            (void)PMM_DisableMonitoring(uiChannel);
        }
#endif

        PDMHandler[uiChannel].chEnable = PDM_OFF;
        /* clear enable bit PDM-A/B/C/D  */
        SAL_WriteReg(SAL_ReadReg(reg) & ~((uint32)0x1U << (PDM_GetOPENEnalbeReg(channel_id))), reg);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          PDM_SetConfig
*
* Set Configuration value for PMM
*
* @param    channel number 0 ~ 11.
* @param    Operation Mode Configure Information.
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t PDM_SetConfig
(
    uint32                              uiChannel,
    PDMModeConfig_t                     *pModeConfig
)
{
    SALRetCode_t    ret;

    ret = PDM_CheckChannelValidation(uiChannel);

    if((ret == SAL_RET_SUCCESS) && (pModeConfig != (PDMModeConfig_t*)NULL))
    {
        (void)PDM_SetPortSelection(uiChannel, pModeConfig->mcPortNumber);
        (void)PDM_SetOutputInverse(uiChannel, pModeConfig->mcInversedSignal);

        if(pModeConfig->mcLoopCount != (uint32)0UL)
        {
            /*Default '0' -> Infinite Loop */
            (void)PDM_SetOutputLoopCount(uiChannel, pModeConfig->mcLoopCount);
        }

        if(pModeConfig->mcOperationMode == PDM_OUTPUT_MODE_PHASE_1)
        {
            if((pModeConfig->mcPosition1 == 0x00000000UL) &&
               (pModeConfig->mcPosition2 == 0xFFFFFFFFUL))
            {
                PDM_D("\n Transit to IDLE State \n");
                PDMHandler[uiChannel].chIdleState = PDM_ON;
                (void)PDM_SetPhaseMode1(uiChannel, 0UL, 0x00000000UL, 0xFFFFFFFFUL);
            }
            else
            {
                PDMHandler[uiChannel].chIdleState = PDM_OFF;
                (void)PDM_SetDutyAndPeriod(uiChannel,
                                     pModeConfig->mcDutyNanoSec1,
                                     pModeConfig->mcPeriodNanoSec1,
                                     0UL,
                                     0UL);
            }
        }
        else if(pModeConfig->mcOperationMode == PDM_OUTPUT_MODE_PHASE_2)
        {

            if((pModeConfig->mcPosition1 == 0x00000000UL) &&
               (pModeConfig->mcPosition2 == 0xFFFFFFFFUL) &&
               (pModeConfig->mcPosition3 == 0x00000000UL) &&
               (pModeConfig->mcPosition4 == 0xFFFFFFFFUL))
            {
                PDM_D("\n Transit to IDLE State \n");
                PDMHandler[uiChannel].chIdleState = PDM_ON;
                (void)PDM_SetPhaseMode2(uiChannel, 0UL, 0x00000000UL, 0xFFFFFFFFUL, 0x00000000UL, 0xFFFFFFFFUL);
            }
            else
            {
                PDMHandler[uiChannel].chIdleState = PDM_OFF;
                (void)PDM_SetDutyAndPeriod(uiChannel,
                                           pModeConfig->mcDutyNanoSec1,
                                           pModeConfig->mcPeriodNanoSec1,
                                           pModeConfig->mcDutyNanoSec2,
                                           pModeConfig->mcPeriodNanoSec2);
            }
        }
        else if(pModeConfig->mcOperationMode == PDM_OUTPUT_MODE_REGISTER_1)
        {
            (void)PDM_SetRegisterOutMode1(uiChannel,
                                          pModeConfig->mcClockDivide,
                                          pModeConfig->mcOutPattern1);
        }
        else if(pModeConfig->mcOperationMode == PDM_OUTPUT_MODE_REGISTER_2)
        {

            (void)PDM_SetRegisterOutMode2(uiChannel,
                                          pModeConfig->mcClockDivide,
                                          pModeConfig->mcOutPattern1,
                                          pModeConfig->mcOutPattern2,
                                          pModeConfig->mcOutPattern3,
                                          pModeConfig->mcOutPattern4);

        }
        else if(pModeConfig->mcOperationMode == PDM_OUTPUT_MODE_REGISTER_3)
        {
            (void)PDM_SetRegisterOutMode3(uiChannel,
                                          pModeConfig->mcClockDivide,
                                          pModeConfig->mcOutPattern1,
                                          pModeConfig->mcOutPattern2,
                                          pModeConfig->mcOutPattern3,
                                          pModeConfig->mcOutPattern4,
                                          pModeConfig->mcMaxCount);
        }
        else
        {
            PDM_Err("\n PDM Invalid Operation Mode \n");
            ret = (SALRetCode_t)SAL_RET_FAILED;
        }
    }
    return ret;
}

