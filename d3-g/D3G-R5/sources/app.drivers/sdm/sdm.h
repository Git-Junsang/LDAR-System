/*
***************************************************************************************************
*
*   FileName : sdm.h
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

#ifndef  MCU_BSP_SDM_HEADER
#define  MCU_BSP_SDM_HEADER

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <reg_phys.h>
#include <sal_internal.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define SDM_VA_CLK_ADDR                 (0x0UL)
#define SDM_VA_CONF_ADDR                (0x100000UL)
#define SDM_VA_VS_ADDR                  (0x820000UL)
#define SDM_VA_TT_DATA_ADDR             (0x800000UL)
#define SDM_VA_TT_HEAD_ADDR             (0x820200UL)
#define SDM_VA_AS_ADDR                  (0x900000UL)

typedef enum{
    SDM_ERR_NONE                        = 0,
    SDM_ERR_FAIL                        = 1,
    SDM_ERR_INVALID_PARAM               = 2,
    SDM_ERR_SPI_NOT_INIT                = 3,
    SDM_ERR_XFER_REG                    = 4,
}SDMError_t;


/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
SDMError_t SDM_ReadReg
(
    uint32                              uiAddr,
    uint32 *                            piValue
);
SDMError_t SDM_WriteReg
(
    uint32                              uiAddr,
    const uint8 *                       pValue,
    uint32                              uiSize
);

/*Common Func*/
SDMError_t SDM_Init
(
    void
);

#endif //_SDM_HEADER_

