/*
***************************************************************************************************
*
*   FileName : sal_freertos_impl.c
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

/*
***************************************************************************************************
*                                             [INCLUDE FILES]
***************************************************************************************************
*/
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>
#include <stdio.h> //QAC-<stdio.h> shall not be used in production code, but freertos recommends using <stdio.h>
#include <string.h>

#include <sal_freertos_impl.h>
#include <version.h>

/*
***************************************************************************************************
*                                             [DEFINITIONS]
***************************************************************************************************
*/
typedef int                             FRInt32;/* For stdio api                        */

/* Task */
typedef struct SALTaskiInfoTable
{
    boolean                             tiUsed; /* if already used or not               */
    StaticTask_t                        tiCb;   /* pointer to Task Control Block        */

} SALTask_t;

/* queue */
typedef struct SALQueueInfoTable
{
    boolean                             qiUsed; /* if already used or not               */
    StaticQueue_t                       qiCb;   /* pointer to Queue control block       */

} SALQueue_t;

/* semaphore */
typedef struct SALSemaphoreInfoTable
{
    boolean                             siUsed; /* if already used or not               */
    StaticSemaphore_t                   siCb;   /* pointer to Semaphore control block   */

} SALSemaphore_t;

/* event */
typedef struct SALEventInfoTable
{
    boolean                             eiUsed; /* if already used or not               */
    StaticEventGroup_t                  eiCb;   /* pointer to Event control block       */

} SALEvent_t;

/*
***************************************************************************************************
*                                             [LOCAL VARIABLES]
***************************************************************************************************
*/
static uint32                           gCPU_SR = 0;

static StaticSemaphore_t                gSemObj;
static SALErrorInfo_t                   gErrorInfo;

/* OS object */
static SALTask_t                        gTaskObj[SAL_MAX_TASK];
static SALQueue_t                       gQueueObj[SAL_MAX_QUEUE];
static SALSemaphore_t                   gSemaObj[SAL_MAX_SEMA];
static SALEvent_t                       gEventObj[SAL_MAX_EVENT];

/*
***************************************************************************************************
*                                        [FUNCTION PROTOTYPES]
***************************************************************************************************
*/
static SALRetCode_t FR_CoreMB
(
    void
);

static SALRetCode_t FR_CoreCriticalEnter
(
    void
);

static SALRetCode_t FR_CoreCriticalExit
(
    void
);

static SALRetCode_t FR_CoreWaitForEvent
(
    void
);

static SALRetCode_t FR_CoreDiv64To32
(
    uint64 *                            pullDividend,
    uint32                              uiDivisor,
    uint32 *                            puiRem
);

static SALRetCode_t FR_OSStart
(
    void
);

static SALRetCode_t FR_MemSet
(
    void *                              pMem,
    uint8                               ucValue,
    SALSize                             uiSize
);

static SALRetCode_t FR_MemCopy
(
    void *                              pDest,
    const void *                        pSrc,
    SALSize                             uiSize
);

static SALRetCode_t FR_MemCmp
(
    const void *                        pMem1,
    const void *                        pMem2,
    SALSize                             uiSize,
    sint32 *                            piRetCmp
);

static SALRetCode_t FR_StrCopy
(
    uint8 *                             pucDestString,
    const uint8 *                       pucSrcString
);

static SALRetCode_t FR_StrCmp
(
    const uint8 *                       puc1String,
    const uint8 *                       puc2String,
    sint32 *                            piRetCmp
);

static SALRetCode_t FR_StrNCmp
(
    const uint8 *                       puc1String,
    const uint8 *                       puc2String,
    SALSize                             uiLength,
    sint32 *                            piRetCmp
);

static SALRetCode_t FR_StrLength
(
    const int8 *                       pucString,
    SALSize *                           puiLength
);

static SALRetCode_t FR_QueueCreate
(
    uint32 *                            puiId,
    const uint8 *                       pucName,
    uint32                              uiDepth,
    SALSize                             uiDataSize
);

static SALRetCode_t FR_QueueGet
(
    uint32                              uiId,
    void *                              pData,
    uint32 *                            puiSizeCopied,
    sint32                              iTimeout,
    SALBlockingOption_t                 ulOptions
);

static SALRetCode_t FR_QueuePut
(
    uint32                              uiId,
    void *                              pData,
    SALSize                             uiSize,
    sint32                              iTimeout,
    SALBlockingOption_t                 ulOptions
);

static SALRetCode_t FR_SemaphoreCreate
(
    uint32 *                            puiId,
    const uint8 *                       pucName,
    uint32                              uiInitialValue,
    SALBlockingOption_t                 ulOptions
);

static SALRetCode_t FR_SemaphoreRelease
(
    uint32                              uiId
);

static SALRetCode_t FR_SemaphoreWait
(
    uint32                              uiId,
    sint32                              iTimeout,
    SALBlockingOption_t                 ulOptions
);

static SALRetCode_t FR_EventCreate
(
    uint32 *                            puiId,
    const uint8 *                       pucName,
    uint32                              uiInitialValue
);

static SALRetCode_t FR_EventSet
(
    uint32                              uiId,
    uint32                              uiEvent,
    SALEventOption_t                    ulOptions
);

static SALRetCode_t FR_EventGet
(
    uint32                              uiId,
    uint32                              uiEvent,
    uint32                              iTimeout,
    uint32                              uiOptions,
    uint32 *                            puiFlags
);

static SALRetCode_t FR_TaskCreate
(
    uint32 *                            puiTaskId,
    const uint8 *                       pucTaskName,
    SALTaskFunc                         fnTask,
    uint32 *                            puiStackPtr,
    uint32                              uiStackSize,
    SALTaskPriority_t                   uiPriority,
    void *                              pTaskParam
);

