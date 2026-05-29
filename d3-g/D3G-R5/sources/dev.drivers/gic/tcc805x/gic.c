/*
***************************************************************************************************
*
*   FileName : gic.c
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

#include  <gic.h>


#include  <reg_phys.h>
#include  <gpio.h>


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

                                                            /* ---------------- GIC DISTRIBUTOR INTERFACE -------------------------- */
typedef  struct  GICDistributor
{
    uint32                              dCTRL;              /* GICD_CTLR        Distributor Control Register.                        */
    uint32                              dTYPER;             /* GICD_TYPER       Interrupt Controller Type Register.                  */
    uint32                              dIIDR;              /* GICD_IIDR        Distributor Implementer Identification Register.     */
    uint32                              dRSVD1[29];         /* Reserved.                                                             */
    uint32                              dIGROUPRn[32];      /* GICD_IGROUPRn    Interrupt Security Registers.                        */
    uint32                              dISENABLERn[32];    /* GICD_ISENABLERn  Interrupt Set-Enable Registers.                      */
    uint32                              dICENABLERn[32];    /* GICD_ICENABLERn  Interrupt Clear-Enable Registers.                    */
    uint32                              dISPENDRn[32];      /* GICD_ISPENDRn    Interrupt Set-Pending Registers.                     */
    uint32                              dICPENDRn[32];      /* GICD_ICPENDRn    Interrupt Clear-Pending Registers.                   */
    uint32                              dISACTIVERn[32];    /* GICD_ISACTIVERn  Interrupt Set-Active Registers.                      */
    uint32                              dICACTIVERn[32];    /* GICD_ICACTIVERn  Interrupt Clear-Active Registers.                    */
    uint32                              dIPRIORITYRn[255];  /* GICD_IPRIORITYRn Interrupt Priority Registers.                        */
    uint32                              dRSVD3[1];          /* Reserved.                                                             */
    uint32                              dITARGETSRn[255];   /* GICD_ITARGETSRn  Interrupt Processor Target Registers.                */
    uint32                              dRSVD4[1];          /* Reserved.                                                             */
    uint32                              dICFGRn[64];        /* GICD_ICFGRn      Interrupt Configuration Registers.                   */
    uint32                              dPPISR[1];          /* GICD_PPISR.                                                           */
    uint32                              dSPISRn[15];        /* GICD_SPISRn                                                           */
    uint32                              dRSVD5[112];        /* Reserved.                                                             */
    uint32                              dSGIR;              /* GICD_SGIR        Software Generate Interrupt Register.                */
    uint32                              dRSVD6[3];          /* Reserved.                                                             */
    uint32                              dCPENDSGIRn[4];     /* GICD_CPENDSGIRn  SGInterrupt Clear-Active Registers.                  */
    uint32                              dSPENDSGIRn[4];     /* GICD_SPENDSGIRn  SGInterrupt Set-Active Registers.                    */
} GICDistributor_t;


                                                            /* ---------------- GIC CPU INTERFACE ---------------------------------- */
typedef  struct  GICCpuInterface
{
    uint32                              cCTLR;              /* GICC_CTLR        CPU Interface Control Register.                      */
    uint32                              cPMR;               /* GICC_PMR         Interrupt Priority Mask Register.                    */
    uint32                              cBPR;               /* GICC_BPR         Binary Point Register.                               */
    uint32                              cIAR;               /* GICC_IAR         Interrupt Acknowledge Register.                      */
    uint32                              cEOIR;              /* GICC_EOIR        End Interrupt Register.                              */
    uint32                              cRPR;               /* GICC_RPR         Running Priority Register.                           */
    uint32                              cHPPIR;             /* GICC_HPPIR       Highest Pending Interrupt Register.                  */
    uint32                              cABPR;              /* GICC_ABPR        Aliased Binary Point Register.                       */
    uint32                              cAIAR;              /* GICC_AIAR        Aliased Interrupt Acknowledge Register               */
    uint32                              cAEOIR;             /* GICC_AEOIR       Aliased End Interrupt Register.                      */
    uint32                              cAHPPIR;            /* GICC_AHPPIR      Aliased Highest Pending Interrupt Register.          */
    uint32                              cRSVD[52];          /* Reserved.                                                             */
    uint32                              cIIDR;              /* GICC_IIDR        CPU Interface Identification Register.               */
} GICCpuInterface_t;

/* 0x1b903000u GICC_DIR  */

#define  GIC_DIST                       ((volatile GICDistributor_t  *)(0x1B901000UL))
#define  GIC_CPU                        ((volatile GICCpuInterface_t *)(0x1B902000UL))


                                                            /* ----------- DISTRIBUTOR CONTROL REGISTER -----------                  */
#define  ARM_BIT_GIC_DIST_ICDDCR_EN     (0x00000001UL)      /* Global GIC enable.                                                    */



                                                            /* ---------- CPU_INTERFACE_CONTROL_REGISTER ----------                  */
