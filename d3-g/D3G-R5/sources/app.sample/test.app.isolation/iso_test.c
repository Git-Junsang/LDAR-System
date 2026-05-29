/*
***************************************************************************************************
*
*   FileName : iso_test.c
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

#include <iso_test.h>

#include <gic.h>
#include <reg_phys.h>
#include <debug.h>

#include <ssm.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void BUS_DeviceTest(void);
static void BUS_AXIMasterIsolationTest(void);
static void BUS_AXISlaveIsolationTest(void);
static void BUS_AHBNormalTest(void);
static void BUS_AHBIsolationTest(void);

uint32 isolationRunningTest = 0U;

static void BUS_MWchIsoHandler(uint32 uiId, uint32 uiStatus)
{
    isolationRunningTest = (uiId * 100000000UL) + uiStatus;
    mcu_printf("BUS_MWchIsoHandler ID(%d), Status : 0x%08x\n", uiId, uiStatus);
}

static void BUS_MRchIsoHandler(uint32 uiId, uint32 uiStatus)
{
    isolationRunningTest = (uiId * 100000000UL) + uiStatus;
    mcu_printf("BUS_MRchIsoHandler ID(%d), Status : 0x%08x\n", uiId, uiStatus);
}

static void BUS_SWchIsoHandler(uint32 uiId, uint32 uiStatus)
{
    isolationRunningTest = (uiId * 100000000UL) + uiStatus;
    mcu_printf("BUS_SWchIsoHandler ID(%d), Status : 0x%08x\n", uiId, uiStatus);
}

static void BUS_SRchIsoHandler(uint32 uiId, uint32 uiStatus)
{
    isolationRunningTest = (uiId * 100000000UL) + uiStatus;
    mcu_printf("BUS_SRchIsoHandler ID(%d), Status : 0x%08x\n", uiId, uiStatus);
}

static void BUS_DeviceTest(void)
{
    uint32 index = 0U;
    
    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Master/SlaveR/W Isolation, Step 2\n");
    mcu_printf("06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AHB Slave Isolation, Step 2\n");
    (void)SSM_BusIsolationEnable(50UL, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Master W Isolation, Step 1\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_MST_WCH_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Master W Isolation, Step 3\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_MST_WCH_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Master R Isolation, Step 1\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_MST_RCH_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Master R Isolation, Step 3\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_MST_RCH_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Slave W Isolation, Step 1\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_SLV_WCH_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Slave W Isolation, Step 3\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_SLV_WCH_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Slave R Isolation, Step 1\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_SLV_RCH_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AXI Slave R Isolation, Step 3\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_SLV_RCH_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AHB Slave Isolation, Step 1\n");
    (void)SSM_BusIsolationEnable(BUS_AHB_MST_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling AHB Slave Isolation, Step 3\n");
    (void)SSM_BusIsolationEnable(BUS_AHB_MST_ISOLATION, 0U);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Handling Fault Interrupt, Step 1\n");
    (void)SSM_BusRegisterFaultHandler(BUS_AXI_MST_WCH_ISOLATION, 0U, (BUS_FAULT_HANDLER)&BUS_MWchIsoHandler);    // -1

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Handling Fault Interrupt, Step 3\n");
    (void)SSM_BusRegisterFaultHandler(BUS_AXI_MST_WCH_ISOLATION, 100U, (BUS_FAULT_HANDLER)&BUS_MWchIsoHandler);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Handling Fault Interrupt, Step 2\n");
    for (index = 0U; index < 10U; index++)
    {
        (void)SSM_BusRegisterFaultHandler(BUS_AXI_MST_WCH_ISOLATION, 0U, (BUS_FAULT_HANDLER)&BUS_MWchIsoHandler);
    }

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Master/SlaveR/W Isolation, Step 2\n");
    mcu_printf("06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AHB Slave Isolation, Step 2\n");
    (void)SSM_BusIsolationDisable(50UL);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Master W Isolation, Step 1\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_MST_WCH_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Master W Isolation, Step 3\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_MST_WCH_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Master R Isolation, Step 1\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_MST_RCH_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Master R Isolation, Step 3\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_MST_RCH_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Slave W Isolation, Step 1\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_SLV_WCH_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Slave W Isolation, Step 3\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_SLV_WCH_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Slave R Isolation, Step 1\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_SLV_RCH_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AXI Slave R Isolation, Step 3\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_SLV_RCH_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AHB Slave Isolation, Step 1\n");
    (void)SSM_BusIsolationDisable(BUS_AHB_MST_ISOLATION);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling AHB Slave Isolation, Step 3\n");
    (void)SSM_BusIsolationDisable(BUS_AHB_MST_ISOLATION);
    

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling Slave Response Timer, Step 1\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_SLV_RES_TIMER, 1000UL);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Enabling Slave Response Timer, Step 3\n");
    (void)SSM_BusIsolationEnable(BUS_AXI_SLV_RES_TIMER, 1000UL);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Setting Slave Response Time, Step 3\n");
    (void)SSM_BusSetAxiSlvResTime(100000001UL);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Setting Slave Response Time, Step 1\n");
    (void)SSM_BusSetAxiSlvResTime(100000000UL);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling Slave Response Timer, Step 1\n");
    (void)SSM_BusIsolationDisable(BUS_AXI_SLV_RES_TIMER);

    mcu_printf("\n06. BUS Isolation Configuration (with Memory Sub System) And Verification, Disabling Slave Response Timer, Step 3 \n");
    (void)SSM_BusIsolationDisable(BUS_AXI_SLV_RES_TIMER);

}

static void BUS_AXIMasterIsolationTest(void)
{
    mcu_printf("\n[IP-PV] MC-06, Bus Isolation with Memory Sub-system, X2X Master FMU operation \n");

#if (BUS_FAULT_INJ_FMU == 0x1U)
    mcu_printf("[SM-PV] SM-MC-X2X_MST.01, Incoming AXI Isolation, X2X MST debug mode read FMU operation\n");
#else
    mcu_printf("[SM-PV] SM-MC-X2X_MST.01, Incoming AXI Isolation, X2X MST read/write FMU operation\n");
#endif

    (void)SSM_BusIsolationEnable(BUS_AXI_MST_WCH_ISOLATION, 0U);
    (void)SSM_BusIsolationEnable(BUS_AXI_MST_RCH_ISOLATION, 0U);

#if (BUS_FAULT_INJ_FMU == 0x1U)
    (void)SSM_BusRegisterFaultHandler(BUS_AXI_MST_WCH_ISOLATION, 0U, (BUS_FAULT_HANDLER)&BUS_MWchIsoHandler);
    (void)SSM_BusRegisterFaultHandler(BUS_AXI_MST_RCH_ISOLATION, 0U, (BUS_FAULT_HANDLER)&BUS_MRchIsoHandler);
#endif
}

static void BUS_AXISlaveIsolationTest(void)
{
    mcu_printf("\n[IP-PV] MC-06, Bus Isolation with Memory Sub-system, X2X Slave FMU operation \n");

#if (BUS_FAULT_INJ_FMU == 0x1U)
    mcu_printf("[SM-PV] SM-MC-X2X_SLV.01, Outgoing AXI Isolation, X2X SLV debug mode read FMU operation\n");
#else
    mcu_printf("[SM-PV] SM-MC-X2X_MST.01, Incoming AXI Isolation, X2X SLV read/write FMU operation\n");
#endif

    (void)SSM_BusIsolationEnable(BUS_AXI_SLV_WCH_ISOLATION, 0U);
    (void)SSM_BusIsolationEnable(BUS_AXI_SLV_RCH_ISOLATION, 0U);

#if (BUS_FAULT_INJ_FMU == 0x1U)
    (void)SSM_BusRegisterFaultHandler(BUS_AXI_SLV_WCH_ISOLATION, 0U, (BUS_FAULT_HANDLER)&BUS_SWchIsoHandler);
    (void)SSM_BusRegisterFaultHandler(BUS_AXI_SLV_RCH_ISOLATION, 0U, (BUS_FAULT_HANDLER)&BUS_SRchIsoHandler);
#endif
}

static void DSEIH_NoramlHandler(void * pArg)
{ 
    (void)pArg;
    
    mcu_printf("\n[SM-PV] SM-MC-HBIC.01, Incoming AHB Isolation, AHB Normal Operation\n");

    if ((SAL_ReadReg(0x1B938000U) & 0x100U) > 0U)
        mcu_printf("This handler is invoked out of AHB isolator's default slave. ==> FAIL");
    else
        mcu_printf("This handler is invoked out of Periphral's default slave. ==> PASS");

    SAL_WriteReg(SAL_ReadReg(0x1B938000U), 0x1B938000U);
}

static void DSEIH_ProtectionHandler(void * pArg)
{
    (void)pArg;
    
    mcu_printf("\n[SM-PV] SM-MC-HBIC.01, Incoming AHB Isolation, AHB Protection Operation\n");

    if ((SAL_ReadReg(0x1B938000U) & 0x100U) > 0U)
        mcu_printf("This handler is invoked out of AHB isolator's default slave. ==> PASS");
    else
        mcu_printf("This handler is invoked out of Periphral's default slave. ==> FAIL");

    SAL_WriteReg(SAL_ReadReg(0x1B938000U), 0x1B938000U);
}

static void BUS_AHBNormalTest(void)
{
    mcu_printf("\n[IP-PV] MC-06, Bus Isolation with Memory Sub-system, Incoming AHB transaction Isolation/Monitoring\n");
    mcu_printf("[SM-PV] SM-MC-HBIC.01, Incoming AHB Isolation, AHB Normal Operation\n");
    
    SSM_BusIsolationDisable(BUS_AHB_MST_ISOLATION);

    (void)GIC_IntVectSet(GIC_DEFAULT_SLV_ERR, 
                         GIC_PRIORITY_NO_MEAN, 
                         GIC_INT_TYPE_LEVEL_HIGH, 
                         (GICIsrFunc)&DSEIH_NoramlHandler, 
                         NULL_PTR);

    (void)GIC_IntSrcEn(GIC_DEFAULT_SLV_ERR);

    SAL_WriteReg(0x0U, 0x1B938004U);
    SAL_WriteReg(0x1U, 0x1B938008U);
    SAL_WriteReg(0x00FF0000U, 0x1B93800CU);
}

static void BUS_AHBIsolationTest(void)
{
    mcu_printf("\n[IP-PV] MC-06, Bus Isolation with Memory Sub-system, Incoming AHB transaction Isolation/Monitoring\n");
    mcu_printf("[SM-PV] SM-MC-HBIC.01, Incoming AHB Isolation, AHB Protection Operation\n");

    SAL_WriteReg(0x0U, 0x1B938004U);
    SAL_WriteReg(0x1U, 0x1B938008U);
    SAL_WriteReg(0x00FF0000U, 0x1B93800CU);
    
    (void)GIC_IntVectSet(GIC_DEFAULT_SLV_ERR, 
                         GIC_PRIORITY_NO_MEAN, 
                         GIC_INT_TYPE_LEVEL_HIGH, 
                         (GICIsrFunc)&DSEIH_ProtectionHandler, 
                         NULL_PTR);

    (void)GIC_IntSrcEn(GIC_DEFAULT_SLV_ERR);

    SSM_BusIsolationDisable(BUS_AHB_MST_ISOLATION);
    SSM_BusIsolationEnable(BUS_AHB_MST_ISOLATION, 0);
}

/*
***************************************************************************************************
*                                          ISO_StartTest
*
* This function is to verify the operation of WDT
*
* @param    ucMode [in] the verification of Watchdog driver development or Watchdog operation
*
* Notes
*
***************************************************************************************************
*/
void ISO_StartTest(int32 ucMode)
{
    switch (ucMode)
    {
        /* 06. BUS Isolation Configuration (with Memory Sub System) And Verification, Full test */
        case 1U :
        {
            BUS_DeviceTest();
            break;
        }

        /* [IP-PV] MC-06, Bus Isolation with Memory Sub-system, X2X Master FMU operation */
        /* [SM-PV] SM-MC-X2X_MST.01, Incoming AXI Isolation, X2X MST read/write FMU operation, X2X MST debug mode read FMU operation */
        case 2U :
        {
            BUS_AXIMasterIsolationTest();
            break;
        }

        /* [IP-PV] MC-06, Bus Isolation with Memory Sub-system, X2X Slave FMU operation */
        /* [SM-PV] SM-MC-X2X_SLV.01, Outgoing AXI Isolation, X2X SLV read/write FMU operation, X2X SLV debug mode read FMU operation */
        case 3U :
        {
            BUS_AXISlaveIsolationTest();
            break;
        }

        /* [IP-PV] MC-06, Bus Isolation with Memory Sub-system, Incoming AHB transaction Isolation/Monitoring */
        /* [SM-PV] SM-MC-HBIC.01, Incoming AHB Isolation, AHB Normal  Operation */
        case 4U :
        {
            BUS_AHBNormalTest();
            break;
        }

        /* [IP-PV] MC-06, Bus Isolation with Memory Sub-system, Incoming AHB transaction Isolation/Monitoring */
        /* [SM-PV] SM-MC-HBIC.01, Incoming AHB Isolation, AHB Protection Operation */
        case 5U :
        {
            BUS_AHBIsolationTest();
            break;
        }
       
        case 10U :
        {
            (void)SSM_BusIsolationDisable(BUS_AXI_MST_WCH_ISOLATION);
            (void)SSM_BusIsolationDisable(BUS_AXI_MST_RCH_ISOLATION);
            (void)SSM_BusIsolationDisable(BUS_AXI_SLV_WCH_ISOLATION);
            (void)SSM_BusIsolationDisable(BUS_AXI_SLV_RCH_ISOLATION);
            (void)SSM_BusIsolationDisable(BUS_AHB_MST_ISOLATION);
            break;
        }
        
        default:
        {
            mcu_printf("Nothing to do\n");
            break;
        }
    }
}

