/*
***************************************************************************************************
*
*   FileName : system_monitoring.c
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
 
#include <app_cfg.h>
 
#if (ACFG_APP_SYSTEM_MONITORING_EN == 1)
 
#include "debug.h"
#include "ipc.h"
#include <system_monitoring.h>
 
/*
***************************************************************************************************
*                                             DEFINITIONS 
***************************************************************************************************
*/
//#define SM_APP_DBG_ENABLE 
#ifdef SM_APP_DBG_ENABLE 
#define SM_APP_DBG(fmt, args...) {mcu_printf(fmt, ## args)}
#else
#define SM_APP_DBG(fmt, args...)  
#endif
 
#ifndef __GNU_C__ 
#   define func_name_t                     (__FUNCTION__)
#endif
 
#define SMA_INTERVAL                    (1000UL)    /* 1 second */
#define SMA_MAX_RETRY                   (3UL)/* 1 second */
 
typedef enum SMAMainApStatus 
{
    SMA_IPC_READY                       = 0,
    SMA_PEER_RECOGNIZE_REQ              = 1,
    SMA_PEER_RECOG_WAIT                 = 2, /* rename for qac 5.2  */
    SMA_PEER_CHECKING_ALIVE             = 3,
    SMA_PEER_WAIT_ALIVE                 = 4,
    SMA_PEER_RE_INIT                    = 5,
    SMA_NOT_SUPPORT                     = 6 
 
} SMAMainApStatus_t;
 
typedef struct SMAPeerCtrlTable
{
    SMAMainApStatus_t          pcControlMainStatus;
    uint32                               pcNoAckTimes;
    uint32                               pcSendAckTimeMs;
    uint32                               pcAckTimeElapsedMs;
    uint32                               pcLastSentTime;
    uint32                               pcRetryCnt;
    uint32                               pcCheckingInterval;
    IPCSvcCh_t                              pcIpcChannel;
    uint32                              pcPeerRecognized;
    uint32                              pcPeerAliveAcked;
 
} SMAPeerCtrl_t;
 
/*
***************************************************************************************************
*                                             STATIC VARIABLES
***************************************************************************************************
*/
static SMAPeerCtrl_t                    gPeerCtrl[IPC_SVC_CH_MAX];
 
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void SMA_Task 
(
    void *                              pArg
);
 
// Need Check Later : Remove to avoid compile warning 
//static void SMA_ExternSignal
//(
//    void
//);
 
static void SMA_PeerCtrlInit 
(
    void 
);
 
static void SMA_IpcCbFunc 
(
    uint16                              uhwCmd,
    uint8 *                             pucData,
    uint16                              uhwLength 
); // CS : Dangerous Function Cast 
 
static void SMA_IpcCbFuncSubCh 
(
    uint16                              uhwCmd,
    uint8 *                             pucData,
    uint16                              uhwLength 
);
 
/*
***************************************************************************************************
*                                         FUNCTIONS 
***************************************************************************************************
*/
//static uint8 last_rcam_det = 0, last_ill_det = 0; //QAC
//static uint8 last_door_det = 0, last_trunk_det = 0; //QAC
//static int32 is_door_det = 0, is_trunk_det = 0; => move to hal layer
//static int32 lastIllreadval = 0, lastSpeedreadval = 0; //QAC
static void SMA_PeerCtrlInit(void)
{
    uint32 i;
 
    (void)SAL_MemSet((void *) &gPeerCtrl[0], 0x00, sizeof(SMAPeerCtrl_t));
 
    for (i =0UL; i < (uint32)IPC_SVC_CH_MAX; i++)
    {
        gPeerCtrl[i].pcIpcChannel = (IPCSvcCh_t)i;
    }
}

