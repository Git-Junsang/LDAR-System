/*
***************************************************************************************************
*
*   FileName : midf.c
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

#include <sal_internal.h>
#include "midf_dev.h"
#include "midf.h"
#include "debug.h"
#ifdef MIDF_SFATEY_MECHANISM
#include "fmu.h"
#endif

#if (DEBUG_ENABLE)
    #define MIDF_LOGD(fmt, args...)     {LOGD(DBG_TAG_MIDF, fmt, ## args)}
    #define MIDF_LOGE(fmt, args...)     {LOGE(DBG_TAG_MIDF, fmt, ## args)}
#else
    #define MIDF_LOGD(fmt, args...)
    #define MIDF_LOGE(fmt, args...)
#endif

/**
 * Filter Register Descriptions
 */
#define MIDF_BASE_L(ch)                 (MIDF_FILTER_BASE + 0x100UL + ((ch) * 0x20UL))
#define MIDF_BASE_H(ch)                 (MIDF_FILTER_BASE + 0x104UL + ((ch) * 0x20UL))
#define MIDF_TOP_L(ch)                  (MIDF_FILTER_BASE + 0x108UL + ((ch) * 0x20UL))
#define MIDF_TOP_H(ch)                  (MIDF_FILTER_BASE + 0x10CUL + ((ch) * 0x20UL))
#define MIDF_ATTRIBUTES(ch)             (MIDF_FILTER_BASE + 0x110UL + ((ch) * 0x20UL))
#define MIDF_ID_ACCESS(ch)              (MIDF_FILTER_BASE + 0x114UL + ((ch) * 0x20UL))

#define MIDF_CFG_PW                 (0x5AFEACE5UL)
#define MIDF_CFG_WR_PW                 (MIDF_FILTER_CFG_BASE + 0x1c)
#define MIDF_CFG_WR_LOCK                 (MIDF_FILTER_CFG_BASE + 0x20)
#define MIDF_CFG_GROUP_EN                 (MIDF_FILTER_CFG_BASE + 0x30)
#define MIDF_CFG_GROUP_STATUS                 (MIDF_FILTER_CFG_BASE + 0x34)
#define MIDF_CFG_FAULT_CTL                 (MIDF_FILTER_CFG_BASE + 0x38)
#define MIDF_CFG_FAULT_STATUS                 (MIDF_FILTER_CFG_BASE + 0x3c)
#define MIDF_CFG_LOCK                 (1UL)
#define MIDF_CFG_UNLOCK                 (0UL)
/**
 * Other Definition
 */
#define MIDF_MAX_CH                     (8UL)
#define MIDF_TYPE_ALL                   (MIDF_TYPE_READ|MIDF_TYPE_WRITE)
#define MIDF_ACCESS_DEFAULT             (0x10001UL << MIDF_ID_CR5)    /* CR5 can access whole area */

/**
 * Static Variables
 */
static uint8 ucMidfFilterInitialized    = 0;
#ifdef MIDF_SFATEY_MECHANISM
static void MIDF_FaultIRQHandler
(
    void * pArg
);


static void MIDF_FaultFIQHandler
(
    void * pArg
);


static void MIDF_RecoverFaultStatus
(
    void
);
#endif

/**
 * Initialize MID Filter
 */
/*
***************************************************************************************************
*                                          MIDF_FilterInitialize
*
* Register MID Filter
*
* @param    None
* @return   None
*
* Notes
*
***************************************************************************************************
*/

