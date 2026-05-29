/*
***************************************************************************************************
*
*   FileName : bootctrl.h
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
#define BCTRL_TASK_STK_SIZE             (256)
#define BCTRL_COMMAND_READY             (0x00U)
#define BCTRL_COMMAND_GO                (0x01U)

#define BCTRL_CORE_A72                  (0x00U)
#define BCTRL_CORE_A53                  (0x01U)
#define BCTRL_CORE_NUM                  (0x02U)

#define BCTRL_TEST_SIGNAL0              (0x00U)
#define BCTRL_TEST_SIGNAL1              (0x01U)

#define BCTRL_ID_BOOT_SEQUENCE          (0U)

typedef enum
{
    BCTRL_STAT_READY,
    BCTRL_STAT_GO,
    BCTRL_STAT_WAIT 
} BCTRLStatEnum_t;

void BCTRL_Init
(
    void
);