static void SMA_IpcCbFunc(uint16 uhwCmd, uint8 *pucData, uint16 uhwLength) // CS : Dangerous Function Cast 
{
    (void) uhwLength;
         
    if (uhwCmd == (uint16)IPC_SYS_MON_RECOG_RES)
    {
        SM_APP_DBG("IPC_SYS_MON_RECOG_RES pucData0[%d], data1[%d], data2[%d]!!!\n",pucData[0], pucData[1], pucData[2]); //QAC
 
        if ((pucData != NULL_PTR) && (pucData[0] != 0U) && (pucData[1] != 0U))
        {
            gPeerCtrl[IPC_SVC_CH_CA53_NONSECURE].pcCheckingInterval = (uint32)((uint32)pucData[0] << 8UL);
            gPeerCtrl[IPC_SVC_CH_CA53_NONSECURE].pcCheckingInterval |= (uint32)(pucData[1]);
        }
        else 
        {
            gPeerCtrl[IPC_SVC_CH_CA53_NONSECURE].pcCheckingInterval = 1000UL;
        }
 
        if ((pucData != NULL_PTR) && (pucData[2] != 0UL))
        {
            gPeerCtrl[IPC_SVC_CH_CA53_NONSECURE].pcRetryCnt = pucData[2];
        }
        else 
        {
            gPeerCtrl[IPC_SVC_CH_CA53_NONSECURE].pcRetryCnt = 3;
        }
 
        gPeerCtrl[IPC_SVC_CH_CA53_NONSECURE].pcPeerRecognized = 1;
    }
    else if (uhwCmd == (uint16)SYS_MONITORING_RES_OK)
    {
        //SM_APP_DBG("%s: IPC_SYS_MON_RECOG_RES SYS_MONITORING_RES_OK!!!\n", func_name_t); //QAC
        gPeerCtrl[IPC_SVC_CH_CA53_NONSECURE].pcPeerAliveAcked = 1;
    }
    else 
    {
        SM_APP_DBG("%s: uhwCmd invalid !!!\n",func_name_t); //QAC
    }
}
 
static void SMA_IpcCbFuncSubCh(uint16 uhwCmd, uint8 *pucData, uint16 uhwLength) // CS : Dangerous Function Cast 
{
    (void) uhwLength;
 
    if (uhwCmd == (uint16)IPC_SYS_MON_RECOG_RES)
    {
        SM_APP_DBG("IPC_SYS_MON_RECOG_RES pucData0[%d], pucData1[%d], pucData2[%d]!!!\n",pucData[0], pucData[1], pucData[2]); //QAC
 
        if ((pucData != NULL_PTR) && (pucData[0] != 0U) && (pucData[1] != 0U))
        {
            gPeerCtrl[IPC_SVC_CH_CA72_NONSECURE].pcCheckingInterval = (uint32)((uint32)pucData[0] << 8UL);
            gPeerCtrl[IPC_SVC_CH_CA72_NONSECURE].pcCheckingInterval |= (uint32)(pucData[1]);
        }
        else 
        {
            gPeerCtrl[IPC_SVC_CH_CA72_NONSECURE].pcCheckingInterval = 1000UL;
        }
 
        if ((pucData != NULL_PTR) && (pucData[2] != 0UL))
        {
            gPeerCtrl[IPC_SVC_CH_CA72_NONSECURE].pcRetryCnt = pucData[2];
        }
        else 
        {
            gPeerCtrl[IPC_SVC_CH_CA72_NONSECURE].pcRetryCnt = 3;
        }
 
        gPeerCtrl[IPC_SVC_CH_CA72_NONSECURE].pcPeerRecognized = 1;
    }
    else if( uhwCmd == (uint16)SYS_MONITORING_RES_OK)
    {
        //SM_APP_DBG("%s : IPC_SYS_MON_RECOG_RES SYS_MONITORING_RES_OK!!!\n",func_name_t); //QAC
        gPeerCtrl[IPC_SVC_CH_CA72_NONSECURE].pcPeerAliveAcked = 1;
    }
    else 
    {
        SM_APP_DBG("%s : uhwCmd invalid !!!\n",func_name_t); //QAC
    }
}
 
