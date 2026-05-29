/*
***************************************************************************************************
*
*   FileName : pmu_reg.h
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

#ifndef PMU_REG_HEADER
#define PMU_REG_HEADER

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*******************************************************************************
*    Register base address
********************************************************************************/
/* MCU sub system configuration register */
#define PMU_RST_REG_SW_RESET_2          ( uint32 ) ( ( uint32 ) MCU_BSP_SUBSYS_BASE + ( uint32 ) 0x14UL)
#define PMU_RST_REG_X2X_PWR_CTRL        ( uint32 ) ( ( uint32 ) MCU_BSP_SUBSYS_BASE + ( uint32 ) 0x2CUL)

/* Memory sub system configuration register */
#define PMU_RST_REG_X2X_CFG1            ( uint32 ) ( ( uint32 ) MCU_BSP_MBUS_CFG_BASE + ( uint32 ) 0x100UL)

/* PMU register */
#define PMU_RST_REG_PMU_HSM_RSTN_MSK        ( uint32 ) ( ( uint32 ) MCU_BSP_PMU_BASE + ( uint32 ) 0x8UL)
#define PMU_RST_REG_PMU_USSTATUS            ( uint32 ) ( ( uint32 ) MCU_BSP_PMU_BASE + ( uint32 ) 0x1CUL)
#define PMU_RST_REG_PMU_WDTRST_MASK_TOP     ( uint32 ) ( ( uint32 ) MCU_BSP_PMU_BASE + ( uint32 ) 0x130UL)
#define PMU_RST_REG_PMU_CRST                ( uint32 ) ( ( uint32 ) MCU_BSP_PMU_BASE + ( uint32 ) 0x17CUL)
#define PMU_RST_REG_PMU_AP_WRST             ( uint32 ) ( ( uint32 ) MCU_BSP_PMU_BASE + ( uint32 ) 0x180UL)
#define PMU_RST_REG_PMU_LVDS_SDM_RSTN_MSK   ( uint32 ) ( ( uint32 ) MCU_BSP_PMU_BASE + ( uint32 ) 0x1F8UL)
#define PMU_RST_REG_PMU_ACCESS_CTRL         ( uint32 ) ( ( uint32 ) MCU_BSP_PMU_BASE + ( uint32 ) 0x254UL)


/*******************************************************************************
*    Register field
********************************************************************************/
/* SW_RESET2 */
#define PMU_RST_SW_RESET2_ABM           (0x1UL << 8UL)
#define PMU_RST_SW_RESET2_ABS           (0x1UL << 9UL)

/* X2X_PWR_CTRL */
#define PMU_RST_M2A_PD_REQ              (0x1UL << 0UL)
#define PMU_RST_M2A_PD_ACK              (0x1UL << 1UL)
#define PMU_RST_A2M_PD_REQ              (0x1UL << 4UL)
#define PMU_RST_A2M_PD_ACK              (0x1UL << 5UL)
#define PMU_RST_M2A_BYPASS              (0x1UL << 8UL)
#define PMU_RST_A2M_SLV_RST             (0x1UL << 16UL)
#define PMU_RST_M2A_MST_RST             (0x1UL << 20UL)

/* X2X_CFG1 */
#define PMU_RST_PWRDNBYPASS             (0x1UL << 2UL)

/* HSM & DP Reset Mask */
/* HSM */
#define PMU_RST_RSTMSK_HSM_WDT          (0x1UL << 2UL)
#define PMU_RST_RSTMSK_HSM_WARM         (0x1UL << 3UL)
#define PMU_RST_RSTMSK_HSM_APWARM       (0x1UL << 4UL)
#define PMU_RST_RSTMSK_HSM_PVT          (0x1UL << 5UL)
#define PMU_RST_RSTMSK_HSM_SSSWDT0      (0x1UL << 6UL)
#define PMU_RST_RSTMSK_HSM_SSSWDT1      (0x1UL << 7UL)
#define PMU_RST_RSTMSK_HSM_SSS          (0x1UL << 8UL)
#define PMU_RST_RSTMSK_HSM_HSMSW        (0x1UL << 9UL)
#define PMU_RST_RSTMSK_HSM_COLD         (0x1UL << 10UL)
/* DP */
#define PMU_RST_RSTMSK_DP_WDT           (0x1UL << 13UL)
#define PMU_RST_RSTMSK_DP_WARM          (0x1UL << 14UL)
#define PMU_RST_RSTMSK_DP_APWARM        (0x1UL << 15UL)
#define PMU_RST_RSTMSK_DP_PVT           (0x1UL << 16UL)
#define PMU_RST_RSTMSK_DP_DPSW          (0x1UL << 17UL)
#define PMU_RST_RSTMSK_DP_COLD          (0x1UL << 18UL)

