/*
***************************************************************************************************
*
*   FileName : pmu_reset.c
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

#include <sal_internal.h>
#include <reg_phys.h>
#include <bsp.h>
#include <gic.h>
#include <debug.h>

#include <pmu_reset.h>


/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/


/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/

static uint32 gSCWdtControlId;


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void PMU_ResetSCWDTIRQHandler
(
    void *                              pArg
);

static void PMU_ResetSCWDTTask
(
    void *                              pArg
);


/*
***************************************************************************************************
*                                       PMU_ResetPreCfg
*
* Function to set reset mask for normal case
*
* @param
*
* @return
*
* Notes
*
***************************************************************************************************
*/

void PMU_ResetPreCfg
(
    void
)
{
    uint32 uiReg;
    uint32 uiRegVal = 0UL;

    /*******************************************************************************
    *    Set access control
    ********************************************************************************/
    uiReg = PMU_RST_REG_PMU_ACCESS_CTRL;
    uiRegVal = SAL_ReadReg( uiReg );
    uiRegVal &= ~( PMU_RST_PMU_ACCESSCTRL_WDT | PMU_RST_PMU_ACCESSCTRL_HSM );
    SAL_WriteReg( uiRegVal, uiReg );


    /*******************************************************************************
    *    Set reset mask
    ********************************************************************************/
    /* Set HSM & DP reset mask */
    uiReg = PMU_RST_REG_PMU_HSM_RSTN_MSK;

    /* Set HSM reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_HSM_COLD | PMU_RST_RSTMSK_HSM_SSS  | PMU_RST_RSTMSK_HSM_SSSWDT1 | PMU_RST_RSTMSK_HSM_SSSWDT0
                 | PMU_RST_RSTMSK_HSM_PVT  | PMU_RST_RSTMSK_HSM_WARM | PMU_RST_RSTMSK_HSM_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_HSM_HSMSW | PMU_RST_RSTMSK_HSM_APWARM );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set DP reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_DP_COLD | PMU_RST_RSTMSK_DP_DPSW | PMU_RST_RSTMSK_DP_PVT | PMU_RST_RSTMSK_DP_APWARM
                 | PMU_RST_RSTMSK_DP_WARM | PMU_RST_RSTMSK_DP_WDT );
    SAL_WriteReg( uiRegVal, uiReg );


    /* Set LVDS & MIPI & SDM & SRVC reset mask */
    uiReg = PMU_RST_REG_PMU_LVDS_SDM_RSTN_MSK;

    /* Set LVDS reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_LVDS_COLD | PMU_RST_RSTMSK_LVDS_HSMSW | PMU_RST_RSTMSK_LVDS_PVT | PMU_RST_RSTMSK_LVDS_APWARM
                 | PMU_RST_RSTMSK_LVDS_WARM | PMU_RST_RSTMSK_LVDS_WDT );
    SAL_WriteReg(uiRegVal, uiReg);

    /* Set MIPI reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_MIPI_COLD | PMU_RST_RSTMSK_MIPI_HSMSW | PMU_RST_RSTMSK_MIPI_PVT | PMU_RST_RSTMSK_MIPI_APWARM
                 | PMU_RST_RSTMSK_MIPI_WARM | PMU_RST_RSTMSK_MIPI_WDT );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set SDM reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_SDM_COLD | PMU_RST_RSTMSK_SDM_HSMSW | PMU_RST_RSTMSK_SDM_PVT | PMU_RST_RSTMSK_SDM_APWARM
                 | PMU_RST_RSTMSK_SDM_WARM | PMU_RST_RSTMSK_SDM_WDT );
    SAL_WriteReg(uiRegVal, uiReg);

    /* Set SRVC reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_SRVC_COLD | PMU_RST_RSTMSK_SRVC_HSMSW | PMU_RST_RSTMSK_SRVC_PVT | PMU_RST_RSTMSK_SRVC_APWARM
                 | PMU_RST_RSTMSK_SRVC_WARM | PMU_RST_RSTMSK_SRVC_WDT );
    SAL_WriteReg( uiRegVal, uiReg );


    /*******************************************************************************
    *    Set WDT-TOP
    ********************************************************************************/
    uiReg = PMU_RST_REG_PMU_WDTRST_MASK_TOP;
    uiRegVal = SAL_ReadReg( uiReg );
    uiRegVal &= ~( PMU_RST_WDT_RSTMSKTOP_MCU );
    SAL_WriteReg( uiRegVal, uiReg );
}

/*
***************************************************************************************************
*                                       PMU_ResetSDMRstMskCfg
*
* Function to set reset mask for SDM case
*
* @param
*
* @return
*
* Notes
*
***************************************************************************************************
*/

