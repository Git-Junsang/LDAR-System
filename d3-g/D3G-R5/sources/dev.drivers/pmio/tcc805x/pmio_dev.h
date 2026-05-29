/*
***************************************************************************************************
*
*   FileName : pmio_dev.h
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

#ifndef  MCU_BSP_PMIO_DEV_HEADER
#define  MCU_BSP_PMIO_DEV_HEADER
/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <sal_internal.h>
#include "reg_phys.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
/*PMIO VALUES*/
/*PMIO va: Bit flags*/
#define PMIO_VA_BITALL                  (0xFFFFFFFFUL)
#define PMIO_VA_BITCLEAR                (0x00000000UL)
#define PMIO_VA_BIT31                   (0x80000000UL)
#define PMIO_VA_BIT30                   (0x40000000UL)
#define PMIO_VA_BIT29                   (0x20000000UL)
#define PMIO_VA_BIT28                   (0x10000000UL)
#define PMIO_VA_BIT27                   (0x08000000UL)
#define PMIO_VA_BIT26                   (0x04000000UL)
#define PMIO_VA_BIT25                   (0x02000000UL)
#define PMIO_VA_BIT24                   (0x01000000UL)
#define PMIO_VA_BIT23                   (0x00800000UL)
#define PMIO_VA_BIT22                   (0x00400000UL)
#define PMIO_VA_BIT21                   (0x00200000UL)
#define PMIO_VA_BIT20                   (0x00100000UL)
#define PMIO_VA_BIT19                   (0x00080000UL)
#define PMIO_VA_BIT18                   (0x00040000UL)
#define PMIO_VA_BIT17                   (0x00020000UL)
#define PMIO_VA_BIT16                   (0x00010000UL)
#define PMIO_VA_BIT15                   (0x00008000UL)
#define PMIO_VA_BIT14                   (0x00004000UL)
#define PMIO_VA_BIT13                   (0x00002000UL)
#define PMIO_VA_BIT12                   (0x00001000UL)
#define PMIO_VA_BIT11                   (0x00000800UL)
#define PMIO_VA_BIT10                   (0x00000400UL)
#define PMIO_VA_BIT9                    (0x00000200UL)
#define PMIO_VA_BIT8                    (0x00000100UL)
#define PMIO_VA_BIT7                    (0x00000080UL)
#define PMIO_VA_BIT6                    (0x00000040UL)
#define PMIO_VA_BIT5                    (0x00000020UL)
#define PMIO_VA_BIT4                    (0x00000010UL)
#define PMIO_VA_BIT3                    (0x00000008UL)
#define PMIO_VA_BIT2                    (0x00000004UL)
#define PMIO_VA_BIT1                    (0x00000002UL)
#define PMIO_VA_BIT0                    (0x00000001UL)

/*PMIO va: Size*/
#define PMIO_VA_BACKUP_RAM_SIZE         (1024UL) // 1 Kbyte Backup RAM

/*PMIO va: Port*/
#define PMIO_VA_PCTL_0_PORT             (PMIO_VA_BIT0)
#define PMIO_VA_PCTL_1_PORT             (PMIO_VA_BIT1)

#define PMIO_VA_LEAKAGE_PU_PORT         (PMIO_VA_BITCLEAR)
#define PMIO_VA_LEAKAGE_PD_PORT         (PMIO_VA_BIT6 | PMIO_VA_BIT10)
#define PMIO_VA_LEAKAGE_OUT_H_PORT      (PMIO_VA_BIT8)
#define PMIO_VA_LEAKAGE_OUT_L_PORT      (PMIO_VA_BIT9  | PMIO_VA_BIT11 | /*PMIO_VA_BIT12 |*/ \
                                         PMIO_VA_BIT13 | PMIO_VA_BIT15 | PMIO_VA_BIT17)
#define PMIO_VA_LEAKAGE_IN_PORT         (PMIO_VA_BIT0  | /*PMIO_VA_BIT1  |*/ PMIO_VA_BIT2  | \
                                         PMIO_VA_BIT3  | PMIO_VA_BIT4  | PMIO_VA_BIT5  | \
                                         PMIO_VA_BIT6  | PMIO_VA_BIT7  | PMIO_VA_BIT10 | \
                                         PMIO_VA_BIT14 | PMIO_VA_BIT16 | PMIO_VA_BIT18)

/*PMIO va: Address*/
#define PMIO_VA_BASE_ADDR               (MCU_BSP_PMIO_BASE)        // R/W, Backup RAM Register
#define PMIO_VA_BACKUP_RAM_ADDR         (0x1B937000UL)             // R/W, Backup RAM Register
#define PMIO_VA_PMGPIO_CTL_ADDR         (0x1B937828UL) //PMGPIO Controll Register
#define PMIO_VA_RST_STS0_ADDR           (0xC003F400UL) //Reset Status 0 Register (0x14400130)
#define PMIO_VA_RST_STS1_ADDR           (0xC003F404UL) //Reset Status 1 Register (0x14400134)
#define PMIO_VA_RST_STS2_ADDR           (0xC003F408UL) //Reset Status 2 Register (0x14400138)
//STR Defines
#define PMIO_VA_PMU_MICOM_USER_ADDR     (0x1B936178UL) //Save boot reason
#define PMIO_VA_PMU_WAKEUP_EN_ADDR      (0x1B936024UL) //Check wakeup by RTC
//#define RTCSTR                        0x16480000 + 0x4C //Check RTC boot reason
#define PMIO_VA_PMU_RST_STS0_ADDR       (0x1B936164UL) //Check Watchdog reset
#define PMIO_VA_PMU_RST_STS1_ADDR       (0x1B936168UL) //Check Watchdog reset
#define PMIO_VA_PMU_RST_STS2_ADDR       (0x1B93616CUL) //Check Watchdog reset


