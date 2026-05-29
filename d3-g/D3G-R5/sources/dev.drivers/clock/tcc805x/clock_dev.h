/*
***************************************************************************************************
*
*   FileName : clock_dev.h
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

#ifndef MCU_BSP_CLOCK_DEV_HEADER
#define MCU_BSP_CLOCK_DEV_HEADER

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define CLOCK_XIN_CLK_RATE              (24000000UL)    // 24MHz
#define CLOCK_XTIN_CLK_RATE             (32768UL)   // 32.768kHz

/* PLL channel index */
typedef enum CLOCK_PLL
{
    CLOCK_PLL_MICOM_0                   = 0,
    CLOCK_PLL_MICOM_1                   = 1
} CLOCKPll_t;

/* MICOM pll source channel index */
typedef enum CLOCK_M_PLL
{
    CLOCK_MPLL_0                        = 0,
    CLOCK_MPLL_1                        = 1,
    CLOCK_MPLL_DIV_0                    = 2,
    CLOCK_MPLL_DIV_1                    = 3,
    CLOCK_MPLL_XIN                      = 4
}CLOCKMpll_t;

typedef enum CLOCK_PCLK_CTRL_SEL
{
    CLOCK_PCLKCTRL_SEL_PLL0             = 0,
    CLOCK_PCLKCTRL_SEL_PLL1             = 1,
    CLOCK_PCLKCTRL_SEL_XIN              = 5,
    CLOCK_PCLKCTRL_SEL_PLL0DIV          = 10,
    CLOCK_PCLKCTRL_SEL_PLL1DIV          = 11,
    CLOCK_PCLKCTRL_SEL_XINDIV           = 23
}CLOCKPclkctrlSel_t;

/* Peripheral Clocks */
typedef enum CLOCK_PERI
{/* Peri. Name */
    // MICOM Peri
    CLOCK_PERI_SFMC                     = 0,
    CLOCK_PERI_CAN0                     = 1,
    CLOCK_PERI_CAN1                     = 2,
    CLOCK_PERI_CAN2                     = 3,
    CLOCK_PERI_GPSB0                    = 4,
    CLOCK_PERI_GPSB1                    = 5,
    CLOCK_PERI_GPSB2                    = 6,
    CLOCK_PERI_GPSB3                    = 7,
    CLOCK_PERI_GPSB4                    = 8,
    CLOCK_PERI_GPSB5                    = 9,
    CLOCK_PERI_UART0                    = 10,
    CLOCK_PERI_UART1                    = 11,
    CLOCK_PERI_UART2                    = 12,
    CLOCK_PERI_UART3                    = 13,
    CLOCK_PERI_UART4                    = 14,
    CLOCK_PERI_UART5                    = 15,
    CLOCK_PERI_I2C0                     = 16,
    CLOCK_PERI_I2C1                     = 17,
    CLOCK_PERI_I2C2                     = 18,
    CLOCK_PERI_PWM0                     = 19,
    CLOCK_PERI_PWM1                     = 20,
    CLOCK_PERI_PWM2                     = 21,
    CLOCK_PERI_ICTC0                    = 22,
    CLOCK_PERI_ICTC1                    = 23,
    CLOCK_PERI_ICTC2                    = 24,
    CLOCK_PERI_ADC                      = 25,
    CLOCK_PERI_TIMER0                   = 26,
    CLOCK_PERI_TIMER1                   = 27,
    CLOCK_PERI_TIMER2                   = 28,
    CLOCK_PERI_TIMER3                   = 29,
    CLOCK_PERI_TIMER4                   = 30,
    CLOCK_PERI_TIMER5                   = 31,
    CLOCK_PERI_MAX                      = 32
}CLOCKPeri_t;

