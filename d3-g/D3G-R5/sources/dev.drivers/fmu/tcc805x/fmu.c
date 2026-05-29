/*
***************************************************************************************************
*
*   FileName : fmu.c
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

#include <fmu.h>
#include <gic.h>
#include <bsp.h>

/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/

#ifdef FMU_DRIVER

#define FMU_FAULT_SOURCE                (64UL)
#define FMU_REG                         ((volatile FMUMc_t *)(FMU_BASE_ADDR))

#define FMU_PW                          (0x5afeace5UL)
#define FMU_PW_ERROR                    (0x5afe8031UL)

#define FMU_BASE_ADDR                   (0x1B936400UL)

#define FMU_FAULT_ACTIVE_LOW            (0)
#define FMU_FAULT_ACTIVE_HIGH           (1)


/* FMU_CTRL */
typedef struct FMUCtrl
{
    uint32                              ctForceFault        :  1;   //  [   00], RW
    uint32                              ctSwReset           :  1;   //  [   01], RW
    uint32                              ctSoftFaultTestEn   :  1;   //  [   02], RW
    uint32                              ctSoftFaultEn       :  1;   //  [   03], RW
    uint32                              ctPeriodicFaultEn   :  1;   //  [   04], RW
    uint32                                                  : 27;   //  [31:05]
} FMUCtrl_t;

typedef union FMUCtrlUnion
{
    uint32                              cuNreg;
    FMUCtrl_t                           cuBreg;
} FMUCtrlUnion_t;

/* FMU_EN */
typedef struct FMUEn
{
    uint32                              enAdc               :  1;   //  [0][   00], RW
    uint32                              enGpsb0             :  1;   //  [0][   01], RW
    uint32                              enGpsb1             :  1;   //  [0][   02], RW
    uint32                              enGpsb2             :  1;   //  [0][   03], RW
    uint32                              enGpsb3             :  1;   //  [0][   04], RW
    uint32                              enGpsb4             :  1;   //  [0][   05], RW
    uint32                              enGpsb5             :  1;   //  [0][   06], RW
    uint32                              enPdm0              :  1;   //  [0][   07], RW
    uint32                              enPdm1              :  1;   //  [0][   08], RW
    uint32                              enPdm2              :  1;   //  [0][   09], RW
    uint32                              enMcCfg             :  1;   //  [0][   10], RW
    uint32                              enCr5Cfg            :  1;   //  [0][   11], RW
    uint32                              enCkcCfg            :  1;   //  [0][   12], RW
    uint32                              enGpioCfg           :  1;   //  [0][   13], RW
    uint32                              enMidCfg            :  1;   //  [0][   14], RW
    uint32                              enCmcCfg            :  1;   //  [0][   15], RW
    uint32                              enSysSmCfg          :  1;   //  [0][   16], RW
    uint32                              enX2xMst0           :  1;   //  [0][   17], RW
    uint32                              enX2xMst1           :  1;   //  [0][   18], RW
    uint32                              enX2xSlv0           :  1;   //  [0][   19], RW
    uint32                              enX2xSlv1           :  1;   //  [0][   20], RW
    uint32                              enX2xSlv2           :  1;   //  [0][   21], RW
    uint32                              enSdm0              :  1;   //  [0][   22], RW
    uint32                              enSdm1              :  1;   //  [0][   23], RW
    uint32                              enSrvc              :  1;   //  [0][   24], RW
    uint32                              enPeriClk           :  1;   //  [0][   25], RW
    uint32                              enPll1Clk           :  1;   //  [0][   26], RW
    uint32                              enSram0Ecc          :  1;   //  [0][   27], RW
    uint32                              enSram1Ecc          :  1;   //  [0][   28], RW
    uint32                              enSfmcEcc           :  1;   //  [0][   29], RW
    uint32                              enMbox0SEcc         :  1;   //  [0][   30], RW
    uint32                              enMbox0NsEcc        :  1;   //  [0][   31], RW
    uint32                              enMbox1SEcc         :  1;   //  [0][   00], RW
    uint32                              enMbox1NsEcc        :  1;   //  [0][   01], RW
    uint32                              enMbox2Ecc          :  1;   //  [0][   02], RW
    uint32                              enOsc               :  1;   //  [1][   03], RW
    uint32                              enVldCore           :  1;   //  [1][   04], RW
    uint32                              enVldOtp            :  1;   //  [1][   05], RW
    uint32                              enWdt               :  1;   //  [1][   06], RW
    uint32                              enPll0Clk           :  1;   //  [1][   07], RW
    uint32                              enCpuClk            :  1;   //  [1][   08], RW
    uint32                              enBusClk            :  1;   //  [1][   09], RW
    uint32                              enChipCfgSoft       :  1;   //  [1][   10], RW
    uint32                              enChipCfgCrc        :  1;   //  [1][   11], RW
    uint32                              enRomCrc            :  1;   //  [1][   12], RW
    uint32                                                  : 18;   //  [1][30:13]
    uint32                              enFmuCfgSoft        :  1;   //  [1][   31], RW
} FMUEn_t;