#define  GIC_CPUIF_CTRL_ENABLEGRP0      (0x00000001UL)      /* Enable secure interrupts.                                             */
#define  GIC_CPUIF_CTRL_ENABLEGRP1      (0x00000002UL)      /* Enable non-secure interrupts.                                         */
#define  GIC_CPUIF_CTRL_ACKCTL          (0x00000004UL)      /* Secure ack of NS interrupts.                                          */


#define  GIC_SGI_TO_TARGETLIST          (0UL)



#define  GIC_SM_WRITEPROTECT            (0x1B921128UL)      /* SW Interrupt Configuration Write Password Register (SW_IRQ_CFG_WR_PW) */
#define  GIC_SM_PASSWORD                (0x5AFEACE5UL)

#define  GIC_SM_IRQ_EN                  (0x1B9210B0UL)      /* SM (SW_IRQ_ENn, n =0~14) 0x1B9210B0 + (0x4 x n)                       */
#define  GIC_SM_IRQ_ASSERT              (0x1B9210ECUL)      /* SM Interrupt-n Register (SW_IRQn, n= 0~14) 0x1B9210EC + (0x4 x n)     */


/**************************************************************************************************/
/*                                             LOCAL VARIABLES                                    */
/***************************************************************************************************/


typedef struct GICIntFuncPtr
{
    GICIsrFunc                          ifpFunc;
    uint8                               ifpIsBothEdge;
    void *                              ifpArg;
}GICIntFuncPtr_t;

static GICIntFuncPtr_t                  GICIsrTable[GIC_INT_SRC_CNT];        /* Interrupt vector table.                              */


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static SALRetCode_t GIC_IntPrioSet_internal
(
    uint32                              uiIntId,
    uint32                              uiPrio
);

static SALRetCode_t GIC_IntConfigSet
(
    uint32                              uiIntId,
    uint8                               ucIntType
);

static uint32 GIC_IntAck
(
    void
);

static void GIC_IntEOI
(
    uint32                              uiIntId
);


/*
***************************************************************************************************
*                                       GIC_IntPrioSet_internal
*
* Set interrupt priority.
*
* @param    uiIntId [in] Index of Interrupt Source id.
* @param    uiPrio  [in] Interrupt priority.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt priority set succeed.
*           SAL_RET_FAILED   Error setting interrupt priority.
*
* Notes     internal function
*
***************************************************************************************************
*/

