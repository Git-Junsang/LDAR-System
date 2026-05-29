/*
***************************************************************************************************
*
*   FileName : fmu_test.c
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

#include <fmu_test.h>
#include <debug.h>
#include <fmu.h>
#include <bsp.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define FMU_WriteReg(a,v)               (SAL_WriteReg(v,a))
#define FMU_ReadReg(a)                  (SAL_ReadReg(a))

#define FMU_BASE_ADDR                   (0x1B936400UL)
#define FMU_REG                         ((volatile FMUMct_t          *)(FMU_BASE_ADDR))

#define FMU_PW                          (0x5afeace5UL)
#define FMU_PW_ERROR                    (0x5afe8031UL)

#define FMU_BASE_ADDR                   (0x1B936400UL)

#define FMU_FAULT_ACTIVE_LOW            (0)
#define FMU_FAULT_ACTIVE_HIGH           (1)


/* FMU_CTRL */
typedef struct FMUCtrlt
{
    uint32                              ctForceFaultt       :  1;   //  [   00], RW
    uint32                              ctSwResett          :  1;   //  [   01], RW
    uint32                              ctSoftFaultTestEnt  :  1;   //  [   02], RW
    uint32                              ctSoftFaultEnt      :  1;   //  [   03], RW
    uint32                              ctPeriodicFaultEnt  :  1;   //  [   04], RW
    uint32                                                  : 27;   //  [31:05]
} FMUCtrlt_t;

typedef union FMUCtrlUniont
{
    uint32                              cuNregt;
    FMUCtrlt_t                          cuBregt;
} FMUCtrlUniont_t;

/* FMU_EN */
typedef struct FMUEnt
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
} FMUEnt_t;

typedef union FMUEnUniont
{
    uint32                              euNreg[2];
    FMUEnt_t                             euBreg;
} FMUEnUniont_t;

// FMU_EXT_FAULT_CTRL
typedef struct FMUExtFaultCtrlt
{
    uint32                              fcPolarity          :  1;   //  [   00], RW
    uint32                                                  :  3;   //  [03:01], RW
    uint32                              fcLow               :  4;   //  [07:04], RW
    uint32                              fcHigh              :  3;   //  [10:08], RW TODO
    uint32                                                  : 21;   //  [31:11]
} FMUExtFaultCtrlt_t;

typedef union FMUExtFaultCtrlUniont
{
    uint32                              fcNreg;
    FMUExtFaultCtrlt_t                   fcBreg;
} FMUExtFaultCtrlUniont_t;

// FMU_SEVERITY_LEVEL
typedef struct FMUSeverityLevelt
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
} FMUSeverityLevelt_t;

typedef union FMUSeverityLevelUniont
{
    uint32                              slNreg[4];
    FMUSeverityLevelt_t                 slBreg;
} FMUSeverityLevelUniont_t;

// FMU_CLK_SEL
typedef struct FMUClkSelt
{
    uint32                              csClkSel            :  1;   //  [   00], RW
    uint32                                                  : 31;   //  [31:01]
} FMUClkSelt_t;

typedef union FMUClkSelUniont
{
    uint32                              csNreg;
    FMUClkSelt_t                        csBreg;
} FMUClkSelUniont_t;

// FMU_OUT_STATUS
typedef struct FMUOutStatust
{
    uint32                              osIrq               :  1;   //  [   00], RO
    uint32                              osFiq               :  1;   //  [   01], RO
    uint32                              osFault             :  1;   //  [   02], RO
    uint32                                                  : 13;   //  [15:03]
    uint32                              osFaultState        :  2;   //  [17:16], RO
    uint32                                                  : 14;   //  [31:18]
} FMUOutStatust_t;

typedef union FMUOutStatusUniont
{
    uint32                              suNreg;
    FMUOutStatust_t                     suBreg;
} FMUOutStatusUniont_t;