typedef union FMUEnUnion
{
    uint32                              euNreg[2];
    FMUEn_t                             euBreg;
} FMUEnUnion_t;

// FMU_EXT_FAULT_CTRL
typedef struct FMUExtFaultCtrl
{
    uint32                              fcPolarity          :  1;   //  [   00], RW
    uint32                                                  :  3;   //  [03:01], RW
    uint32                              fcLow               :  4;   //  [07:04], RW
    uint32                              fcHigh              :  3;   //  [10:08], RW TODO
    uint32                                                  : 21;   //  [31:11]
} FMUExtFaultCtrl_t;

typedef union FMUExtFaultCtrlUnion
{
    uint32                              fcNreg;
    FMUExtFaultCtrl_t                   fcBreg;
} FMUExtFaultCtrlUnion_t;

// FMU_SEVERITY_LEVEL
typedef struct FMUSeverityLevel
{
    uint32                              slAdc               :  2;   //  [0][01:00], RW
    uint32                              slGpsb0             :  2;   //  [0][03:02], RW
    uint32                              slGpsb1             :  2;   //  [0][05:04], RW
    uint32                              slGpsb2             :  2;   //  [0][07:06], RW
    uint32                              slGpsb3             :  2;   //  [0][09:08], RW
    uint32                              slGpsb4             :  2;   //  [0][11:10], RW
    uint32                              slGpsb5             :  2;   //  [0][13:12], RW
    uint32                              slPdm0              :  2;   //  [0][15:14], RW
    uint32                              slPdm1              :  2;   //  [0][17:16], RW
    uint32                              slPdm2              :  2;   //  [0][19:18], RW
    uint32                              slMcCfg             :  2;   //  [0][21:20], RW
    uint32                              slCr5Cfg            :  2;   //  [0][23:22], RW
    uint32                              slCkcCfg            :  2;   //  [0][25:24], RW
    uint32                              slGpioCfg           :  2;   //  [0][27:26], RW
    uint32                              slMidCfg            :  2;   //  [0][29:28], RW
    uint32                              slCmcCfg            :  2;   //  [0][31:30], RW
    uint32                              slSysSmCfg          :  2;   //  [1][01:00], RW
    uint32                              slX2xMst0           :  2;   //  [1][03:02], RW
    uint32                              slX2xMst1           :  2;   //  [1][05:04], RW
    uint32                              slX2xSlv0           :  2;   //  [1][07:06], RW
    uint32                              slX2xSlv1           :  2;   //  [1][09:08], RW
    uint32                              slX2xSlv2           :  2;   //  [1][11:10], RW
    uint32                              slSdm0              :  2;   //  [1][13:12], RW
    uint32                              slSdm1              :  2;   //  [1][15:14], RW
    uint32                              slSrvc              :  2;   //  [1][17:16], RW
    uint32                              slPeriClk           :  2;   //  [1][19:18], RW
    uint32                              slPll1Clk           :  2;   //  [1][21:20], RW
    uint32                              slSram0Ecc          :  2;   //  [1][23:22], RW
    uint32                              slSram1Ecc          :  2;   //  [1][25:24], RW
    uint32                              slSfmcEcc           :  2;   //  [1][27:26], RW
    uint32                              slMbox0SEcc         :  2;   //  [1][29:28], RW
    uint32                              slMbox0NsEcc        :  2;   //  [1][31:30], RW
    uint32                              slMbox1SEcc         :  2;   //  [2][01:00], RW
    uint32                              slMbox1NsEcc        :  2;   //  [2][03:02], RW
    uint32                              slMbox2Ecc          :  2;   //  [2][05:04], RW
    uint32                              slOsc               :  2;   //  [2][07:06], RW
    uint32                              slVldCore           :  2;   //  [2][09:08], RW
    uint32                              slVldOtp            :  2;   //  [2][11:10], RW
    uint32                              slWdt               :  2;   //  [2][13:12], RW
    uint32                              slPll0Clk           :  2;   //  [2][15:14], RW
    uint32                              slCpuClk            :  2;   //  [2][17:16], RW
    uint32                              slBusClk            :  2;   //  [2][19:18], RW
    uint32                              slChipCfgSoft       :  2;   //  [2][21:20], RW
    uint32                              slChipCfgCrc        :  2;   //  [2][23:22], RW
    uint32                              slRomCrc            :  2;   //  [2][25:24], RW
    uint32                                                  :  6;   //  [2][31:26]
    uint32                                                  : 30;   //  [3][29:00]
    uint32                              slFmuCfgSoft        :  2;   //  [3][31:30], RW
} FMUSeverityLevel_t;

