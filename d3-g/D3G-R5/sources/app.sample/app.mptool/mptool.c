/*
***************************************************************************************************
*
*   FileName : mp_tool.c
**
*   Description :
*
*
***************************************************************************************************
*/

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <app_cfg.h>

#if (ACFG_APP_MPTOOL_EN == 1)
#include "bsp.h"
#include "gic.h"
#include "gpio.h"
#include <debug.h>

#include "mptool.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/


/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/
static void MPTool_Task
(
    void *                              pArg
);


void MPTool_CreateApp
(
    void
)
{
    static uint32 uiMPToolAppTaskID;
    static uint32 uiMPToolAppTaskStk[MP_TOOL_TASK_STK_SIZE];

    ( void ) SAL_TaskCreate( &uiMPToolAppTaskID,
                   ( const uint8 * ) "MP Tool Task",
                   ( SALTaskFunc ) &MPTool_Task,
                   ( uint32 * const ) &uiMPToolAppTaskStk[0],
                   MP_TOOL_TASK_STK_SIZE,
                   SAL_PRIO_LOWEST,
                   NULL_PTR);
}

static void MPTool_Task
(
    void *                              pArg
)
{
    ( void ) pArg;
    uint8 i;

    while( 1 )
    {
        // GPIO
        MPTL_E("GPIO TEST START \n");
        for(i=0; i< (uint32)19 ; i++)
        {
            (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
            (void)GPIO_Set(GPIO_GPK(i), 1UL);

            (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_INPUT));

            if(GPIO_Get(GPIO_GPK(i)) == (uint8)1UL) {
                MPTL_E("GPIO_GPK(%d) = %d \n", i, 1);
            }
            else {
                MPTL_E("GPIO_GPK(%d) = %d \n", i, 0);
            }
        }

        ( void ) SAL_TaskSleep( 1000 );
    }
}

#endif //#if (ACFG_APP_MPTOOL_EN == 1)