typedef struct FMUMct
{
    FMUCtrlUniont_t                     mcCtrl;             // 0x00
    FMUEnUniont_t                       mcEn;               // 0x04 ~ 0x08
    FMUExtFaultCtrlUniont_t             mcExtFaultCtrl; // 0x0c
    FMUSeverityLevelUniont_t            mcSeverityLevel;    // 0x10 ~ 0x1c
    FMUEnUniont_t                       mcIrqMask;          // 0x20 ~ 0x24
    FMUEnUniont_t                       mcIrqClr;           // 0x28 ~ 0x2c
    FMUEnUniont_t                       mcFiqMask;          // 0x30 ~ 0x34
    FMUEnUniont_t                       mcFiqClr;           // 0x38 ~ 0x3c
    FMUEnUniont_t                       mcFaultMask;        // 0x40 ~ 0x44
    FMUEnUniont_t                       mcFaultClr;         // 0x48 ~ 0x4c
    FMUEnUniont_t                       mcIrqStatus;        // 0x50 ~ 0x54
    FMUEnUniont_t                       mcFiqStatus;        // 0x58 ~ 0x5c
    FMUEnUniont_t                       mcFaultStatus;      // 0x60 ~ 0x64
    FMUClkSelUniont_t                   mcClkSel;           // 0x68
    FMUOutStatusUniont_t                mcOutStatus;        // 0x6c
    uint32                              mcCfgWrPw;          // 0x70
} FMUMct_t;


 // FMU_EN
typedef struct FMUStst
{
     FMUFaultid_t                       id;
     uint32                             cnt;
} FMUStst_t;



static uint32 fault_id                  = 0UL;
static uint32 fmu_irq_done              = 0UL;



/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/


static FMUStst_t get_fmu_irq_status (void) {

    uint8   id_cnt;
    uint32  reg0;
    uint32  reg1;
    uint32  id_status;
    FMUStst_t status;
    uint32  check_out;

    status.id   = (FMUFaultid_t)63;
    status.cnt  = 0;
    check_out   = 0;
    reg0        = FMU_REG->mcIrqStatus.euNreg[0];
    reg1        = FMU_REG->mcIrqStatus.euNreg[1];

    for(id_cnt=0; id_cnt<(uint32)64; id_cnt++) {
        id_status = reg0 & (uint32)0x1;

        if (id_status == 1UL) {
            if (check_out == 0UL) {
                status.id = (FMUFaultid_t)id_cnt;
                check_out = 1;
            } else {
                status.cnt++;
                mcu_printf("[%s]status.cnt = %d\n",status.cnt);
            }
        }

        reg0 = (((reg1 & (uint32)0x1) << (uint32)31) | (reg0 >> (uint32)1));
        reg1 = (reg1 >> (uint32)1);
    }

    return status;
}

static void fmu_soft_fault_test_en (uint32 value) {
    FMU_REG->mcCfgWrPw                  = FMU_PW;
    FMU_WriteReg(0x1B936470, 0x5afeace5);
    FMU_REG->mcCtrl.cuBregt.ctSoftFaultTestEnt = value;

#ifdef FMU_DEBUG
    if (value == 0) {
        mcu_printf("FMU soft fault test enabled\n");
    }
    else if (value = 1) {
        mcu_printf("FMU soft fault test disabled\n");
    }
    else {
        mcu_printf("Soft fault test enable setting value is invalid\n");

    }
#endif
}


static void fmu_soft_fault_irq_isr (void * pArg) {
     // FMU IRQ status clear
     FMUStst_t status;
     uint32 read_reg;

    (void)pArg;
     status = get_fmu_irq_status();

     if (status.id != FMU_ID_FMU_CFG_SOFT) {
         mcu_printf("IRQ for unexpected FMU IRQ ID occured\n");

     } else {
         // Restore original data
         fmu_soft_fault_test_en((uint32)0);
         FMU_REG->mcCfgWrPw = FMU_PW;
         read_reg = FMU_BASE_ADDR + (fault_id<<(uint32)2);
         FMU_WriteReg(read_reg, FMU_ReadReg(read_reg));

         (void)FMU_IsrClr(status.id);
#ifdef DEBUG_EN
         mcu_printf("FMU Fault ID-%d IRQ status cleared\n", status.id);
#endif
     }

#ifdef DEBUG_EN
     mcu_printf("Soft fault check success for FMU_CFG[%d]\n", fault_id);
#endif

     fmu_irq_done = 1;
}