typedef union FMUSeverityLevelUnion
{
    uint32                              slNreg[4];
    FMUSeverityLevel_t                  slBreg;
} FMUSeverityLevelUnion_t;

// FMU_CLK_SEL
typedef struct FMUClkSel
{
    uint32                              csClkSel            :  1;   //  [   00], RW
    uint32                                                  : 31;   //  [31:01]
} FMUClkSel_t;

typedef union FMUClkSelUnion
{
    uint32                              csNreg;
    FMUClkSel_t                         csBreg;
} FMUClkSelUnion_t;

// FMU_OUT_STATUS
typedef struct FMUOutStatus
{
    uint32                              osIrq               :  1;   //  [   00], RO
    uint32                              osFiq               :  1;   //  [   01], RO
    uint32                              osFault             :  1;   //  [   02], RO
    uint32                                                  : 13;   //  [15:03]
    uint32                              osFaultState        :  2;   //  [17:16], RO
    uint32                                                  : 14;   //  [31:18]
} FMUOutStatus_t;

typedef union FMUOutStatusUnion
{
    uint32                              suNreg;
    FMUOutStatus_t                      suBreg;
} FMUOutStatusUnion_t;

typedef struct FMUMc
{
    FMUCtrlUnion_t                      mcCtrl;             // 0x00
    FMUEnUnion_t                        mcEn;               // 0x04 ~ 0x08
    FMUExtFaultCtrlUnion_t              mcExtFaultCtrl; // 0x0c
    FMUSeverityLevelUnion_t             mcSeverityLevel;    // 0x10 ~ 0x1c
    FMUEnUnion_t                        mcIrqMask;          // 0x20 ~ 0x24
    FMUEnUnion_t                        mcIrqClr;           // 0x28 ~ 0x2c
    FMUEnUnion_t                        mcFiqMask;          // 0x30 ~ 0x34
    FMUEnUnion_t                        mcFiqClr;           // 0x38 ~ 0x3c
    FMUEnUnion_t                        mcFaultMask;        // 0x40 ~ 0x44
    FMUEnUnion_t                        mcFaultClr;         // 0x48 ~ 0x4c
    FMUEnUnion_t                        mcIrqStatus;        // 0x50 ~ 0x54
    FMUEnUnion_t                        mcFiqStatus;        // 0x58 ~ 0x5c
    FMUEnUnion_t                        mcFaultStatus;      // 0x60 ~ 0x64
    FMUClkSelUnion_t                    mcClkSel;           // 0x68
    FMUOutStatusUnion_t                 mcOutStatus;        // 0x6c
    uint32                              mcCfgWrPw;          // 0x70
} FMUMc_t;

 // FMU_EN
