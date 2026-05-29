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
 * vector.S
 * Telechips TCC805x SoCs Cortex-A7 Single Processor
 *
 * History
 * -------------
 * Created by : SangWon, Lee  2016/8/19
 *
 *****************************************************/
.section	.vector	,"ax"

#define REMAP_MICOM_BASE	0x1B936014
#define COLD_RESET_BASE	0x1B93617C

.global	_vector	
.global ARM_Reserved
.global _start
_start:
_vector:
	ldr pc, =reset_handler
	ldr pc, =ARM_ExceptUndefInstrHndlr
	ldr pc, =ARM_ExceptSwiHndlr
	ldr pc, =ARM_ExceptPrefetchAbortHndlr
	ldr pc, =ARM_ExceptDataAbortHndlr
	ldr pc, =ARM_Reserved
	ldr pc, =ARM_ExceptIrqHndlr
	ldr pc, =ARM_ExceptFiqHndlr

	
reset_handler:
	ldr pc, = reset

ARM_Reserved:
	wfi
	b ARM_Reserved

	.end