void PMU_ResetSDMRstMskCfg
(
    void
)
{
    uint32 uiReg;
    uint32 uiRegVal = 0UL;


    /*******************************************************************************
    *    Set reset mask
    ********************************************************************************/
    /* Set HSM & DP reset mask */
    uiReg = PMU_RST_REG_PMU_HSM_RSTN_MSK;

    /* Set HSM reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_HSM_COLD | PMU_RST_RSTMSK_HSM_SSS  | PMU_RST_RSTMSK_HSM_SSSWDT1 | PMU_RST_RSTMSK_HSM_SSSWDT0
                 | PMU_RST_RSTMSK_HSM_PVT  | PMU_RST_RSTMSK_HSM_WARM | PMU_RST_RSTMSK_HSM_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_HSM_HSMSW | PMU_RST_RSTMSK_HSM_APWARM );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set DP reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_DP_COLD | PMU_RST_RSTMSK_DP_DPSW | PMU_RST_RSTMSK_DP_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_DP_PVT | PMU_RST_RSTMSK_DP_APWARM | PMU_RST_RSTMSK_DP_WARM );
    SAL_WriteReg( uiRegVal, uiReg );


    /* Set LVDS & MIPI & SDM & SRVC reset mask */
    uiReg = PMU_RST_REG_PMU_LVDS_SDM_RSTN_MSK;

    /* Set LVDS reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_LVDS_COLD | PMU_RST_RSTMSK_LVDS_HSMSW | PMU_RST_RSTMSK_LVDS_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_LVDS_PVT | PMU_RST_RSTMSK_LVDS_APWARM | PMU_RST_RSTMSK_LVDS_WARM );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set MIPI reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_MIPI_COLD | PMU_RST_RSTMSK_MIPI_HSMSW | PMU_RST_RSTMSK_MIPI_PVT | PMU_RST_RSTMSK_MIPI_APWARM
                 | PMU_RST_RSTMSK_MIPI_WARM | PMU_RST_RSTMSK_MIPI_WDT );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set SDM reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_SDM_COLD | PMU_RST_RSTMSK_SDM_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_SDM_HSMSW | PMU_RST_RSTMSK_SDM_PVT | PMU_RST_RSTMSK_SDM_APWARM | PMU_RST_RSTMSK_SDM_WARM );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set SRVC reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_SRVC_COLD | PMU_RST_RSTMSK_SRVC_HSMSW | PMU_RST_RSTMSK_SRVC_PVT | PMU_RST_RSTMSK_SRVC_APWARM
                 | PMU_RST_RSTMSK_SRVC_WARM | PMU_RST_RSTMSK_SRVC_WDT );
    SAL_WriteReg( uiRegVal, uiReg );
}

/*
***************************************************************************************************
*                                       PMU_ResetTRVCRstMskCfg
*
* Function to set reset mask for TRVC case
*
* @param
*
* @return
*
* Notes
*
***************************************************************************************************
*/

void PMU_ResetTRVCRstMskCfg
(
    void
)
{
    uint32 uiReg;
    uint32 uiRegVal = 0UL;


    /*******************************************************************************
    *    Set reset mask
    ********************************************************************************/
    /* Set HSM & DP reset mask */
    uiReg = PMU_RST_REG_PMU_HSM_RSTN_MSK;

    /* Set HSM reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_HSM_COLD | PMU_RST_RSTMSK_HSM_SSS  | PMU_RST_RSTMSK_HSM_SSSWDT1 | PMU_RST_RSTMSK_HSM_SSSWDT0
                 | PMU_RST_RSTMSK_HSM_PVT  | PMU_RST_RSTMSK_HSM_WARM | PMU_RST_RSTMSK_HSM_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_HSM_HSMSW | PMU_RST_RSTMSK_HSM_APWARM );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set DP reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_DP_COLD | PMU_RST_RSTMSK_DP_DPSW | PMU_RST_RSTMSK_DP_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_DP_PVT | PMU_RST_RSTMSK_DP_APWARM | PMU_RST_RSTMSK_DP_WARM );
    SAL_WriteReg( uiRegVal, uiReg );


    /* Set LVDS & MIPI & SDM & SRVC reset mask */
    uiReg = PMU_RST_REG_PMU_LVDS_SDM_RSTN_MSK;

    /* Set LVDS reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_LVDS_COLD | PMU_RST_RSTMSK_LVDS_HSMSW | PMU_RST_RSTMSK_LVDS_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_LVDS_PVT | PMU_RST_RSTMSK_LVDS_APWARM | PMU_RST_RSTMSK_LVDS_WARM );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set MIPI reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_MIPI_COLD | PMU_RST_RSTMSK_MIPI_HSMSW | PMU_RST_RSTMSK_MIPI_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_MIPI_PVT | PMU_RST_RSTMSK_MIPI_APWARM | PMU_RST_RSTMSK_MIPI_WARM );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set SDM reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_SDM_COLD   | PMU_RST_RSTMSK_SDM_HSMSW | PMU_RST_RSTMSK_SDM_PVT
                 | PMU_RST_RSTMSK_SDM_APWARM | PMU_RST_RSTMSK_SDM_WARM  | PMU_RST_RSTMSK_SDM_WDT );
    SAL_WriteReg( uiRegVal, uiReg );

    /* Set SRVC reset mask */
    uiRegVal = SAL_ReadReg( uiReg );
    /* Allow reset */
    uiRegVal &= ~( PMU_RST_RSTMSK_SRVC_COLD | PMU_RST_RSTMSK_SRVC_HSMSW  | PMU_RST_RSTMSK_SRVC_WDT );
    /* Mask reset */
    uiRegVal |= ( PMU_RST_RSTMSK_SRVC_PVT   | PMU_RST_RSTMSK_SRVC_APWARM | PMU_RST_RSTMSK_SRVC_WARM );
    SAL_WriteReg( uiRegVal, uiReg );
}

/*
***************************************************************************************************
*                                       PMU_ResetColdReset
*
* Function to do cold reset
*
* @param
*
* @return
*
* Notes
*
***************************************************************************************************
*/

void PMU_ResetColdReset
(
    void
)
{
    SAL_WriteReg( ( uint32 ) 0x1, PMU_RST_REG_PMU_CRST ); //Write any value
}

/*
***************************************************************************************************
*                                       PMU_ResetAPSystemReboot
*
* Function to do AP system reset
*
* @param
*
* @return
*
* Notes
*
***************************************************************************************************
*/

void PMU_ResetAPSystemReboot
(
    void
)
{
    uint32 uiTempVal = 0UL;
    uint32 uiTimeOut = 0UL;


    /*******************************************************************************
    *    1. Power down AXI bridge bypass mode configuration
    ********************************************************************************/
    /* MCU */
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal &= ~( PMU_RST_M2A_BYPASS );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );

    /* AP */
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_CFG1 );
    uiTempVal &= ~( PMU_RST_PWRDNBYPASS );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_CFG1 );


    /*******************************************************************************
    *    2. Power down AXI bridge (MCU -> AP)
    ********************************************************************************/
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal &= ~( PMU_RST_M2A_PD_REQ );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );

    uiTimeOut = PMU_RST_PWRDN_ACK_TIMEOUT;
    while( ( SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL ) & PMU_RST_M2A_PD_ACK ) != 0UL )
    {
        mcu_printf( " Wait for M2A PD ACK \n " );

        uiTimeOut--;

        if( uiTimeOut == 0UL )
        {
            mcu_printf( " Fail to AP System Reboot \n " );
            break;
        }
    }


    /*******************************************************************************
    *    3. Power down AXI bridge (AP -> MCU)
    ********************************************************************************/
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal &= ~( PMU_RST_A2M_PD_REQ );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );

    uiTimeOut = PMU_RST_PWRDN_ACK_TIMEOUT;
    while( ( SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL ) & PMU_RST_A2M_PD_ACK ) != 0UL )
    {
        mcu_printf( " Wait for A2M PD ACK \n " );

        uiTimeOut--;

        if( uiTimeOut == 0UL )
        {
            mcu_printf( " Fail to AP System Reboot \n " );
            break;
        }
    }


    /*******************************************************************************
    *    4. Reset ADB400 in MCU sub system
    ********************************************************************************/
    /* MCU */
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal &= ~( PMU_RST_M2A_MST_RST );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );

    /* AP */
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal &= ~( PMU_RST_A2M_SLV_RST );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );


    /*******************************************************************************
    *    5. Do warm reset
    ********************************************************************************/
    SAL_WriteReg( ( uint32 ) 0x1, PMU_RST_REG_PMU_AP_WRST ); //Write any value


    /*******************************************************************************
    *    6. Release reset of ADB400 in MCU sub system
    ********************************************************************************/
    /* MCU */
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal |= ( PMU_RST_M2A_MST_RST );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );

    /* AP */
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal |= ( PMU_RST_A2M_SLV_RST );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );



    /*******************************************************************************
    *    7. Release power down AXI bridge (MCU -> AP)
    ********************************************************************************/
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal |= ( PMU_RST_M2A_PD_REQ );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );

    uiTimeOut = PMU_RST_PWRDN_ACK_TIMEOUT;
    while( ( SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL ) & PMU_RST_M2A_PD_ACK ) != PMU_RST_M2A_PD_ACK )
    {
        mcu_printf( " Wait for M2A PD ACK \n " );

        uiTimeOut--;

        if( uiTimeOut == 0UL )
        {
            mcu_printf( " Fail to AP System Reboot \n " );
            break;
        }
    }


    /*******************************************************************************
    *    8. Release power down AXI bridge (AP -> MCU)
    ********************************************************************************/
    uiTempVal = SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL );
    uiTempVal |= ( PMU_RST_A2M_PD_REQ );
    SAL_WriteReg( uiTempVal, PMU_RST_REG_X2X_PWR_CTRL );

    uiTimeOut = PMU_RST_PWRDN_ACK_TIMEOUT;
    while( (SAL_ReadReg( PMU_RST_REG_X2X_PWR_CTRL ) & PMU_RST_A2M_PD_ACK ) != PMU_RST_A2M_PD_ACK )
    {
        mcu_printf( " Wait for A2M PD ACK \n " );

        uiTimeOut--;

        if( uiTimeOut == 0UL )
        {
            mcu_printf( " Fail to AP System Reboot \n " );
            break;
        }
    }
}