typedef struct FMUSts
{
     FMUFaultid_t                       id;
     uint32                             cnt;
} FMUSts_t;

typedef struct FMUIntVectPtr
{
     FMUSeverityLevelType_t             severity_level;
     FMUIntFnctPtr                      func;
     void *                             arg;
} FMUIntVectPtr_t;

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

static FMUIntVectPtr_t FMU_IntVectTbl[FMU_FAULT_SOURCE];

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void FMU_IrqCallBack
(
    void *                              arg
);

static void FMU_FiqCallBack
(
    void *                              arg
);

static void FMU_SetExtFault
(
    uint8                               uiPolarity,
    uint32                              uiLowDuration,
    uint32                              uiHighDuration
);

static void FMU_SwReset
(
    void
);

static FMUErrTypes_t FMU_IsrMaskClear
(
    FMUFaultid_t                        uiId
);


static FMUErrTypes_t FMU_IsrClear
(
    FMUFaultid_t                        uiId
);

static FMUErrTypes_t FMU_DsCompare
(
    uint32                              uiValue,
    uint32                              uiReg
);

static FMUSts_t FMU_GetIrqStatus
(
    void
);

static FMUSts_t FMU_GetFiqStatus
(
    void
);

/*
***************************************************************************************************
*                                          FMU_IrqCallBack
*
* @param    void *arg [in]
*
*
* Notes
*
***************************************************************************************************
*/
static void FMU_IrqCallBack
(
    void *                              arg
)
{
    FMUSts_t        status;
    FMUIntFnctPtr   p_isr;
    void *          int_arg;

    FMU_D("[%s] IN!!\n", __func__);

    //Unused Parameter
    (void)arg;

    status = FMU_GetIrqStatus();

    //range check
    if((status.id) > FMU_ID_FMU_CFG_SOFT)
    {
        FMU_E("[%s]IRQ id(%d) is invalid\n",__func__, status.id);
    }
    else if ((status.id > FMU_ID_ROM_CRC) && (status.id < FMU_ID_FMU_CFG_SOFT))
    {
        FMU_E("[%s]IRQ id(%d) is reserved \n",__func__, status.id);
    }
    else
    {
        p_isr   = FMU_IntVectTbl[status.id].func;                     /* Fetch ISR handler.                                   */
        int_arg = FMU_IntVectTbl[status.id].arg;

        if(p_isr != (void *) 0)
        {
            FMU_E("[%s] Call Safety Mechanism IRQ handler[%d]\n",__func__,status.id);
            (*p_isr)(int_arg);                                      /* Call ISR handler.                                    */
        }
    }
}

/*
***************************************************************************************************
*                                          FMU_FiqCallBack
*
* @param    void *arg [in]
*
*
* Notes
*
***************************************************************************************************
*/
static void FMU_FiqCallBack
(
    void *                              arg
)
{
    FMUSts_t        status;
    FMUIntFnctPtr   p_isr;
    void *          int_arg;

    FMU_D("[%s] IN!!\n", __func__);

    //Unused Parameter
    (void)arg;

    status = FMU_GetFiqStatus();

    //range check
    if((status.id) > FMU_ID_FMU_CFG_SOFT)\
    {
        FMU_E("[%s]FIQ id(%d) is invalid\n",__func__, status.id);
    }
    else if ((status.id > FMU_ID_ROM_CRC) && (status.id < FMU_ID_FMU_CFG_SOFT))
    {
        FMU_E("[%s]FIQ id(%d) is reserved \n",__func__, status.id);
    }
    else
    {
        p_isr   = FMU_IntVectTbl[status.id].func;                     /* Fetch ISR handler.                                   */
        int_arg = FMU_IntVectTbl[status.id].arg;

        if(p_isr != (void *) 0)
        {
            FMU_E("[%s] Call Safety Mechanism FIQ handler[%d]\n",__func__,status.id);
            (*p_isr)(int_arg);                                      /* Call ISR handler.                                    */
        }
    }
}

