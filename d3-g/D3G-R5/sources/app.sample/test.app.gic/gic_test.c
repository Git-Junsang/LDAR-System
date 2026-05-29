/*
***************************************************************************************************
*
*   FileName : gic_test.c
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

#include <gic_test.h>
#include <gic.h>
#include <bsp.h>

#include <debug.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/




/***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/


static uint32                           gWaitSmIrq = 0;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void GIC_Test_Isr(void *pArg)
{
    uint32* puiIrqIndex;

    puiIrqIndex = NULL_PTR;
	
    if(pArg != NULL_PTR)
    {
        puiIrqIndex = (uint32 *)pArg;
        // Clear SW interrupt
        (void)GIC_SmIrqSetHigh(*puiIrqIndex, 0);

        (void)GIC_SmIrqEn(*puiIrqIndex, 0);
        // Clear corresponding interrupt enable
        (void)GIC_IntSrcDis(*puiIrqIndex);

        // Clear interrupt handler for irq_index
        (void)GIC_IntVectSet(*puiIrqIndex, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, NULL, NULL);

        mcu_printf(" Get IRQ-%d  Ok\n", *puiIrqIndex);

        gWaitSmIrq--;// = 0;
    }   
	
}

/*
***************************************************************************************************
*                                       GIC_Test_SmInt
*
* GIC interrupt test code(SPI)
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void GIC_Test_SmInt(void)
{
    uint32        uiIrqIndex;
    uint32        uiIsrArg;
    uint32        uiK;
    const uint32  uiDelaycnt = (5000000UL);

    uiIrqIndex = 0;
    uiIsrArg   = 0;	 
    uiK        = 0UL;

    //---------------------------------------------------------------------------------------------
    // SW test using HW GIC test block
    //---------------------------------------------------------------------------------------------

    mcu_printf("================== SM SPI interrupt test ==================\n");

    // Disable all interrupts for test
    for(uiIrqIndex = GIC_SPI_START; uiIrqIndex < GIC_INT_SRC_CNT; uiIrqIndex++)
    {
        (void)GIC_IntSrcDis(uiIrqIndex);
    }


    for(uiIrqIndex = GIC_SPI_START; uiIrqIndex < GIC_INT_SRC_CNT; uiIrqIndex++)
    {
        (void)GIC_SmIrqEn(uiIrqIndex, 0);
    }

    // GIC SW interrupt generation

    gWaitSmIrq = (GIC_INT_SRC_CNT - GIC_SPI_START);
	
    for(uiIrqIndex = GIC_SPI_START; uiIrqIndex < GIC_INT_SRC_CNT; uiIrqIndex++)
    {
        uiIsrArg = uiIrqIndex;
        (void)GIC_IntVectSet(uiIrqIndex,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&GIC_Test_Isr,
                             &uiIsrArg);

        (void)GIC_SmIrqEn(uiIrqIndex, 1);

        (void)GIC_IntSrcEn(uiIrqIndex);

        (void)GIC_SmIrqSetHigh(uiIrqIndex, 1);

        for ( ; (uiK < uiDelaycnt) ; uiK++)
        {
            if(gWaitSmIrq < (GIC_INT_SRC_CNT - uiIrqIndex))
            {
                break;
            }       
			
            BSP_NOP_DELAY();
        }
		
        if(uiK == uiDelaycnt)
        {
            mcu_printf("Interrupt not occured\n");
        }

    }

    mcu_printf("gWaitSmIrq %d\n",gWaitSmIrq);

    mcu_printf("====== GIC Test Done            ============================\n");
    mcu_printf("====== All of the interrupt has been disabled. =============\n");
    mcu_printf("====== Please. reset the board for normal operation. =======\n");
    return;
}

/*
***************************************************************************************************
*                                       GIC_Test_Sgi
*
* GIC interrupt test code(SGI)
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void GIC_Test_Sgi(void)
{

    uint32        uiIrqIndex;
    uint32        uiIsrArg;
    uint32        uiK;
    const uint32  uiDelaycnt = (5000000UL);

    uiIrqIndex = 0;
    uiIsrArg   = 0;		
    uiK        = 0UL;

    //---------------------------------------------------------------------------------------------
    // SW test using HW GIC test block
    //---------------------------------------------------------------------------------------------

    mcu_printf("================== SGI interrupt test ==================\n");

    // Disable all interrupts for test
    for( ; uiIrqIndex < GIC_PPI_START; uiIrqIndex++)
    {
        (void)GIC_IntSrcDis(uiIrqIndex);
    }

    // GIC SW interrupt generation
    gWaitSmIrq = (GIC_PPI_START);
	
    for(uiIrqIndex = 0; uiIrqIndex < GIC_PPI_START; uiIrqIndex++)
    {
        uiIsrArg = uiIrqIndex;
        (void)GIC_IntVectSet(uiIrqIndex,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&GIC_Test_Isr,
                             &uiIsrArg);


        (void)GIC_IntSrcEn(uiIrqIndex);

        (void)GIC_IntSGI(uiIrqIndex);

        for ( ; (uiK < uiDelaycnt) ; uiK++)
        {
            if(gWaitSmIrq < (GIC_PPI_START- uiIrqIndex))  
            {                                             
                break;
            }        
			
            BSP_NOP_DELAY();
        }
		
        if(uiK == uiDelaycnt)
        {
            mcu_printf("Interrupt not occured\n");
        }
		
    }
	
    mcu_printf("gWaitSmIrq %d\n",gWaitSmIrq);

    mcu_printf("====== GIC SGI Test Done        ============================\n");
    mcu_printf("====== All of the SGI interrupt has been disabled. =========\n");
    mcu_printf("====== Please. reset the board for normal operation. =======\n");
    return;
}

