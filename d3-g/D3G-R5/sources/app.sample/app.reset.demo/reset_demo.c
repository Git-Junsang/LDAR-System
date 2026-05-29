/*
***************************************************************************************************
*
*   FileName : reset_demo.c
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

#if (ACFG_APP_RESET_DEMO_EN == 1)
#include <debug.h>

#include "pmu_reg.h"
#include "pmu_reset.h"

#include "reset_demo.h"


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

/*
***************************************************************************************************
*                                       PMU_ResetTest
*
*
*
* @param
*
* @return
*
* Notes
*
***************************************************************************************************
*/

void PMU_ResetTest
(
    uint8                               ucArgc,
    void *                              pArgv[]
)
{
    const uint8 *   pucStr0;
    const uint8 *   pucStr1;
    sint32          ret;

    pucStr0         = NULL_PTR;
    pucStr1         = NULL_PTR;

    if( pArgv != NULL_PTR )
    {
        pucStr0 = ( const uint8 * ) pArgv[ 0 ];
        pucStr1 = ( const uint8 * ) pArgv[ 1 ];

        if( ucArgc == 2UL )
        {
            if( ( pucStr0 != NULL_PTR ) && ( pucStr1 != NULL_PTR ) )
            {
                if( ( SAL_StrNCmp( pucStr0, ( const uint8 * ) "reset", 5, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                {
                    if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "cold", 4, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        PMU_ResetColdReset();
                    }
                    else if( ( SAL_StrNCmp( pucStr1, ( const uint8 * ) "ap", 2, &ret ) == SAL_RET_SUCCESS ) && ( ret == 0 ) )
                    {
                        PMU_ResetAPSystemReboot();
                    }
                    else
                    {
                        mcu_printf( "Wrong argument\n" );
                    }
                }
                else
                {
                    mcu_printf( "Wrong argument\n" );
                }
            }
            else
            {
                mcu_printf( "Wrong argument\n" );
            }
        }
        else
        {
            mcu_printf( "Wrong argument\n" );
        }
    }
}


#endif //#if (ACFG_APP_RESET_DEMO_EN == 1)