/*
***************************************************************************************************
*                                          FMU_SetExtFault
*
* @param    uint8 uiPolarity [in]
* @param    uint32 low_duration [in]
* @param    uint32 high_duration [in]
*
* Notes
*
***************************************************************************************************
*/
static void FMU_SetExtFault
(
    uint8                               uiPolarity,
    uint32                              uiLowDuration,
    uint32                              uiHighDuration
)
{
    FMU_D("[%s] IN!! polarity %d low_duration %d high_duration %d\n", __func__, uiPolarity, uiLowDuration, uiHighDuration);

    if (uiPolarity > (uint8)2)
    {
        FMU_E("Polarity type(%d) is invalid @ FMU_SetExtFault()\n", uiPolarity);

    }
    else if (uiLowDuration > (uint32)16)
    {
        FMU_E("Fault low duration(%d) is invalid @ FMU_SetExtFault()\n", uiLowDuration);

    }
    else if (uiHighDuration > (uint32)2)
    {
        FMU_E("Fault high duration(%d) is invalid @ FMU_SetExtFault()\n", uiHighDuration);

    }
    else
    {
        FMU_REG->mcCfgWrPw              = FMU_PW;
        FMU_REG->mcExtFaultCtrl.fcNreg  = ((uiHighDuration << (uint32)8) | (uiLowDuration << (uint32)4) | ((uint32)uiPolarity << (uint32)0));
    }
}


/*
***************************************************************************************************
*                                          FMU_SwReset
*
*
* Notes
*
***************************************************************************************************
*/
static void FMU_SwReset
(
    void
)
{
    FMU_REG->mcCfgWrPw                  = FMU_PW;
    FMU_REG->mcCtrl.cuBreg.ctSwReset    = 0x1;

    FMU_REG->mcCfgWrPw                  = FMU_PW;
    FMU_REG->mcCtrl.cuBreg.ctSwReset    = 0x0;
}

