/*
***************************************************************************************************
*
*   FileName : main.c
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

#include <main.h>

#include <sal_api.h>
#include <app_cfg.h>
#include <debug.h>
#include <bsp.h>

#if (ACFG_APP_KEY_EN == 1)
#   include <key.h>
#endif
#if (ACFG_APP_SYSTEM_MONITORING_EN ==1)
#   include <system_monitoring.h>
#endif
#if (ACFG_APP_CONSOLE_EN == 1)
#   include <console.h>
#endif
#if (ACFG_DRV_SDM_EN == 1)
#   include <sdm.h>
#endif
#if (ACFG_APP_CAN_DEMO_EN == 1)
#include <can_demo.h>
#endif
#if (ACFG_DRV_IPC_EN == 1)
#include "ipc.h"
#endif
#if (ACFG_APP_FWUG_EN == 1)
#include <fwug.h>
#endif
#if (ACFG_APP_TRVC_EN == 1)
#   include <trvc.h>
#endif
// MP Tool
#if (ACFG_APP_MPTOOL_EN == 1)
#   include <mptool.h>
#endif
// AUTOMOTIVE_IVI_CLUSTER_DEMO
#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
#include <ivi_cluster_demo.h>
#endif

#include <bootctrl.h>
/*
***************************************************************************************************
*                                         GLOBAL VARIABLES
***************************************************************************************************
*/
uint32                                  gALiveMsgOnOff;
static uint32                                  gALiveCount;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void Main_StartTask
(
    void *                              pArg
);

static void AppTaskCreate
(
    void
);

static void DisplayAliveLog
(
    void
);

/*
***************************************************************************************************
*                                         FUNCTIONS
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          cmain
*
* This is the standard entry point for C code.
*
* Notes
*   It is assumed that your code will call main() once you have performed all necessary
*   initialization.
*
***************************************************************************************************
*/
void cmain (void)
{
    static uint32           AppTaskStartID = 0;
    static uint32           AppTaskStartStk[ACFG_TASK_MEDIUM_STK_SIZE];
    SALRetCode_t            err;
    SALMcuVersionInfo_t     versionInfo = {0,0,0,0};

    (void)SAL_Init();  

    BSP_PreInit(); /* Initialize basic BSP functions */

#if (ACFG_APP_CAN_DEMO_EN == 1)
    (void)CAN_DemoInitialize();
#endif

    BSP_Init(); /* Initialize BSP functions */

    (void)SAL_GetVersion(&versionInfo);
    mcu_printf("\nMCU BSP Version: V%d.%d.%d\n",
           versionInfo.viMajorVersion,
           versionInfo.viMinorVersion,
           versionInfo.viPatchVersion);

    // create the first app task...
    err = (SALRetCode_t)SAL_TaskCreate(&AppTaskStartID,
                         (const uint8 *)"App Task Start",
                         (SALTaskFunc) &Main_StartTask,
                         &AppTaskStartStk[0],
                         ACFG_TASK_MEDIUM_STK_SIZE,
                         SAL_PRIO_APP_CFG,
                         NULL);

    if (err == SAL_RET_SUCCESS)
    {
        // start woring os.... never return from this function
        (void)SAL_OsStart();
    }
}

/*
***************************************************************************************************
*                                          Main_StartTask
*
* This is an example of a startup task.
*
* Notes
*   As mentioned in the book's text, you MUST initialize the ticker only once multitasking has
*   started.
*
*   1) The first line of code is used to prevent a compiler warning because 'pArg' is not used.
*      The compiler should not generate any code for this statement.
*
***************************************************************************************************
*/
static void Main_StartTask(void * pArg)
{
    (void)pArg;
    (void)SAL_OsInitFuncs();

    BCTRL_Init();
    /* Service Init*/
#if (ACFG_DRV_IPC_EN == 1)
    IPC_Create();
#endif

#if (ACFG_DRV_SDM_EN == 1)
    (void)SDM_Init();
#endif

    /* Create application tasks */
    AppTaskCreate();

    while (1)
    {  /* Task body, always written as an infinite loop.       */
        DisplayAliveLog();
        //mcu_printf("\n MCU Idle !!!");
        (void)SAL_TaskSleep(5000);
    }
}

static void AppTaskCreate(void)
{
#if (ACFG_APP_CONSOLE_EN == 1)
    CreateConsoleTask();
#endif

#if (ACFG_APP_KEY_EN == 1)
    KEY_AppCreate();
#endif

#if (ACFG_APP_SYSTEM_MONITORING_EN == 1)
    SM_CreateAppTask();
#endif

#if (ACFG_APP_CAN_DEMO_EN == 1)
    CAN_DemoCreateApp();
    CAN_Edu_DemoCreateApp();  //send
#endif

#if (ACFG_APP_FWUG_EN == 1)
    FWUG_CreateApp();
#endif
#if (ACFG_APP_TRVC_EN == 1)
    TRVC_CreateAppTasks();
#endif

#if (ACFG_APP_MPTOOL_EN == 1)
    MPTool_CreateApp();
#endif

#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
    IVI_Cluster_demmo_CreateApp();
#endif
}

static void DisplayAliveLog(void)
{
    if (gALiveMsgOnOff != 0U)
    {
        mcu_printf("\n %d", gALiveCount);

        gALiveCount++;

        if(gALiveCount >= MAIN_UINT_MAX_NUM)
        {
            gALiveCount = 0;
        }
    }
    else
    {
        gALiveCount = 0;
    }
}

