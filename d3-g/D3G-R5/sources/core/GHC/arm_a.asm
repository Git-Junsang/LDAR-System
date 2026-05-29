;/*******************************************************************************
  
;*   Copyright (c) Telechips Inc.
 
 
;*   TCC Version 1.0
  
;This source code contains confidential information of Telechips.
 
;Any unauthorized use without a written permission of Telechips including not
;limited to re-distribution in source or binary form is strictly prohibited.
 
;This source code is provided "AS IS" and nothing contained in this source code
;shall constitute any express or implied warranty of any kind, including without
;limitation, any warranty of merchantability, fitness for a particular purpose
;or non-infringement of any patent, copyright or other third party intellectual
;property right.
;No warranty is made, express or implied, regarding the information's accuracy,
;completeness, or performance.
 
;In no event shall Telechips be liable for any claim, damages or other
;liability arising from, out of or in connection with this source code or
;the use in the source code.
  
;This source code is provided subject to the terms of a Mutual Non-Disclosure
;Agreement between Telechips and Company.
;*
;*******************************************************************************/
;*****************************************************
;
;  arm_a.asm
;  Telechips TCC802x SoCs Cortex-A7 Single Processor
;
;  History
;  -------------
;  Created by : SangWon, Lee  2017/1/31
; 
;*****************************************************/

;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

	GLOBAL	ARM_TIMER_Get_Rate
	GLOBAL	ARM_TIMER_Get_CNTV_CTL
	GLOBAL	ARM_TIMER_Set_CNTV_CTL
	GLOBAL	ARM_TIMER_Set_CNTV_CVAL
	GLOBAL	ARM_TIMER_Get_CNTVCT
	GLOBAL	ARM_Set_VBAR
	GLOBAL	ARM_DSB


;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

;	RSEG CODE:CODE:NOROOT(2)
	CODE32


;********************************************************************************************************
;
;********************************************************************************************************

ARM_Set_VBAR
        MCR     p15, 0, r0, c12, c0, 0
        BX      LR


ARM_TIMER_Get_Rate
        MRC     p15, 0, r0, c14, c0, 0
        BX      LR


ARM_TIMER_Get_CNTV_CTL
        MRC     p15, 0, r0, c14, c3, 1
        BX      LR


ARM_TIMER_Set_CNTV_CTL
        MCR     p15, 0, r0, c14, c3, 1
        BX      LR


ARM_TIMER_Set_CNTV_CVAL
        MCRR    p15, 3, r0, r1, c14
        BX      LR


ARM_TIMER_Get_CNTVCT
        MRRC    p15, 1, r0, r1, c14
        BX      LR

ARM_DSB
        DSB
        ISB
        BX      LR
        

	END
