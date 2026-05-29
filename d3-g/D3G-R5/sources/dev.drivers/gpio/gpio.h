/*
***************************************************************************************************
*
*   FileName : gpio.h
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

#ifndef MCU_BSP_GPIO_HEADER
#define MCU_BSP_GPIO_HEADER

#include <sal_internal.h>

/*
***************************************************************************************************
*                                            GLOBAL DEFINITIONS
***************************************************************************************************
*/

#if (DEBUG_ENABLE)
    #include "debug.h"

    #define GPIO_D(fmt,args...)         {LOGD(DBG_TAG_GPIO, fmt, ## args)}
    #define GPIO_E(fmt,args...)         {LOGE(DBG_TAG_GPIO, fmt, ## args)}
#else
    #define GPIO_D(fmt,args...)
    #define GPIO_E(fmt,args...)
#endif


/*
 * gpio cfg structures
 *   [31:14]: reserved
 *   [13]   : slew rate
 *   [12]   : input type
 *   [11:10]: input buffer
 *   [9]    : direction
 *   [8:6]  : driver strength (0~3)
 *   [5:4]  : pull up/down
 *   [3:0]  : function selection (0~15)
 */

#define GPIO_SLEW_RATE_SHIFT            (13)
#define GPIO_SLEW_RATE_SLOW             (1UL << (uint32)GPIO_SLEW_RATE_SHIFT)
#define GPIO_SLEW_RATE_FAST             (0UL << (uint32)GPIO_SLEW_RATE_SHIFT)

#define GPIO_INPUT_TYPE_SHIFT           (12)
#define GPIO_INPUT_SCHMITT              (1UL << (uint32)GPIO_INPUT_TYPE_SHIFT)
#define GPIO_INPUT_CMOS                 (0UL << (uint32)GPIO_INPUT_TYPE_SHIFT)

#define GPIO_INPUTBUF_SHIFT             (10)
#define GPIO_INPUTBUF_MASK              (0x3UL)
#define GPIO_INPUTBUF_EN                ((2UL | 1UL) << (uint32)GPIO_INPUTBUF_SHIFT)
#define GPIO_INPUTBUF_DIS               ((2UL | 0UL) << (uint32)GPIO_INPUTBUF_SHIFT)

#define GPIO_OUTPUT_SHIFT               (9)
#define GPIO_OUTPUT                     (1UL << (uint32)GPIO_OUTPUT_SHIFT)
#define GPIO_INPUT                      (0UL << (uint32)GPIO_OUTPUT_SHIFT)

#define GPIO_CD_SHIFT                   (6)
#define GPIO_CD_MASK                    (0x7UL)
#define GPIO_CD(x)                      ((((x) & (uint32)GPIO_CD_MASK) | 0x4UL) << (uint32)GPIO_CD_SHIFT)

#define GPIO_PULL_SHIFT                 (4)
#define GPIO_PULL_MASK                  (0x3UL)
#define GPIO_NOPULL                     (0UL << (uint32)GPIO_PULL_SHIFT)
#define GPIO_PULLUP                     (1UL << (uint32)GPIO_PULL_SHIFT)
#define GPIO_PULLDN                     (2UL << (uint32)GPIO_PULL_SHIFT)

// [todo] update
#define GPIO_DS(x)                      ((((x) & (uint32)GPIO_CD_MASK) | 0x4UL) << (uint32)GPIO_CD_SHIFT)
//~

#define GPIO_FUNC_MASK                  (0xFUL)
#define GPIO_FUNC(x)                    ((x) & (uint32)GPIO_FUNC_MASK)


/*
 * gpio port & pin structures
 *   [31:10]: reserved
 *   [9:5] : port (A,B,C,...)
 *   [4:0] : pin number (0~31)
 */

#define GPIO_PIN_MASK                   (0x1FUL)
#define GPIO_PIN_NUM_MASK               (0x3FUL) // original 1FUL , avoid code sonar warning


#define GPIO_PORT_SHIFT                 (5)
#define GPIO_PORT_MASK                  ((uint32)0x1F << (uint32)GPIO_PORT_SHIFT)