static void fmu_soft_fault_en (uint32 value) {
    FMU_REG->mcCfgWrPw              = FMU_PW;
    FMU_REG->mcCtrl.cuBregt.ctSoftFaultEnt    = value;

#ifdef FMU_DEBUG
    if (value == 0) {
        mcu_printf("FMU soft fault enabled\n");
    }
    else if (value = 1) {
        mcu_printf("FMU soft fault disabled\n");
    }
    else {
        mcu_printf("Soft fault enable setting value is invalid\n");

    }
#endif
}



static void FMU_SoftFaultTest(void)
{

    uint32 rd_addr;
    uint32 wr_addr; 
    uint32 uiK = 0UL;
    uint32 i;
    const uint32  uiDelaycnt = (5000000UL);
    
#ifdef DEBUG_EN
    mcu_printf ("Soft fault test start !!\n");
#endif

    (void)FMU_IsrHandler(FMU_ID_FMU_CFG_SOFT, FMU_SVL_LOW, (FMUIntFnctPtr)&fmu_soft_fault_irq_isr, NULL);


    // FMU SW reset
    //fmu_sw_reset();

    // Set FMU for soft fault
    (void)FMU_Set(FMU_ID_FMU_CFG_SOFT);

    // Set soft fault test enable
    fmu_soft_fault_en((uint32)0);

    for(i=(uint32)1; i<(uint32)27; i++) 
    {
        fault_id = i; 
        if((i < (uint32)20) || (i  > (uint32)25)) {

            fmu_soft_fault_test_en((uint32)0);
            FMU_REG->mcCfgWrPw = FMU_PW;
            FMU_REG->mcCfgWrPw =0x5afeace5;

            fmu_irq_done = 0;

            rd_addr = (FMU_BASE_ADDR + (fault_id<<(uint32)2));
            wr_addr = FMU_BASE_ADDR + (fault_id<<(uint32)2);
            FMU_WriteReg(wr_addr , (FMU_ReadReg(rd_addr) ^ 0x11111111UL));

            for ( ; (uiK < uiDelaycnt) ; uiK++)
            {
                BSP_NOP_DELAY();
                if(fmu_irq_done == 0UL)
                {
                    break;
                }        
                
            }
            //wait_soft_fault_irq();
            
            mcu_printf ("****** reg [%d][0x%08X]******** \n",fault_id,FMU_BASE_ADDR + (fault_id<<2));
        }
    }


    mcu_printf ("Soft fault test end !!\n");
}

static void FMU_PasswordWriteProtection(void)
{
    uint32 i;
    for(i=0; i< (uint32)27; i++) {
        if((i < (uint32)20) || (i  > (uint32)25)) {
            uint32 temp_addr;

            fault_id = i; 
            mcu_printf ("****** reg [%d][0x%08X]******** \n",fault_id,FMU_BASE_ADDR + (fault_id<<(uint32)2));
            temp_addr = FMU_BASE_ADDR + (fault_id<<(uint32)2);
            mcu_printf("original         0x%08X\n", FMU_ReadReg(temp_addr));

            //incorrect FMU_PW
            FMU_REG->mcCfgWrPw = FMU_PW_ERROR;
            FMU_WriteReg((uint32)(FMU_BASE_ADDR + (uint32)(fault_id<<2)) , (FMU_ReadReg(temp_addr) ^ 0x11111111UL));
            mcu_printf("INCURRECT_PASS   0x%08X\n", FMU_ReadReg(temp_addr));

            //correct FMU_PW
            FMU_REG->mcCfgWrPw = FMU_PW;
            FMU_WriteReg((uint32)(FMU_BASE_ADDR + (uint32)(fault_id<<2)) , (FMU_ReadReg(temp_addr) ^ 0x11111111UL));
            mcu_printf("CURRECT_PASS     0x%08X\n\n", FMU_ReadReg(temp_addr));
        }
    }
}


void FMU_StartFmuTest(int32 ucMode)
{

    switch (ucMode)
    {

        case 1 :
        {
            FMU_SoftFaultTest();
            break;
        }

        case 2 :
        {
            FMU_PasswordWriteProtection();
            break;
        }
        default:
        {
            mcu_printf("Invalid Test Mode Value\n");
            break;
        }
    }
}