static SALRetCode_t GIC_IntPrioSet_internal
(
    uint32                              uiIntId,
    uint32                              uiPrio
)
{
    uint32        uiRegOffset;
    uint32        uiRegBitField;
    uint32        uiGICD_IPRIORITYRn;
    SALRetCode_t  ucRet;

    uiRegOffset        = 0;
    uiRegBitField      = 0;
    uiGICD_IPRIORITYRn = 0;
    ucRet              = (SALRetCode_t)SAL_RET_FAILED;

    if ( (uiPrio < GIC_PRIORITY_NO_MEAN) && (uiIntId < GIC_INT_SRC_CNT) )
    {
        uiRegOffset = (uiIntId >> 2u);
        uiRegBitField = (uiIntId & 0x03u);

        uiGICD_IPRIORITYRn = GIC_DIST->dIPRIORITYRn[uiRegOffset];
        uiGICD_IPRIORITYRn = (uint32)(uiGICD_IPRIORITYRn & ~((uint32)0xFFu << (uiRegBitField * 8u)));
        uiGICD_IPRIORITYRn = (uint32)(uiGICD_IPRIORITYRn | (((uiPrio<<4) & 0xF0u) << (uiRegBitField * 8u)));

        GIC_DIST->dIPRIORITYRn[uiRegOffset] = uiGICD_IPRIORITYRn;
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}


/*
***************************************************************************************************
*                                       GIC_IntConfigSet
*
* Set interrupt trigger type.
*
* @param    uiIntId  [in] Index of Interrupt Source id.
* @param    ucIntType[in] Interrupt trigger type.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt trigger type set succeed.
*           SAL_RET_FAILED   Error setting interrupt trigger type.
*
* Notes     internal function
*
***************************************************************************************************
*/

static SALRetCode_t GIC_IntConfigSet
(
    uint32                          uiIntId,
    uint8                           ucIntType
)
{
    uint32         uiRegOffset;
    uint32         uiRegMask;
    uint32         uiGICD_ICFGRn;
    SALRetCode_t   ucRet;

    uiRegOffset   = 0;
    uiRegMask     = 0;
    uiGICD_ICFGRn = 0;
    ucRet         = (SALRetCode_t)SAL_RET_FAILED;

    if ( uiIntId < GIC_INT_SRC_CNT)
    {
        uiRegOffset   = (uiIntId >> 4u);
        uiRegMask     = (uint32)((uint32)0x2u << ((uiIntId & 0xfu) * 2u));
        uiGICD_ICFGRn = GIC_DIST->dICFGRn[uiRegOffset];

        if (((ucIntType & (uint8)GIC_INT_TYPE_LEVEL_HIGH)== (uint8)GIC_INT_TYPE_LEVEL_HIGH)  || \
			((ucIntType & (uint8)GIC_INT_TYPE_LEVEL_LOW)== (uint8)GIC_INT_TYPE_LEVEL_LOW))
        {
            uiGICD_ICFGRn = (uint32)(uiGICD_ICFGRn & ~uiRegMask);
        }
        else
        {
            uiGICD_ICFGRn = (uint32)(uiGICD_ICFGRn | uiRegMask);
        }

        GIC_DIST->dICFGRn[uiRegOffset] = uiGICD_ICFGRn;
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}



/*
***************************************************************************************************
*                                       GIC_IntAck
*
* obtain the interrupt ID
*
* @param    none
*
* @return   return GICC_IAR
*
* Notes     internal function
*           Caller(s)   : GIC_IntHandler().
***************************************************************************************************
*/

static uint32  GIC_IntAck
(
    void
)
{
    uint32 int_id;

    int_id = GIC_CPU->cIAR;

    return (int_id);
}

/*
***************************************************************************************************
*                                       GIC_IntEOI
*
* inform End of interrupt to CPU Interface of GIC
*
* @param    uiIntId  [in] Index of Interrupt Source id.
*
* @return   none
*
* Notes     internal function
*               Caller(s) : GIC_IntHandler().
***************************************************************************************************
*/

static void GIC_IntEOI
(
    uint32                              uiIntId
)
{
    GIC_CPU->cEOIR = uiIntId;
    return;
}


/**************************************************************************************************
*                                       GIC_Init
*
* Init GIC400
*
* @param    none
*
* @return   none
*
* Notes
*               Caller(s) : GIC_Init().
*               set default priority value to 0xA0
**************************************************************************************************/

void GIC_Init
(
    void
)
{
    uint32 uiRegOffset;

    uiRegOffset = 0;

    GIC_DIST->dCTRL    &= (uint32)(~ARM_BIT_GIC_DIST_ICDDCR_EN);
    GIC_DIST->dCTRL    |= (uint32)ARM_BIT_GIC_DIST_ICDDCR_EN;

    for (; uiRegOffset <= ((uint32)GIC_PMGPIO / 4UL); uiRegOffset++)
    {
        GIC_DIST->dIPRIORITYRn[uiRegOffset] = 0xA0A0A0A0UL;
    }

    GIC_CPU->cPMR  = 0xFFUL;
    GIC_CPU->cCTLR |= GIC_CPUIF_CTRL_ENABLEGRP0;
    (void)SAL_CoreMB();

    return;
}

/*
***************************************************************************************************
*                                       GIC_IntSrcEn
*
* Enable interrupt source id.
*
* @param    uiIntId  [in] Index of Interrupt Source id.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt enable succeed.
*           SAL_RET_FAILED   Error enabling interrupt.
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GIC_IntSrcEn
(
    uint32                              uiIntId
)
{
    SALRetCode_t ucRet;
    uint32       uiRegOffset;
    uint32       uiBit;

    ucRet       = (SALRetCode_t)SAL_RET_FAILED;
    uiRegOffset = 0;
    uiBit       = 0;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        uiRegOffset = (uiIntId >> 5u);                      /* Calculate the register offset.                                        */
        uiBit       = (uiIntId & 0x1Fu);                    /* Mask bit ID.                                                          */

        GIC_DIST->dISENABLERn[uiRegOffset] = ((uint32)1UL << uiBit);

        if (GICIsrTable[uiIntId].ifpIsBothEdge == (1UL))
        {
            uiRegOffset = ((uiIntId + 10UL) >> 5UL);        /* Calculate the register offset.                                        */
            uiBit       = ((uiIntId + 10UL) & 0x1FUL);      /* Mask bit ID.                                                          */

            GIC_DIST->dISENABLERn[uiRegOffset] = ((uint32)1UL << uiBit);
        }

        (void)SAL_CoreMB();
        ucRet           = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}



/*
***************************************************************************************************
*                                       GIC_IntSrcDis
*
* Disable interrupt source id.
*
* @param    uiIntId  [in] Index of Interrupt Source id.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt disable succeed.
*           SAL_RET_FAILED   Error disabling interrupt.
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GIC_IntSrcDis
(
    uint32                              uiIntId
)
{
    SALRetCode_t   ucRet;
    uint32         uiRegOffset;
    uint32         uiBit;

    ucRet       = (SALRetCode_t)SAL_RET_FAILED;
    uiRegOffset = 0;
    uiBit       = 0;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        uiRegOffset = (uiIntId >> 5UL);                       /* Calculate the register offset.                                      */
        uiBit       = (uiIntId & 0x1FUL);                     /* Mask bit ID.                                                        */

        GIC_DIST->dICENABLERn[uiRegOffset] = ((uint32)1UL << uiBit);

        if (GICIsrTable[uiIntId].ifpIsBothEdge == (1UL))
        {
            uiRegOffset = ((uiIntId + 10UL) >> 5UL);          /* Calculate the register offset.                                      */
            uiBit       = ((uiIntId + 10UL) & 0x1FUL);        /* Mask bit ID.                                                        */

            GIC_DIST->dICENABLERn[uiRegOffset] = ((uint32)1UL << uiBit);
        }
        ucRet           = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}



