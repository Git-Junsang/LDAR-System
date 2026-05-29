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
 * vfp_init.S
 * Telechips TCC805x SoCs Cortex-R5 Single Processor
 *
 * History
 * -------------
 * Created by : Yongseok, oh  2018/5/17
 *****************************************************/

.global vfp_init

vfp_init:

#ifdef VFP_D16

/* Globally enable VFP */
    mrc p15, #0, r0, c1, c0, #2     @ r0 = Access Control Register
    orr r0, r0, #(0xf << 20)        @ enable full access for p10,11
    mcr p15, #0, r0, c1, c0, #2     @ Access Control Register = r0

    /* enable fpu access  */
    vmrs    r1, c8
    orr     r1, r1, #(1<<30)
    vmsr    c8, r1

    /* clear the floating point register*/
    mov     r1,#0
    vmov    d0,r1,r1
    vmov    d1,r1,r1
    vmov    d2,r1,r1
    vmov    d3,r1,r1
    vmov    d4,r1,r1
    vmov    d5,r1,r1
    vmov    d6,r1,r1
    vmov    d7,r1,r1
    vmov    d8,r1,r1
    vmov    d9,r1,r1
    vmov    d10,r1,r1
    vmov    d11,r1,r1
    vmov    d12,r1,r1
    vmov    d13,r1,r1
    vmov    d14,r1,r1
    vmov    d15,r1,r1
#endif

    bx  lr

    .end



