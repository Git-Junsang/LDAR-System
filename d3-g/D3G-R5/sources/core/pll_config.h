/**************************************************************************************************
*
*   FileName : pll_config.h
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
*    re-distribution in source or binary form is strictly prohibited.
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
*    out of or in connection with this source code or the use in the source code.
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
*   between Telechips and Company.
*
**************************************************************************************************/

#ifndef _PLL_H_
#define _PLL_H_

#define USE_SET_PLL_DIRECT
#define MICOM_MAIN_PLL_SEL  2   // Select MICOM dedicated pll

//#define MICOM_MAIN_PLL_VAL    0x00028005  // 614.4MHz (for T-soound)

#define VEHICLE_DEMO_PLL_933  // PWM turnned based on this base clock

#ifdef USE_SET_PLL_DIRECT

#ifdef VEHICLE_DEMO_PLL_933
#define MICOM_MAIN_PLL_VAL 0x00014dc4   // 933MHz
#else
#define MICOM_MAIN_PLL_VAL 0x00014a43   // 1188MHz
#endif

#else

#ifdef VEHICLE_DEMO_PLL_933
#define MICOM_MAIN_PLL_VAL (933000000)  // 933MHz
#else
#define MICOM_MAIN_PLL_VAL (1188000000) // 1188MHz
#endif

#endif // USE_SET_PLL_DIRECT

#endif