/*
***************************************************************************************************
*                                       PMU_ResetSCWDTIRQHandler
*
* Function to handle SC watchdog interrupt
*
* @param    pArg [in]
*
* @return
*
* Notes
*
***************************************************************************************************
*/

static void PMU_ResetSCWDTIRQHandler
(
    void *                              pArg
)
{
    uint32 uiData;

    ( void ) pArg;

    uiData = 0UL;

    ( void ) SAL_QueuePut( gSCWdtControlId,
                      ( void * ) &uiData,
                      sizeof( uint32 ),
                      0,
                      SAL_OPT_BLOCKING );
}

/*
***************************************************************************************************
*                                       PMU_ResetSCWDTTask
*
* Function to wait SC watchdog interrupt
*
* @param    pArg [in]
*
* @return
*
* Notes
*
***************************************************************************************************
*/

static void PMU_ResetSCWDTTask
(
    void *                              pArg
)
{
    uint32 uiData;
    uint32 uiMsgSize;
    uint32 uiRstType = 0x0UL;

    ( void ) pArg;

    while( 1 )
    {
        /* Wait for WDT IRQ from Storage Core */
        ( void ) SAL_QueueGet( gSCWdtControlId,
                        ( void * ) &uiData,
                        ( uint32 * ) &uiMsgSize,
                        0,
                        SAL_OPT_BLOCKING );


        uiRstType = (SAL_ReadReg( PMU_RST_REG_PMU_USSTATUS ) >> PMU_RST_PMU_USSTATUS_RST_TYPE) & (0x1UL);

        if ( uiRstType == PMU_RST_TYPE_COLD )
        {
            mcu_printf( " Start Cold Reset by SC watchdog \n " );

            PMU_ResetColdReset();
        }
        else /* PMU_RST_TYPE_APSYS */
        {
            mcu_printf( " Start AP System Reset by SC watchdog \n " );

            PMU_ResetAPSystemReboot();
        }


        ( void ) SAL_TaskSleep( 100 );

    }
}