/* LVDS & MIPI & SDM & SRVC Reset Mask */
/* LVDS */
#define PMU_RST_RSTMSK_LVDS_WDT         (0x1UL << 2UL)
#define PMU_RST_RSTMSK_LVDS_WARM        (0x1UL << 3UL)
#define PMU_RST_RSTMSK_LVDS_APWARM      (0x1UL << 4UL)
#define PMU_RST_RSTMSK_LVDS_PVT         (0x1UL << 5UL)
#define PMU_RST_RSTMSK_LVDS_HSMSW       (0x1UL << 6UL)
#define PMU_RST_RSTMSK_LVDS_COLD        (0x1UL << 7UL)
/* MIPI */
#define PMU_RST_RSTMSK_MIPI_WDT         (0x1UL << 10UL)
#define PMU_RST_RSTMSK_MIPI_WARM        (0x1UL << 11UL)
#define PMU_RST_RSTMSK_MIPI_APWARM      (0x1UL << 12UL)
#define PMU_RST_RSTMSK_MIPI_PVT         (0x1UL << 13UL)
#define PMU_RST_RSTMSK_MIPI_HSMSW       (0x1UL << 14UL)
#define PMU_RST_RSTMSK_MIPI_COLD        (0x1UL << 15UL)
/* SDM */
#define PMU_RST_RSTMSK_SDM_WDT          (0x1UL << 18UL)
#define PMU_RST_RSTMSK_SDM_WARM         (0x1UL << 19UL)
#define PMU_RST_RSTMSK_SDM_APWARM       (0x1UL << 20UL)
#define PMU_RST_RSTMSK_SDM_PVT          (0x1UL << 21UL)
#define PMU_RST_RSTMSK_SDM_HSMSW        (0x1UL << 22UL)
#define PMU_RST_RSTMSK_SDM_COLD         (0x1UL << 23UL)
/* SRVC */
#define PMU_RST_RSTMSK_SRVC_WDT         (0x1UL << 26UL)
#define PMU_RST_RSTMSK_SRVC_WARM        (0x1UL << 27UL)
#define PMU_RST_RSTMSK_SRVC_APWARM      (0x1UL << 28UL)
#define PMU_RST_RSTMSK_SRVC_PVT         (0x1UL << 29UL)
#define PMU_RST_RSTMSK_SRVC_HSMSW       (0x1UL << 30UL)
#define PMU_RST_RSTMSK_SRVC_COLD        (0x1UL << 31UL)

/* PMU_USSTATUS */
#define PMU_RST_PMU_USSTATUS_RST_TYPE   (25UL)

/* WDTRST_MASK_TOP */
#define PMU_RST_WDT_RSTMSKTOP_MCU       (0x1UL << 7UL)

/* PMU_ACCESS_CTRL */
#define PMU_RST_PMU_ACCESSCTRL_HSM      (0x1UL << 2UL)
#define PMU_RST_PMU_ACCESSCTRL_WDT      (0x1UL << 8UL)


/*******************************************************************************
*    type definitions
********************************************************************************/
/* Reset type flag of PMU_USSTATUS[25] */
#define PMU_RST_TYPE_APSYS              (0x0UL)
#define PMU_RST_TYPE_COLD               (0x1UL)



/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/



#endif //PMU_REG_HEADER

