/*
***************************************************************************************************
*
*   FileName : ivi_cluster_demo.c
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

#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <debug.h>

#include "bsp.h"
#include "ipc.h"
#include "ivi_cluster_demo.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/
uint8_t set_time = 2;
uint8_t enable_log = 2;

/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/
static void IVI_Cluster_demo_Task
(
    void *                              pArg
);

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/
#define IVI_CLUSTER_DEMO_D(fmt, args...)            do {if(enable_log > 1)  mcu_printf("(E)[%-7s][%s:%d] "fmt"\n", "CLUSTER", __func__, __LINE__, ## args);} while(0)
#define IVI_CLUSTER_DEMO_E(fmt, args...)            do {if(enable_log > 0)  mcu_printf("(E)[%-7s][%s:%d] "fmt"\n", "CLUSTER", __func__, __LINE__, ## args);} while(0)


void IVI_Cluster_demo_DemoUsage
(
    void
)
{
    mcu_printf( "=== USAGE INFO ===\n\n" );
    mcu_printf( "=== CMD LIST ===\n" );
    mcu_printf( " clust start [sec]\n" );
    mcu_printf( " clust stop\n" );
    mcu_printf( " clust log [num]\n" );
    mcu_printf( " clust send\n" );
    mcu_printf( "\r\n" );
    mcu_printf( "=== CMD INFORMATION ===\n" );
    mcu_printf( "clust start [sec]:      Start cluster demo, [sec] periodically send ramdom cluster message to cluster app\n" );
    mcu_printf( "clust stop:             Stop cluster demo\n" );
    mcu_printf( "clust log [num]:        Set log, [0: disable 1: error, 2: debug]\n" );
    mcu_printf( "clust send:             Send cluster data immediately\n" );
}

void IVI_Cluster_DemoTest
(
    uint8                               ucArgc,
    void *                              pArgv[]
)
{
    sint32 ret;
    if( pArgv != NULL_PTR )
    {
        if(ucArgc > 0 && (const uint8 * )pArgv[0] != NULL_PTR)
        {
            if(( SAL_StrNCmp((const uint8 * )pArgv[0], ( const uint8 * ) "start", 5, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
            {
                // time
                if(ucArgc == 2 && (const uint8 * )pArgv[1] != NULL_PTR)
                {
                    set_time = ((uint8)atoi(pArgv[1]));
                    mcu_printf("\n----> Set time : %d\n", set_time);
                    return;
                }
            }
            else if( ( SAL_StrNCmp((const uint8 * )pArgv[0], ( const uint8 * ) "stop", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
            {
                set_time = 0;
                mcu_printf("\n----> Set time : %d\n", set_time);
                return;
            }
            else if(( SAL_StrNCmp((const uint8 * )pArgv[0], ( const uint8 * ) "log", 3, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
            {
                // time
                if(ucArgc == 2 && (const uint8 * )pArgv[1] != NULL_PTR)
                {
                    enable_log = ((uint8)atoi(pArgv[1]));
                    mcu_printf("\n----> Set log : %d\n", enable_log);
                    return;
                }
            } 
            else if(( SAL_StrNCmp((const uint8 * )pArgv[0], ( const uint8 * ) "send", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
            {
                uint8_t temp = enable_log;
                enable_log = 2;
                IVI_Cluster_SendData();
                enable_log = temp;
                return;
            }             
        }
    }

    mcu_printf( "Wrong argument: %d\n", ret);
    IVI_Cluster_demo_DemoUsage();
    mcu_printf("\n----> Set time : %d\n", set_time);
    mcu_printf("\n----> Set log : %d\n", enable_log);

}

void IVI_Cluster_SendData
(
    void
)
{
    uint8_t keyData_one[8] = {180, 0x08, 0xBB, 2, 100, 100, 0, 0};
    uint8_t keyData_two[8] = {1, 1, 1, 1, 1, 1, 1, 8};
    uint16_t random_value;

    keyData_one[0] =(uint8)(rand() % 181); // speed
    random_value = (uint16) (rand() % 3001); // rpm
    keyData_one[1] =(uint8)(random_value & 0xFF); // rpm
    keyData_one[2] =(uint8)(random_value >> 8) & 0xFF; // rpm
    keyData_one[3] =(uint8)(rand() % 3); // torque
    keyData_one[4] =(uint8)(rand() % 101); // fuel
    keyData_one[5] =(uint8)(rand() % 101); // battery

    keyData_two[0] =(uint8)(rand() % 2); // break_signal
    keyData_two[1] =(uint8)(rand() % 2); // overheet_signal
    keyData_two[2] =(uint8)(rand() % 2); // engine_signal
    keyData_two[3] =(uint8)(rand() % 2); // seatbelt_signal
    keyData_two[4] =(uint8)(rand() % 2); // feul_signal
    keyData_two[5] =(uint8)(rand() % 2); // turn_left
    keyData_two[6] =(uint8)(rand() % 2); // turn_right
    random_value =(uint8)(rand() % 9); // gear
    if (random_value == 0)
    keyData_two[7] = 1;
    else if (random_value == 1)
    keyData_two[7] = 2;
    else if (random_value == 2)
    keyData_two[7] = 4;
    else
    keyData_two[7] = 8;

    IVI_CLUSTER_DEMO_D("%d, %d, %d, %d %d, %d, %d, %d\n", 
                        keyData_one[0], keyData_one[1], keyData_one[2], keyData_one[3], keyData_one[4], keyData_one[5], keyData_one[6], keyData_one[7]);
    IVI_CLUSTER_DEMO_D("%d, %d, %d, %d %d, %d, %d, %d\n", 
                        keyData_two[0], keyData_two[1], keyData_two[2], keyData_two[3], keyData_two[4], keyData_two[5], keyData_two[6], keyData_two[7]);

    IPC_SendPacket(0x01/*IPC_SVC_CH_CA53_NONSECURE*/, (uint16)TCC_IPC_CMD_CAN_MESSAGE_ONE, (uint16)TCC_IPC_CMD_CAN_MESSAGE_ONE, (const uint8*)keyData_one, sizeof(keyData_one));
    IPC_SendPacket(0x01/*IPC_SVC_CH_CA53_NONSECURE*/, (uint16)TCC_IPC_CMD_CAN_MESSAGE_TWO, (uint16)TCC_IPC_CMD_CAN_MESSAGE_TWO, (const uint8*)keyData_two, sizeof(keyData_two));

    IPC_SendPacket(0x03/*IPC_SVC_CH_CA72_NONSECURE*/, (uint16)TCC_IPC_CMD_CAN_MESSAGE_ONE, (uint16)TCC_IPC_CMD_CAN_MESSAGE_ONE, (const uint8*)keyData_one, sizeof(keyData_one));
    IPC_SendPacket(0x03/*IPC_SVC_CH_CA72_NONSECURE*/, (uint16)TCC_IPC_CMD_CAN_MESSAGE_TWO, (uint16)TCC_IPC_CMD_CAN_MESSAGE_TWO, (const uint8*)keyData_two, sizeof(keyData_two));
}

void IVI_Cluster_demo_CreateApp
(
    void
)
{
    static uint32 uiIVIClusterDemoAppTaskID;
    static uint32 uiIVIClusterDemoAppTaskStk[IVI_CLUSTER_DEMO_TASK_STK_SIZE];

    ( void ) SAL_TaskCreate( &uiIVIClusterDemoAppTaskID,
                   ( const uint8 * ) "IVI Cluster Demo Task",
                   ( SALTaskFunc ) &IVI_Cluster_demo_Task,
                   ( uint32 * const ) &uiIVIClusterDemoAppTaskStk[0],
                   IVI_CLUSTER_DEMO_TASK_STK_SIZE,
                   SAL_PRIO_LOWEST,
                   NULL_PTR);
}

static void IVI_Cluster_demo_Task
(
    void *                              pArg
)
{
    ( void ) pArg;

    srand(time(NULL));

    while( 1 )
    {
        if (set_time > 0)
        {
            IVI_Cluster_SendData();
        }

        ( void ) SAL_TaskSleep( set_time * 1000 );
    }
}
#endif //#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)