/*
***************************************************************************************************
*                                       PMU_ResetSCWDTInit
*
* Function to init SC watchdog task, queue, and interrupt
*
* @param    void
*
* @return
*           SAL_RET_SUCCESS  Initialize PMU reset SC(Storage Core) watchdog is succeed
*           SAL_RET_FAILED   Initialize PMU reset SC(Storage Core) watchdog is failed
* Notes
*
***************************************************************************************************
*/

SALRetCode_t PMU_ResetSCWDTInit
(
    void
)
{
    static uint32   uiPmuRstSCWdtTaskId;
    static uint32   uiPmuRstSCWdtTaskStk[ PMU_RST_SC_WDT_TASK_STK_SIZE ];
    SALRetCode_t    ret;

    ret = ( SALRetCode_t ) SAL_RET_SUCCESS;

    ret = SAL_QueueCreate( ( uint32 * ) &gSCWdtControlId,
                        ( const uint8 * ) "PMU Reset SC WDT Control Queue",
                        1UL,
                        sizeof( uint32 ) );

    if( ret == SAL_RET_SUCCESS )
    {
        ret = SAL_TaskCreate( ( uint32 * ) &uiPmuRstSCWdtTaskId,
                        ( const uint8 * ) "PMU Reset SC WDT Control Task",
                        ( SALTaskFunc ) &PMU_ResetSCWDTTask,
                        ( uint32 * const ) &uiPmuRstSCWdtTaskStk[ 0 ],
                        PMU_RST_SC_WDT_TASK_STK_SIZE,
                        SAL_PRIO_POWER_MANAGER,
                        NULL_PTR);
    }


    ( void ) GIC_IntVectSet( ( uint32 ) GIC_STRGB_WDT, GIC_PRIORITY_NO_MEAN,
                        GIC_INT_TYPE_EDGE_RISING,
                        ( GICIsrFunc ) &PMU_ResetSCWDTIRQHandler,
                        NULL_PTR);


	( void ) GIC_IntSrcEn( ( uint32 ) GIC_STRGB_WDT );


    return ret;
}

