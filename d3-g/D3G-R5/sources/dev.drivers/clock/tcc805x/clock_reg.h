/*
***************************************************************************************************
*
*   FileName : clock_reg.h
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

#ifndef MCU_BSP_CLOCK_REG_HEADER
#define MCU_BSP_CLOCK_REG_HEADER

#define CLOCK_BASE_ADDR                 (MCU_BSP_CKC_BASE)

#define CLOCK_PLL_MAX_NUM               (2)
#define CLOCK_SRC_MAX_NUM               (6) //  ((CLOCK_PLL_MAX_NUM * 2 ) + 2)

/*
 * CKC Register Offsets
 */
#define CLOCK_CKC_CLKCTRL               (0x000)
#define CLOCK_CKC_PLLPMS                (0x040)
#define CLOCK_CKC_PLLCON                (0x060)
#define CLOCK_CKC_PLLMON                (0x080)
#define CLOCK_CKC_CLKDIVC               (0x0A0)
#define CLOCK_CKC_SWRESET               (0x0B0)
#define CLOCK_CKC_PCLKCTRL              (0x0D0)

// Dedicated CLKCTRL Register Offsets
#define CLOCK_DCKC_CLKCTRL              (0x000)
#define CLOCK_DCKC_CLKCTRL2             (0x004)
#define CLOCK_DCKC_PLLPMS               (0x008)
#define CLOCK_DCKC_PLLCON               (0x00C)
#define CLOCK_DCKC_PLLMON               (0x010)
#define CLOCK_DCKC_PLLDIVC              (0x014)

// MICOM Register Offsets
#define CLOCK_MCKC_PLL0PMS              (0x000)
#define CLOCK_MCKC_PLL1PMS              (0x00C)
#define CLOCK_MCKC_CLKDIVC              (0x018)
#define CLOCK_MCKC_CLKCTRL              (0x01C)
#define CLOCK_MCKC_PCLKCTRL             (0x024)

// MICOM Subsystem Register Offsets
#define CLOCK_MCKC_HCLK0                (0x000)
#define CLOCK_MCKC_HCLK1                (0x004)
#define CLOCK_MCKC_HCLK2                (0x008)
#define CLOCK_MCKC_HCLKSWR0             (0x00C)
#define CLOCK_MCKC_HCLKSWR1             (0x010)
#define CLOCK_MCKC_HCLKSWR2             (0x014)

/*
 * MICOM CLKCTRL Configuration Register
 */
#define CLOCK_MCLKCTRL_SEL_MIN          (0)
#define CLOCK_MCLKCTRL_SEL_MAX          (7)
#define CLOCK_MCLKCTRL_SEL_SHIFT        (0)
#define CLOCK_MCLKCTRL_SEL_MASK         (0x7)
#define CLOCK_MCLKCTRL_CONFIG_MIN       (1)
#define CLOCK_MCLKCTRL_CONFIG_MAX       (15)
#define CLOCK_MCLKCTRL_CONFIG_SHIFT     (5)
#define CLOCK_MCLKCTRL_CONFIG_MASK      (0xF)
#define CLOCK_MCLKCTRL_EN_SHIFT         (22)  // Caution : Do NOT change this value to LOW.
#define CLOCK_MCLKCTRL_DIVSTS_SHIFT     (29)
#define CLOCK_MCLKCTRL_CLKCHG_SHIFT     (31)

/*
 * PLL Configuration Register
 */
#define CLOCK_PLL_MAX_RATE              (3200000000)    // Max. 3200MHz
#define CLOCK_PLL_MIN_RATE              (25000000)  // Min.   25MHz
#define CLOCK_PLL_VCO_MAX               (3200000000)    // Max. 3200MHz
#define CLOCK_PLL_VCO_MIN               (1600000000)    // Min. 1600MHz
#define CLOCK_PLL_P_MAX                 (6)   // 63   FREF = FIN/p  (4MHz ~ 12MHz)
#define CLOCK_PLL_P_MIN                 (2)  // 1    FREF = FIN/p  (4MHz ~ 12MHz)
#define CLOCK_PLL_P_SHIFT               (0)
#define CLOCK_PLL_P_MASK                (0x3F)
#define CLOCK_PLL_M_MAX                 (1023)
#define CLOCK_PLL_M_MIN                 (64)
#define CLOCK_PLL_M_SHIFT               (6)
#define CLOCK_PLL_M_MASK                (0x3FF)
#define CLOCK_PLL_S_MAX                 (6)
#define CLOCK_PLL_S_MIN                 (0)
#define CLOCK_PLL_S_SHIFT               (16)
#define CLOCK_PLL_S_MASK                (0x7)
#define CLOCK_PLL_SRC_SHIFT             (19)
#define CLOCK_PLL_SRC_MASK              (0x3)
#define CLOCK_PLL_BYPASS_SHIFT          (21)
#define CLOCK_PLL_LOCKST_SHIFT          (23)
#define CLOCK_PLL_CHGPUMP_SHIFT         (24)
#define CLOCK_PLL_CHGPUMP_MASK          (0x3)
#define CLOCK_PLL_LOCKEN_SHIFT          (26)
#define CLOCK_PLL_RSEL_SHIFT            (27)
#define CLOCK_PLL_RSEL_MASK             (0xF)
#define CLOCK_PLL_EN_SHIFT              (31)

