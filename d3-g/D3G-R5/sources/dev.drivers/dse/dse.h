/*
***************************************************************************************************
*
*   FileName : dse.h
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

#if !defined(MCU_BSP_DSE_HEADER)
#define MCU_BSP_DSE_HEADER

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
                                                            /* Default Slave Error configuration Flag */
#define DSE_INTERCON                    (0x00000001UL)
#define DSE_AP_MBOX                     (0x00000002UL)
#define DSE_SYSPERI                     (0x00000004UL)
#define DSE_UART                        (0x00000008UL)
#define DSE_PDM                         (0x00000010UL)
#define DSE_MC_MBOX                     (0x00000020UL)
#define DSE_CAN                         (0x00000040UL)
#define DSE_GPSB                        (0x00000080UL)
#define DSE_ISOLATE                     (0x00000100UL)

#define DES_SEL_ALL                     (DSE_INTERCON | DSE_AP_MBOX | DSE_SYSPERI | DSE_UART | DSE_PDM | DSE_MC_MBOX | DSE_CAN | DSE_GPSB | DSE_ISOLATE)
#define DES_SEL_MASK                    (DES_SEL_ALL)

#define DSE_INT_MODE                    (0x00000000UL)      /* Default Slave Error interrupt mode     */
#define DSE_RES_MODE                    (0x10000000UL)      /* Default Slave Error response  mode     */
#define DSE_RES_INT_MODE                (0x20000000UL)      /* Default Slave Error int & res mode     */
#define DSE_MODE_MASK                   (0x30000000UL)

#define DSE_SM_IRQ_MASK                 (0x2UL)
#define DSE_SM_IRQ_EN                   (0x4UL)
#define DSE_SM_SLV_CFG                  (0x8UL)
#define DSE_SM_CFG_WR_PW                (0x10000UL)
#define DSE_SM_CFG_WR_LOCK              (0x20000UL)

#define DSE_SM_SF_CHK_GRP_SEL           (DSE_SM_IRQ_MASK | DSE_SM_IRQ_EN | DSE_SM_SLV_CFG | DSE_SM_CFG_WR_PW | DSE_SM_CFG_WR_LOCK)

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

extern uint32                           gWait_irq;          /* using by unittest                      */

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                       DSE_WriteLock
*
* Default Slave Error Register Write Protection Enable/Disable
*
* @param    uiLock [in] Write Protection 1 : Enable, 0 : Disable
*
* @return   The result of Write Protection Function setting
*           SAL_RET_SUCCESS  Write Protection register setting succeed
*           SAL_RET_FAILED   Write Protection register setting fail
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t DSE_WriteLock
(
    uint32                              uiLock
);

/*
***************************************************************************************************
*                                       DSE_Deinit
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void DSE_Deinit
(
    void
);

/*
***************************************************************************************************
*                                       DSE_Init
*
* Default Slave Error Controller Deinit
*
* @param    uiModeFlag  [in] Default Slave Error configuration Flag (refer to dse.h)
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t DSE_Init
(
    uint32                              uiModeFlag
);

/*
***************************************************************************************************
*                                       DSE_GetGrpSts
*
* Default Slave Error Controller Deinit
*
* @param    none
*
* @return   rSF_CHK_GRP_STS : Soft Fault Check Group Status Register value
*
* Notes
*
***************************************************************************************************
*/

uint32 DSE_GetGrpSts
(
    void
);

#endif