/*
***************************************************************************************************
*                                       GIC_IntPrioSet
*
* Set interrupt priority.
*
* @param    uiIntId [in] Index of Interrupt Source id.
* @param    uiPrio  [in] Interrupt priority.
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt priority set succeed.
*           SAL_RET_FAILED   Error setting interrupt priority.
* Notes     internal function
*
***************************************************************************************************
*/


SALRetCode_t GIC_IntPrioSet
(
    uint32                              uiIntId,
    uint32                              uiPrio
)
{
    SALRetCode_t ucRet;

    ucRet = (SALRetCode_t)SAL_RET_SUCCESS;

    (void)SAL_CoreCriticalEnter();
    ucRet = GIC_IntPrioSet_internal(uiIntId, uiPrio);
    (void)SAL_CoreCriticalExit();
    return ucRet;
}



/*
***************************************************************************************************
*                                       GIC_IntVectSet
*
* Configure interrupt vector.
*
* @param    uiIntId   [in] Index of Interrupt Source id.
* @param    uiPrio    [in] Interrupt priority. (0~15)
* @param    ucIntType [in] Interrupt trigger type.
* @param    fnIntFunc [in] ISR function pointer.
* @param    pIntArg   [in] ISR function Argument
*
* @return   Interrupt configuration result.
*           SAL_RET_SUCCESS  Interrupt successfully set.
*           SAL_RET_FAILED   Error setting interrupt.
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GIC_IntVectSet
(
    uint32                              uiIntId,
    uint32                              uiPrio,
    uint8                               ucIntType,
    GICIsrFunc                          fnIntFunc,
    void *                              pIntArg
)
{
    uint32        uiRevIntId;
    SALRetCode_t  ucRet;

    uiRevIntId = 0;
    ucRet      = (SALRetCode_t)SAL_RET_SUCCESS;

    if((uiPrio > GIC_PRIORITY_NO_MEAN)||(uiIntId >= GIC_INT_SRC_CNT))
    {
        ucRet = (SALRetCode_t)SAL_RET_FAILED;
    }
    else
    {
        (void)SAL_CoreCriticalEnter();                                      /* Prevent partially configured interrupts.              */

        (void)GIC_IntPrioSet_internal(uiIntId, uiPrio);
        (void)GIC_IntConfigSet(uiIntId, ucIntType);

        GICIsrTable[uiIntId].ifpFunc = fnIntFunc;
        GICIsrTable[uiIntId].ifpArg = pIntArg;
        GICIsrTable[uiIntId].ifpIsBothEdge = 0;

        if ((uiIntId >= (uint32)GIC_EINT_START_INT) && (uiIntId <= (uint32)GIC_EINT_END_INT)  /* Set reversed external interrupt     */
            && (ucIntType == (uint8)GIC_INT_TYPE_EDGE_BOTH))
        {                                                                   /* for supporting both edge.                             */

            uiRevIntId = (uiIntId + GIC_EINT_NUM);                          /* add offset of IRQn                                    */

            (void)GIC_IntPrioSet_internal(uiRevIntId, uiPrio);
            (void)GIC_IntConfigSet(uiRevIntId, ucIntType);

            GICIsrTable[uiRevIntId].ifpFunc = fnIntFunc;
            GICIsrTable[uiRevIntId].ifpArg = pIntArg;
            GICIsrTable[uiIntId].ifpIsBothEdge = (1U);
        }

        (void)SAL_CoreCriticalExit();
    }

    return ucRet;
}



/*
***************************************************************************************************
*                                       GIC_IntHandlerWithParam
*
*  Call the ISR function registered in the GICIsrTable that matches the interrupt source id.
*
* @param    uiGICC_IAR [in] uiGICC_IAR
*
* @return   none
*
* Notes
*               This function used in freeRTOS
***************************************************************************************************
*/

void GIC_IntHandlerWithParam
(
    uint32                              uiGICC_IAR
)
{
    uint32        uiIAR;
    uint32        uiIntId;
    GICIsrFunc    fnIsr;
    void          *pIntArg;

    uiIAR   = uiGICC_IAR;
    uiIntId = uiIAR & 0x3FFU;                               /* Mask away the CPUID.                                                  */
    fnIsr   = (GICIsrFunc)NULL;
    pIntArg = NULL_PTR;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        fnIsr   = GICIsrTable[uiIntId].ifpFunc;             /* Fetch ISR handler.                                                    */
        pIntArg = GICIsrTable[uiIntId].ifpArg;

        if(fnIsr != (GICIsrFunc)NULL)
        {
            (*fnIsr)(pIntArg);                              /* Call ISR handler.                                                     */
        }
        (void)SAL_CoreMB();                                 /* Memory barrier before ending the interrupt.                           */
    }

    return;
}