/* I/O Bus pwdn/swreset */
typedef enum CLOCK_IO_BUS
{
    CLOCK_IOBUS_NSAID_FILTER_BUS        = 0,
    CLOCK_IOBUS_NSAID_FILTER_CORE       = 1,
    CLOCK_IOBUS_NSAID_CFG_BUS           = 2,
    CLOCK_IOBUS_SGID_FILTER_BUS         = 3,
    CLOCK_IOBUS_SGID_FILTER_CORE        = 4,
    CLOCK_IOBUS_SGID_CFG_BUS            = 5,
    CLOCK_IOBUS_SFMC_BUS                = 6,
    CLOCK_IOBUS_SFMC_MID_FILTER_BUS     = 7,
    CLOCK_IOBUS_SFMC_MID_CFG_BUS        = 8,
    CLOCK_IOBUS_SFMC_BRIDGE             = 9,
    CLOCK_IOBUS_SFMC                    = 10,

    CLOCK_IOBUS_GIC                     = 15,
    CLOCK_IOBUS_GIC_BRIDGE              = 16,
    CLOCK_IOBUS_GPIO_BUS                = 17,
    CLOCK_IOBUS_SOC400                  = 18,
    CLOCK_IOBUS_JTAG                    = 19,
    CLOCK_IOBUS_DMA_GRP                 = 20,
    CLOCK_IOBUS_CAN_GRP                 = 21,
    CLOCK_IOBUS_CAN0                    = 22,
    CLOCK_IOBUS_CAN1                    = 23,
    CLOCK_IOBUS_CAN2                    = 24,
    CLOCK_IOBUS_CAN_BUS                 = 25,

    CLOCK_IOBUS_UART_GRP                = 32,
    CLOCK_IOBUS_UART0                   = 33,
    CLOCK_IOBUS_UART1                   = 34,
    CLOCK_IOBUS_UART2                   = 35,
    CLOCK_IOBUS_UART3                   = 36,
    CLOCK_IOBUS_UART4                   = 37,
    CLOCK_IOBUS_UART5                   = 38,
    CLOCK_IOBUS_UDMA0                   = 39,
    CLOCK_IOBUS_UDMA1                   = 40,
    CLOCK_IOBUS_UDMA2                   = 41,
    CLOCK_IOBUS_UDMA3                   = 42,
    CLOCK_IOBUS_UDMA4                   = 43,
    CLOCK_IOBUS_UDMA5                   = 44,
    CLOCK_IOBUS_UART_BUS                = 45,
    CLOCK_IOBUS_I2C_GRP                 = 46,
    CLOCK_IOBUS_I2C0                    = 47,
    CLOCK_IOBUS_I2C1                    = 48,
    CLOCK_IOBUS_I2C2                    = 49,
    CLOCK_IOBUS_I2C_M_PORTCFG           = 50,
    CLOCK_IOBUS_PWM_GRP                 = 51,
    CLOCK_IOBUS_PWM0                    = 52,
    CLOCK_IOBUS_PWM1                    = 53,
    CLOCK_IOBUS_PWM2                    = 54,
    CLOCK_IOBUS_PWM_BUS                 = 55,
    CLOCK_IOBUS_ICTC_GRP                = 56,
    CLOCK_IOBUS_ICTC0                   = 57,
    CLOCK_IOBUS_ICTC1                   = 58,
    CLOCK_IOBUS_ICTC2                   = 59,
    CLOCK_IOBUS_ADC                     = 60,
    CLOCK_IOBUS_MAILBOX                 = 61,

    CLOCK_IOBUS_TIMER_GRP               = 64,
    CLOCK_IOBUS_TIMER0                  = 65,
    CLOCK_IOBUS_TIMER1                  = 66,
    CLOCK_IOBUS_TIMER2                  = 67,
    CLOCK_IOBUS_TIMER3                  = 68,
    CLOCK_IOBUS_TIMER4                  = 69,
    CLOCK_IOBUS_TIMER5                  = 70,
    CLOCK_IOBUS_WDT                     = 71,
    CLOCK_IOBUS_AXI_BRIGE_M             = 72,
    CLOCK_IOBUS_AXI_BRIGE_S             = 73,
    CLOCK_IOBUS_GPSB_GRP                = 74,
    CLOCK_IOBUS_GPSB0                   = 75,
    CLOCK_IOBUS_GPSB1                   = 76,
    CLOCK_IOBUS_GPSB2                   = 77,
    CLOCK_IOBUS_GPSB3                   = 78,
    CLOCK_IOBUS_GPSB4                   = 79,
    CLOCK_IOBUS_GPSB5                   = 80,
    CLOCK_IOBUS_GPSB_BUS                = 81,
    CLOCK_IOBUS_GPSB_M                  = 82,
    CLOCK_IOBUS_CMU                     = 83,
    CLOCK_IOBUS_SYSSM                   = 84,

    CLOCK_IOBUS_MAX                     = 85
}CLOCKIobus_t;

typedef struct CLOCK_PMS
{
    uint32  uiFpll;
    uint32  uiEn;
    uint32  uiP;
    uint32  uiM;
    uint32  uiS;
    uint32  uiSrc;
} CLOCKPms_t;

typedef struct CLOCK_CLK_CTRL
{
    uint32  uiFreq;
    uint32  uiEn;
    uint32  uiConf;
    uint32  uiSel;
} CLOCKClkCtrl_t;

typedef struct CLOCK_PCLK_CTRL
{
    uint32  uiPeriName;
    uint32  uiFreq;
    uint32  uiMd;
    uint32  uiEn;
    uint32  uiSel;
    uint32  uiDivVal;
} CLOCKPclkCtrl_t;

#endif /* __MCU_BSP_CLOCK_DEV_HEADER__ */

