/*
***************************************************************************************************
*
*   FileName : gdma.h
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

#ifndef GDMA_HEADER_H
#define GDMA_HEADER_H

#include <sal_internal.h>

#define GDMA_CH_MAX                     (7UL)

// DMA Request Peripheral
//typedef enum _GDMARequestPeripheral_
//{
//    GDMA_ICTC0                           = 0,
//    GDMA_ICTC1                           = 1,
//    GDMA_ICTC2                           = 2,
//    GDMA_ADC                           = 3
//}GDMARequestPeripheral_t;

//typedef volatile struct _GDMALLIType_
//{
//    uint32                              ltSourceAddress; //linked list source address
//    uint32                              ltDestinationAddress; //linked list destination address
//    uint32                              ltAddress; //next linked list item address
//    uint32                              ltcontrol; //linked list control flags
//}GDMALLIType_t;

typedef struct GDMAControlData
{
    uint32                              cdControl; /*gdma control flags */
    uint32                              cdChannel; /* control channel number for using */
}GDMAControlData_t;

/*
***************************************************************************************************
*                                         FUNCTION DECLEARATION
***************************************************************************************************
*/

 /*
***************************************************************************************************
*                                          GDMA_Initialize
*
* Initialize the gdma driver.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    uiInterruptPriority [in] priority of gdma interrupt.
* @return
*
* Notes
*
***************************************************************************************************
*/

extern sint32 GDMA_Initialize
(
    GDMAControlData_t *                 pGdmaControl,
    uint32                              uiInterruptPriority
);

 /*
***************************************************************************************************
*                                          GDMA_TransferFunctionForM2M
*
* Request DMA transfer from source address to destination address with given data length.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    uiSourceAddress [in] source address. This address is valid only for physical address.
* @param    uiDestinationAddress [in] destination address. This address is valid only for physical address.
* @param    uiLength [in] lengh fo data requested for transfer.
* @return   the result of transfer.
*
* Notes
*
***************************************************************************************************
*/

extern sint32 GDMA_TransferFunctionForM2M
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              uiSourceAddress,
    uint32                              uiDestinationAddress,
    uint32                              uiLength
 );
#endif  /* GDMA_HEADER_H */

