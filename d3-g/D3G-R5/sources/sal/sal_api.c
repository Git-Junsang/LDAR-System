/*
***************************************************************************************************
*
*   FileName : sal_com.c
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
#include "sal_api.h"

/*
***************************************************************************************************
*                                             [DEFINITIONS]
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                             [LOCAL VARIABLES]
***************************************************************************************************
*/

SALCoreFunc_t *                         gSALCoreTable;
SALOSRunFunc_t *                        gSALOSRunTable;
SALMemFunc_t *                          gSALMemoryTable;
SALFileFunc_t *                         gSALFileTable;
SALDataStructFunc_t *                   gSALDataStructTable;
SALSyncObjectFunc_t *                   gSALSyncObjectTable;
SALTaskFunc_t *                         gSALTaskTable;
SALSocketFunc_t *                       gSALSocketTable;
SALTraceFunc_t *                        gSALTraceTable;
SALEtcFunc_t *                          gSALEtcTable;

static SALCoreFunc_t                    gCoreTable;
static SALOSRunFunc_t                   gOSRunTable;
static SALMemFunc_t                     gMemFuncTable;
static SALFileFunc_t                    gFileTable;
static SALDataStructFunc_t              gDataStructTable;
static SALSyncObjectFunc_t              gSyncObjectTable;
static SALTaskFunc_t                    gTaskTable;
static SALSocketFunc_t                  gSocketTable;
static SALTraceFunc_t                   gTraceTable;
static SALEtcFunc_t                     gEtcTable;

/*
***************************************************************************************************
*                                         [FUNCTIONS]
***************************************************************************************************
*/
 /*
***************************************************************************************************
*                                          [SAL_RegisterFunction]
*
* Registers  functions needed by specific application.
*
* @param    uiFunctionIndex [in] Index of function to register.
* @param    pFunc [in] Pointer to function that corresponds with function index.
* @param    pPreFunc [in] Pointer to function that
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t SAL_RegisterFunction
(
    uint32 uiFunctionIndex,
    void * pFunc,
    void ** ppPreFunc
)
{
    SALAddr *     tableAddr;

    uint32        funcCategory = uiFunctionIndex & 0xFF00UL;
    uint32        funcIndex = uiFunctionIndex & 0x00FFUL;

    SALRetCode_t    ret = SAL_RET_SUCCESS;

    (void)ppPreFunc;

    switch (funcCategory)
    {
        case SAL_CORE_FUNCTIONS_INDEX :
        {
            tableAddr = (SALAddr *)(&(gCoreTable.fnCoreMB) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALCoreTable = &gCoreTable;

            break;

        }

        case SAL_OS_RUN_FUNCTIONS_INDEX :
        {
            tableAddr = (SALAddr *)(&(gOSRunTable.fnOsInitFuncs) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALOSRunTable = &gOSRunTable;

            break;

        }

        case SAL_MEM_FUNCTIONS_INDEX:
        {
            tableAddr = (SALAddr *)(&(gMemFuncTable.fnMemAlloc) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALMemoryTable = &gMemFuncTable;

            break;
        }

        case SAL_FILE_FUNCTIONS_INDEX:
        {
            tableAddr = (SALAddr *)(&(gFileTable.fnFileOpenA) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALFileTable = &gFileTable;

            break;
        }

        case SAL_DATAS_FUNCTIONS_INDEX:
        {
            tableAddr = (SALAddr *)(&(gDataStructTable.fnQueueCreate) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALDataStructTable = &gDataStructTable;

            break;
        }

        case SAL_SYNCOBJ_FUNCTIONS_INDEX:
        {
            tableAddr = (SALAddr *)(&(gSyncObjectTable.fnEventCreate) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALSyncObjectTable = &gSyncObjectTable;

            break;
        }

        case SAL_TASK_FUNCTIONS_INDEX:
        {
            tableAddr = (SALAddr *)(&(gTaskTable.fnTaskCreate) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALTaskTable = &gTaskTable;

            break;
        }

        case SAL_SOCKET_FUNCTIONS_INDEX:
        {
            tableAddr = (SALAddr *)(&(gSocketTable.fnNetSocket) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALSocketTable = &gSocketTable;

            break;
        }

        case SAL_TRACE_FUNCTIONS_INDEX:
        {
            tableAddr = (SALAddr *)(&(gTraceTable.fnReportError) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALTraceTable = &gTraceTable;

            break;
        }

        case SAL_ETC_FUNCTIONS_INDEX:
        {
            tableAddr = (SALAddr *)(&(gEtcTable.fnGetTickCount) + funcIndex);
            *tableAddr = (SALAddr)pFunc;

            gSALEtcTable = &gEtcTable;

            break;
        }

        default :
        {
            ret = SAL_RET_FAILED;
            break;
        }
    }

    return ret;
}

