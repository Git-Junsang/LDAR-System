/*
***************************************************************************************************
*
*   FileName : dse_test.c
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

#include <dse_test.h>
#include <bsp.h>
#include <dse.h>
#include <reg_phys.h>
#include <fmu.h>
#include <debug.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define DSETEST_SM_PW                   (0x5AFEACE5UL)
#define DSETEST_SM_SF_CTRL_PW           (0x1B9211C0UL)
#define DSETEST_SM_SF_CTRL_CFG          (0x1B9211F0UL)
#define DSETEST_SM_SF_CTRL_STS          (0x1B9211F4UL)

#define DSETEST_DSE_IRQ_MASK            (0x1B938004UL)
#define DSETEST_DSE_IRQ_EN              (0x1B938008UL)
#define DSETEST_DEF_SLV_CFG             (0x1B93800CUL)



/***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/



static uint32                           gWaitIrq = 0UL;
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void DSE_TEST_Fmu_IRQ_ISR
(
    void *                              pArg
);


/*
***************************************************************************************************
*                                       DSE_Test_R5AccessFault
*
* Default Slave Error Sample test code
*
* @param    none
*
* @return   none
*
* Notes     Access the inaccessible address and check the dse error interrupt.
*
***************************************************************************************************
*/

void DSE_Test_R5AccessFault(void)
{
    uint32       uiI;
    uint32       uiK;
    uint32       uiTargetAddr;
    uint32       uiArraynum;
	
    const uint32 uiDelaycnt   = (5000000UL);

    const uint32 micom_accessible[156] =
    {
        0x1B000000UL, 0x200UL,                                  /* CAN-0              */
        0x1B010000UL, 0x200UL,                                  /* CAN-1              */
        0x1B020000UL, 0x200UL,                                  /* CAN-2              */
        0x1B030000UL, 0x40UL,                                   /* CAN-cfg            */

        0x1B100000UL, 0x100UL,                                  /* GPSB-0             */
        0x1B110000UL, 0x100UL,                                  /* GPSB-1             */
        0x1B120000UL, 0x100UL,                                  /* GPSB-2             */
        0x1B130000UL, 0x100UL,                                  /* GPSB-3             */
        0x1B140000UL, 0x100UL,                                  /* GPSB-4             */
        0x1B150000UL, 0x100UL,                                  /* GPSB-5             */
        0x1B160000UL, 0x20UL,                                   /* GPSB-cfg           */
        0x1B170000UL, 0x20UL,                                   /* GPSB-IO Monitor    */

        0x1B200000UL, 0x1000UL,                                 /* UART-0             */
        0x1B210000UL, 0x1000UL,                                 /* UART-1             */
        0x1B220000UL, 0x1000UL,                                 /* UART-2             */
        0x1B230000UL, 0x1000UL,                                 /* UART-3             */
        0x1B240000UL, 0x1000UL,                                 /* UART-4             */
        0x1B250000UL, 0x1000UL,                                 /* UART-5             */
        0x1B260000UL, 0x1000UL,                                 /* UART-dma0          */
        0x1B270000UL, 0x1000UL,                                 /* UART-dma1          */
        0x1B280000UL, 0x1000UL,                                 /* UART-dma2          */
        0x1B290000UL, 0x1000UL,                                 /* UART-dma3          */
        0x1B2A0000UL, 0x1000UL,                                 /* UART-dma4          */
        0x1B2B0000UL, 0x1000UL,                                 /* UART-dma5          */
        0x1B2C0000UL, 0x20UL,                                   /* UART-cfg           */

        0x1B300000UL, 0x40UL,                                   /* I2C-m0             */
        0x1B310000UL, 0x40UL,                                   /* I2C-m1             */
        0x1B320000UL, 0x40UL,                                   /* I2C-m2             */
        0x1B330000UL, 0x10UL,                                   /* I2C-cfg            */
        0x1B340000UL, 0x40UL,                                   /* I2C-s0             */
        0x1B350000UL, 0x40UL,                                   /* I2C-s1             */
        0x1B360000UL, 0x40UL,                                   /* I2C-s2             */

        0x1B400000UL, 0x100UL,                                  /* PDM-0              */
        0x1B410000UL, 0x100UL,                                  /* PDM-1              */
        0x1B420000UL, 0x100UL,                                  /* PDM-2              */
        0x1B430000UL, 0x40UL,                                   /* PDM-cfg            */
        0x1B440000UL, 0x100UL,                                  /* PMM-0              */
        0x1B450000UL, 0x100UL,                                  /* PMM-1              */
        0x1B460000UL, 0x100UL,                                  /* PMM-2              */

        0x1B500000UL, 0x100UL,                                  /* ICTC-0             */
        0x1B510000UL, 0x100UL,                                  /* ICTC-1             */
        0x1B520000UL, 0x100UL,                                  /* ICTC-2             */
        0x1B530000UL, 0x100UL,                                  /* ICTC-r0            */
        0x1B540000UL, 0x100UL,                                  /* ICTC-r1            */
        0x1B550000UL, 0x100UL,                                  /* ICTC-r2            */

        0x1B600000UL, 0x1000UL,                                 /* GDMA               */

        0x1B700000UL, 0x100UL,                                  /* ADC                */
        0x1B710000UL, 0x100UL,                                  /* AMM                */

        0x1B800000UL, 0x80UL,                                   /* S  MBOX-0          */
        0x1B810000UL, 0x80UL,                                   /* NS MBOX-0          */
        0x1B820000UL, 0x80UL,                                   /* S  MBOX-1          */
        0x1B830000UL, 0x80UL,                                   /* NS MBOX-2 slave    */
        0x1B840000UL, 0x80UL,                                   /* MBOX-2             */

        0x1B900000UL, 0x10000UL,                                /* Interrupt          */
        0x1B910000UL, 0x10000UL,                                /* SFMC               */
        0x1B920000UL, 0x400UL,                                  /* CMU                */
        0x1B921000UL, 0x200UL,                                  /* SM control         */

        0x1B930000UL, 0x80UL,                                   /* Micom sub cfg      */
        0x1B931000UL, 0x100UL,                                  /* CCU                */
        0x1B932000UL, 0x80UL,                                   /* R5 cfg             */
        0x1B933000UL, 0x20UL,                                   /* Timer-0            */
        0x1B933100UL, 0x20UL,                                   /* Timer-1            */
        0x1B933200UL, 0x20UL,                                   /* Timer-2            */
        0x1B933300UL, 0x20UL,                                   /* Timer-3            */
        0x1B933400UL, 0x20UL,                                   /* Timer-4            */
        0x1B933500UL, 0x20UL,                                   /* Timer-5            */

        0x1B934000UL, 0x20UL,                                   /* Windowed WDT       */
        0x1B935000UL, 0x1000UL,                                 /* GPIO               */
        0x1B936000UL, 0x400UL,                                  /* PMU                */
        0x1B936400UL, 0x400UL,                                  /* FMU                */
        0x1B937000UL, 0x1000UL,                                 /* PMIO               */
        0x1B938000UL, 0x80UL,                                   /* Default slave      */
        0x1B939000UL, 0x40UL,                                   /* MID cfg            */
        0x1B93A000UL, 0x1000UL,                                 /* SFMC MID filter    */
        0x1B93B000UL, 0x1000UL,                                 /* IMC MID filter     */
        0x1B93C000UL, 0x1000UL,                                 /* SGID filter        */
        0x1B93D000UL, 0x1000UL,                                 /* NSAID filter       */
        0x1B940000UL, 0x4000UL,                                 /* LVDS               */
    };
                                                                /* Default Slave addr */
    const uint32 micom_non_accessible_dse_addr[18] =
    {
        0x1B040000UL,                                           /* CAN                */
        0x1B180000UL,                                           /* GPSB               */
        0x1B2D0000UL,                                           /* UART               */
        0x1B370000UL,
        0x1B470000UL,
        0x1B560000UL,
        0x1B610000UL,
        0x1B720000UL,
        0x1B850000UL,
        0x1B922000UL,
        0x1B933600UL,
        0x1B936800UL,
        0x1B93E000UL,
        0x1B944400UL,
        0x1B950000UL,
        0x1BA50000UL,
        0x1BE00000UL,
        0x1BFF8000UL
    };

    uiI          = 0UL;
    uiK          = 0UL;
    uiTargetAddr = 0UL;
    uiArraynum = sizeof(micom_non_accessible_dse_addr)/sizeof(micom_non_accessible_dse_addr[0]);
	
    DSE_Deinit();
    (void)DSE_Init(DES_SEL_ALL);

    mcu_printf("=========== DSE_Test_R5AccessFault ===========\n");
    mcu_printf("=========== Default slave address access ===========\n");
	
    for( ; uiI < uiArraynum; uiI++)
    {
        uiTargetAddr = micom_non_accessible_dse_addr[uiI];

        if(uiTargetAddr != 0UL)
       	{
            mcu_printf("Try to access @0x%x\n", uiTargetAddr);
            gWait_irq = 1UL;
            SAL_ReadReg(uiTargetAddr);
        }

        for ( ; uiK < uiDelaycnt ; uiK++)
        {
            if(gWait_irq == 0UL)
            {
                break;
            }
            BSP_NOP_DELAY();
        }
		
        if(uiK == uiDelaycnt)
        {
            mcu_printf("Interrupt not occured\n");
        }

    }

    mcu_printf("\n\n=========== not allowed address access ===========\n");

    uiArraynum = sizeof(micom_accessible)/sizeof(micom_accessible[0]);
	
    for(uiI = 0; uiI < uiArraynum; uiI = (uiI+2u))
    {
        uiTargetAddr = micom_accessible[uiI] + micom_accessible[(uiI + 1u)];
		
        if( ((uiI + 2u) < uiArraynum) && (uiTargetAddr == micom_accessible[(uiI + 2u)]) )
        {
            continue;
        }
		
        if(uiTargetAddr != 0UL)
       	{
            mcu_printf("Try to access @0x%x\n", uiTargetAddr);
            gWait_irq = 1UL;
            SAL_ReadReg(uiTargetAddr);
       	}

        for (uiK = 0 ; (uiK < uiDelaycnt) ; uiK++)
        {
            if(gWait_irq == 0UL)
            {
                break;
            }      
			
            BSP_NOP_DELAY();
        }
		
        if(uiK == uiDelaycnt)
        {
            mcu_printf("Interrupt not occured\n");
        }

    }

    mcu_printf("=========== Done ===========\n");
    DSE_Deinit();
    return;
}


