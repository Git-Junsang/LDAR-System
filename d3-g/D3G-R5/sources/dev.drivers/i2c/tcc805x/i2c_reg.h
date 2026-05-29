/*
***************************************************************************************************
*
*   FileName : i2c_reg.h
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

#if !defined(I2C_TCC805X_HEADER)
#define I2C_TCC805X_HEADER

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <i2c.h>
#include <clock_dev.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/* Macro */

/* I2C Common Options */
#define I2C_WR                          (uint32)(0UL)           /* Write Flag */
#define I2C_RD                          (uint32)(1UL)           /* Read Flag */
#define I2C_PCLK                        (uint32)(6000000UL)     /* I2C Default Peri Clock */
#define I2C_POLL_TIMEOUT                (uint32)(100000UL)
#define I2C_CH_NUM                      (uint32)(3UL)
#define I2C_PORT_NUM                    (uint32)(40UL)
#define I2C_CH_OFFSET                   (uint32)(0x10000UL)
#define I2C_LB_INTERNAL                 (uint32)(0x0UL)
#define I2C_LB_EXTERNAL                 (uint32)(0x1UL)

/* I2C Transfer Options */
#define I2C_WR_WITHOUT_STOP             (uint32)(0x01UL)      /* do not generate STOP condition after write last byte */
#define I2C_RD_WITHOUT_ACK              (uint32)(0x10UL)      /* do not send ACK when read last byte */
#define I2C_RD_WITHOUT_STOP             (uint32)(0x20UL)      /* do not generate STOP condition after read last byte */
#define I2C_RD_WITHOUT_RPEAT_START      (uint32)(0x40UL)      /* do not generate START condition between write and read sequence */

/* I2C Busy Flag */
#define I2C_BUSY_AFTER_STOP             (uint32)(0x0UL)
#define I2C_BUSY_AFTER_START            (uint32)(0x1UL)
#define I2C_BUSY_TIMEOUT                (uint32)(100000UL)

/* I2C Channel Base Address */
#define I2C_CH0_BASE                    ((uint32)MCU_BSP_I2C_BASE + (uint32)0x00000UL)
#define I2C_CH1_BASE                    ((uint32)MCU_BSP_I2C_BASE + (uint32)0x10000UL)
#define I2C_CH2_BASE                    ((uint32)MCU_BSP_I2C_BASE + (uint32)0x20000UL)
#define I2C_PCFG_BASE                   ((uint32)MCU_BSP_I2C_BASE + (uint32)0x30000UL)
#define I2C_CH0_VSLAVE                  ((uint32)MCU_BSP_I2C_BASE + (uint32)0x40000UL)
#define I2C_CH1_VSLAVE                  ((uint32)MCU_BSP_I2C_BASE + (uint32)0x50000UL)
#define I2C_CH2_VSLAVE                  ((uint32)MCU_BSP_I2C_BASE + (uint32)0x60000UL)

/* I2C Control Register Offset */
#define I2C_PRES                        (uint32)(0x00UL)
#define I2C_CTRL                        (uint32)(0x04UL)
#define I2C_CTRL_EN_CORE                (BSP_BIT_07)
#define I2C_CTRL_EN_INTR                (BSP_BIT_06)

#define I2C_TXR                         (uint32)(0x08UL)
#define I2C_CMD                         (uint32)(0x0CUL)
#define I2C_CMD_STA                     (BSP_BIT_07)
#define I2C_CMD_STO                  	(BSP_BIT_06)
#define I2C_CMD_RD                      (BSP_BIT_05)
#define I2C_CMD_WR                      (BSP_BIT_04)
#define I2C_CMD_ACK                     (BSP_BIT_03)
#define I2C_CMD_IACK                    (BSP_BIT_00)
#define I2C_CMD_PROGRESS_MASK           (uint32)(0xF0UL)

#define I2C_RXR                         (uint32)(0x10UL)
#define I2C_SR                          (uint32)(0x14UL)
#define I2C_SR_STATUS_MACHINE           (uint32)(0x3fUL << 16UL)
#define I2C_SR_STATUS_OTHER             (uint32)(0xfUL << 8UL)
#define I2C_SR_RX_ACK                   (BSP_BIT_07)
#define I2C_SR_BUSY                     (BSP_BIT_06)
#define I2C_SR_AL                       (BSP_BIT_05)
#define I2C_SR_TIP                      (BSP_BIT_01)
#define I2C_SR_IF                       (BSP_BIT_00)

#define I2C_TIME_0                      (uint32)(0x18UL)
#define I2C_FCLV_MASK                   (uint32)(0x1ffUL)
#define I2C_FCLV_SHIFT                  (uint32)(20UL)

#define I2C_TIME_1                      (uint32)(0x24UL)

/* I2C Port Configuration Offset */
#define I2C_PORT_SEL                    (uint32)(0x00UL)
#define I2C_PORT_CH0_SHIFT              (uint32)(0UL)
#define I2C_PORT_CH1_SHIFT              (uint32)(8UL)
#define I2C_PORT_CH2_SHIFT              (uint32)(16UL)

#define I2C_LB_CFG                      (uint32)(0x04UL)
#define I2C_LB_CFG_BIT                  (BSP_BIT_00)
#define I2C_WR_PW                       (uint32)(0x08UL)
#define I2C_LOCK_PW                     (uint32)(0x5AFEACE5UL)

#define I2C_WR_LOCK                     (uint32)(0x0CUL)

/* I2C Virtual Slave Offset */
#define I2C_VSCTL                       (uint32)(0x04UL)
#define I2C_VSCTL_EN                    (BSP_BIT_00)

#define I2C_VSADDR                      (uint32)(0x08UL)
#define I2C_VSTIN                       (uint32)(0x0CUL)
#define I2C_VSINT_EN_0                  (BSP_BIT_08)
#define I2C_VSINT_EN_1                  (BSP_BIT_10)

#define I2C_VSMBL                       (uint32)(0x10UL)
#define I2C_VSMBF                       (uint32)(0x1CUL)
#define I2C_VSMB0                       (uint32)(0x20UL)
#define I2C_VSMB1                       (uint32)(0x24UL)

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

extern I2CDev_t i2c[I2C_CH_NUM];

extern const uint32 i2cport[I2C_PORT_NUM+1UL][3];

#endif /* _I2C_TCC805X_HEADER_ */