/*
***************************************************************************************************
*                                          FMU_IsrMaskClear
*
* @param    uint8 id [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUErrTypes_t FMU_IsrMaskClear
(
    FMUFaultid_t                        uiId
)
{
    uint32          nth_reg;
    uint32          nth_bit;
    uint32          IRQ_reg_64[2];
    uint32          FIQ_reg_64[2];
    uint32          FOULT_reg_64[2];
    FMUErrTypes_t   ret;

    ret     = FMU_OK;
    nth_reg = ((uint32)uiId >> (uint32)5) & (uint32)0x1;
    nth_bit = ((uint32)uiId) & (uint32)0x1f;

    if (FMU_IntVectTbl[uiId].severity_level == FMU_SVL_LOW)
    {
        // for Double storage test
        IRQ_reg_64[nth_reg] = FMU_REG->mcIrqMask.euNreg[nth_reg];

        // Clear IRQ mask enable for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcIrqMask.euNreg[nth_reg]  = (FMU_REG->mcIrqMask.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        // for Double storage test
        IRQ_reg_64[nth_reg] = (IRQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(IRQ_reg_64[nth_reg], FMU_REG->mcIrqMask.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

    }
    else if(FMU_IntVectTbl[uiId].severity_level == FMU_SVL_MID)
    {
        // for Double storage test
        FIQ_reg_64[nth_reg] = FMU_REG->mcFiqMask.euNreg[nth_reg];

        // Clear FIQ mask enable for given FMU ID                                               // 0 ~ 31
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqMask.euNreg[nth_reg]  = (FMU_REG->mcFiqMask.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        // for Double storage test
        FIQ_reg_64[nth_reg] = (FIQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FIQ_reg_64[nth_reg], FMU_REG->mcFiqMask.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

    }
    else if(FMU_IntVectTbl[uiId].severity_level == FMU_SVL_HIGH)
    {
        // for Double storage test
        FIQ_reg_64[nth_reg] = FMU_REG->mcFiqMask.euNreg[nth_reg];

        // Clear FIQ mask enable for given FMU ID                                               // 0 ~ 31
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqMask.euNreg[nth_reg]  = (FMU_REG->mcFiqMask.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        // for Double storage test
        FIQ_reg_64[nth_reg] = (FIQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FIQ_reg_64[nth_reg], FMU_REG->mcFiqMask.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        // for Double storage test
        FOULT_reg_64[nth_reg] = FMU_REG->mcFaultMask.euNreg[nth_reg];

        // Clear Fault mask enable for given FMU ID                                         // 0 ~ 31
        FMU_REG->mcCfgWrPw                      = FMU_PW;
        FMU_REG->mcFaultMask.euNreg[nth_reg]    = (FMU_REG->mcFaultMask.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        // for Double storage test
        FOULT_reg_64[nth_reg] = (FOULT_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FOULT_reg_64[nth_reg], FMU_REG->mcFaultMask.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }
    }
    else
    {
        ret = FMU_INVALID_SEVERITY_ERR;
    }

    return ret;
 }

/*
***************************************************************************************************
*                                          FMU_IsrClear
*
* @param    uint32 id [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUErrTypes_t FMU_IsrClear
(
    FMUFaultid_t                        uiId
)
{
    uint32          nth_reg;
    uint32          nth_bit;
    uint32          IRQ_reg_64[2];
    uint32          FIQ_reg_64[2];
    uint32          FOULT_reg_64[2];
    FMUErrTypes_t   ret;

    ret = FMU_OK;

    FMU_D("[%s] IN!! id %d \n", __func__, uiId);

    nth_reg = ((uint32)uiId >> (uint32)5) & (uint32)0x1;                                                    // 0 ~ 1
    nth_bit = ((uint32)uiId   ) & (uint32)0x1f;                                                 // 0 ~ 31

    if (FMU_IntVectTbl[uiId].severity_level == FMU_SVL_LOW)
    {
        // for Double storage test
        IRQ_reg_64[nth_reg] = FMU_REG->mcIrqClr.euNreg[nth_reg];

        // Clear IRQ clear for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcIrqClr.euNreg[nth_reg]   = ((uint32)0x1 << nth_bit);

        // for Double storage test
        IRQ_reg_64[nth_reg] = (IRQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(IRQ_reg_64[nth_reg], FMU_REG->mcIrqClr.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        // for Double storage test
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcIrqClr.euNreg[nth_reg]   = 0;
    }
    else if(FMU_IntVectTbl[uiId].severity_level == FMU_SVL_MID)
    {
        // for Double storage test
        FIQ_reg_64[nth_reg] = FMU_REG->mcFiqClr.euNreg[nth_reg];

        // Clear FIQ for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqClr.euNreg[nth_reg]   = ((uint32)0x1 << nth_bit);

        // for Double storage test
        FIQ_reg_64[nth_reg] = (FIQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FIQ_reg_64[nth_reg], FMU_REG->mcFiqClr.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqClr.euNreg[nth_reg]   = 0;
    }
    else if(FMU_IntVectTbl[uiId].severity_level == FMU_SVL_HIGH)
    {
        // for Double storage test
        FIQ_reg_64[nth_reg] = FMU_REG->mcFiqClr.euNreg[nth_reg];

        // Clear FIQ for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqClr.euNreg[nth_reg]   = ((uint32)0x1 << nth_bit);

        // for Double storage test
        FIQ_reg_64[nth_reg] = (FIQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FIQ_reg_64[nth_reg], FMU_REG->mcFiqClr.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqClr.euNreg[nth_reg]   = 0;

        // for Double storage test
        FOULT_reg_64[nth_reg] = FMU_REG->mcFaultClr.euNreg[nth_reg];

        // Clear Fault clear for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFaultClr.euNreg[nth_reg] = ((uint32)0x1 << nth_bit);

        // for Double storage test
        FOULT_reg_64[nth_reg] = (FOULT_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FOULT_reg_64[nth_reg], FMU_REG->mcFaultClr.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFaultClr.euNreg[nth_reg] = 0;

    }
    else
    {
        ret = FMU_INVALID_SEVERITY_ERR;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          DS_Compare
*
* @param    uint32 value [in]
* @param    uint32 reg [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUErrTypes_t FMU_DsCompare
(
    uint32                              uiValue,
    uint32                              uiReg
)
{
    FMUErrTypes_t ret;

    ret = FMU_OK;

    if(uiValue != uiReg)
    {
        ret = FMU_DS_COMPARE_FAIL_ERR;
    }
    else
    {
        FMU_E("[%s]DS compare result sucess\n",__func__);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          FMU_GetIrqStatus
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUSts_t FMU_GetIrqStatus
(
    void
)
{
    uint8       id_cnt;
    uint32      reg0;
    uint32      reg1;
    uint32      id_status;
    FMUSts_t    status;
    uint32      check_out;

    status.id   = (FMUFaultid_t)63; //initial value
    status.cnt  = 0;
    check_out   = 0;
    reg0        = FMU_REG->mcIrqStatus.euNreg[0];
    reg1        = FMU_REG->mcIrqStatus.euNreg[1];

    for(id_cnt = 0 ; id_cnt < (uint8)64 ; id_cnt++)
    {
        id_status = reg0 & (uint32)0x1;

        if (id_status == (uint32)1)
        {
            if (check_out == (uint32)0)
            {
                status.id = (FMUFaultid_t)id_cnt;
                check_out = (uint32)1;
            }
            else
            {
                status.cnt++;
                FMU_D("[%s]status.cnt = %d\n",status.cnt);
            }
        }

        reg0 = (((reg1 & (uint32)0x1) << (uint32)31) | (reg0 >> (uint32)1));
        reg1 = (reg1 >> (uint32)1);
    }

    return status;
}


/*
***************************************************************************************************
*                                          FMU_GetFiqStatus
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUSts_t FMU_GetFiqStatus
(
    void
)
{
    uint8       id_cnt;
    uint32      reg0;
    uint32      reg1;
    uint32      id_status;
    FMUSts_t    status;
    uint32      check_out;

    status.id   = (FMUFaultid_t)63; //initial value
    status.cnt  = 0;
    check_out   = 0;
    reg0        = FMU_REG->mcFiqStatus.euNreg[0];
    reg1        = FMU_REG->mcFiqStatus.euNreg[1];

    for(id_cnt = 0 ; id_cnt < (uint8)64 ; id_cnt++)
    {
        id_status = reg0 & (uint32)0x1;

        if (id_status == (uint32)1)
        {
            if (check_out == (uint32)0)
            {
                status.id = (FMUFaultid_t)id_cnt;
                check_out = (uint32)1;
            }
            else
            {
                status.cnt++;
            }
        }

        reg0 = (((reg1 & (uint32)0x1) << (uint32)31) | (reg0 >> (uint32)1));
        reg1 = (reg1 >> (uint32)1);
    }

    return status;
}


/*
***************************************************************************************************
*                                          FMU_init
*
* @return
*
* Notes
*
***************************************************************************************************
*/

