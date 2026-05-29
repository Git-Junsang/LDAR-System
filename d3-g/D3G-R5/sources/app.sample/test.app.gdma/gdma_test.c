/*
***************************************************************************************************
*
*   FileName : gdma_test.c
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

#include "gdma_test.h"
#include "gdma.h"
#include "clock.h"
#include <gic.h>
#include <debug.h>


#if (DEBUG_ENABLE)
#define GDMA_DBG(fmt, args...)          {LOGD(DBG_TAG_GDMA, fmt, ## args)}
#else
#define GDMA_DBG(fmt, args...)
#endif

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         LOCAL FUNCTION PROTOTYPES
***************************************************************************************************
*/


 /*
***************************************************************************************************
*                                          GDMA_SampleForM2M
*
* Sample test function for transferring data using dma by memory to memory scheme.
*
* @param    none.
* @return
*
* Notes
*
***************************************************************************************************
*/

void GDMA_SampleForM2M
(
    void
)
{
    uint32 request_size =  (1024 + (4*16) + 3);
    uint32 src_base = 0xC1000000UL;
    uint32 dest_base = (0xC1000000UL + 0x1000UL);
    GDMAControlData_t sample_pGdmaControl;
    sint8   * pSrcPtr;
    sint8   * pDstPtr;
    sint32 ret = 0;
    uint32 i;
    sample_pGdmaControl.cdChannel = 0;

    GDMA_DBG("\ngdma_m2m_sample test start \n");
    //make source pattern & clear destination memory by 0
    pSrcPtr = (sint8 *)(src_base);
    pDstPtr = (sint8 *)(dest_base);
    for(i =0; i < request_size; i++)
    {
        *(pSrcPtr+i) = (sint8)127;//0x7F
        *(pDstPtr+i) = (sint8)0;
    }
    // initialize GDMA CH0
    GDMA_DBG("GDMA_Initialize call start \n");
    ret= GDMA_Initialize(&sample_pGdmaControl, GIC_PRIORITY_NO_MEAN);

    if (ret == 0)
    {
        GDMA_DBG("GDMA_Initialize ret = %d Success \n", ret);

        GDMA_DBG("gdma_transfer_m2m call M2M source %x to destination %x requested bytes %d \n",src_base, dest_base, request_size );
        // Requst GDMA Momory To Momory Transfer.
        ret = GDMA_TransferFunctionForM2M(&sample_pGdmaControl, src_base, dest_base, request_size);
        if ( ret ==0)
        {
            GDMA_DBG("compare souce and destination memory\n");
            // compare souce and destination memory
            for(i =0; i < request_size; i++)
            {
                if (*(pSrcPtr+i) != *(pDstPtr+i))
                {
                    break;
                }
            }

            if ( i == request_size)
            {
                GDMA_DBG("GDMA M2M Verify Success \n");
            }
            else
            {
                GDMA_DBG("GDMA M2M Verify  Fail \n");
            }
        }
        else
        {
            GDMA_DBG("GDMA Fail \r");
        }
    }
}

