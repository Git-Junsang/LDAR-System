/*
***************************************************************************************************
*
*   FileName : sal_freertos_impl.h
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

#if !defined(SAL_FREERTOS_IMPL_HEADER)
#define SAL_FREERTOS_IMPL_HEADER

/*
***************************************************************************************************
*                                             [INCLUDE FILES]
***************************************************************************************************
*/

#include <sal_internal.h>

/*
***************************************************************************************************
*                                             [DEFINITIONS]
***************************************************************************************************
*/

#ifdef __cplusplus
extern "C"
{
#endif

/*
***************************************************************************************************
*                                         [FUNCTION PROTOTYPES]
***************************************************************************************************
*/
/* Defined on ASM */
extern void __DSB
(
    void
);

extern uint32 CPU_SR_Save
(
    void
);

extern void CPU_SR_Restore
(
    uint32 uiStatus
);

extern void CPU_WaitForEvent
(
    void
);

#ifdef __cplusplus
}
#endif

#endif // SAL_FREERTOS_IMPL_HEADER