static void SMA_Task(void * pArg)
{
    uint32 peerIndex                 = 0UL;
    int32 ipc_ret                    = 0;
    uint32 ack_time_elapsed_ms = 0UL;
    uint32 retTick                  = 0UL;
    uint32 ipc_ready                = 0UL;
    uint32 printMod                  = 0UL;
    uint32 no_ack_times              = 0UL;
    uint32 send_ack_time_ms          = 0UL;
    uint32 last_sent_time            = 0UL;
    uint32 retry_cnt                 = 0UL;
    uint32 maxRetry                  = 3UL; // default 3 times //Codesonar uninitialized variable
    uint32 maxInterval               = 1000UL; //default 1S //Codesonar uninitialized variable
    uint32 taskDelay                 = 5UL;
    uint32 externalPoolingInterval   = 100UL; // unit ms
    IPCSvcCh_t ch                   = (IPCSvcCh_t)0;
 
    SMAMainApStatus_t MainStatus    = SMA_IPC_READY;

    (void)pArg;
	
    SMA_PeerCtrlInit();
 
//#if (SERVICE_IPC_EN == 1)
    // register ipc handler 
    IPC_RegisterCbFunc(IPC_SVC_CH_CA53_NONSECURE, (uint8)TCC_IPC_CMD_SYS_MONITORING, (IPCCallback)&SMA_IpcCbFunc, NULL_PTR, NULL_PTR);
    IPC_RegisterCbFunc(IPC_SVC_CH_CA72_NONSECURE, (uint8)TCC_IPC_CMD_SYS_MONITORING, (IPCCallback)&SMA_IpcCbFuncSubCh, NULL_PTR, NULL_PTR);
//#endif
 
    (void)SAL_TaskSleep(1000);

    for(;;)
    {
        for (peerIndex = 0; peerIndex < (uint32)IPC_SVC_CH_MAX; peerIndex++)
        {
            ch                  = gPeerCtrl[peerIndex].pcIpcChannel;
            // load control var 
            MainStatus          = gPeerCtrl[peerIndex].pcControlMainStatus;
            last_sent_time      = gPeerCtrl[peerIndex].pcLastSentTime;
            send_ack_time_ms    = gPeerCtrl[peerIndex].pcSendAckTimeMs;
            ack_time_elapsed_ms = gPeerCtrl[peerIndex].pcAckTimeElapsedMs;
            retry_cnt           = gPeerCtrl[peerIndex].pcRetryCnt;
            no_ack_times        = gPeerCtrl[peerIndex].pcNoAckTimes;
 
            switch (MainStatus)
            {
                case SMA_IPC_READY:
                case SMA_NOT_SUPPORT: //move here to initialize status in ipc close->open case.
                {
                    ipc_ready = 0;
 
//#if (SERVICE_IPC_EN == 1)
                    ipc_ret = IPC_IsReady((uint32)ch, (uint32 *)&ipc_ready); //QAC
 
                    if ((ipc_ready == 1UL) && (ipc_ret == 0))
                    {
                        SM_APP_DBG("%s : goto SMA_PEER_RECOGNIZE_REQ !!!\n", func_name_t); //QAC
                        MainStatus = SMA_PEER_RECOGNIZE_REQ;
                        last_sent_time = 0UL;
                    }
//#endif
                    break;
                }
 
                case SMA_PEER_RECOGNIZE_REQ:
                {
                    (void)SAL_GetTickCount(&retTick);
 
                    if (((uint32)(retTick - last_sent_time)) > SMA_INTERVAL)
                    {
                        (void)IPC_SendPacket(ch, (uint16)TCC_IPC_CMD_SYS_MONITORING, (uint16)IPC_SYS_MON_RECOG_REQ, NULL_PTR, 0);
                        gPeerCtrl[peerIndex].pcPeerRecognized = 0;
                        (void)SAL_GetTickCount(&retTick);
                        send_ack_time_ms = (uint32)retTick;
                        last_sent_time = send_ack_time_ms;
                        SM_APP_DBG("%s : goto SMA_PEER_RECOG_WAIT !!!\n",func_name_t); //QAC
                        MainStatus = SMA_PEER_RECOG_WAIT;
                    }
 
                    break;
                }
 
                case SMA_PEER_RECOG_WAIT:
                {
                    if (gPeerCtrl[peerIndex].pcPeerRecognized == 0UL)
                    {
                        (void)SAL_GetTickCount(&retTick);
                        ack_time_elapsed_ms = (uint32)(retTick - send_ack_time_ms);
 
                        if(ack_time_elapsed_ms > SMA_INTERVAL) // 1 second
                        {
                            retry_cnt ++;
 
                            if ( retry_cnt == SMA_MAX_RETRY)
                            {
                                SM_APP_DBG("%s :goto SMA_NOT_SUPPORT !!!\n",func_name_t); //QAC
                                MainStatus = SMA_NOT_SUPPORT;
                            }
                            else 
                            {
                                SM_APP_DBG("%s : goto SMA_PEER_RECOGNIZE_REQ !!!\n", func_name_t); //QAC
                                MainStatus = SMA_PEER_RECOGNIZE_REQ;
                            }
                        }
                    }
                    else 
                    {
                        maxRetry= gPeerCtrl[peerIndex].pcRetryCnt ;
                        maxInterval= gPeerCtrl[peerIndex].pcCheckingInterval;
                        MainStatus = SMA_PEER_CHECKING_ALIVE;
                    }
 
                    break;
                }
 
                case SMA_PEER_WAIT_ALIVE:
                {
                    if (gPeerCtrl[peerIndex].pcPeerAliveAcked == 0UL)
                    {
                        (void)SAL_GetTickCount(&retTick);
                        ack_time_elapsed_ms = (uint32)(retTick - send_ack_time_ms);
 
                        if ((ack_time_elapsed_ms > maxInterval) && (maxInterval >= 500UL)) // deafult 1 second 
                        {
                            no_ack_times ++;
 
                            if (no_ack_times > maxRetry)
                            {
                                SM_APP_DBG("%s: goto SMA_PEER_RE_INIT !!!\n", func_name_t); //QAC
                                MainStatus = SMA_PEER_RE_INIT;
                            }
                            else 
                            {
                                // retry ack msg 
                                SM_APP_DBG("%s : goto SMA_PEER_CHECKING_ALIVE !!!\n", func_name_t); //QAC
                                MainStatus = SMA_PEER_CHECKING_ALIVE;
                            }
                        }
                    }
                    else 
                    {
                        printMod++;
 
                        if ((printMod % 5UL) == 0UL)
                        {
                            #ifdef SM_APP_DBG_ENABLE 
                            SM_APP_DBG("[System Monitoring] Main AP OK [%d]!!!\n",MainStatus); //QAC
                            #else
				printMod = 0;	
				#endif
                        }
 
                        MainStatus = SMA_PEER_CHECKING_ALIVE;
                    }
                    break;
                }

                case SMA_PEER_CHECKING_ALIVE:
                {
                    (void)SAL_GetTickCount(&retTick);
                    if (((uint32)(retTick - last_sent_time)) > maxInterval)
                    {
                        (void)IPC_SendPacket(ch, (uint16)TCC_IPC_CMD_SYS_MONITORING, (uint16)SYS_MONITORING_IPC_OK, NULL_PTR, 0U);
                        //start checking 
                        gPeerCtrl[peerIndex].pcPeerAliveAcked = 0;
                        // timer reset
                        (void)SAL_GetTickCount(&retTick);
                        send_ack_time_ms = retTick;
                        last_sent_time = send_ack_time_ms;
                        MainStatus = SMA_PEER_WAIT_ALIVE;
                        //SM_APP_DBG("%s: goto SMA_PEER_WAIT_ALIVE !!!\n", func_name_t); //QAC
                    }
 
                    break;
                }
 
#if FALSE 
                case SMA_MAIN_AP_NO_ACK:
                {
                    no_ack_times ++;
 
                    if (no_ack_times > MAIN_AP_NO_ACK_TIME_LIMIT)
                    {
                        MainStatus = SMA_PEER_RE_INIT;
                    }
                    else 
                    {
                        // retry ack msg 
                        MainStatus = SMA_PEER_CHECKING_ALIVE;
                    }
 
                    break;
                }
 
                case SMA_NOT_SUPPORT:
                {
                    break;
                }
#endif
                case SMA_PEER_RE_INIT:
                {
                    // reset system
                    // test 
                    SM_APP_DBG("[System Monitoring] Main AP Not OK.. Reset [%d]!!!\n",MainStatus);//QAC
                    no_ack_times = 0;
                    send_ack_time_ms = 0;
                    ack_time_elapsed_ms = 0UL;
                    MainStatus = SMA_IPC_READY;
                    SM_APP_DBG("%s : goto SMA_IPC_READY !!!\n", func_name_t); //QAC
 
                    break;
                }
 
                default:
                {                
                    SM_APP_DBG("unexpected code flow\n");
                    break;
                }
            }
 
            // store control var 
            gPeerCtrl[peerIndex].pcControlMainStatus = MainStatus;
            gPeerCtrl[peerIndex].pcLastSentTime = last_sent_time;
            gPeerCtrl[peerIndex].pcSendAckTimeMs = send_ack_time_ms;
            gPeerCtrl[peerIndex].pcAckTimeElapsedMs = ack_time_elapsed_ms;
            gPeerCtrl[peerIndex].pcRetryCnt =retry_cnt;
            gPeerCtrl[peerIndex].pcNoAckTimes= no_ack_times;
        }
 
#if defined(VSD_SENDER ) || defined(VSD_RECEIVER)
        // use case conflict 
#else
        externalPoolingInterval -= taskDelay;
 
        if (externalPoolingInterval <= 0UL)
        {
            externalPoolingInterval = 100UL;
            // Need Check Later : Remove to avoid compile warning 
           // SMA_ExternSignal();
        }
#endif
        (void)SAL_TaskSleep(taskDelay);
    }
}
 
void SM_CreateAppTask(void)
{
    //int32 err; // CS :
    static uint32 SystemMonitoringAppTaskID;
    static uint32 SystemMonitoringAppTaskStk[ACFG_TASK_MEDIUM_STK_SIZE];

    (void)SAL_TaskCreate(&SystemMonitoringAppTaskID,
                         (const uint8 *)"SYSMON",
                         (SALTaskFunc)&SMA_Task,
                         (void * const)&SystemMonitoringAppTaskStk[0],
                         ACFG_TASK_MEDIUM_STK_SIZE,
                         SAL_PRIO_SYSTEM_MON, NULL_PTR);
}
#endif