/*
***************************************************************************************************
*                                       GIC_IntHandler
*
*  Call the ISR function registered in the GICIsrTable that matches the interrupt source id.
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

void GIC_IntHandler
(
    void
)
{
    uint32        uiIAR;
    uint32        uiIntId;
    GICIsrFunc    fnIsr;
    void          *pIntArg;

    uiIAR   = GIC_IntAck();
    uiIntId = uiIAR & 0x3FFU;                               /* Mask away the CPUID.                                                  */
    fnIsr   = (GICIsrFunc)NULL;
    pIntArg = NULL_PTR;

    if(uiIntId < GIC_INT_SRC_CNT)
    {
        fnIsr   = GICIsrTable[uiIntId].ifpFunc;             /* Fetch ISR handler.                                                    */
        pIntArg = GICIsrTable[uiIntId].ifpArg;

        if(fnIsr != (GICIsrFunc)NULL)
        {
            (*fnIsr)(pIntArg);                              /* Call ISR handler.                                                     */
        }

        (void)SAL_CoreMB();                                 /* Memory barrier before ending the interrupt.                           */
        GIC_IntEOI(uiIAR);
    }

    return;
}



/**************************************************************************************************
*                                       GIC_IntExtSet
*
* Configure interrupt vector.
*
* @param    uiIntId   [in] Index of Interrupt Source id.
* @param    uiGpio    [in] register offset of gpio, refer to TCC_GPx(n) macro
*
* @return   Configuration result.
*           SAL_RET_SUCCESS  Externel Interrupt successfully set.
*           SAL_RET_FAILED   Error setting interrupt.
*
* Notes
*           ExtIntr table .. refer to External Interrupt and External Input Clock Selection at the datasheet.
*           GPIO_MB, GPIO_MC, GPIO_MD is not apply to TCC8059
*           A : 0 ~31 GPIO_A[31:00]
*           B :	32 ~ 60 GPIO_B[28:00]
*           C :	61 ~ 90 GPIO_C[29:00]
*           E :	91 ~ 110 GPIO_E[19:00]
*           G :	111~121 GPIO_G[10:00]
*           H :	122~133 GPIO_H[11:00]
*           MA:	134~163 GPIO_MA[29:00]
*           MB:	164~195 GPIO_MB[31:00]
*           MC:	196~225 GPIO_MA[29:00]
*           MD:	226~243 GPIO_MD[17:00]

**************************************************************************************************/
SALRetCode_t GIC_IntExtSet
(
    uint32                              uiIntId,
    uint32                              uiGpio
)
{
    SALRetCode_t  ucRet;
    uint32        uiEintSel;
    uint32        uiBitField;
    uint32        uiExtIntGpioIdx;
    uint32        uiEIntSel;
    uint32        uiEIntSelMask;

    static const uint32 ExtIntr[244] =
    {
        GPIO_GPA(0UL),   GPIO_GPA(1UL),   GPIO_GPA(2UL),   GPIO_GPA(3UL),   GPIO_GPA(4UL),   GPIO_GPA(5UL),   GPIO_GPA(6UL),   GPIO_GPA(7UL),
        GPIO_GPA(8UL),   GPIO_GPA(9UL),   GPIO_GPA(10UL),  GPIO_GPA(11UL),  GPIO_GPA(12UL),  GPIO_GPA(13UL),  GPIO_GPA(14UL),  GPIO_GPA(15UL),
        GPIO_GPA(16UL),  GPIO_GPA(17UL),  GPIO_GPA(18UL),  GPIO_GPA(18UL),  GPIO_GPA(20UL),  GPIO_GPA(21UL),  GPIO_GPA(22UL),  GPIO_GPA(23UL),
        GPIO_GPA(24UL),  GPIO_GPA(25UL),  GPIO_GPA(26UL),  GPIO_GPA(27UL),  GPIO_GPA(28UL),  GPIO_GPA(29UL),  GPIO_GPA(30UL),  GPIO_GPA(31UL),

        GPIO_GPB(0UL),   GPIO_GPB(1UL),   GPIO_GPB(2UL),   GPIO_GPB(3UL),   GPIO_GPB(4UL),   GPIO_GPB(5UL),   GPIO_GPB(6UL),   GPIO_GPB(7UL),
        GPIO_GPB(8UL),   GPIO_GPB(9UL),   GPIO_GPB(10UL),  GPIO_GPB(11UL),  GPIO_GPB(12UL),  GPIO_GPB(13UL),  GPIO_GPB(14UL),  GPIO_GPB(15UL),
        GPIO_GPB(16UL),  GPIO_GPB(17UL),  GPIO_GPB(18UL),  GPIO_GPB(19UL),  GPIO_GPB(20UL),  GPIO_GPB(21UL),  GPIO_GPB(22UL),  GPIO_GPB(23UL),
        GPIO_GPB(24UL),  GPIO_GPB(25UL),  GPIO_GPB(26UL),  GPIO_GPB(27UL),  GPIO_GPB(28UL),

        GPIO_GPC(0UL),   GPIO_GPC(1UL),   GPIO_GPC(2UL),   GPIO_GPC(3UL) ,  GPIO_GPC(4UL),   GPIO_GPC(5UL),   GPIO_GPC(6UL),   GPIO_GPC(7UL),
        GPIO_GPC(8UL),   GPIO_GPC(9UL),   GPIO_GPC(10UL),  GPIO_GPC(11UL),  GPIO_GPC(12UL),  GPIO_GPC(13UL),  GPIO_GPC(14UL),  GPIO_GPC(15UL),
        GPIO_GPC(16UL),  GPIO_GPC(17UL),  GPIO_GPC(18UL),  GPIO_GPC(19UL),  GPIO_GPC(20UL),  GPIO_GPC(21UL),  GPIO_GPC(22UL),  GPIO_GPC(23UL),
        GPIO_GPC(24UL),  GPIO_GPC(25UL),  GPIO_GPC(26UL),  GPIO_GPC(27UL),  GPIO_GPC(28UL),  GPIO_GPC(29UL),

        GPIO_GPE(0UL),   GPIO_GPE(1UL),   GPIO_GPE(2UL),   GPIO_GPE(3UL),   GPIO_GPE(4UL),   GPIO_GPE(5UL),   GPIO_GPE(6UL),   GPIO_GPE(7UL),
        GPIO_GPE(8UL),   GPIO_GPE(9UL),   GPIO_GPE(10UL),  GPIO_GPE(11UL),  GPIO_GPE(12UL),  GPIO_GPE(13UL),  GPIO_GPE(14UL),  GPIO_GPE(15UL),
        GPIO_GPE(16UL),  GPIO_GPE(17UL),  GPIO_GPE(18UL),  GPIO_GPE(19UL),

        GPIO_GPG(0UL),   GPIO_GPG(1UL),   GPIO_GPG(2UL),   GPIO_GPG(3UL),   GPIO_GPG(4UL),   GPIO_GPG(5UL),   GPIO_GPG(6UL),   GPIO_GPG(7UL),
        GPIO_GPG(8UL),   GPIO_GPG(9UL),   GPIO_GPG(10UL),

        GPIO_GPH(0UL),   GPIO_GPH(1UL),   GPIO_GPH(2UL),   GPIO_GPH(3UL),   GPIO_GPH(4UL),   GPIO_GPH(5UL),   GPIO_GPH(6UL),   GPIO_GPH(7UL),
        GPIO_GPH(8UL),   GPIO_GPH(9UL),   GPIO_GPH(10UL),  GPIO_GPH(11UL),

        GPIO_GPMA(0UL),  GPIO_GPMA(1UL),  GPIO_GPMA(2UL),  GPIO_GPMA(3UL),  GPIO_GPMA(4UL),  GPIO_GPMA(5UL),  GPIO_GPMA(6UL),  GPIO_GPMA(7UL),
        GPIO_GPMA(8UL),  GPIO_GPMA(9UL),  GPIO_GPMA(10UL), GPIO_GPMA(11UL), GPIO_GPMA(12UL), GPIO_GPMA(13UL), GPIO_GPMA(14UL), GPIO_GPMA(15UL),
        GPIO_GPMA(16UL), GPIO_GPMA(17UL), GPIO_GPMA(18UL), GPIO_GPMA(19UL), GPIO_GPMA(20UL), GPIO_GPMA(21UL), GPIO_GPMA(22UL), GPIO_GPMA(23UL),
        GPIO_GPMA(24UL), GPIO_GPMA(25UL), GPIO_GPMA(26UL), GPIO_GPMA(27UL), GPIO_GPMA(28UL), GPIO_GPMA(29UL),

        GPIO_GPMB(0UL),  GPIO_GPMB(1UL),  GPIO_GPMB(2UL),  GPIO_GPMB(3UL),  GPIO_GPMB(4UL),  GPIO_GPMB(5UL),  GPIO_GPMB(6UL),  GPIO_GPMB(7UL),
        GPIO_GPMB(8UL),  GPIO_GPMB(9UL),  GPIO_GPMB(10UL), GPIO_GPMB(11UL), GPIO_GPMB(12UL), GPIO_GPMB(13UL), GPIO_GPMB(14UL), GPIO_GPMB(15UL),
        GPIO_GPMB(16UL), GPIO_GPMB(17UL), GPIO_GPMB(18UL), GPIO_GPMB(19UL), GPIO_GPMB(20UL), GPIO_GPMB(21UL), GPIO_GPMB(22UL), GPIO_GPMB(23UL),
        GPIO_GPMB(24UL), GPIO_GPMB(25UL), GPIO_GPMB(26UL), GPIO_GPMB(27UL), GPIO_GPMB(28UL), GPIO_GPMB(29UL), GPIO_GPMB(30UL), GPIO_GPMB(31UL),

        GPIO_GPMC(0UL),  GPIO_GPMC(1UL),  GPIO_GPMC(2UL),  GPIO_GPMC(3UL),  GPIO_GPMC(4UL),  GPIO_GPMC(5UL),  GPIO_GPMC(6UL),  GPIO_GPMC(7UL),
        GPIO_GPMC(8UL),  GPIO_GPMC(9UL),  GPIO_GPMC(10UL), GPIO_GPMC(11UL), GPIO_GPMC(12UL), GPIO_GPMC(13UL), GPIO_GPMC(14UL), GPIO_GPMC(15UL),
        GPIO_GPMC(16UL), GPIO_GPMC(17UL), GPIO_GPMC(18UL), GPIO_GPMC(19UL), GPIO_GPMC(20UL), GPIO_GPMC(21UL), GPIO_GPMC(22UL), GPIO_GPMC(23UL),
        GPIO_GPMC(24UL), GPIO_GPMC(25UL), GPIO_GPMC(26UL), GPIO_GPMC(27UL), GPIO_GPMC(28UL), GPIO_GPMC(29UL),

        GPIO_GPMD(0UL),  GPIO_GPMD(1UL),  GPIO_GPMD(2UL),  GPIO_GPMD(3UL),  GPIO_GPMD(4UL),  GPIO_GPMD(5UL),  GPIO_GPMD(6UL),  GPIO_GPMD(7UL),
        GPIO_GPMD(8UL),  GPIO_GPMD(9UL),  GPIO_GPMD(10UL), GPIO_GPMD(11UL), GPIO_GPMD(12UL), GPIO_GPMD(13UL), GPIO_GPMD(14UL), GPIO_GPMD(15UL),
        GPIO_GPMD(16UL), GPIO_GPMD(17UL)
    };

    ucRet			= (SALRetCode_t)SAL_RET_SUCCESS;
    uiEintSel		= ((MCU_BSP_GPIO_BASE + 0x280UL) + (4UL*((uiIntId-(uint32)GIC_EXT0)/4UL))); /* EINT_SEL0,2,3 */
    uiBitField		= 0;
    uiExtIntGpioIdx = 0;
    uiEIntSel		= 0;
    uiEIntSelMask	= 0;

    if ((uiIntId < (uint32)GIC_EINT_START_INT) || (uiIntId > (uint32)GIC_EINT_END_INT))
    {
        ucRet = (SALRetCode_t)SAL_RET_FAILED;
    }
    else
    {

        for ( ; uiExtIntGpioIdx < SAL_ArraySize(ExtIntr) ; uiExtIntGpioIdx++)
        {
            if (ExtIntr[uiExtIntGpioIdx] == uiGpio)
            {
                break;
            }
        }

        if (uiExtIntGpioIdx >= SAL_ArraySize(ExtIntr))
        {
            ucRet = (SALRetCode_t)SAL_RET_FAILED;
        }
        else
        {
            uiBitField      = (uint32)(8UL * ((uiIntId - (uint32)GIC_EXT0) % 4UL));
            uiEIntSelMask   = ((uint32)0x7FUL << uiBitField);

            uiEIntSel = (uint32)SAL_ReadReg(uiEintSel);
            uiEIntSel = (uint32)((uiEIntSel & ~uiEIntSelMask) | (uiExtIntGpioIdx << uiBitField));
            SAL_WriteReg(uiEIntSel, uiEintSel);
        }
    }

    return ucRet;
}