static SALRetCode_t FR_TaskSleep
(
    uint32                              uiMilliSec
);

static SALRetCode_t FR_ReportError
(
    SALDriverId_t                       uiDriverId,
    uint32                              uiApiId, 
    SALErrorCode_t                      uiErrorCode,
    const int8 *                        pucEtc
);

static SALRetCode_t FR_GetCurrentError
(
    SALErrorInfo_t *                    psInfo
);

static SALRetCode_t FR_GetTickCount
(
    uint32 *                            puiTickCount
);

static SALRetCode_t FR_HandleTick
(
    void
);

static SALRetCode_t FR_GetMcuVersionInfo
(
    SALMcuVersionInfo_t *             psVersion
);

/*
***************************************************************************************************
*                                        [FUNCTIONS]
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          [FR_CoreMB]
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_CoreMB (void)
{
    __DSB();

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [FR_CoreCriticalEnter]
*
* Save CPU status word & Disable interrupts.
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_CoreCriticalEnter (void)
{
    gCPU_SR = 0;
    gCPU_SR = CPU_SR_Save();

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [FR_CoreCriticalExit]
*
* Restore CPU status word & Re-enable interrupts.
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_CoreCriticalExit (void)
{
    CPU_SR_Restore(gCPU_SR);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [FR_CoreWaitForEvent]
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_CoreWaitForEvent (void)
{
    CPU_WaitForEvent();

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [FR_CoreDiv64To32]
*
*
* @param    pullDividend [in]
* @param    uiDivisor [in]
* @param    puiRem [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_CoreDiv64To32
(
    uint64 * pullDividend,
    uint32 uiDivisor,
    uint32 * puiRem
) {
    SALRetCode_t retVal     = SAL_RET_SUCCESS;
    uint64 rem              = 0;
    uint64 b                = uiDivisor;
    uint64 d                = 1;
    uint64 res              = 0;
    uint32 high             = 0;

    if (pullDividend != NULL_PTR)
    {
        rem = *pullDividend;
        high = (uint32)(rem >> 32UL);
        
        /* Reduce the thing a bit first */
        if (high >= uiDivisor)
        {
            high /= uiDivisor;
            res = ((uint64)high) << 32UL;
            rem -= ((uint64)high * (uint64)uiDivisor) << 32UL;
        }

        while (((b > 0ULL) && (b < rem)) != 0)
        {
            b = b+b;
            d = d+d;
        }

        do
        {
            if (rem >= b)
            {
                rem -= b;
                res += d;
            }

            b >>= 1UL;
            d >>= 1UL;
        }
        while (d != 0ULL);

        *pullDividend = res;
    }
    else
    {
        retVal = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_CORE_DIV_64_32,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    if (puiRem != NULL_PTR)
    {
        *puiRem = (uint32)rem;
    }
    
    return retVal;
}