/*
 * PCLKCTRL Configuration Register
 */
#define CLOCK_PCLKCTRL_MAX_FCKS         (1600000000)
#define CLOCK_PCLKCTRL_DIV_MIN          (0)
#define CLOCK_PCLKCTRL_DIV_DCO_MIN      (1)
#define CLOCK_PCLKCTRL_DIV_SHIFT        (0)
#define CLOCK_PCLKCTRL_DIV_XXX_MAX      (0xFFF)
#define CLOCK_PCLKCTRL_DIV_XXX_MASK     (CLOCK_PCLKCTRL_DIV_XXX_MAX)
#define CLOCK_PCLKCTRL_DIV_YYY_MAX      (0xFFFFFF)
#define CLOCK_PCLKCTRL_DIV_YYY_MASK     (CLOCK_PCLKCTRL_DIV_YYY_MAX)
#define CLOCK_PCLKCTRL_SEL_MIN          (0)
#define CLOCK_PCLKCTRL_SEL_MAX          (26)
#define CLOCK_PCLKCTRL_SEL_SHIFT        (24)
#define CLOCK_PCLKCTRL_SEL_MASK         (0x1F)
#define CLOCK_PCLKCTRL_EN_SHIFT         (29)
#define CLOCK_PCLKCTRL_OUTEN_SHIFT      (30)
#define CLOCK_PCLKCTRL_MD_SHIFT         (31)

/* CLKCTRL SEL */
typedef enum CLOCK_M_CLK_CTRL_SEL
{
    CLOCK_MCLKCTRL_SEL_XIN              = 0,
    CLOCK_MCLKCTRL_SEL_PLL0             = 1,
    CLOCK_MCLKCTRL_SEL_PLL1             = 2,
    CLOCK_MCLKCTRL_SEL_XINDIV           = 3,
    CLOCK_MCLKCTRL_SEL_PLL0DIV          = 4,
    CLOCK_MCLKCTRL_SEL_PLL1DIV          = 5,
    CLOCK_MCLKCTRL_SEL_EXTIN0           = 6,
    CLOCK_MCLKCTRL_SEL_EXTIN1           = 7
} CLOCKMclkCtrlSel_t;

/* PCLK Type */
typedef enum CLOCK_PCLK_CTRL_TYPE
{
    CLOCK_PCLKCTRL_TYPE_XXX             = 0,
    CLOCK_PCLKCTRL_TYPE_YYY             = 1,
    CLOCK_PCLKCTRL_TYPE_MAX             = 2
} CLOCKPclkCtrlType_t;

/* PCLK Mode Selection */
typedef enum CLOCK_PCLK_CTRL_MODE
{
    CLOCK_PCLKCTRL_MODE_DCO             = 0,
    CLOCK_PCLKCTRL_MODE_DIVIDER         = 1,
    CLOCK_PCLKCTRL_MODE_MAX             = 2
} CLOCKPclkCtrlMode_t;

typedef enum CLOCK_M_PCLK_CTRL_SEL
{
    CLOCK_MPCLKCTRL_SEL_PLL0            = 0,
    CLOCK_MPCLKCTRL_SEL_PLL1            = 1,
    CLOCK_MPCLKCTRL_SEL_XIN             = 5,
    CLOCK_MPCLKCTRL_SEL_PLL0DIV         = 10,
    CLOCK_MPCLKCTRL_SEL_PLL1DIV         = 11,
    CLOCK_MPCLKCTRL_SEL_XINDIV          = 23
} CLOCKMpclkCtrlSel_t;

#endif /* __MCU_BSP__CLOCK_REG_HEADER__ */