/*
***************************************************************************************************
*                                       GIC_IntSGI
*
* Controls the generation of SGIs
*
* @param    uiIntId   [in] Index of Interrupt Source id(SGI).
*
* @return
*           SAL_RET_SUCCESS  register setting succeed
*           SAL_RET_FAILED   register setting fail(out of range)
*
* Notes
*           This function can be used to test the operation of GIC.
***************************************************************************************************
*/

SALRetCode_t GIC_IntSGI
(
    uint32                              uiIntId
)
{
    SALRetCode_t  ucRet;
    uint32        uiTargetListFilter;
    uint32        uiCPUTargetList;
    uint32        uiNASTT;

    ucRet              = (SALRetCode_t)SAL_RET_FAILED;
    uiTargetListFilter = GIC_SGI_TO_TARGETLIST;
    uiCPUTargetList    = 0x1UL;                             /* bitfiled 0 : cpu #0, bitfield n : cpu #n, n : 0 ~ 7          */
    uiNASTT            = 0UL;                               /* 0 : group 0 , 1: group 1                                     */

    if(uiIntId <= 15UL)
    {
        GIC_DIST->dSGIR = (uint32)((uiTargetListFilter & 0x3UL) << 24) | ((uiCPUTargetList & 0xffUL)<<16)\
                          | ((uiNASTT & 0x1UL)<<15) | (uiIntId & 0xfUL);

        ucRet           = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}

/*
***************************************************************************************************
*                                       GIC_SmIrqEn
*
* System Safety Mechanism (SM)'s SW_IRQn(GIC SPI) Enable or Disable
*
* @param    uiIntId   [in] Index of Interrupt Source id.
* @param    bEnable   [in] SM SW interrupt 1 : Enable, 0 : Disable
*
* @return   The result of SM SW Interrupt setting
*           SAL_RET_SUCCESS  register setting succeed
*           SAL_RET_FAILED   register setting fail
*
* Notes
*           This function can be used to test the operation of GIC.
***************************************************************************************************
*/

SALRetCode_t GIC_SmIrqEn
(
    uint32                              uiIntId,
    boolean                             bEnable
)
{
    uint32        uiSmIrqEn;
    uint32        uiRegOffset;
    uint32        uiBit;
    uint32        uiIntIdForSm;
    SALRetCode_t  ucRet;

    uiSmIrqEn    = 0;
    uiRegOffset  = 0;
    uiBit        = 0;
    uiIntIdForSm = 0;
    ucRet        = (SALRetCode_t)SAL_RET_FAILED;

    if(((uiIntId >= GIC_SPI_START) && (uiIntId < GIC_INT_SRC_CNT)) && (bEnable < (uint8)2))
    {
        uiIntIdForSm = (uiIntId - (uint32)GIC_SPI_START);
        uiRegOffset = (uint32)(((uiIntIdForSm & ~0x1FUL)>>5UL) << 2UL);

        uiBit = ((uint32)0x1UL << (uiIntIdForSm & 0x1FUL));

        uiSmIrqEn = SAL_ReadReg((uint32)(GIC_SM_IRQ_EN + uiRegOffset));

        if(bEnable == (uint8)0)
        {
            uiSmIrqEn = (uiSmIrqEn & ~uiBit);
        }
        else
        {
            uiSmIrqEn = (uiSmIrqEn | uiBit);
        }

        SAL_WriteReg(GIC_SM_PASSWORD, GIC_SM_WRITEPROTECT);
        SAL_WriteReg(uiSmIrqEn, (uint32)(GIC_SM_IRQ_EN + uiRegOffset));
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}

/*
***************************************************************************************************
*                                       GIC_SmIrqSetHigh
*
* System Safety Mechanism (SM)'s SW_IRQn(GIC SPI) input to GIC or not
*
* @param    uiIntId   [in] Index of Interrupt Source id.
* @param    bEnable   [in] SM SW interrupt If bEnable is 1, SW_IRQn(GIC SPI) input to GIC, 0 :
*
* @return   The result of SM SW Interrupt setting
*           SAL_RET_SUCCESS  register setting succeed
*           SAL_RET_FAILED   register setting fail
*
* Notes
*           This function can be used to test the operation of GIC.
***************************************************************************************************
*/

SALRetCode_t GIC_SmIrqSetHigh
(
    uint32                              uiIntId,
    boolean                             bEnable
)
{
    uint32        uiSmIrqAssert;
    uint32        uiRegOffset;
    uint32        uiBit;
    uint32        uiIntIdForSm;
    SALRetCode_t  ucRet;

    uiSmIrqAssert = 0;
    uiRegOffset   = 0;
    uiBit         = 0;
    uiIntIdForSm  = 0;
    ucRet         = (SALRetCode_t)SAL_RET_FAILED;

    if(((uiIntId >= GIC_SPI_START) && (uiIntId < GIC_INT_SRC_CNT)) && (bEnable < (uint8)2))
    {
        uiIntIdForSm = (uiIntId - (uint32)GIC_SPI_START);
        uiRegOffset  = (uint32)(((uiIntIdForSm & ~0x1FUL)>>5UL) << 2UL);
        uiBit        = ((uint32)0x1UL << (uiIntIdForSm & 0x1FUL));

        uiSmIrqAssert = SAL_ReadReg((uint32)(GIC_SM_IRQ_ASSERT + uiRegOffset));

        if(bEnable == (uint8)0)
        {
            uiSmIrqAssert = (uint32)(uiSmIrqAssert & ~uiBit);
        }
        else
        {
            uiSmIrqAssert = (uint32)(uiSmIrqAssert | uiBit);
        }


        SAL_WriteReg(GIC_SM_PASSWORD, GIC_SM_WRITEPROTECT);
        SAL_WriteReg(uiSmIrqAssert,   (uint32)(GIC_SM_IRQ_ASSERT + uiRegOffset));
        ucRet = (SALRetCode_t)SAL_RET_SUCCESS;
    }

    return ucRet;
}

