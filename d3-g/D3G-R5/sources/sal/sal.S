 /*******************************************************************************
  
*   Copyright (c) Telechips Inc.
 
 
*   TCC Version 1.0
  
This source code contains confidential information of Telechips.
 
Any unauthorized use without a written permission of Telechips including not
limited to re-distribution in source or binary form is strictly prohibited.
 
This source code is provided "AS IS" and nothing contained in this source code
shall constitute any express or implied warranty of any kind, including without
limitation, any warranty of merchantability, fitness for a particular purpose
or non-infringement of any patent, copyright or other third party intellectual
property right.
No warranty is made, express or implied, regarding the information's accuracy,
completeness, or performance.
 
In no event shall Telechips be liable for any claim, damages or other
liability arising from, out of or in connection with this source code or
the use in the source code.
  
This source code is provided subject to the terms of a Mutual Non-Disclosure
Agreement between Telechips and Company.
*
*******************************************************************************/

/*****************************************************
 *
 * osal.S
 * Telechips OS abstraction Layer
 *
 * History
 * -------------
 * Created by : Bong Sik, Kim  2017/10/31
 *
 *****************************************************/
 
    .global  ARM_ExceptUndefInstrHndlr
    .global  ARM_ExceptSwiHndlr
    .global  ARM_ExceptPrefetchAbortHndlr
    .global  ARM_ExceptDataAbortHndlr
    .global  ARM_ExceptIrqHndlr
    .global  ARM_ExceptFiqHndlr


/*********************************************************************************************************
*                               UNDEFINED INSTRUCTION EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptUndefInstrHndlr:
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptUndefInstrHndlr
#else
    B            OS_CPU_ARM_ExceptUndefInstrHndlr
#endif


/*********************************************************************************************************
*                                SOFTWARE INTERRUPT EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptSwiHndlr:
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptSwiHndlr
#else
    B            OS_CPU_ARM_ExceptSwiHndlr
#endif


/*********************************************************************************************************
*                                  PREFETCH ABORT EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptPrefetchAbortHndlr:
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptPrefetchAbortHndlr
#else
    B            OS_CPU_ARM_ExceptPrefetchAbortHndlr
#endif


/*********************************************************************************************************
*                                    DATA ABORT EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptDataAbortHndlr:
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptDataAbortHndlr
#else
    B            OS_CPU_ARM_ExceptDataAbortHndlr
#endif


/*********************************************************************************************************
*                                 INTERRUPT REQUEST EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptIrqHndlr:
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptIrqHndlr
#else
    B            OS_CPU_ARM_ExceptIrqHndlr
#endif


/*********************************************************************************************************
*                              FAST INTERRUPT REQUEST EXCEPTION HANDLER
*********************************************************************************************************/

ARM_ExceptFiqHndlr:
#ifdef OS_FREERTOS
    B            FreeRTOS_ARM_ExceptFiqHndlr
#else
    B            OS_CPU_ARM_ExceptFiqHndlr
#endif


	.end