/*PMIO REGISTERS*/
/*PMIO reg: GPK by PMIO*/
#define PMIO_REG_GPK_FDAT               (*(SALReg32 *)(0x1B937A00UL)) //GPK Filtered Data Port
#define PMIO_REG_GPK_IRQST              (*(SALReg32 *)(0x1B937A10UL)) //GPK Interrupt Status Register
#define PMIO_REG_GPK_IRQEN              (*(SALReg32 *)(0x1B937A14UL)) //GPK Interrupt Enable Register
#define PMIO_REG_GPK_IRQPOL             (*(SALReg32 *)(0x1B937A18UL)) //GPK Interrupt Polarity Register
#define PMIO_REG_GPK_IRQTM0             (*(SALReg32 *)(0x1B937A1CUL)) //GPK Interrupt Trigger Mode 0 Reg
#define PMIO_REG_GPK_IRQTM1             (*(SALReg32 *)(0x1B937A20UL)) //GPK Interrupt Trigger Mode 1 Reg
#define PMIO_REG_GPK_FCK                (*(SALReg32 *)(0x1B937A24UL)) //GPK Filter Clock Conf Reg
#define PMIO_REG_GPK_FBP                (*(SALReg32 *)(0x1B937A28UL)) //GPK Filter Bypass Register
#define PMIO_REG_GPK_CTL                (*(SALReg32 *)(0x1B937A2CUL)) //GPK Miscellaneous Control Reg

/*PMIO reg: Backup Ram*/
#define PMIO_REG_BACKUP_RAM             (*(SALReg32 *)(0x1B937000UL)) // R/W, Backup RAM Register

/*PMIO reg: PMGPIO*/
#define PMIO_REG_PMGPIO_DAT             (*(SALReg32 *)(0x1B937800UL)) //PMGPIO Data Reg
#define PMIO_REG_PMGPIO_EN              (*(SALReg32 *)(0x1B937804UL)) //PMGPIO Direction Control Reg
#define PMIO_REG_PMGPIO_FS              (*(SALReg32 *)(0x1B937808UL)) //PMGPIO Function Select Reg
#define PMIO_REG_PMGPIO_IEN             (*(SALReg32 *)(0x1B93780CUL)) //PMGPIO Input Enable Reg
#define PMIO_REG_PMGPIO_PE              (*(SALReg32 *)(0x1B937810UL)) //PMGPIO PULL Down/Up Enable Reg
#define PMIO_REG_PMGPIO_PS              (*(SALReg32 *)(0x1B937814UL)) //PMGPIO PULL Down/Up Select Reg
#define PMIO_REG_PMGPIO_CD0             (*(SALReg32 *)(0x1B937818UL)) //PMGPIO Driver Strength 1 Reg
#define PMIO_REG_PMGPIO_CD1             (*(SALReg32 *)(0x1B93781CUL)) //PMGPIO Driver Strength 1 Reg
#define PMIO_REG_PMGPIO_EE0             (*(SALReg32 *)(0x1B937820UL)) //PMGPIO Event Enable 0 Reg
#define PMIO_REG_PMGPIO_EE1             (*(SALReg32 *)(0x1B937824UL)) //PMGPIO Event Enable 1 Reg
#define PMIO_REG_PMGPIO_CTL             (*(SALReg32 *)(0x1B937828UL)) //PMGPIO Control Reg
#define PMIO_REG_PMGPIO_DI              (*(SALReg32 *)(0x1B93782CUL)) //PMGPIO Input Raw Status Reg
#define PMIO_REG_PMGPIO_STR             (*(SALReg32 *)(0x1B937830UL)) //PMGPIO Rising Event Status Reg
#define PMIO_REG_PMGPIO_STF             (*(SALReg32 *)(0x1B937834UL)) //PMGPIO Falling Event Status Reg
#define PMIO_REG_PMGPIO_POL             (*(SALReg32 *)(0x1B937838UL)) //PMGPIO Event Polarity Reg
#define PMIO_REG_PMGPIO_PROT            (*(SALReg32 *)(0x1B93783CUL)) //PMGPIO Protect Reg
#define PMIO_REG_PMGPIO_APB             (*(SALReg32 *)(0x1B937900UL)) //PMGPIO & PMRAM APB Timing Reg

/*PMIO reg: PCTLIO*/
#define PMIO_REG_PCTLIO_DAT             (*(SALReg32 *)(0x1B937840UL)) //PCTLIO Data Reg
#define PMIO_REG_PCTLIO_EN              (*(SALReg32 *)(0x1B937844UL)) //PCTLIO Direction Control Reg
#define PMIO_REG_PCTLIO_FS              (*(SALReg32 *)(0x1B937848UL)) //PCTLIO Function Select Reg
#define PMIO_REG_PCTLIO_IEN             (*(SALReg32 *)(0x1B93784CUL)) //PCTLIO Input Enable Reg
#define PMIO_REG_PCTLIO_PE              (*(SALReg32 *)(0x1B937850UL)) //PCTLIO PULL Down/Up Enable Reg
#define PMIO_REG_PCTLIO_PS              (*(SALReg32 *)(0x1B937854UL)) //PCTLIO PULL Down/Up Reg
#define PMIO_REG_PCTLIO_CD0             (*(SALReg32 *)(0x1B937858UL)) //PCTLIO Driver Strength 0 Reg
#define PMIO_REG_PCTLIO_CD1             (*(SALReg32 *)(0x1B93785CUL)) //PCTLIO Driver Strength 1 Reg

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
#endif