/*
***************************************************************************************************
*                                          [FR_OSStart]
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_OSStart (void)
{
    SAL_D("~ Done to initialize Free RT Operating System ~\n");

    vTaskStartScheduler();

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [FR_MemSet]
*
*
* @param    pMem [out]
* @param    ucValue [in]
* @param    uiSize [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_MemSet
(
    void * pMem,
    uint8 ucValue,
    SALSize uiSize
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pMem != NULL_PTR) && (uiSize > 0UL))
    {
        memset(pMem, (FRInt32)ucValue, (size_t)uiSize); //QAC-Not use return value
    }
    else
    {
        retVal = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_MEM_SET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [FR_MemCopy]
*
*
* @param    pDest [out]
* @param    pSrc [in]
* @param    uiSize [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_MemCopy
(
    void * pDest,
    const void * pSrc,
    SALSize uiSize
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pDest != NULL_PTR) && (pSrc != NULL_PTR) && (uiSize > 0UL))
    {
        memcpy(pDest, pSrc, (size_t)uiSize); //QAC-Not use return value
    }
    else
    {
        retVal = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_MEM_COPY,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [FR_MemCmp]
*
*
* @param    pMem1 [in]
* @param    pMem2 [in]
* @param    uiSize [in]
* @param    piRetCmp [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_MemCmp
(
    const void * pMem1,
    const void * pMem2,
    SALSize uiSize,
    sint32 * piRetCmp
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pMem1 != NULL_PTR) && (pMem2 != NULL_PTR) && (uiSize > 0UL) && (piRetCmp != NULL_PTR))
    {
        *piRetCmp = (sint32)memcmp(pMem1, pMem2, (size_t)uiSize);
    }
    else
    {
        retVal = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_MEM_CMP,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [FR_StrCopy]
*
*
* @param    pucDestString [out]
* @param    pucSrcString [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_StrCopy
(
    uint8 * pucDestString,
    const uint8 * pucSrcString
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pucDestString != NULL_PTR) && (pucSrcString != NULL_PTR))
    {
        uint32 str_len = 0;
        str_len = strnlen((const int8 *)pucSrcString, (size_t)(SAL_MAX_INT_VAL - 1UL));

        if (str_len != 0UL)
        {
            strncpy((int8 *)pucDestString, (const int8 *)pucSrcString, (size_t)str_len); //Codesonar Security Warning
        }
    }
    else
    {
        retVal = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_STR_COPY,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [FR_StrCmp]
*
*
* @param    puc1String [in]
* @param    puc2String [in]
* @param    piRetCmp [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_StrCmp
(
    const uint8 * puc1String,
    const uint8 * puc2String,
    sint32 * piRetCmp
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((puc1String != NULL_PTR) && (puc2String != NULL_PTR) && (piRetCmp != NULL_PTR))
    {
        *piRetCmp = strncmp((const int8 *)puc1String, 
                            (const int8 *)puc2String, 
                            (size_t)(SAL_MAX_INT_VAL - 1UL));
    }
    else
    {
        retVal = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_STR_CMP,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [FR_StrNCmp]
*
*
* @param    puc1String [in]
* @param    puc2String [in]
* @param    uiLength [in]
* @param    piRetCmp [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_StrNCmp
(
    const uint8 * puc1String,
    const uint8 * puc2String,
    SALSize uiLength,
    sint32 * piRetCmp
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((puc1String != NULL_PTR) && (puc2String!= NULL_PTR) && (uiLength > 0UL) && (piRetCmp != NULL_PTR))
    {
        *piRetCmp = strncmp((const int8 *)puc1String, (const int8 *)puc2String, (size_t)uiLength);
    }
    else
    {
        retVal = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_STR_NCMP,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [FR_StrLength]
*
*
* @param    pucString [in]
* @param    puiLength [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_StrLength
(
    const int8 * pucString,
    SALSize * puiLength
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pucString != NULL_PTR) && (puiLength != NULL_PTR))
    {
        *puiLength = strnlen(pucString, (size_t)(SAL_MAX_INT_VAL - 1UL)); //Codesonar Security Warning
    }
    else
    {
        retVal = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_STR_LENGTH,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [FR_QueueCreate]
*
*
* @param    puiId [out]
* @param    pucName [in]
* @param    uiDepth [in]
* @param    uiDataSize [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_QueueCreate
(
    uint32 * puiId,
    const uint8 * pucName,
    uint32 uiDepth,
    SALSize uiDataSize
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    sint32 i                            = -1;
    sint32 search_id                    = -1;
    SemaphoreHandle_t semParam;

    if (puiId == NULL_PTR)
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else if ((pucName == NULL_PTR) || (uiDepth < 1UL))
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiId = (uint32)search_id;
    }
    else
    {
        /* obtain sema. wait forever*/
        uiNestingCnt = xPortGetInterruptNestingCount();

        (void)FR_MemCopy(&semParam, &gSemObj, sizeof(SemaphoreHandle_t));

        if (uiNestingCnt == 0UL)
        {
            status = xSemaphoreTake(semParam, (TickType_t )portMAX_DELAY);
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            status = xSemaphoreTakeFromISR(semParam, &xHigherPriorityTaskWoken);
        }

        *puiId = (uint32)search_id;

        if (status == pdTRUE)
        {
            /*search free table*/
            for (i = 0; i < SAL_MAX_QUEUE; i++)
            {
                if (gQueueObj[i].qiUsed == FALSE)
                {
                    search_id = i;
                    break;
                }
            }

            if (search_id >= 0)// CS : Redundant Condition
            {
                void * pQueueBuffer;
                QueueHandle_t pQueueHandle = NULL;

                pQueueBuffer = pvPortMalloc((size_t)(uiDepth * uiDataSize));

                if (pQueueBuffer != NULL_PTR)
                {
                    pQueueHandle = xQueueCreateStatic(uiDepth,
                                                      uiDataSize,
                                                      pQueueBuffer,
                                                      (StaticQueue_t *)&gQueueObj[search_id].qiCb);
                }

                if(pQueueHandle != NULL_PTR)
                {
                    gQueueObj[search_id].qiUsed = TRUE;
                    *puiId = (uint32)search_id;
                }
                else
                {
                    retval = FR_ReportError(SAL_DRVID_SAL, 
                                            SAL_API_DATA_QUEUE_CREATE,
                                            SAL_ERR_FAIL_CREATE, 
                                            __FUNCTION__);
                }
            }
            else
            {
                retval = FR_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_DATA_QUEUE_CREATE,
                                        SAL_ERR_NO_SPACE, 
                                        __FUNCTION__);
            }

            /*release sema*/
            uiNestingCnt = xPortGetInterruptNestingCount();

            if (uiNestingCnt == 0UL)
            {
                (void)xSemaphoreGive(semParam);
            }
            else
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                (void)xSemaphoreGiveFromISR(semParam, &xHigherPriorityTaskWoken);
            }
        }
        else
        {
            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_DATA_QUEUE_CREATE,
                                    SAL_ERR_FAIL_CREATE, 
                                    __FUNCTION__);
        }
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_QueueGet]
*
*
* @param    uiId [in]
* @param    pData [out]
* @param    puiSizeCopied [out]
* @param    iTimeout [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_QueueGet
(
    uint32 uiId,
    void * pData,
    uint32 * puiSizeCopied,
    sint32 iTimeout,
    SALBlockingOption_t  ulOptions
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    uint32 timeDly; //CS : useless assignment
    SALReg32 address                    = 0;
    QueueHandle_t queueHandle;

    if (puiSizeCopied != NULL_PTR)
    {
        *puiSizeCopied = 0;
    }

    if ((uiId < (uint32)SAL_MAX_QUEUE) 
        && (gQueueObj[uiId].qiUsed == TRUE) 
        && (pData != NULL_PTR) 
        && (puiSizeCopied != NULL_PTR))
    {
        if (((uint32)ulOptions & 0x00008000UL) == (uint32)SAL_OPT_NON_BLOCKING)  //CS : Redundant Condition
        {
            timeDly = 0UL; // no block time
        }
        else
        {
            if (iTimeout == 0)
            {
                timeDly = portMAX_DELAY;
            }
            else
            {
                timeDly = (uint32)iTimeout;
            }
        }

        uiNestingCnt = xPortGetInterruptNestingCount();

        address = (SALReg32)&(gQueueObj[uiId].qiCb);
        (void)FR_MemCopy(&queueHandle, (const void *)&address, sizeof(QueueHandle_t));

        if (uiNestingCnt == 0UL)
        {
            status = xQueueReceive(queueHandle,
                                   (void *)pData,
                                   (TickType_t)timeDly);
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            status = xQueueReceiveFromISR(queueHandle,
                                          (void *)pData,
                                          &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken )//;QAC
        }

        if ((status == pdPASS))
        {
            *puiSizeCopied = (uint32)gQueueObj[uiId].qiCb.uxDummy4[2];// referenced by uxItemSize on uxItemSize ;
        }
        else
        {
         	if (timeDly != portMAX_DELAY)
				retval = SAL_RET_FAILED;
			else
	            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_DATA_QUEUE_GET,
                                    SAL_ERR_FAIL_GET_DATA, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_GET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_QueuePut]
*
*
* @param    uiId [in]
* @param    pData [in]
* @param    uiSize [in]
* @param    iTimeout [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_QueuePut
(
    uint32 uiId,
    void * pData,
    SALSize uiSize,
    sint32 iTimeout,
    SALBlockingOption_t  ulOptions
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    uint32 timeDly; //CS : useless assignment
    SALReg32 address                    = 0;
    QueueHandle_t queueHandle;

    if ((uiId < (uint32)SAL_MAX_QUEUE) 
        && (gQueueObj[uiId].qiUsed == TRUE) 
        && (pData != NULL_PTR) 
        && (uiSize > 0UL))
    {
        uint32 uxItemSize;
        uint32 i;
        uint32 pushCnt;
        uint8 * queueDataPtr = (uint8 *)pData;

        if (((uint32)ulOptions & 0x00008000UL) == (uint32)SAL_OPT_NON_BLOCKING)  //CS : Redundant Condition
        {
            timeDly = 0UL; // no block time
        }
        else
        {
            if (iTimeout == 0)
            {
                timeDly = portMAX_DELAY;
            }
            else
            {
                timeDly = (uint32)iTimeout;
            }
        }

        uxItemSize = (uint32)gQueueObj[uiId].qiCb.uxDummy4[2];// referenced by uxItemSize on uxItemSize ;

        if (uxItemSize != 0UL) //CS : Uninitialized Variable
        {
            pushCnt = (uiSize / uxItemSize);

            if (pushCnt != 0UL)
            {
                for (i =0; i < pushCnt; i++)
                {
                    uiNestingCnt = xPortGetInterruptNestingCount();

                    address = (SALReg32)&(gQueueObj[uiId].qiCb);
                    (void)FR_MemCopy(&queueHandle, (const void *)&address, sizeof(QueueHandle_t));

                    if (uiNestingCnt == 0UL)
                    {
                        status = xQueueSend(queueHandle,
                                            (void *)(queueDataPtr + (i * uxItemSize)),
                                            (TickType_t)timeDly);
                    }
                    else
                    {
                        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                        status = xQueueSendFromISR(queueHandle,
                                                   (void *)(queueDataPtr + (i * uxItemSize)),
                                                   &xHigherPriorityTaskWoken);

                        portYIELD_FROM_ISR( xHigherPriorityTaskWoken )//;QAC
                    }

                    if (status != pdTRUE)
                    {
                        retval = FR_ReportError(SAL_DRVID_SAL, 
                                                SAL_API_DATA_QUEUE_PUT,
                                                SAL_ERR_FAIL_SEND_DATA, 
                                                __FUNCTION__);

                        break;
                    }
                    else
                    {
                        retval = SAL_RET_SUCCESS;
                    }
                }
            }
            else
            {
                retval = FR_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_DATA_QUEUE_PUT,
                                        SAL_ERR_OUTOF_SIZE, 
                                        __FUNCTION__);
            }
        }
        else
        {
            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_DATA_QUEUE_PUT,
                                    SAL_ERR_UNINIT_ITEM, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_PUT,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_SemaphoreCreate]
*
*
* @param    puiId [out]
* @param    pucName [in]
* @param    uiInitialValue [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_SemaphoreCreate
(
    uint32 * puiId,
    const uint8 * pucName,
    uint32 uiInitialValue,
    SALBlockingOption_t ulOptions
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    sint32 i                            = -1;
    sint32 search_id                    = -1;
    SALReg32 address                    = 0;
    SemaphoreHandle_t semParam;
    SemaphoreHandle_t semHandle;

    (void)ulOptions;

    if (puiId == NULL_PTR)
    {
        retval = FR_ReportError(SAL_DRVID_SAL,
                                SAL_API_SEMAPHORE_CREATE,
                                SAL_ERR_INVALID_PARAMETER,
                                __FUNCTION__);
    }
    else if ((pucName == NULL_PTR) || (uiInitialValue == 0UL))
    {
        retval = FR_ReportError(SAL_DRVID_SAL,
                                SAL_API_SEMAPHORE_CREATE,
                                SAL_ERR_INVALID_PARAMETER,
                                __FUNCTION__);
        *puiId = (uint32)search_id;
    }
    else
    {
        /* obtain sema. wait forever*/
        uiNestingCnt = xPortGetInterruptNestingCount();

        (void)FR_MemCopy(&semParam, &gSemObj, sizeof(SemaphoreHandle_t));

        if (uiNestingCnt == 0UL)
        {
            status = xSemaphoreTake(semParam, (TickType_t )portMAX_DELAY);
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            status = xSemaphoreTakeFromISR(semParam, &xHigherPriorityTaskWoken);
        }

        *puiId = (uint32)search_id;
        
        if (status == pdTRUE)
        {
            /*search free table*/
            for (i = 0; i < SAL_MAX_SEMA; i++)
            {
                if (gSemaObj[i].siUsed == FALSE)
                {
                    search_id = i;
                    break;
                }
            }

            if ( search_id >= 0)
            {
                (void)xSemaphoreCreateBinaryStatic((StaticSemaphore_t *)&(gSemaObj[search_id].siCb));

                gSemaObj[search_id].siUsed = TRUE;
                *puiId = (uint32)search_id;

                uiNestingCnt = xPortGetInterruptNestingCount();

                address = (SALReg32)&(gSemaObj[search_id].siCb);
                (void)FR_MemCopy(&semHandle, (const void *)&address, sizeof(SemaphoreHandle_t));

                if (uiNestingCnt == 0UL)
                {
                    (void)xSemaphoreGive(semHandle);
                }
                else
                {
                    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                    (void)xSemaphoreGiveFromISR(semHandle, &xHigherPriorityTaskWoken);
                }
            }
            else
            {
                retval = FR_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_SEMAPHORE_CREATE,
                                        SAL_ERR_NO_SPACE, 
                                        __FUNCTION__);
            }

            /*release sema*/
            uiNestingCnt = xPortGetInterruptNestingCount();

            if (uiNestingCnt == 0UL)
            {
                xSemaphoreGive(semParam);
            }
            else
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                (void)xSemaphoreGiveFromISR(semParam, &xHigherPriorityTaskWoken);
            }
        }
        else
        {
            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_SEMAPHORE_CREATE,
                                    SAL_ERR_FAIL_GET_KEY, 
                                    __FUNCTION__);
        }
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_SemaphoreRelease]
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_SemaphoreRelease (uint32 uiId)
{
    SALRetCode_t retval;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    SALReg32 address                    = 0;
    SemaphoreHandle_t semHandle;

    if ((uiId < (uint32)SAL_MAX_SEMA) && (gSemaObj[uiId].siUsed == TRUE))
    {
        uiNestingCnt = xPortGetInterruptNestingCount();

        address = (SALReg32)&(gSemaObj[uiId].siCb);
        (void)FR_MemCopy(&semHandle, (const void *)&address, sizeof(SemaphoreHandle_t));

        if (uiNestingCnt == 0UL)
        {
            status = xSemaphoreGive(semHandle);
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            status = xSemaphoreGiveFromISR(semHandle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken )//; QAC
        }

        if (status == pdTRUE)
        {
            retval = SAL_RET_SUCCESS;
        }
        else
        {
            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_SEMAPHORE_RELEASE,
                                    SAL_ERR_FAIL_RELEASE, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_SEMAPHORE_RELEASE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_SemaphoreWait]
*
*
* @param    uiId [in]
* @param    iTimeout [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_SemaphoreWait
(
    uint32 uiId,
    sint32 iTimeout,
    SALBlockingOption_t ulOptions
) {
    SALRetCode_t retval;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    uint32 timeDly; //CS : useless assignment
    SALReg32 address                    = 0;
    SemaphoreHandle_t semHandle;

    if ((uiId < (uint32)SAL_MAX_SEMA) && (gSemaObj[uiId].siUsed == TRUE))
    {
        if (((uint32)ulOptions & 0x00008000UL) == (uint32)SAL_OPT_NON_BLOCKING)  //CS : Redundant Condition
        {
            timeDly = 0UL; // no block time
        }
        else
        {
            if (iTimeout == 0L)
            {
                timeDly = portMAX_DELAY;
            }
            else
            {
                timeDly = (uint32)iTimeout;
            }
        }

        /* obtain sema. wait forever*/
        uiNestingCnt = xPortGetInterruptNestingCount();

        address = (SALReg32)&(gSemaObj[uiId].siCb);
        (void)FR_MemCopy(&semHandle, (const void *)&address, sizeof(SemaphoreHandle_t));

        if (uiNestingCnt == 0UL)
        {
            status = xSemaphoreTake(semHandle, (TickType_t)timeDly);
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            status = xSemaphoreTakeFromISR(semHandle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken )//; QAC
        }

        if (status == pdTRUE)
        {
            retval = SAL_RET_SUCCESS;
        }
        else
        {
            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_SEMAPHORE_WAIT,
                                    SAL_ERR_FAIL_GET_DATA, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_SEMAPHORE_WAIT,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_EventCreate]
*
*
* @param    puiId [out]
* @param    pucName [in]
* @param    uiInitialValue [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_EventCreate
(
    uint32 * puiId,
    const uint8 * pucName,
    uint32 uiInitialValue
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    sint32 i                            = -1;
    sint32 search_id                    = -1;
    SemaphoreHandle_t semParam;

    (void)uiInitialValue;

    if (puiId == NULL_PTR)
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else if (pucName == NULL_PTR)
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiId = (uint32)search_id;
    }
    else
    {
        /* obtain sema. wait forever*/
        uiNestingCnt = xPortGetInterruptNestingCount();

        (void)FR_MemCopy(&semParam, &gSemObj, sizeof(SemaphoreHandle_t));

        if (uiNestingCnt == 0UL)
        {
            status = xSemaphoreTake(semParam, (TickType_t )portMAX_DELAY);
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            status = xSemaphoreTakeFromISR(semParam, &xHigherPriorityTaskWoken);
        }

        *puiId = (uint32)search_id;

        if (status == pdTRUE)
        {
            /*search free table*/
            for (i = 0; i < SAL_MAX_EVENT; i++)
            {
                if (gEventObj[i].eiUsed == FALSE)
                {
                    search_id = i;
                    break;
                }
            }

            if (search_id >= 0)// CS : Redundant Condition
            {
                (void)xEventGroupCreateStatic((StaticEventGroup_t *)&(gEventObj[search_id].eiCb));

                gEventObj[search_id].eiUsed = TRUE;
                *puiId = (uint32)search_id;
                
            }
            else
            {
                retval = FR_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_EVENT_CREATE,
                                        SAL_ERR_NO_SPACE, 
                                        __FUNCTION__);
            }

            /*release sema*/
            uiNestingCnt = xPortGetInterruptNestingCount();

            if (uiNestingCnt == 0UL)
            {
                (void)xSemaphoreGive(semParam);
            }
            else
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                (void)xSemaphoreGiveFromISR(semParam, &xHigherPriorityTaskWoken);
            }
        }
        else
        {
            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_EVENT_CREATE,
                                    SAL_ERR_FAIL_GET_KEY, 
                                    __FUNCTION__);
        }
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_EventSet]
*
*
* @param    uiId [in]
* @param    uiEvent [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_EventSet
(
    uint32 uiId,
    uint32 uiEvent,
    SALEventOption_t ulOptions
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    SALReg32 address                    = 0;
    EventGroupHandle_t eventHandle;

    if (uiId < (uint32)SAL_MAX_EVENT)
    {
        if (gEventObj[uiId].eiUsed == TRUE)
        {
            uiNestingCnt = xPortGetInterruptNestingCount();
        
            address = (SALReg32)&(gEventObj[uiId].eiCb);
            (void)FR_MemCopy(&eventHandle, (const void *)&address, sizeof(EventGroupHandle_t));
        
            if (((uint32)ulOptions & 0x00000001UL) == (uint32)SAL_EVENT_OPT_FLAG_SET)
            {
                if(uiNestingCnt == 0UL)
                {
                    (void)xEventGroupSetBits(eventHandle, (EventBits_t)uiEvent); //QAC-Not use return value
                }
                else
                {
                    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                    status = xEventGroupSetBitsFromISR((eventHandle), (uiEvent), &xHigherPriorityTaskWoken);
        
                    if (status == pdTRUE)
                    {
                        portYIELD_FROM_ISR(xHigherPriorityTaskWoken)
                    }
                    else
                    {
                        retval = FR_ReportError(SAL_DRVID_SAL, 
                                                SAL_API_EVENT_SET,
                                                SAL_ERR_FAIL_SET_CONFIG, 
                                                __FUNCTION__);
                    }
                }
            }
            else if (((uint32)ulOptions & 0x00000001UL) == (uint32)SAL_EVENT_OPT_CLR_ALL)
            {
                if(uiNestingCnt == 0UL)
                {
                    (void)xEventGroupClearBits(eventHandle, (EventBits_t)uiEvent); //QAC-Not use return value
                }
                else
                {
                    status = xEventGroupClearBitsFromISR((eventHandle), (uiEvent));
        
                    if (status == pdFALSE)
                    {
                        retval = FR_ReportError(SAL_DRVID_SAL, 
                                                SAL_API_EVENT_SET,
                                                SAL_ERR_FAIL_SET_CONFIG, 
                                                __FUNCTION__);
                    }
                    else
                    {;}
                }
            }
            else
            {
                /* MISRA C:2012-14.3, All if... else if constructs shall be terminated with an else statement  */
                retval = FR_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_EVENT_SET,
                                        SAL_ERR_INVALID_PARAMETER, 
                                        __FUNCTION__);
            }
        }
        else
        {
            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_EVENT_SET,
                                    SAL_ERR_NOT_USEFUL, 
                                    __FUNCTION__);
        }
    }
    else
    {
        /* MISRA C:2012-14.3, All if... else if constructs shall be terminated with an else statement  */
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_SET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_EventGet]
*
*
* @param    uiId [in]
* @param    uiEvent [in]
* @param    iTimeout [in]
* @param    options [in]
* @param    puiFlags [out]
* @return
*
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t FR_EventGet
(
    uint32 uiId,
    uint32 uiEvent,
    uint32 iTimeout,
    uint32 uiOptions,
    uint32 * puiFlags
) {
    SALRetCode_t retval = SAL_RET_SUCCESS;

    if (puiFlags == NULL_PTR)
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_GET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else if (uiId < (uint32)SAL_MAX_EVENT)
    {
        *puiFlags = 0;
        
        if (gEventObj[uiId].eiUsed == TRUE)
        {
            EventBits_t uxBits;
            BaseType_t is_Consume               = pdFALSE;
            BaseType_t is_All                   = pdFALSE;
            uint32 timeDly; // CS : Redundant Condition
            SALReg32 address                    = 0;
            EventGroupHandle_t eventHandle;

            if ((uiOptions & (uint32)SAL_EVENT_OPT_SET_ALL) != 0UL)
            {
                is_All = pdTRUE;
            }
            else if ((uiOptions & (uint32)SAL_EVENT_OPT_CONSUME) != 0UL)
            {
                is_Consume = pdTRUE;
            }
            else
            {;} /* MISRA C:2012-14.3, All if... else if constructs shall be terminated with an else statement  */

            if ((uiOptions & 0x00008000UL) == (uint32)SAL_OPT_NON_BLOCKING)  //CS : Redundant Condition
            {
                timeDly = 0; // no block time
            }
            else
            {
                if (iTimeout == 0UL)
                {
                    timeDly = portMAX_DELAY;
                }
                else
                {
                    timeDly = iTimeout;
                }
            }

            address = (SALReg32)&(gEventObj[uiId].eiCb);
            (void)FR_MemCopy(&eventHandle, (const void *)&address, sizeof(EventGroupHandle_t));

            uxBits = xEventGroupWaitBits(eventHandle, uiEvent, is_Consume, is_All, (TickType_t)timeDly);

            if ((uxBits & uiEvent) == 0UL)
            {   // the event bits being waited for became set, or the block time expired
                SAL_D("EventGet : Waited for became set, or the block time expired\n");
            }
            else
            {;} /* MISRA C:2012-14.3, All if... else if constructs shall be terminated with an else statement  */

            if (puiFlags != NULL_PTR)
            {
                *puiFlags = (uxBits & uiEvent);
            }
            else
            {;} /* MISRA C:2012-14.3, All if... else if constructs shall be terminated with an else statement  */
        }
        else
        {
            retval = FR_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_EVENT_GET,
                                    SAL_ERR_NOT_USEFUL, 
                                    __FUNCTION__);
        }
    }
    else
    {
        /* MISRA C:2012-14.3, All if... else if constructs shall be terminated with an else statement  */
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_GET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiFlags = 0;
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_TaskCreate]
*
*
* @param    puiTaskId [out]
* @param    pucTaskName [in]
* @param    fnTask [in]
* @param    puiStackPtr [out]
* @param    uiStackSize [in]
* @param    uiPriority [in]
* @param    pTaskParam [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_TaskCreate
(
    uint32 * puiTaskId,
    const uint8 * pucTaskName,
    SALTaskFunc fnTask,
    uint32 * puiStackPtr,
    uint32 uiStackSize,
    SALTaskPriority_t uiPriority,
    void * pTaskParam
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    SALReg32 uiNestingCnt               = 0;
    BaseType_t status                   = 0;
    sint32 i                            = -1;
    sint32 search_id                    = -1;
    SemaphoreHandle_t semParam;

    if (puiTaskId == NULL_PTR)
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_TASK_CREATE, 
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else if ((pucTaskName == NULL_PTR) || (fnTask == NULL_PTR) ||(puiStackPtr == NULL_PTR))
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_TASK_CREATE, 
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiTaskId = (uint32)search_id;
    }
    else
    {
        /* obtain sema. wait forever*/
        uiNestingCnt = xPortGetInterruptNestingCount();

        (void)FR_MemCopy(&semParam, &gSemObj, sizeof(SemaphoreHandle_t));

        if (uiNestingCnt == 0UL)
        {
            status = xSemaphoreTake(semParam, (TickType_t )portMAX_DELAY);
        }
        else
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            status = xSemaphoreTakeFromISR(semParam, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken )//;QAC
        }

        *puiTaskId = (uint32)search_id;

        if (status == pdTRUE)
        {
            /*search free table*/
            for (i = 0; i < SAL_MAX_TASK; i++)
            {
                if (gTaskObj[i].tiUsed == FALSE)
                {
                    search_id = i;
                    break;
                }
            }

            if (search_id >= 0)
            {
                TaskHandle_t task = NULL;

                task = xTaskCreateStatic((TaskFunction_t)fnTask,
                                         (const int8 * const)pucTaskName,
                                         uiStackSize,
                                         pTaskParam,
                                         (UBaseType_t)((16UL - (uint32)uiPriority) | portPRIVILEGE_BIT),  /* FREERTOS_MAX_PRIORITY = 16 */
                                         puiStackPtr,  /* QAC-use const */
                                         (StaticTask_t * const)&gTaskObj[search_id].tiCb);  /* QAC-use const */

                if (task != NULL)
                {
                    gTaskObj[search_id].tiUsed = TRUE;
                    *puiTaskId = (uint32)search_id;
                }
                else
                {
                    retval = FR_ReportError(SAL_DRVID_SAL, 
                                            SAL_API_TASK_CREATE,
                                            SAL_ERR_FAIL_CREATE, 
                                            __FUNCTION__);
                }
            }
            else
            {
                retval = FR_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_TASK_CREATE,
                                        SAL_ERR_NO_SPACE, 
                                        __FUNCTION__);
            }

            uiNestingCnt = xPortGetInterruptNestingCount();

            if (uiNestingCnt == 0UL)
            {
                (void)xSemaphoreGive(semParam);
            }
            else
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                (void)xSemaphoreGiveFromISR(semParam, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken ) /* QAC */
            }
        }

    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_TaskSleep]
*
*
* @param    uiMilliSec [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_TaskSleep (uint32 uiMilliSec)
{
    uint32 ticks = 0;

    ticks = ( ( uint32 ) ( ( ( uint32 ) ( uiMilliSec ) * ( uint32 ) configTICK_RATE_HZ ) / ( uint32 )1000UL ) );
    vTaskDelay(ticks);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [FR_ReportError]
*
*
* @param    uiDrvierId [in]
* @param    uiApiId [in]
* @param    uiErrorCode [in]
* @param    pucEtc [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_ReportError
(
    SALDriverId_t                       uiDriverId,
    uint32                              uiApiId, 
    SALErrorCode_t                      uiErrorCode,
    const int8 *                        pucEtc
) {
    SAL_E("[%d][API : %d][ERROR Code: %d][From: %s]\n", uiDriverId, uiApiId, uiErrorCode, pucEtc);

    (void)FR_CoreCriticalEnter();
    gErrorInfo.eiDrvId = uiDriverId;
    gErrorInfo.eiApiId = uiApiId;
    gErrorInfo.eiErrorId = uiErrorCode;
    (void)FR_CoreCriticalExit();

    return SAL_RET_FAILED;
}

/*
***************************************************************************************************
*                                          [FR_GetCurrentError]
*
*
* @param    psInfo [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_GetCurrentError(SALErrorInfo_t * psInfo) 
{
    SALRetCode_t retval = SAL_RET_SUCCESS;
    
    if (psInfo != NULL_PTR)
    {
        (void)FR_CoreCriticalEnter();
        (void)FR_MemCopy(psInfo, &gErrorInfo, sizeof(SALErrorInfo_t));

        // Right after returing the information of error, info table is initialized.
        (void)FR_MemSet(&gErrorInfo, 0, sizeof(SALErrorInfo_t));
        (void)FR_CoreCriticalExit();
    }
    else
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_GET_CURRENT_ERR, 
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}


/*
***************************************************************************************************
*                                          [FR_GetTickCount]
*
*
* @param    puiTickCount [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_GetTickCount (uint32 * puiTickCount)
{
    SALRetCode_t retval         = SAL_RET_SUCCESS;
    SALReg32 uiNestingCnt       = 0;

    if (puiTickCount != NULL_PTR)
    {
        /* obtain sema. wait forever*/
        uiNestingCnt = xPortGetInterruptNestingCount();

        if (uiNestingCnt == 0UL)
        {
            *puiTickCount = xTaskGetTickCount();
        }
        else
        {
            *puiTickCount = xTaskGetTickCountFromISR();
        }
    }
    else
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_GET_TICKCOUNT,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [FR_HandleTick]
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_HandleTick (void)
{
    FreeRTOS_Tick_Handler();

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [FR_GetMcuVersionInfo]
*
* @param    psVersion [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t FR_GetMcuVersionInfo (SALMcuVersionInfo_t * psVersion)
{
    SALRetCode_t retval = SAL_RET_SUCCESS;

    static const SALMcuVersionInfo_t MCU_Version =
    {
        SDK_MAJOR_VERSION,
        SDK_MINOR_VERSION,
        SDK_PATCH_VERSION,
        0U
    };

    if (psVersion != NULL_PTR)
    {
        (void)FR_MemCopy(psVersion, &MCU_Version, sizeof(SALMcuVersionInfo_t));
    }
    else
    {
        retval = FR_ReportError(SAL_DRVID_SAL, 
                                SAL_API_GET_VERSION,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}


/*
***************************************************************************************************
*                                          [S4AL_Init]
*
* Initializes every elements and functions
*
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t SAL_Init (void)
{
    uint32 index;
    BaseType_t status                   = 0;
    SALReg32 uiNestingCnt               = 0;
    SemaphoreHandle_t semParam;

    SALRetCode_t ret                    = SAL_RET_SUCCESS;

    heap_init();

    (void)SAL_RegisterFunction(SAL_API_CORE_MB, &FR_CoreMB, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_CORE_CRITICAL_ENTER, &FR_CoreCriticalEnter, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_CORE_CRITICAL_EXIT, &FR_CoreCriticalExit, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_CORE_WAITFOREVENT, &FR_CoreWaitForEvent, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_CORE_DIV_64_32, &FR_CoreDiv64To32, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_OS_START, &FR_OSStart, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_MEM_SET, &FR_MemSet, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_MEM_COPY, &FR_MemCopy, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_MEM_CMP, &FR_MemCmp, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_STR_COPY, &FR_StrCopy, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_STR_CMP, &FR_StrCmp, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_STR_NCMP, &FR_StrNCmp, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_STR_LENGTH, &FR_StrLength, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_DATA_QUEUE_CREATE, &FR_QueueCreate, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_DATA_QUEUE_GET, &FR_QueueGet, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_DATA_QUEUE_PUT, &FR_QueuePut, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_EVENT_CREATE, &FR_EventCreate, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_EVENT_SET, &FR_EventSet, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_EVENT_GET, &FR_EventGet, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_SEMAPHORE_CREATE, &FR_SemaphoreCreate, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_SEMAPHORE_RELEASE, &FR_SemaphoreRelease, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_SEMAPHORE_WAIT, &FR_SemaphoreWait, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_TASK_CREATE, &FR_TaskCreate, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_TASK_SLEEP, &FR_TaskSleep, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_GET_TICKCOUNT, &FR_GetTickCount, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_HANDLE_TICK, &FR_HandleTick, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_GET_VERSION, &FR_GetMcuVersionInfo, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_DBG_REPORT_ERR, &FR_ReportError, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_GET_CURRENT_ERR, &FR_GetCurrentError, NULL_PTR);

    /* SAL default manage object*/
    (void)xSemaphoreCreateBinaryStatic(&gSemObj); //CS : redundant condition

    /* semaphore must first be given using the xSemaphoreGive() API function before it can subsequently be taken (obtained) using the xSemaphoreTake() function. */
    uiNestingCnt = xPortGetInterruptNestingCount();

    (void)FR_MemCopy(&semParam, &gSemObj, sizeof(SemaphoreHandle_t));

    if (uiNestingCnt == 0UL)
    {
        status = xSemaphoreGive(semParam);
    }
    else
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        status = xSemaphoreGiveFromISR(semParam, &xHigherPriorityTaskWoken);
    }

    if (status == pdTRUE)
    {
        /* Init Object */
        for (index = 0; index < (uint32)SAL_MAX_TASK; index++)
        {
            //SAL_MemSet(&gTaskObj[index], 0x00, sizeof(SALTask_t));
            //modify for time save
            gTaskObj[index].tiUsed = FALSE;
        }

        for (index = 0; index < (uint32)SAL_MAX_QUEUE; index++)
        {
            //SAL_MemSet(&gQueueObj[index], 0x00, sizeof(SALQueue_t));
            //modify for time save
            gQueueObj[index].qiUsed = FALSE;
        }

        for (index = 0; index < (uint32)SAL_MAX_SEMA; index++)
        {
            //SAL_MemSet(&gSemaObj[index], 0x00, sizeof(SALSemaphore_t));
            //modify for time save
            gSemaObj[index].siUsed = FALSE;
        }

        for (index = 0; index < (uint32)SAL_MAX_EVENT; index++)
        {
            //SAL_MemSet(&gEventObj[index], 0x00, sizeof(SALEvent_t));
            //modify for time save
            gEventObj[index].eiUsed = FALSE;
        }
    }
    else
    {
        ret = FR_ReportError(SAL_DRVID_SAL, 
                             SAL_API_COMMON_INIT,
                             SAL_ERR_INIT, 
                             __FUNCTION__);
    }

    return ret;
}

