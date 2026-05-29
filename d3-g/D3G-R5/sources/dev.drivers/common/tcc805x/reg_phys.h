/*
***************************************************************************************************
*
*   FileName : reg_phys.h
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

#ifndef REG_PHYS_HEADER
#define REG_PHYS_HEADER

/* Part 0. */
#define MCU_BSP_SNOR_BASE               (0x08000000UL)
#define MCU_BSP_BASE                    (0x1B000000UL)

/* Part 2. SMU & PMU */
#define MCU_BSP_SMU_BUS_BASE            (0x14000000UL)
#define MCU_BSP_SMU_VIC_BASE            (0x14100000UL)
#define MCU_BSP_SMU_GPIO_BASE           (0x14200000UL)
#define MCU_BSP_SMU_PMU_BASE            (0x14400000UL)
#define MCU_BSP_SMU_CFG_BASE            (0x14600000UL)
#define MCU_BSP_PMU_BASE                (0x1B936000UL)

/* Part 3. Graphic */
#define MCU_BSP_GPU3D_BASE              (0x10000000UL)
#define MCU_BSP_GPU2D_BASE              (0x1A000000UL)

/* Part 4. Memory */
#define MCU_BSP_MBUS_CFG_BASE           (0x13500000UL)

/* Part 5. IO */
#define MCU_BSP_IOBUS_CFG_BASE          (0x16051000UL)
#define MCU_BSP_IOBUS_CFG1_BASE         (0x16494400UL)

/* Part 6. High Speed H/O */
#define MCU_BSP_HSIOBUS_CFG_BASE        (0x11DA0000UL)

/* Part 7. Display */
#define MCU_BSP_DBUS_VIOC_BASE          (0x12000000UL)

/* Part 8. Video */
#define MCU_BSP_VBUS_CFG_BASE           (0x15100000UL)

/* Part 9. CM */
#define MCU_BSP_CMBUS_CFG_BASE          (0x19100000UL)

/* Part 10. CPU */
#define MCU_BSP_CBUS_CFG_BASE           (0x17000000UL)
#define MCU_BSP_CBUS_GIC_BASE           (0x17300000UL)

/* Part 11. MICOM */
#define MCU_BSP_CAN_FD_BASE             (0x1B000000UL)
#define MCU_BSP_GPSB_BASE               (0x1B100000UL)
#define MCU_BSP_UART_BASE               (0x1B200000UL)
#define MCU_BSP_UDMA_BASE               (0x1B260000UL)
#define MCU_BSP_I2C_BASE                (0x1B300000UL)
#define MCU_BSP_PWM_BASE                (0x1B400000UL)
#define MCU_BSP_PWM_PORT_CFG_BASE       (0x1B430000UL)
#define MCU_BSP_ICTC_BASE               (0x1B500000UL)
#define MCU_BSP_ADC_BASE                (0x1B700000UL)
#define MCU_BSP_MBOX_BASE               (0x1B800000UL)
#define MCU_BSP_GIC_BASE                (0x1B900000UL)
#define MCU_BSP_SFMC_BASE               (0x1B910000UL)
#define MCU_BSP_CMU_BASE                (0x1B921000UL)
#define MCU_BSP_SM_CTRL_BASE            (0x1B921000UL)
#define MCU_BSP_SUBSYS_BASE             (0x1B930000UL)
#define MCU_BSP_CKC_BASE                (0x1B931000UL)
#define MCU_BSP_TIMER_BASE              (0x1B933000UL)
#define MCU_BSP_WDT_BASE                (0x1B934000UL)
#define MCU_BSP_GPIO_BASE               (0x1B935000UL)
#define MCU_BSP_PMU_BASE                (0x1B936000UL)
#define MCU_BSP_FMU_BASE                (0x1B936400UL)
#define MCU_BSP_PMIO_BASE               (0x1B937000UL)
#define MCU_BSP_DSE_BASE                (0x1B938000UL)
#define MCU_BSP_MID_CFG_BASE            (0x1B939000UL)
#define MCU_BSP_SFMC_MID_BASE           (0x1B93A000UL)
#define MCU_BSP_MID_BASE                (0x1B93B000UL)
#define MCU_BSP_MIPI_BASE               (0x1BC00000UL)
#define MCU_BSP_DP_BASE                 (0x1BD00000UL)

//#define SRAM_DEVICE_SHARE_CONFIG_BASE 0xF0009FD0UL
//#define SRAM_BOOT_SEQ_CHECK_BASE  0xF0009FF8UL
#define SRAM_EXCEPTION_SAVE_BASE        (0xF000B1F0UL)  /* 0xF000_B200 (16bytes) */

#endif /* REG_PHYS_HEADER */

