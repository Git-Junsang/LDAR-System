/**************************************************************************************************

*   FileName : bsp_os.h

*   Copyright (c) Telechips Inc.

*   Description : System interface for RTOS operation.


***************************************************************************************************
*
*   TCC Version 1.0

This source code contains confidential information of Telechips.

Any unauthorized use without a written permission of Telechips including not limited to re-distribution
in source or binary form is strictly prohibited.

This source code is provided "AS IS" and nothing contained in this source code shall constitute any
express or implied warranty of any kind, including without limitation, any warranty of merchantability,
fitness for a particular purpose or non-infringement of any patent, copyright or other third party
intellectual property right. No warranty is made, express or implied,regarding the information's
accuracy,completeness, or performance.

In no event shall Telechips be liable for any claim, damages or other liability arising from, out of
or in connection with this source code or the use in the source code.

This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between
Telechips and Company.

**************************************************************************************************/
/*-------------------------------------------------------------------------------------------------
Revision History

Author      Date             Version      Description of Changes
---------------------------------------------------------------------------------------------------
hskim       2020/07/16       0.1          Draft
JMP         2020/08/10       0.2          Adding OS Timer tick for uCOS
-------------------------------------------------------------------------------------------------*/

#ifndef  _BSP_OS_HEADER_
#define  _BSP_OS_HEADER_
/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include <sal_internal.h>


/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void OS_TmrTickInit
(
    void
);

#endif                                                          /* End of module include.   _BSP_OS_HEADER_ */
