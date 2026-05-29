/*
***************************************************************************************************
*
*   FileName : bootctrl.c
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
#include <sal_internal.h>
#include <sal_com.h>
#include <mbox.h>
#include <bootctrl.h>
#include <debug.h>
#include <pmio.h>

static uint32 BCTRLTaskId[BCTRL_CORE_NUM];
static uint32 BCTRLTaskSt[BCTRL_CORE_NUM][BCTRL_TASK_STK_SIZE];
static uint32 BCTRLCmd[BCTRL_CORE_NUM][MBOX_CMD_FIFO_SIZE];
static BCTRLStatEnum_t BCTRLStat[BCTRL_CORE_NUM];

/*
***************************************************************************************************
*                                      <BCTRL_SignalDramInit>
*
* This function is allowed to be modified by customers.
*
* @param    uiCore  [in] A defined core number for Application Processor.
* @return   A signal that user defines.
*
***************************************************************************************************
*/
static uint32 BCTRL_SignalDramInit
(
    uint32                              uiCore
)
{
    uint32 ret;

    if (uiCore == BCTRL_CORE_A72)
    {
        ret = BCTRL_TEST_SIGNAL1;
    }
    else if (uiCore == BCTRL_CORE_A53)
    {
        ret = BCTRL_TEST_SIGNAL0;
    }
    else
    {
        ret = BCTRL_TEST_SIGNAL0;
    }

    return ret; 
}

/*
***************************************************************************************************
*                                      <BCTRL_CheckStatus>
*
* If you want to modify this function, you must let the telechips know how it is modified. 
*
* @param    uiCore  [in] A defined core number for Application Processor.
* @param    puiCmd  [in] A pointer of 8 word command (fixed length).
* @return   A status of Boot Sequence.
*
***************************************************************************************************
*/
static BCTRLStatEnum_t BCTRL_CheckStatus
(
    uint32                              uiCore,
    uint32*                             puiCmd
)
{
    BCTRLStatEnum_t ret;

    mcu_printf("[BCTRL] status 0x%02X\n", puiCmd[0]);
    if (puiCmd[1] == 0x00U) // ID=0: Reserved as Boot Sequece.
    {
        ret = BCTRL_STAT_READY;
    }
    else
    {
        ret = BCTRLStat[uiCore];
    }

    BCTRLStat[uiCore] = ret;
    return ret;
}
 
/*
***************************************************************************************************
*                                       <BCTRL_TaskRun>
*
* If you want to modify this function, you must let the telechips know how it is modified. 
*
* @param    uiCore    [in] A defined core number for Application Processor.
* @param    siCh      [in] A Mailbox channel number for data to be received.
* @return 
*
***************************************************************************************************
*/
static void BCTRL_TaskRun
(
    uint32                              uiCore,
    MBOXCh_t                            siCh
)
{   
    MBOXDvice_t *BCTRLMboxDev;

    BCTRLMboxDev = spMboxDev[(uint32) siCh].mboxDev;
    while(BCTRLMboxDev != NULL_PTR)
    {
        SAL_MemSet(&BCTRLCmd[uiCore][0], 0x00, MBOX_CMD_FIFO_SIZE);
        if (MBOX_DevRecvCmd(BCTRLMboxDev, &BCTRLCmd[uiCore][0]) >= 0)
        {
            // Check ID From BL0, 1, 2, 3.
            if (BCTRLCmd[uiCore][0] == BCTRL_ID_BOOT_SEQUENCE) 
            { 
                if (BCTRL_CheckStatus(uiCore, &BCTRLCmd[uiCore][0]) == BCTRL_STAT_READY)
                {
                    BCTRLCmd[uiCore][1] = BCTRL_COMMAND_GO;
                    BCTRLCmd[uiCore][2] = BCTRL_SignalDramInit(uiCore);
    
                    MBOX_DevSendCmd(BCTRLMboxDev, &BCTRLCmd[uiCore][0]);
                    BCTRLStat[uiCore] = BCTRL_STAT_GO;
                    mcu_printf("[BCTRL] ch %d status %d\n", (int32) siCh, (int32) BCTRLStat[uiCore]);
                    PMIO_APBootOk();
                }
                else
                {
                    mcu_printf("[E][BCTRL] ch %d status %d\n", (int32) siCh, (int32) BCTRLStat[uiCore]);
                }
            }
            else 
            {
                mcu_printf("[E][BCTRL] ch %d Unused BCTRL ID %d\n", (int32) siCh, (int32) BCTRLStat[uiCore]);
            }
        }
    }

    mcu_printf("[E][BCTRL] termnated %d", (int32) siCh);
}

/*
***************************************************************************************************
*                                       <BCTRL_TaskA72>
*
* If you want to modify this function, you must let the telechips know how it is modified. 
* A task for waiting ready signal after boot compelete. it receives "ready message" for A72 reset.
*
* @param    pArg    [in] not used.
* @return   void 
*
***************************************************************************************************
*/
static void BCTRL_TaskA72
(
    void*                               pArg
)
{
    (void) pArg;
    BCTRL_TaskRun(BCTRL_CORE_A72, MBOX_CH_CA72MP_NONSECURE);
}

/*
***************************************************************************************************
*                                       <BCTRL_TaskA53>
*
* If you want to modify this function, you must let the telechips know how it is modified. 
* A task for waiting ready signal after boot compelete. it receives "ready message" for A53 reset.
*
* @param    pArg    [in] not used.
* @return   void 
*
***************************************************************************************************
*/

static void BCTRL_TaskA53
(
    void*                               pArg
)
{
    (void) pArg;
    BCTRL_TaskRun(BCTRL_CORE_A53, MBOX_CH_CA53MP_NONSECURE);
}

/*
***************************************************************************************************
*                                       <BCTRL_Init>
*
* If you want to modify this function, you must let the telechips know how it is modified. 
*
* This function reads mailbox for "ready messasge" from application processor (A72, A53). And 
* it sends "go mesasge" to application processor to be booted. Fianally, it craetes tasks for 
* receiving "ready message" for A53 and A72 for AP reset cases while mcu alive.
*
* @param    void
* @return   void 
*
***************************************************************************************************
*/
void BCTRL_Init
(
    void
)
{

    uint32 idx;
    static const uint8 *BCTRL_A72 = (const uint8 *) "BCTRL_NAME_A72";
    static const uint8 *BCTRL_A53 = (const uint8 *) "BCTRL_NAME_A53";

    for (idx = 0; idx < BCTRL_CORE_NUM; idx++)
    {
        BCTRLStat[idx] = BCTRL_STAT_WAIT;
    }

    if (SAL_TaskCreate(&BCTRLTaskId[BCTRL_CORE_A72], BCTRL_A72, &BCTRL_TaskA72, &BCTRLTaskSt[BCTRL_CORE_A72][0], BCTRL_TASK_STK_SIZE, SAL_PRIO_LOWEST, NULL_PTR) != SAL_RET_SUCCESS)
    {
        mcu_printf("%s Fatal!! Task Create Fail. A72 will not be reboot\n", __func__);
    }

    if (SAL_TaskCreate(&BCTRLTaskId[BCTRL_CORE_A53], BCTRL_A53, &BCTRL_TaskA53, &BCTRLTaskSt[BCTRL_CORE_A53][0], BCTRL_TASK_STK_SIZE, SAL_PRIO_LOWEST, NULL_PTR) != SAL_RET_SUCCESS)
    {
        mcu_printf("%s Fatal!! Task Create Fail. A53 will not be reboot\n", __func__);
    }
}