FMUErrTypes_t FMU_Init
(
    void
)
{
    FMUErrTypes_t ret = FMU_OK;

    FMU_SwReset();
    FMU_SetExtFault((uint8)FMU_FAULT_ACTIVE_LOW, 0, 0);

    (void)GIC_IntVectSet(GIC_FMU_IRQ, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&FMU_IrqCallBack, NULL);
    (void)GIC_IntSrcEn(GIC_FMU_IRQ);

    (void)GIC_IntVectSet(GIC_FMU_FIQ, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&FMU_FiqCallBack, NULL);
    (void)GIC_IntSrcEn(GIC_FMU_FIQ);

    return ret;
}

/*
***************************************************************************************************
*                                          FMU_IsrHandler
*
*
* @param    FMUFaultid_t id [in]
* @param    FMUSeverityLevelType_t severity_level [in]
* @param    FMUIntFnctPtr int_fnct [in]
* @param    void * arg [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_IsrHandler
(
    FMUFaultid_t                        uiId,
    FMUSeverityLevelType_t              uiSeverityLevel,
    FMUIntFnctPtr                       fnIntFnct,
    void *                              pArg
)
{
    FMUErrTypes_t ret;

    ret = FMU_OK;

    if(uiId > FMU_ID_FMU_CFG_SOFT)
    {
        ret =  FMU_INVALID_ID_ERR;
    }
    else if ((uiId > FMU_ID_ROM_CRC) && (uiId < FMU_ID_FMU_CFG_SOFT))
    {
        ret =  FMU_RESERVE_ID_ERR;
    }
    else if (uiSeverityLevel > FMU_SVL_HIGH)
    {
        ret =  FMU_INVALID_SEVERITY_ERR;
    }
    else if (fnIntFnct == ((void *)0))
    {
        ret =  FMU_ISR_HANDLER_ERR;
    }
    else
    {
        FMU_IntVectTbl[uiId].severity_level = uiSeverityLevel;
        FMU_IntVectTbl[uiId].func           = fnIntFnct;
        FMU_IntVectTbl[uiId].arg            = pArg;
    }

    return ret;
}



/*
***************************************************************************************************
*                                          FMU_Set
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Set
(
    FMUFaultid_t                        uiId
)
{
    uint32          nth_reg;
    uint32          nth_bit;
    uint32          ds_reg_128[4];
    uint32          ds_reg_64[2];
    FMUErrTypes_t   ret;
    //uint8   ds_ret;//unused variable

    ret = FMU_OK;

    if(uiId > FMU_ID_FMU_CFG_SOFT)
    {
        ret = FMU_INVALID_ID_ERR;
    }
    else if ((uiId > FMU_ID_ROM_CRC) && (uiId < FMU_ID_FMU_CFG_SOFT))
    {
        ret =  FMU_RESERVE_ID_ERR;
    }
    else
    {
        // Set FMU severity level for given FMU ID
        nth_reg = ((uint32)uiId >> (uint32)4) & (uint32)0x3;                                              // 0 ~ 3
        nth_bit = (((uint32)(uiId) & (uint32)0xf) << 1);                                         // (0 ~ 15) x 2

        // for double storage
        ds_reg_128[nth_reg] = FMU_REG->mcSeverityLevel.slNreg[nth_reg];

        FMU_REG->mcCfgWrPw                       = FMU_PW;
        FMU_REG->mcSeverityLevel.slNreg[nth_reg] = ((FMU_REG->mcSeverityLevel.slNreg[nth_reg] & ((uint32)0xFFFFFFFFU - ((uint32)0x3 << nth_bit))) | ((uint32)(FMU_IntVectTbl[uiId].severity_level) << nth_bit));

        ds_reg_128[nth_reg] = ((ds_reg_128[nth_reg] & (0xFFFFFFFFU - ((uint32)0x3 << nth_bit))) | ((uint32)(FMU_IntVectTbl[uiId].severity_level) << nth_bit));

        if(FMU_OK != FMU_DsCompare(ds_reg_128[nth_reg], FMU_REG->mcSeverityLevel.slNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        ret = FMU_IsrMaskClear(uiId);

        // Set FMU enable for given FMU ID
        nth_reg = ((uint32)uiId >> (uint32)5) & (uint32)0x1;                                                    // 0 ~ 1
        nth_bit = ((uint32)uiId   ) & (uint32)0x1f;                                                 // 0 ~ 31

        //for double storage
        ds_reg_64[nth_reg] = FMU_REG->mcEn.euNreg[nth_reg] ;

        FMU_REG->mcCfgWrPw              = FMU_PW;
        FMU_REG->mcEn.euNreg[nth_reg]   = (FMU_REG->mcEn.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        ds_reg_64[nth_reg] = (ds_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(ds_reg_64[nth_reg], FMU_REG->mcEn.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          FMU_IsrClr
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_IsrClr
(
    FMUFaultid_t                        uiId
)
{
    FMUErrTypes_t   ret;

    ret = FMU_OK;

    if(uiId > FMU_ID_FMU_CFG_SOFT)
    {
        ret = FMU_INVALID_ID_ERR;
    }
    else if ((uiId > FMU_ID_ROM_CRC) && (uiId < FMU_ID_FMU_CFG_SOFT))
    {
        ret = FMU_RESERVE_ID_ERR;
    }
    else
    {
        ret = FMU_IsrClear(uiId);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          FMU_Deinit
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Deinit
(
    void
)
{
    FMUErrTypes_t ret;

    ret = FMU_OK;

    (void)GIC_IntSrcDis(GIC_FMU_IRQ);
    (void)GIC_IntSrcDis(GIC_FMU_FIQ);

    return ret;
}

#endif

