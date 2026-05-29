/*
***************************************************************************************************
*
*   FileName : gic_enum.h
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

#if !defined(MCU_BSP_GIC_ENUM_HEADER)
#define MCU_BSP_GIC_ENUM_HEADER




/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/




#define GIC_INT_TYPE_LEVEL_HIGH         (0x1U)
#define GIC_INT_TYPE_LEVEL_LOW          (0x2U)
#define GIC_INT_TYPE_EDGE_RISING        (0x4U)
#define GIC_INT_TYPE_EDGE_FALLING       (0x8U)
#define GIC_INT_TYPE_EDGE_BOTH          (GIC_INT_TYPE_EDGE_RISING | GIC_INT_TYPE_EDGE_FALLING)

#define GIC_PRIORITY_NO_MEAN            (16UL)

#define GIC_PPI_START                   (16UL)
#define GIC_SPI_START                   (32UL)

#define GIC_EINT_START_INT              (GIC_EXT0)
#define GIC_EINT_END_INT                (GIC_EXT9)
#define GIC_EINT_NUM                    ((uint32)GIC_EXTn0 - (uint32)GIC_EXT0)

#define GIC_SMU_CB_WDT0                 (GIC_SPI_START + 286UL)
#define GIC_SMU_PMU_WDT                 (GIC_SPI_START + 294UL)
#define GIC_SMU_HSMB_MBOX3_M            (GIC_SPI_START + 414UL)
#define GIC_SMU_HSMB_MBOX3_M_TXDONE     (GIC_SPI_START + 442UL)
#define GIC_INT_SRC_CNT                 (GIC_SPI_START + 480UL)



enum
{
    GIC_CAN0_0                          = (GIC_SPI_START + 0UL),
    GIC_CAN0_1                          = (GIC_SPI_START + 1UL),
    GIC_CAN1_0                          = (GIC_SPI_START + 2UL),
    GIC_CAN1_1                          = (GIC_SPI_START + 3UL),
    GIC_CAN2_0                          = (GIC_SPI_START + 4UL),
    GIC_CAN2_1                          = (GIC_SPI_START + 5UL),
    GIC_GPSB                            = (GIC_SPI_START + 6UL),
    GIC_GPSB1                           = (GIC_SPI_START + 7UL),
    GIC_GPSB2                           = (GIC_SPI_START + 8UL),
    GIC_GPSB3                           = (GIC_SPI_START + 9UL),
    GIC_GPSB4                           = (GIC_SPI_START + 10UL),
    GIC_GPSB5                           = (GIC_SPI_START + 11UL),
    GIC_GPSB0_DMA                       = (GIC_SPI_START + 12UL),
    GIC_GPSB1_DMA                       = (GIC_SPI_START + 13UL),
    GIC_GPSB2_DMA                       = (GIC_SPI_START + 14UL),
    GIC_GPSB3_DMA                       = (GIC_SPI_START + 15UL),
    GIC_GPSB4_DMA                       = (GIC_SPI_START + 16UL),
    GIC_GPSB5_DMA                       = (GIC_SPI_START + 17UL),
    GIC_UART0                           = (GIC_SPI_START + 18UL),
    GIC_UART1                           = (GIC_SPI_START + 19UL),
    GIC_UART2                           = (GIC_SPI_START + 20UL),
    GIC_UART3                           = (GIC_SPI_START + 21UL),
    GIC_UART4                           = (GIC_SPI_START + 22UL),
    GIC_UART5                           = (GIC_SPI_START + 23UL),
    GIC_UT_DMA0                         = (GIC_SPI_START + 24UL),
    GIC_UT_DMA1                         = (GIC_SPI_START + 25UL),
    GIC_UT_DMA2                         = (GIC_SPI_START + 26UL),
    GIC_UT_DMA3                         = (GIC_SPI_START + 27UL),
    GIC_UT_DMA4                         = (GIC_SPI_START + 28UL),
    GIC_UT_DMA5                         = (GIC_SPI_START + 29UL),
    GIC_I2C                             = (GIC_SPI_START + 30UL),
    GIC_I2C1                            = (GIC_SPI_START + 31UL),
    GIC_I2C2                            = (GIC_SPI_START + 32UL),
    GIC_VS_I2C0                         = (GIC_SPI_START + 33UL),
    GIC_VS_I2C1                         = (GIC_SPI_START + 34UL),
    GIC_VS_I2C2                         = (GIC_SPI_START + 35UL),
    GIC_IC_TC0                          = (GIC_SPI_START + 36UL),
    GIC_IC_TC1                          = (GIC_SPI_START + 37UL),
    GIC_IC_TC2                          = (GIC_SPI_START + 38UL),
    GIC_RED_IC_TC0                      = (GIC_SPI_START + 39UL),
    GIC_RED_IC_TC1                      = (GIC_SPI_START + 40UL),
    GIC_RED_IC_TC2                      = (GIC_SPI_START + 41UL),
    GIC_DMA_PL080                       = (GIC_SPI_START + 42UL),
    GIC_ADC                             = (GIC_SPI_START + 43UL),
    GIC_TIMER_0                         = (GIC_SPI_START + 44UL),
    GIC_TIMER_1                         = (GIC_SPI_START + 45UL),
    GIC_TIMER_2                         = (GIC_SPI_START + 46UL),
    GIC_TIMER_3                         = (GIC_SPI_START + 47UL),
    GIC_TIMER_4                         = (GIC_SPI_START + 48UL),
    GIC_TIMER_5                         = (GIC_SPI_START + 49UL),
    GIC_WATCHDOG                        = (GIC_SPI_START + 50UL),
    GIC_DEFAULT_SLV_ERR                 = (GIC_SPI_START + 51UL),
    GIC_MBOX0_SLV_S_W                   = (GIC_SPI_START + 52UL),
    GIC_MBOX0_SLV_S_R                   = (GIC_SPI_START + 53UL),
    GIC_MBOX0_SLV_NS_W                  = (GIC_SPI_START + 54UL),
    GIC_MBOX0_SLV_NS_R                  = (GIC_SPI_START + 55UL),
    GIC_MBOX1_SLV_S_W                   = (GIC_SPI_START + 56UL),
    GIC_MBOX1_SLV_S_R                   = (GIC_SPI_START + 57UL),
    GIC_MBOX1_SLV_NS_W                  = (GIC_SPI_START + 58UL),
    GIC_MBOX1_SLV_NS_R                  = (GIC_SPI_START + 59UL),
    GIC_MBOX2_SLV_W                     = (GIC_SPI_START + 60UL),
    GIC_MBOX2_SLV_R                     = (GIC_SPI_START + 61UL),

    GIC_MBOX0_MST_S_W                   = (GIC_SPI_START + 62UL),
    GIC_MBOX0_MST_S_R                   = (GIC_SPI_START + 63UL),
    GIC_MBOX0_MST_NS_W                  = (GIC_SPI_START + 64UL),
    GIC_MBOX0_MST_NS_R                  = (GIC_SPI_START + 65UL),
    GIC_MBOX1_MST_S_W                   = (GIC_SPI_START + 66UL),
    GIC_MBOX1_MST_S_R                   = (GIC_SPI_START + 67UL),
    GIC_MBOX1_MST_NS_W                  = (GIC_SPI_START + 68UL),
    GIC_MBOX1_MST_NS_R                  = (GIC_SPI_START + 69UL),
    GIC_MBOX2_MST_W                     = (GIC_SPI_START + 70UL),
    GIC_MBOX2_MST_R                     = (GIC_SPI_START + 71UL),
    GIC_SFMC                            = (GIC_SPI_START + 72UL),
    GIC_NSAID_FILTER                    = (GIC_SPI_START + 73UL),
    GIC_SGID_FILTER                     = (GIC_SPI_START + 74UL),

    GIC_SFMC_MID_FILTER                 = (GIC_SPI_START + 75UL),
    GIC_IMC_MID_FILTER                  = (GIC_SPI_START + 76UL),
    GIC_EXT0                            = (GIC_SPI_START + 77UL),
    GIC_EXT1                            = (GIC_SPI_START + 78UL),
    GIC_EXT2                            = (GIC_SPI_START + 79UL),
    GIC_EXT3                            = (GIC_SPI_START + 80UL),
    GIC_EXT4                            = (GIC_SPI_START + 81UL),
    GIC_EXT5                            = (GIC_SPI_START + 82UL),
    GIC_EXT6                            = (GIC_SPI_START + 83UL),
    GIC_EXT7                            = (GIC_SPI_START + 84UL),
    GIC_EXT8                            = (GIC_SPI_START + 85UL),
    GIC_EXT9                            = (GIC_SPI_START + 86UL),
    GIC_EXTn0                           = (GIC_SPI_START + 87UL),
    GIC_EXTn1                           = (GIC_SPI_START + 88UL),
    GIC_EXTn2                           = (GIC_SPI_START + 89UL),
    GIC_EXTn3                           = (GIC_SPI_START + 90UL),
    GIC_EXTn4                           = (GIC_SPI_START + 91UL),
    GIC_EXTn5                           = (GIC_SPI_START + 92UL),
    GIC_EXTn6                           = (GIC_SPI_START + 93UL),
    GIC_EXTn7                           = (GIC_SPI_START + 94UL),
    GIC_EXTn8                           = (GIC_SPI_START + 95UL),
    GIC_EXTn9                           = (GIC_SPI_START + 96UL),
    GIC_CR5_PMU                         = (GIC_SPI_START + 97UL),
    GIC_FMU_IRQ                         = (GIC_SPI_START + 98UL),
    GIC_FMU_FIQ                         = (GIC_SPI_START + 99UL),
    GIC_RESERVED_100                    = (GIC_SPI_START + 100UL),
    GIC_RESERVED_101                    = (GIC_SPI_START + 101UL),
    GIC_RESERVED_102                    = (GIC_SPI_START + 102UL),
    GIC_PMGPIO                          = (GIC_SPI_START + 103UL),

    GIC_RTC                             = (GIC_SPI_START + 177UL), /*AP INTR*/
    GIC_STRGB_WDT                       = (GIC_SPI_START + 431UL), /*SC WDT*/
};

#endif

