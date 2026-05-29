/*
***************************************************************************************************
*
*   FileName : fwug.c
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

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <app_cfg.h>

#if (ACFG_APP_FWUG_EN == 1)
#include "bsp.h"
#include "gic.h"
#include "gpio.h"
#include "debug.h"

#include "wdt.h"
#include "fwug.h"
#include "mpu.h"

#include "hsm_manager.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static FWUGDev_t gFWUGDevHandle         = { NULL_PTR };
static FWUGMsg_t gFWUGRecvData;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static void FWUG_Task
(
    void *                              pArg
);

static void FWUG_CopyAndJump
(
    void
);


/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

void FWUG_CreateApp( void )
{
    static uint32 uiFWUGAppTaskID;
    static uint32 uiFWUGAppTaskStk[ FWUG_TASK_STK_SIZE ];

    ( void ) SAL_TaskCreate( &uiFWUGAppTaskID, ( const uint8 * ) "FWUG_Task",
                        ( SALTaskFunc ) &FWUG_Task,
                        ( uint32 * const ) &uiFWUGAppTaskStk[ 0 ], FWUG_TASK_STK_SIZE, SAL_PRIO_FWUG_APP, NULL_PTR );
}

static void FWUG_Task( void *pArg )
{
    int32   ret;
    uint32  uiCmd;
    int8    cDevName[ 7 ];

    ret     = -1;
    uiCmd   = 0;

    cDevName[ 0 ] = 'F';
    cDevName[ 1 ] = 'W';
    cDevName[ 2 ] = 'U';
    cDevName[ 3 ] = 'G';
    cDevName[ 4 ] = ( int8 ) 0x00;
    cDevName[ 5 ] = ( int8 ) 0x00;
    cDevName[ 6 ] = ( int8 ) 0x00;

    ( void ) pArg; //unused

    gFWUGDevHandle.dDev = MBOX_DevOpen( MBOX_CH_CA72MP_NONSECURE, cDevName, 0 );

    while( 1 )
    {
        ( void ) SAL_MemSet( gFWUGRecvData.mCmd, 0x00, MBOX_CMD_FIFO_SIZE );
        ( void ) SAL_MemSet( gFWUGRecvData.mData, 0x00, MBOX_DATA_FIFO_SIZE );

        ret = MBOX_DevRecv( gFWUGDevHandle.dDev, gFWUGRecvData.mCmd, gFWUGRecvData.mData );

        if( ret >= 0 )
        {
            uiCmd = gFWUGRecvData.mCmd[ 0 ];

            FWUG_Start( uiCmd );
        }
        else
        {
            ret = -1;
        }

        ( void ) SAL_TaskSleep( 100 );
    }
}

void FWUG_Start( uint32 uiCmd )
{
    if( uiCmd == FWUG_CMD_UPDATE_START )
    {
        ( void ) HSM_Init();

        FWUG_CopyAndJump();
    }
    else
    {
        FWUG_E( "%s, Invalid command \n", __func__ );
    }
}

void FWUG_CopyAndJump( void )
{
    int32           ret;
    uint32          uiImage_offset;
    uint32          uiImage_size;
    uint32          uiTempAddr;
    SNORRomInfo_t   sSnor_info;

    /* disable interrupt */
    ( void ) GIC_IntSrcDis( ( uint32 ) GIC_TIMER_0 + SAL_OS_TIMER_ID );
    ( void ) SAL_CoreCriticalEnter();

    /* disable watchdog timer */
    WDT_Stop();

    ( void ) SAL_MemSet( &sSnor_info, 0x00, sizeof( SNORRomInfo_t ) );

    /* get S-NOR information */
    ( void ) SAL_MemCopy( ( void * ) &sSnor_info, ( void * ) FWUG_SNOR_BASEADDR, sizeof( SNORRomInfo_t ) );

    if( sSnor_info.riRomId == SNOR_ROM_ID )
    {
        uiImage_offset = sSnor_info.riSectionInfo[ SNOR_MICOM_SUB_BINARY_ID ].siOffset;
        uiImage_size = sSnor_info.riSectionInfo[ SNOR_MICOM_SUB_BINARY_ID ].siDataSize; /* r5_sub_fw.rom cert + image size */

        /* set MPU */
        MPU_Disable();
        MPU_SetRegion( MPU_SRAM1_ADDR, ( MPU_NORM_SHARED_WB_WA | MPU_PRIV_RW_USER_RW ) );
        MPU_Enable();

        /* search for r5_sub_fw.rom */
        uiTempAddr = FWUG_SNOR_BASEADDR + uiImage_offset;
        ( void ) SAL_MemCopy( ( void * ) FWUG_SRAM1_BASEADDR, ( void * ) uiTempAddr, uiImage_size ); /* copy r5_sub_fw.rom to SRAM1 */

        /* verify r5_sub_fw.rom */
        uiTempAddr = FWUG_SRAM1_BASEADDR + 0x104UL;
        uiImage_size = *( volatile uint32 * ) ( uiTempAddr );


        ret = HSM_VerifyFw( HSM_MICOM_SRAM1_BASEADDR, 0x200, HSM_MICOM_SRAM1_BASEADDR + 0x200UL, uiImage_size, HSM_MC_SUB_FW_IMAGE_ID );

        if( ret != 0 )
        {
            FWUG_E( "%s, Verify fail F/W upgrade code \n", __func__ );
        }
        else
        {
            FWUG_D( "%s, jump to F/W upgrade code address : 0x%08x\n", __func__, FWUG_SRAM1_BASEADDR + 0x200UL );

            FWUG_JUMP_TO_ADDR; /* jump & execute r5_sub_fw.rom */

            while( 1 ) /* never reach */
            {
                BSP_NOP_DELAY();
            }
        }
    }
    else
    {
        FWUG_E( "%s, can't find F/W upgrade code \n", __func__ );
    }
}

#endif

