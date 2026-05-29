/*
***************************************************************************************************
*
*   FileName : fmu.h
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

#ifndef  FMU_H
#define  FMU_H

#include <sal_internal.h>

/*
***************************************************************************************************
*                                            GLOBAL DEFINITIONS
***************************************************************************************************
*/

#if (DEBUG_ENABLE)
    #include "debug.h"
    #define FMU_D(fmt,args...)          {LOGD(DBG_TAG_FMU, fmt, ## args)}
    #define FMU_E(fmt,args...)          {LOGE(DBG_TAG_FMU, fmt, ## args)}
#else
    #define FMU_D(fmt,args...)          {}
    #define FMU_E(fmt,args...)          {}
#endif



typedef void (*FMUIntFnctPtr)(void * pArg);

// FMU fault ID enumeration
typedef enum FMUFaultid
{
    FMU_ID_ADC                          =  0,
    FMU_ID_GPSB0                        =  1,
    FMU_ID_GPSB1                        =  2,
    FMU_ID_GPSB2                        =  3,
    FMU_ID_GPSB3                        =  4,
    FMU_ID_GPSB4                        =  5,
    FMU_ID_GPSB5                        =  6,
    FMU_ID_PDM0                         =  7,
    FMU_ID_PDM1                         =  8,
    FMU_ID_PDM2                         =  9,
    FMU_ID_MC_CFG                       = 10,
    FMU_ID_CR5_CFG                      = 11,
    FMU_ID_CKC_CFG                      = 12,
    FMU_ID_GPIO_CFG                     = 13,
    FMU_ID_MID_CFG                      = 14,
    FMU_ID_CMU_CFG                      = 15,
    FMU_ID_SYS_SM_CFG                   = 16,
    FMU_ID_X2X_MST0                     = 17,
    FMU_ID_X2X_MST1                     = 18,
    FMU_ID_X2X_SLV0                     = 19,
    FMU_ID_X2X_SLV1                     = 20,
    FMU_ID_X2X_SLV2                     = 21,
    FMU_ID_SDM_VIDEO                    = 22,
    FMU_ID_SDM_AUDIO                    = 23,
    FMU_ID_SRVC                         = 24,
    FMU_ID_PERI_CLK                     = 25,
    FMU_ID_PLL1_CLK                     = 26,
    FMU_ID_SRAM0_ECC                    = 27,
    FMU_ID_SRAM1_ECC                    = 28,
    FMU_ID_SFMC_ECC                     = 29,
    FMU_ID_MBOX0_S_ECC                  = 30,
    FMU_ID_MBOX0_NS_ECC                 = 31,
    FMU_ID_MBOX1_S_ECC                  = 32,
    FMU_ID_MBOX1_NS_ECC                 = 33,
    FMU_ID_MBOX2_ECC                    = 34,
    FMU_ID_OSC                          = 35,
    FMU_ID_VLD_CORE                     = 36,
    FMU_ID_VLD_OTP                      = 37,
    FMU_ID_WDT                          = 38,
    FMU_ID_PLL0_CLK                     = 39,
    FMU_ID_CPU_CLK                      = 40,
    FMU_ID_BUS_CLK                      = 41,
    FMU_ID_CHIP_CFG_SOFT                = 42,
    FMU_ID_CHIP_CFG_CRC                 = 43,
    FMU_ID_ROM_CRC                      = 44,
    FMU_ID_FMU_CFG_SOFT                 = 63
} FMUFaultid_t;

// FMU fault severity level enumeration
typedef enum FMUSeverityLevelType
{
    FMU_SVL_LOW                         = 0,
    FMU_SVL_MID                         = 1,
    FMU_SVL_HIGH                        = 2,
    FMU_SVL_REV                         = 3
}FMUSeverityLevelType_t;


typedef enum FMUErrTypes
{
    FMU_OK                              = 0,
    FMU_FAIL                            = 1,
    FMU_INVALID_ID_ERR                  = 2,
    FMU_RESERVE_ID_ERR                  = 3,
    FMU_INVALID_SEVERITY_ERR            = 4,
    FMU_ISR_HANDLER_ERR                 = 5,
    FMU_DS_COMPARE_FAIL_ERR             = 6,
    FMU_INTSET_ERR                      = 7,
    FMU_INTEN_ERR                       = 8,
    FMU_INTDIS_ERR                      = 9
}FMUErrTypes_t;



/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          FMU_Init
*
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Init
(
    void
);

/*
***************************************************************************************************
*                                          FMU_IsrHandler
*
*
* @param    uiId [in]
* @param    uiSeverityLevel [in]
* @param    fnIntFnct [in]
* @param    pArg [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_IsrHandler
(
    FMUFaultid_t                        uiId,
    FMUSeverityLevelType_t              uiSeverityLevel,
    FMUIntFnctPtr                       fnIntFnct,
    void *                              pArg
);

/*
***************************************************************************************************
*                                          FMU_Set
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Set
(
    FMUFaultid_t                        uiId
);

/*
***************************************************************************************************
*                                          FMU_IsrClr
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_IsrClr
(
    FMUFaultid_t                        uiId
);


/*
***************************************************************************************************
*                                          FMU_Deinit
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Deinit
(
    void
);

#endif //_FMU_H_