void MIDF_FilterInitialize
(
    void
)
{
    uint32  ch;
    uint32  access;
    uint32  regBase;
    uint32  config;

    if (ucMidfFilterInitialized == 0UL)
    {
        /* clear all region */
        ch  = MIDF_MAX_CH;

        while(ch!=0UL)
        {
            regBase = MIDF_ATTRIBUTES(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_BASE_L(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_BASE_H(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_TOP_L(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_TOP_H(ch);
            SAL_WriteReg(0x0UL, regBase);
            regBase = MIDF_ID_ACCESS(ch);
            SAL_WriteReg(0x0UL, regBase);
            ch--;
        }

        /* configure Master IDs */
        regBase = MIDF_FILTER_CFG_BASE;
        config  = (uint32)((MIDF_ID_CR5 << (uint32)20UL) | ((uint32)MIDF_ID_CR5 << (uint32)16UL) | ((uint32)MIDF_ID_AP << (uint32)4UL) | ((uint32)MIDF_ID_AP << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        regBase = MIDF_FILTER_CFG_BASE + 0x04UL;
        config  = (uint32)(((uint32)MIDF_ID_JTAG << (uint32)20UL) | ((uint32)MIDF_ID_JTAG<<(uint32)16UL) | \
                ((uint32)MIDF_ID_DMA1 << (uint32)12UL) | ((uint32)MIDF_ID_DMA1 << (uint32)8UL) | ((uint32)MIDF_ID_DMA0 << (uint32)4UL) | ((uint32)MIDF_ID_DMA0 << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        regBase = MIDF_FILTER_CFG_BASE + 0x08UL;
        config  = (uint32)(((uint32)MIDF_ID_CAN2 << (uint32)20UL) | ((uint32)MIDF_ID_CAN2<<(uint32)16UL) | \
                ((uint32)MIDF_ID_CAN1 << (uint32)12UL) | ((uint32)MIDF_ID_CAN1 << (uint32)8UL) | ((uint32)MIDF_ID_CAN0 << (uint32)4UL) | ((uint32)MIDF_ID_CAN0 << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        regBase = MIDF_FILTER_CFG_BASE + 0x0CUL;
        config  = (uint32)(((uint32)MIDF_ID_GPSB3_4_5 << (uint32)28UL) | ((uint32)MIDF_ID_GPSB3_4_5 << (uint32)24UL) | ((uint32)MIDF_ID_GPSB2 << (uint32)20UL) | ((uint32)MIDF_ID_GPSB2 << (uint32)16UL) | \
                ((uint32)MIDF_ID_GPSB1 << (uint32)12UL) | ((uint32)MIDF_ID_GPSB1 << (uint32)8UL) | ((uint32)MIDF_ID_GPSB0 << (uint32)4UL) | ((uint32)MIDF_ID_GPSB0 << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        regBase = MIDF_FILTER_CFG_BASE + 0x10UL;
        config  = (uint32)(((uint32)MIDF_ID_GPSB3_4_5 << (uint32)28UL) | ((uint32)MIDF_ID_GPSB3_4_5 << (uint32)24UL) | ((uint32)MIDF_ID_UART2 << (uint32)20UL) | ((uint32)MIDF_ID_UART2 << (uint32)16UL) | \
                ((uint32)MIDF_ID_UART1 << (uint32)12UL) | ((uint32)MIDF_ID_UART1 << (uint32)8UL) | ((uint32)MIDF_ID_UART0 << (uint32)4UL) | ((uint32)MIDF_ID_UART0 << (uint32)0UL));
        SAL_WriteReg(config, regBase);

        /* set region0 */
        regBase = MIDF_BASE_L(0UL);
        SAL_WriteReg(0x0UL, regBase);
     regBase = MIDF_BASE_H(0UL);
        SAL_WriteReg(0UL, regBase);
     regBase = MIDF_TOP_L(0UL);
        SAL_WriteReg(0xFFFFFFFFU, regBase);
     regBase = MIDF_TOP_H(0UL);
        SAL_WriteReg(0UL, regBase);
        access = (1UL<<MIDF_ID_CR5) | (1UL<<MIDF_ID_JTAG);
        access |= (access<<16);
     regBase = MIDF_ID_ACCESS(0UL);
        SAL_WriteReg(access, regBase);
     regBase = MIDF_ATTRIBUTES(0UL);
        SAL_WriteReg(1UL, regBase);

        ucMidfFilterInitialized = 1;
    }
}

/*
***************************************************************************************************
*                                          MIDF_RegisterFilterRegion
*
* Register MID Filter
*
* @param    Base address of memory buffer for 'id'. it should be 4kb aligned
* @param    Size of memory buffer for 'id'. it should be 4kb aligned
* @param    id [in] Master id.
* @param    type [in] Read or Write type of memory buffer for 'id'
* @return   MID Result
*
* Notes
*
***************************************************************************************************
*/

int32 MIDF_RegisterFilterRegion
(
    uint32                              uiMemoryBase,
    uint32                              uiSize,
    uint32                              uiID,
    uint32                              uiType
)
{
    uint32  attr;
    uint32  access;
    uint32  ch;
    uint32  top;
    uint32  found;
    uint32  regBase;
    uint32  config;
    uint32  regBase1;
    uint32  config1;
    int32   result;

    result = MIDF_SUCCESS;

    if ( ucMidfFilterInitialized == 0UL )
    {
        result = MIDF_ERROR_NOT_INITIALIZE;
    }

    if (uiID > MIDF_ID_UART3_4_5)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if (((uiMemoryBase&(MIDF_REGION_MIN_SIZE-1UL)) != 0UL) || ((uiSize&(MIDF_REGION_MIN_SIZE-1UL)) != 0UL))
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    top = uiMemoryBase + (uiSize - 1UL);

    if (uiMemoryBase >= top)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    found = 0;

    if (result == MIDF_SUCCESS)
    {
        ch = MIDF_MAX_CH;

        while (ch!=0UL)
        {
            /* checking empty region */
            regBase = MIDF_ATTRIBUTES(ch);
            config  = SAL_ReadReg(regBase);

            if ((config & (uint32)0x1UL) == 0UL)
            {
                found = ch;
            }

            /* checking overlaps of range */
            regBase     = MIDF_TOP_L(ch);
            config      = SAL_ReadReg(regBase);
	        regBase1    = MIDF_BASE_L(ch);
            config1     = SAL_ReadReg(regBase1);

            if ((config > uiMemoryBase) && (config1 < top))
            {
                result = MIDF_ERROR_BAD_RANGES;
                break;
            }

            ch--;
        }

        if (found == 0UL )
        {
            result = MIDF_ERROR_NO_RESOURCE;
        }
    }

    if (result == MIDF_SUCCESS)
    {
        attr    = 0;
        access  = 0x00000000;

        if ((uiType&MIDF_TYPE_READ) != 0UL)
        {
            attr    = 1;
            access  |= (uint32)1UL << (uint32)(uiID & 0xFUL);
        }

        if ((uiType&MIDF_TYPE_WRITE) != 0UL)
        {
            attr    = 1;
            access  |= (uint32)1UL << (uint32)((uiID & 0xFUL) + 16UL);
        }

        regBase = MIDF_ATTRIBUTES(found);
        SAL_WriteReg(0, regBase);
        regBase = MIDF_BASE_L(found);
        SAL_WriteReg(uiMemoryBase, regBase);
        regBase = MIDF_BASE_H(found);
        SAL_WriteReg(0, regBase);
        regBase = MIDF_TOP_L(found);
        SAL_WriteReg(top, regBase);
        regBase = MIDF_TOP_H(found);
        SAL_WriteReg(0, regBase);
        regBase = MIDF_ID_ACCESS(found);
        SAL_WriteReg(access | MIDF_ACCESS_DEFAULT, regBase);
        regBase = MIDF_ATTRIBUTES(found);
        SAL_WriteReg(attr, regBase);
    }

    return result;
}

 /*
***************************************************************************************************
*                                          MIDF_UnregisterFilterRegion
*
* Unregister MID Filter
*
* @param    base [in] Base address of memory buffer for 'id'.
* @param    size [in] Size of memory buffer for 'id'.
* @param    id [in] Master id.
* @param    type [in] Read or Write type of memory buffer for 'id'
* @return   MID Result
*
* Notes
*
***************************************************************************************************
*/

int32 MIDF_UnregisterFilterRegion
(
    uint32                              uiMemoryBase,
    uint32                              uiSize,
    uint32                              uiID,
    uint32                              uiType
)
{
    uint32  ch ;
    uint32  top;
    uint32  access;
    uint32  regBase;
    uint32  config;
    uint32  regBase1;
    uint32  config1;
    int32   result;

    result  = MIDF_SUCCESS;

    if (ucMidfFilterInitialized == 0UL)
    {
        result = MIDF_ERROR_NOT_INITIALIZE;
    }

    if (uiID > MIDF_ID_UART3_4_5)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if (((uiMemoryBase & (MIDF_REGION_MIN_SIZE - 1UL)) != 0UL) || ((uiSize & (MIDF_REGION_MIN_SIZE - 1UL)) != 0UL))
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    top = uiMemoryBase + (uiSize - 1UL);

    if (uiMemoryBase >= top)
    {
        result = MIDF_ERROR_BAD_PARAMETERS;
    }

    if (result == MIDF_SUCCESS)
    {
        /* find matched region */
        ch  = MIDF_MAX_CH;

        while(ch!=0UL)
        {
            regBase     = MIDF_BASE_L(ch);
            config      = SAL_ReadReg(regBase);
    	    regBase1    = MIDF_TOP_L(ch);
            config1     =  SAL_ReadReg(regBase1);

            if ((config == uiMemoryBase) /*BASE LOW*/
                &&(config1 == top)) /*TOP LOW*/
            {
                access = MIDF_ACCESS_DEFAULT;

                if ((uiType&MIDF_TYPE_READ) != 0UL)
                {
                    access |= (uint32)1UL<<(uint32)(uiID&0xFUL);
                }

                if ((uiType&MIDF_TYPE_WRITE) != 0UL)
                {
                    access |= (uint32)1UL<<(uint32)((uiID&0xFUL) + 16UL);
                }

                regBase = MIDF_ID_ACCESS(ch);
                config  = SAL_ReadReg(regBase);

                if (config != access)
                {
                    result = MIDF_ERROR_BAD_PARAMETERS;
                    break;
                }

                regBase = MIDF_ATTRIBUTES(ch);
                SAL_WriteReg(0, regBase);
                regBase = MIDF_BASE_L(ch);
                SAL_WriteReg(0, regBase);
                regBase = MIDF_TOP_L(ch);
                SAL_WriteReg(0, regBase);
                regBase = MIDF_ID_ACCESS(ch);
                SAL_WriteReg(0, regBase);
                break;
            }

            ch--;
        }

        if (ch == 0UL)
        {
            result = MIDF_ERROR_NOT_FOUND;
        }
    }

    return result;
}

#ifdef MIDF_SFATEY_MECHANISM
static void MIDF_FaultIRQHandler
(
    void * pArg
)
{
     MIDF_RecoverFaultStatus();
     FMU_IsrClr(FMU_ID_MID_CFG);    
}

static void MIDF_FaultFIQHandler
(
    void * pArg
)
{
    MIDF_RecoverFaultStatus();
    FMU_IsrClr(FMU_ID_MID_CFG);    
}

static void MIDF_RecoverFaultStatus
(
    void
)
{
    uint32  uiRegister;
    uint32  uiRegisterVal;
    uint32  uiStatus;
    uint32  uiIndex;

    uiRegister = MIDF_CFG_GROUP_STATUS;
    uiStatus = SAL_ReadReg(uiRegister);
       if (uiStatus !=0)
       {
        for (uiIndex=0; uiIndex < MIDF_GHECK_GRP_NUM; uiIndex++)
        {
            if (((uiStatus) & (1UL << uiIndex)) != 0UL)
            {
                uiRegister = (MIDF_FILTER_CFG_BASE + (0x4UL*uiIndex));
                uiRegisterVal = SAL_ReadReg(uiRegister);
                // re-write
                SAL_WriteReg(uiRegisterVal, uiRegister);
            }
        }
    }
   
       // write password to modify configuration
    uiRegister = MIDF_CFG_WR_PW;
    SAL_WriteReg((uint32)MIDF_CFG_PW, uiRegister);

    // Clear Fault Status
    uiRegister = MIDF_CFG_FAULT_STATUS;
    uiRegisterVal = SAL_ReadReg(uiRegister);
    uiRegisterVal |= 0x1UL; // Set FAULT_STS bit
    SAL_WriteReg(uiRegisterVal, uiRegister);

}

int32 MIDF_RegisterWriteLockSafetyMechanism
(
    uint32                              uiLock
)
{
    int32   iResult = MIDF_SUCCESS;
    uint32  uiRegPassword;
    uint32  uiRegLock;
    uint32 uiLocked;
    uiRegPassword = MIDF_CFG_WR_PW;
    uiRegLock = MIDF_CFG_WR_LOCK;
    if (uiLock == MIDF_CFG_LOCK)
    {
        uiLocked = SAL_ReadReg(uiRegLock);
        if (uiLocked == MIDF_CFG_UNLOCK)
        {
            // write password to modify configuration
            SAL_WriteReg((uint32)MIDF_CFG_PW, uiRegPassword);
            // set register lock
            SAL_WriteReg((uint32)MIDF_CFG_LOCK, uiRegLock);
            // verify really locked
            uiLocked = SAL_ReadReg(uiRegLock);
            if (uiLocked != MIDF_CFG_LOCK)
            {
                iResult = MIDF_ERROR_NOT_SET;
            }
        }
    }
    else
    {
        uiLocked = SAL_ReadReg(uiRegLock);
        if (uiLocked == MIDF_CFG_LOCK)
        {
            // write password to modify configuration
            SAL_WriteReg((uint32)MIDF_CFG_PW, uiRegPassword);
            // set register unlock
            SAL_WriteReg((uint32)MIDF_CFG_UNLOCK, uiRegLock);
            // verify really unlocked
            uiLocked = SAL_ReadReg(uiRegLock);
            if (uiLocked != MIDF_CFG_UNLOCK)
            {
                iResult = MIDF_ERROR_NOT_SET;
            }
        }
    }
    return iResult;
}

int32 MIDF_ConfigSoftFaultCheckSafetyMechanism
(
    uint32 uiSvlLevel,
    uint32 uiFaultCheckGroup
)
{
    int32   iResult = MIDF_SUCCESS;
    uint32  uiRegister;
    uint32  uiRegisterVal;        

    if (ucMidfFilterInitialized == 1UL)
    {
        if (uiSvlLevel <= FMU_SVL_HIGH)
        {
            switch (uiSvlLevel)
            {
                case FMU_SVL_LOW :
                    (void)FMU_IsrHandler(FMU_ID_MID_CFG, FMU_SVL_LOW, (FMUIntFnctPtr)&MIDF_FaultIRQHandler, NULL_PTR);        
                    break;
                case FMU_SVL_MID :
                    (void)FMU_IsrHandler(FMU_ID_MID_CFG, FMU_SVL_MID, (FMUIntFnctPtr)&MIDF_FaultFIQHandler, NULL_PTR);        
                    break;
                case FMU_SVL_HIGH :
                    (void)FMU_IsrHandler(FMU_ID_MID_CFG, FMU_SVL_HIGH, (FMUIntFnctPtr)&MIDF_FaultFIQHandler, NULL_PTR);        
                    break;

                default :
                    (void)FMU_IsrHandler(FMU_ID_MID_CFG, FMU_SVL_HIGH, (FMUIntFnctPtr)&MIDF_FaultFIQHandler, NULL_PTR);        
                    break;
            }
            (void)FMU_Set(FMU_ID_MID_CFG);
            FMU_IsrClr(FMU_ID_MID_CFG); 
        }

       // write password to modify configuration
       uiRegister = MIDF_CFG_WR_PW; 
       SAL_WriteReg((uint32)MIDF_CFG_PW, uiRegister);
       
       uiRegister = MIDF_CFG_GROUP_EN;
       SAL_WriteReg((uint32)uiFaultCheckGroup, uiRegister);

       //read & verify
       uiRegisterVal = SAL_ReadReg(uiRegister);
       if (uiRegisterVal != uiFaultCheckGroup)
       {
           iResult = MIDF_ERROR_NOT_SET;
       }

				// write password to modify configuration
       uiRegister = MIDF_CFG_WR_PW; 
       SAL_WriteReg((uint32)MIDF_CFG_PW, uiRegister);

       // Soft Fault Check En
       uiRegister = MIDF_CFG_FAULT_CTL;
       uiRegisterVal = SAL_ReadReg(uiRegister);
       SAL_WriteReg((uint32)(uiRegisterVal|0x1UL), uiRegister);
  
    }
    else
    {
        iResult = MIDF_ERROR_NOT_INITIALIZE;
    }
    
    return iResult;
}
#endif
