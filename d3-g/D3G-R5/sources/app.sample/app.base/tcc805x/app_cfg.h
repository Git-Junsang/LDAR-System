/*
***************************************************************************************************
*
*   FileName : app_cfg.h
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

#ifndef APP_CFG_HEADER
#define APP_CFG_HEADER

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
/* TASK STACK SIZES : Size of the task stacks (# of WARD entries)                   */
#define ACFG_TASK_USER_STK_SIZE         (128U)
/* normal measn that task has no deep fucnction call or large local variable/array  */
#define ACFG_TASK_NORMAL_STK_SIZE       (128U)
/* medium measn that task has some fucnction call or small local variables/arrays   */
#define ACFG_TASK_MEDIUM_STK_SIZE       (256U)

/* Sample Application                                                               */
/* Key(ADC+Rotary) manager applicaiton                                              */
#define ACFG_APP_KEY_EN                 (0)
/* Console(uart) demo applicaiton                                                   */
#define ACFG_APP_CONSOLE_EN             (1)
/* F/W upgrade application                                                          */
#define ACFG_APP_FWUG_EN                (1)
/* System monitoring applicaiton                                                    */
#define ACFG_APP_SYSTEM_MONITORING_EN   (0)
/* CAN demo applicaiton                                                             */
#define ACFG_APP_CAN_DEMO_EN            (1)
/* Trust RVC application                                                            */
#define ACFG_APP_TRVC_EN                (0)
/* Reset demo applicaiton                                                           */
#ifdef MCU_BSP_SOC_REV_TCC805x_ES
#define ACFG_APP_RESET_DEMO_EN          (0)
#else /* CS */
#define ACFG_APP_RESET_DEMO_EN          (1)
#endif
/* MP TOOL */
#if defined(MPTOOL_EN)
#define ACFG_APP_MPTOOL_EN              (0)
#endif
/* AUTOMOTIVE_IVI_CLUSTER_DEMO */
#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
#define ACFG_APP_AUTOMOTIVE_IVI_CLUSTER_DEMO_EN     (1)
#endif
/* CAN_EDUCATION_DEMO */
#if defined(EDUCATION_CAN_DEMO_EN)
#define ACFG_APP_CAN_EDUCATION_DEMO_EN  (1)
#endif

/* Drvier Application                                                               */
/* Inter processor Communication service                                            */
#define ACFG_DRV_IPC_EN                 (1)

#define ACFG_DRV_SDM_EN                 (1)

#define ACFG_DRV_HSMMANAGER_EN          (1)

#endif  //n _APP_CFG_HEADER_