//(n<<GPIO_PORT_SHIFT)                  n = ofset/0x40
#define GPIO_PORT_A                     ((uint32)0 << (uint32)GPIO_PORT_SHIFT)        // offset: 0x000
#define GPIO_PORT_B                     ((uint32)1 << (uint32)GPIO_PORT_SHIFT)        // offset: 0x040
#define GPIO_PORT_C                     ((uint32)2 << (uint32)GPIO_PORT_SHIFT)        // offset: 0x080
//#define GPIO_PORT_D                   ((uint32)3 << (uint32)GPIO_PORT_SHIFT)        // offset: 0x0c0
#define GPIO_PORT_E                     ((uint32)4 << (uint32)GPIO_PORT_SHIFT)        // offset: 0x100
//#define GPIO_PORT_F                   ((uint32)5 << (uint32)GPIO_PORT_SHIFT)        // offset: 0x140
#define GPIO_PORT_G                     ((uint32)6 << (uint32)GPIO_PORT_SHIFT)        // offset: 0x180
//#define GPIO_PORT_SD                  ((uint32)9 << (uint32)GPIO_PORT_SHIFT)        // offset: 0x240
#define GPIO_PORT_H                     ((uint32)25 << (uint32)GPIO_PORT_SHIFT)       // offset: 0x640
#define GPIO_PORT_K                     ((uint32)26 << (uint32)GPIO_PORT_SHIFT)       // offset: 0x680
#define GPIO_PORT_MA                    ((uint32)27 << (uint32)GPIO_PORT_SHIFT)       // offset: 0x6c0
#define GPIO_PORT_MB                    ((uint32)28 << (uint32)GPIO_PORT_SHIFT)       // offset: 0x700
#define GPIO_PORT_MC                    ((uint32)29 << (uint32)GPIO_PORT_SHIFT)       // offset: 0x740
#define GPIO_PORT_MD                    ((uint32)30 << (uint32)GPIO_PORT_SHIFT)       // offset: 0x780

#define GPIO_GPA(x)                     (GPIO_PORT_A | ((x) & (uint32)0x1F))
#define GPIO_GPB(x)                     (GPIO_PORT_B | ((x) & (uint32)0x1F))
#define GPIO_GPC(x)                     (GPIO_PORT_C | ((x) & (uint32)0x1F))
//#define TCC_GPD(x)                    (GPIO_PORT_D | ((x) & (uint32)0x1F))
#define GPIO_GPE(x)                     (GPIO_PORT_E | ((x) & (uint32)0x1F))
//#define TCC_GPF(x)                    (GPIO_PORT_F | ((x) & (uint32)0x1F))
#define GPIO_GPG(x)                     (GPIO_PORT_G | ((x) & (uint32)0x1F))
#define GPIO_GPH(x)                     (GPIO_PORT_H | ((x) & (uint32)0x1F))
#define GPIO_GPK(x)                     (GPIO_PORT_K | ((x) & (uint32)0x1F))
//#define TCC_GPSD(x)                   (GPIO_PORT_SD | ((x) & (uint32)0x1F))
#define GPIO_GPMA(x)                    (GPIO_PORT_MA | ((x) & (uint32)0x1F))
#define GPIO_GPMB(x)                    (GPIO_PORT_MB | ((x) & (uint32)0x1F))
#define GPIO_GPMC(x)                    (GPIO_PORT_MC | ((x) & (uint32)0x1F))
#define GPIO_GPMD(x)                    (GPIO_PORT_MD | ((x) & (uint32)0x1F))


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          init_fmu
*
*
* @param    uiPort [in]
* @param    uiConfig [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t GPIO_Config
(
    uint32                              uiPort,
    uint32                              uiConfig
);

/*
***************************************************************************************************
*                                          GPIO_Get
*
*
* @param    uiPort [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
uint8 GPIO_Get
(
    uint32                              uiPort
);

/*
***************************************************************************************************
*                                          GPIO_Set
*
*
* @param    uiPort [in]
* @param    uiData [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t GPIO_Set
(
    uint32                              uiPort,
    uint32                              uiData
);

/*
***************************************************************************************************
*                                          GPIO_ToNum
*
*
* @param    uiPort [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
uint32 GPIO_ToNum
(
    uint32                              uiPort
);

#endif /* __BSP_GPIO__H__ */