/*
***************************************************************************************************
*                                       DSE_TEST_Fmu_IRQ_ISR
*
* DSE Soft fault check , fmu callback isr function
*
* @param    pArg : uiFmuArg at the DSE_Test_SoftFaultCheck
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

static void DSE_TEST_Fmu_IRQ_ISR(void *pArg)
{
    uint32 uiRdata;
    uint32 uiReadSts;
    uint32 *uiTargetAddr;
    uint32 *uiTargetValue;

    uiRdata          = 0UL;
    uiReadSts        = 0UL;
    uiTargetAddr     = (uint32 *)0;
    uiTargetValue    = (uint32 *)0;
		
    if((pArg != NULL_PTR) && (*(uint32*)(pArg + 4) != (uint32)NULL))
    {
        uiTargetValue = (uint32*)pArg;
        uiTargetAddr  = (uint32*)(pArg + 4);   

        uiReadSts = SAL_ReadReg(DSETEST_SM_SF_CTRL_STS);
        mcu_printf("FMU interrupt ... SF_CTRL_STS x%x\n", uiReadSts);

        if(uiReadSts == 0x8UL)
        {
            uiRdata = DSE_GetGrpSts();

            if((uiRdata & DSE_SM_IRQ_MASK) == DSE_SM_IRQ_MASK)
            {
                mcu_printf("DSE_IRQ_MASK error\n");
            }
			
            if((uiRdata & DSE_SM_IRQ_EN) == DSE_SM_IRQ_EN) 
            {
                mcu_printf("DSE_SM_IRQ_EN error\n");
            }
			
            if((uiRdata & DSE_SM_SLV_CFG) == DSE_SM_SLV_CFG)
            {
                mcu_printf("DSE_SM_SLV_CFG error\n");
            }
			
            if((uiRdata & DSE_SM_CFG_WR_PW) == DSE_SM_CFG_WR_PW)
            {
                mcu_printf("DSE_SM_CFG_WR_PW error\n");
            }
			
            if((uiRdata & DSE_SM_CFG_WR_LOCK) == DSE_SM_CFG_WR_LOCK)
            {
                mcu_printf("DSE_SM_CFG_WR_LOCK error\n");
            }
			
        }

        SAL_WriteReg(DSETEST_SM_PW, DSETEST_SM_SF_CTRL_PW);
        uiRdata = SAL_ReadReg(DSETEST_SM_SF_CTRL_PW);
		
        if(uiRdata == 1UL)
        {
            mcu_printf("restore previous reg x%x, val x%x.. \n", *uiTargetAddr, *uiTargetValue);
            SAL_WriteReg(*uiTargetValue, *uiTargetAddr);    
            SAL_WriteReg(uiReadSts, DSETEST_SM_SF_CTRL_STS);
            uiReadSts = SAL_ReadReg(DSETEST_SM_SF_CTRL_STS);
			
            if(uiReadSts != 0UL)
            {
                mcu_printf("Fail.. DSETEST_SM_SF_CTRL_STS not 0..maybe..exist another Fault\n");
            }
			
        }
        else
        {
            mcu_printf("Fail.. SM_SF_CTRL_PW set\n");
        }

        (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_SYS_SM_CFG);
        gWaitIrq = 0UL;
    }

    return;
}




/*
***************************************************************************************************
*                                       DSE_Test_SoftFaultCheck
*
* DSE Soft fault check test code
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void DSE_Test_SoftFaultCheck(void)
{
#if defined(FMU_DRIVER)

    uint32 uiRdata;
    uint32 uiErr;
    uint32 uiFmuArg[2];
    uint32 uiI;

    uiRdata     = 0UL;
    uiErr       = 0UL;    
    uiI         = 0UL;
    uiFmuArg[0] = 0UL;
    uiFmuArg[1] = 0UL;	

    mcu_printf("\n Fault Injection Test \n");
    (void)DSE_Init(DES_SEL_ALL);
    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_SYS_SM_CFG, (FMUSeverityLevelType_t)FMU_SVL_LOW, (FMUIntFnctPtr)&DSE_TEST_Fmu_IRQ_ISR, (void *)uiFmuArg);
    (void)FMU_Set((FMUFaultid_t)FMU_ID_SYS_SM_CFG);
    (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_SYS_SM_CFG);

    SAL_WriteReg(DSETEST_SM_PW, DSETEST_SM_SF_CTRL_PW);
    uiRdata = SAL_ReadReg(DSETEST_SM_SF_CTRL_PW);
	
    if(uiRdata != 1UL)
    {
        mcu_printf("  Fail.. DSETEST_SM_SF_CTRL_PW set\n");
        uiErr |= (0x10UL);
    }
    else
    {
        uiRdata = SAL_ReadReg(DSETEST_SM_SF_CTRL_CFG);
        SAL_WriteReg(uiRdata | 0x88UL, DSETEST_SM_SF_CTRL_CFG);
        mcu_printf("  SF_CTRL_CFG set\n");
    }

    if(uiErr == 0UL)
    {
    
        if(DSE_WriteLock(0UL) == SAL_RET_SUCCESS)
        {
            mcu_printf("  Fault injection>> DSE_IRQ_MASK\n");
            uiFmuArg[0] = SAL_ReadReg(DSETEST_DSE_IRQ_MASK);
            uiFmuArg[1] = DSETEST_DSE_IRQ_MASK;
            SAL_WriteReg( ~uiFmuArg[0], DSETEST_DSE_IRQ_MASK);

            mcu_printf("  Fault injection>> DSE_IRQ_EN\n");
            uiFmuArg[0] = SAL_ReadReg(DSETEST_DSE_IRQ_EN);
            uiFmuArg[1] = DSETEST_DSE_IRQ_EN;
            SAL_WriteReg( ~uiFmuArg[0], DSETEST_DSE_IRQ_EN);

            mcu_printf("  Fault injection>> DEF_SLV_CFG\n");
            uiFmuArg[0] = SAL_ReadReg(DSETEST_DEF_SLV_CFG);
            uiFmuArg[1] = DSETEST_DEF_SLV_CFG;
            SAL_WriteReg( ~uiFmuArg[0], DSETEST_DEF_SLV_CFG);

            gWaitIrq = 1UL;
        }

        for( ; uiI < 50000UL ; uiI++)
        {
            if(gWaitIrq == 0UL)
            {
                break;
            }
        }
		
        SAL_WriteReg(DSETEST_SM_PW, DSETEST_SM_SF_CTRL_PW);
        uiRdata = SAL_ReadReg(DSETEST_SM_SF_CTRL_PW);
		
        if(uiRdata != 1UL)
        {
            mcu_printf("  Fail.. DSETEST_SM_SF_CTRL_PW set\n");
            uiErr |= (0x40UL);
        }
        else
        {
            uiRdata = SAL_ReadReg(DSETEST_SM_SF_CTRL_CFG);
            SAL_WriteReg(uiRdata & (~0x88UL), DSETEST_SM_SF_CTRL_CFG);
        }

    }

    mcu_printf("  Fault Injection Test Done uiErr x%x\n",uiErr);

    (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_SYS_SM_CFG, (FMUSeverityLevelType_t)FMU_SVL_LOW, NULL, NULL );
    DSE_Deinit();
	
#else
    mcu_printf("FMU DRIVER not included...set define FMU_DRIVER\n");
#endif

    return;
}

