/*
***************************************************************************************************
*
*   FileName : can_config.h
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

#ifndef CAN_CONFIG_HEADER
#define CAN_CONFIG_HEADER

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include "debug.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

/* Debug message control */
#if (DEBUG_ENABLE)
#define CAN_D(fmt, args...)             {LOGD(DBG_TAG_CAN, fmt, ## args)}
#define CAN_E(fmt, args...)             {LOGE(DBG_TAG_CAN, fmt, ## args)}
#else
#define CAN_D(fmt, args...)
#define CAN_E(fmt, args...)
#endif


#define CAN_CHANNEL_ADDR_OFFSET         (0x00010000UL)    /* 0x1B010000 */
#define CAN_CONFIG_ADDR                 (0x1B030000UL)
#define CAN_CONFIG_BASE_ADDR            (0x1B030004UL)
#define CAN_CONFIG_EXTS_CTRL0_ADDR      (0x1B030010UL)
#define CAN_CONFIG_EXTS_CTRL1_ADDR      (0x1B030014UL)
#define CAN_CONFIG_WR_PW_ADDR           (0x1B030020UL)
#define CAN_CONFIG_WR_LOCK_ADDR         (0x1B030024UL)

#define CAN_REG_PSR_PXE                 (14)
#define CAN_REG_PSR_RFDF                (13)
#define CAN_REG_PSR_RBRS                (12)
#define CAN_REG_PSR_RESI                (11)
#define CAN_REG_PSR_DLEC                (8)
#define CAN_REG_PSR_BO                  (7)
#define CAN_REG_PSR_EW                  (6)
#define CAN_REG_PSR_EP                  (5)
#define CAN_REG_PSR_ACT                 (3)
#define CAN_REG_PSR_LEC                 (0)


#define CAN_NON_CACHE_MEMORY_START      (0xC1000000UL) /* SRAM 1 */

#define CAN_SW_RESET_REG_0              (*(SALReg32 *)0x1B93000CUL)
#define CAN_BUS_CLK_MASK_REG_0          (*(SALReg32 *)0x1B930000UL)

#define CAN_CFG_REG_FIELD_ALL           (21UL)
#define CAN_CFG_REG_FIELD_CAN_0         (22UL)
#define CAN_CFG_REG_FIELD_CAN_1         (23UL)
#define CAN_CFG_REG_FIELD_CAN_2         (24UL)
#define CAN_CFG_REG_FIELD_CAN_CFG       (25UL)

#define CAN_CONTROLLER_NUMBER           (3U)
#define CAN_DATA_LENGTH_SIZE            (64U)

#define CAN_STANDARD_ID_FILTER_NUMBER_MAX   (128UL)
#define CAN_EXTENDED_ID_FILTER_NUMBER_MAX   (64UL)

#if defined(EDUCATION_CAN_DEMO_EN)
#define CAN_STANDARD_ID_FILTER_NUMBER   (5U)
#else
#define CAN_STANDARD_ID_FILTER_NUMBER   (6U)
#endif
#define CAN_EXTENDED_ID_FILTER_NUMBER   (6U)

#define CAN_RX_FIFO_0_MAX               (64UL)
#define CAN_RX_FIFO_1_MAX               (64UL)
#define CAN_RX_BUFFER_MAX               (64UL)
#define CAN_TX_EVENT_FIFO_MAX           (32UL)
#define CAN_TX_BUFFER_MAX               (32UL)

#define CAN_STANDARD_ID_FILTER_SIZE     (4U)
#define CAN_EXTENDED_ID_FILTER_SIZE     (8U)
#define CAN_BUFFER_SIZE                 (8U+CAN_DATA_LENGTH_SIZE)
#define CAN_TX_EVENT_SIZE               (8U)

#define CAN_CONTROLLER_CLOCK            (40000000)    /* 40MHz */


#define CAN_RX_MSG_RING_BUFFER_MAX      (192UL)

/* TimeStamp */
#define CAN_TIMESTAMP_PRESCALER         (15)  /* Prescaler = MAX(15) +1 */
#define CAN_TIMESTAMP_TYPE              (1)   /* 0 : Internal Timestamp, 1: External Timestamp */
#define CAN_TIMESTAMP_RATIO             (CAN_TIMESTAMP_PRESCALER)
#define CAN_TIMESTAMP_COMP              (0xFFFF)

/* TimerCount */
#define CAN_TIMEOUT_VALUE               (0xFFFF)  /* reset = 0xFFFF(MAX) */
#define CAN_TIMEOUT_TYPE                (0x01) /* 0x00:Continuous, 0x01:TxEventFIFO, 0x02:RxFIFO0, 0x03:RxFIFO1 */

/* Interrupt */
#define CAN_INTERRUPT_LINE              (0U) /* 0:Line0, 1:Line1 */

#define CAN_INTERRUPT_ENABLE            (0x3FFEFFFFUL)

#if (CAN_INTERRUPT_LINE == 0U)
    #define CAN_INTERRUPT_LINE_SEL      (0x0)
    #define CAN_INTERRUPT_LINE_ENABLE   (0x1)
#elif (CAN_INTERRUPT_LINE == 1U)
    #define CAN_INTERRUPT_LINE_SEL      (0x3FFFFFFFUL)
    #define CAN_INTERRUPT_LINE_ENABLE   (0x2)
#else
##### ERROR - Select CAN Interrupt Line #####
#endif


/* WatchDog */
#define CAN_WATCHDOG_VALUE              (0xFF)


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/


#endif // CAN_CONFIG_HEADER

