/*
***************************************************************************************************
*
*   FileName : ipc_parser.c
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

#include <sal_com.h>
#include <sal_internal.h>
#include "ipc.h"
#include "ipc_parser.h"
#include "ipc_api.h"
#include "bsp.h"

static uint8        packetReceive[IPC_SVC_CH_MAX][IPC_MAX_PACKET_SIZE];
static IPCChInfo_t  ipcChInfo[IPC_SVC_CH_MAX] =
{
#if defined(TCC805x)
    {IPC_SVC_CH_CA53_SECURE,    NULL},
    {IPC_SVC_CH_CA53_NONSECURE, NULL},
    {IPC_SVC_CH_CA72_SECURE,    NULL},
    {IPC_SVC_CH_CA72_NONSECURE, NULL},
#else
    #error no target device designated.
#endif
};

static int32 IPC_ReceivePacket
(
    IPCSvcCh_t                          siCh
)
{
    int32  ret;
    uint32 read_size;
    uint16 packet_size;

    ret         = IPC_ERR_COMMON;
    read_size   = IPC_PACKET_PREPARE_SIZE;
    packet_size = 0;
    ret         = IPC_Ioctl((uint32) siCh, (uint32) IOCTL_IPC_READ, (void *) &packetReceive[siCh][0], (void *) &read_size, NULL_PTR, NULL);

    if (ret > 0)
    {
        packet_size = (uint16) ((((uint32) packetReceive[siCh][7]) << 8U) | packetReceive[siCh][8]);

        if (packet_size > (((uint16) IPC_MAX_PACKET_SIZE) - ((uint16) IPC_PACKET_PREPARE_SIZE) - ((uint16) 2)) /* crc byte */)
        {
            ret = IPC_ERR_READ; //IPC read size error
        }
        else
        {
            if (packet_size < ((uint16) 1))
            {
                packet_size = ((uint16) 1) + ((uint16) 2); // 1 is data length , 2 is crc byte
            }
            else
            {
                packet_size += ((uint16) 2); // 2 is crc byte
            }

            read_size = packet_size;
            (void) IPC_Ioctl(((uint16) siCh), ((uint16) IOCTL_IPC_READ),  (void *) &packetReceive[siCh][9], (void *) &read_size, NULL_PTR, NULL);
            ret = ((int32) read_size) + IPC_PACKET_PREPARE_SIZE;
        }
    }

    return ret;
}

static int32 IPC_PacketParser
(
    IPCSvcCh_t                          siCh
)
{
    int32  ret;
    uint16 crc1;
    uint16 crc2;
    uint16 cmd1;
    uint16 cmd2;
    uint16 totalLength;
    uint16 dataLength;

    ret        = IPC_SUCCESS;
    dataLength = (uint16) ((((uint32) packetReceive[siCh][7]) << 8U) | packetReceive[siCh][8]);

    if(dataLength == ((uint16) 0))
    {
        dataLength++; //dummy data length
    }
    //mcu_printf( "IPC_PacketParser siCh %d >> \n", siCh );

    totalLength = ((uint16) IPC_PACKET_PREPARE_SIZE) + dataLength;

    if (totalLength < ((uint16) IPC_MAX_PACKET_SIZE - (uint16) 1))
    {
        crc1 = IPC_CalcCrc16(&packetReceive[siCh][0], totalLength, 0);
        crc2 = (uint16)((uint16)((uint16)packetReceive[siCh][totalLength] << 8) |
               (uint16)(packetReceive[siCh][totalLength + ((uint16) 1)]));

        if (crc1 == crc2)
        {
            if ((packetReceive[siCh][0] == (uint8) IPC_SYNC) &&
                (packetReceive[siCh][1] == (uint8) IPC_START1) &&
                (packetReceive[siCh][2] == (uint8) IPC_START2))
            {
                cmd1 = (uint16)((uint16)((uint16)packetReceive[siCh][3] << 8) | (uint16)packetReceive[siCh][4]);
                cmd2 = (uint16)((uint16)((uint16)packetReceive[siCh][5] << 8) | (uint16)packetReceive[siCh][6]);

                if(cbFunc[siCh][cmd1].ipcCbFunc != NULL_PTR) // null pointer checking
                {
                    cbFunc[siCh][cmd1].ipcCbFunc(cmd2, &packetReceive[siCh][9], dataLength);
                }
            }
        }
    }
    else
    {
        ret = IPC_ERR_READ;
    }

    return ret;
}

static void IPC_MainParserTask
(
    void *                              pArg
)
{
    int32 ipc_ret;

    (void) pArg;

    while (1)
    {
        ipc_ret = IPC_ReceivePacket(IPC_SVC_CH_CA53_NONSECURE);

        if ( ipc_ret > 0)
        {
            (void) IPC_PacketParser(IPC_SVC_CH_CA53_NONSECURE);
        }

        (void) SAL_TaskSleep(10);
    }
}

static void IPC_SubParserTask
(
    void *                              pArg
)
{
    int32 ipc_ret;

    (void) pArg;

    while (1)
    {
        ipc_ret = IPC_ReceivePacket(IPC_SVC_CH_CA72_NONSECURE);

        if ( ipc_ret > 0)
        {
            (void) IPC_PacketParser(IPC_SVC_CH_CA72_NONSECURE);
        }

        (void) SAL_TaskSleep(10);
    }
}

void IPC_CreateIPCParserTask
(
    void
)
{
    int32 i;
    uint8 isHalHandle;

    static uint32 IPC_PARSER_TASK_ID[IPC_SVC_CH_MAX];
    static uint32 IPC_PARSER_TASK_STK[IPC_SVC_CH_MAX][128];

    isHalHandle = 0;

#if defined(TCC805x)
    ipcChInfo[IPC_SVC_CH_CA53_NONSECURE].ipcHandle = IPC_Open((uint32) IPC_SVC_CH_CA53_NONSECURE, IPC_MODE_0);
    ipcChInfo[IPC_SVC_CH_CA72_NONSECURE].ipcHandle = IPC_Open((uint32) IPC_SVC_CH_CA72_NONSECURE, IPC_MODE_0);
#else
    #error no target device designated.
#endif

    for(i = 0 ; i < (int32) IPC_SVC_CH_MAX ; i++)
    {
        if(ipcChInfo[i].ipcHandle == IPC_SUCCESS)
        {
            isHalHandle = 1;
        }
    }

    if (isHalHandle == (uint8) 1)
    {
        (void) SAL_TaskCreate
        (
            &(IPC_PARSER_TASK_ID[IPC_SVC_CH_CA53_NONSECURE]),
            (const uint8 *) "IPC_A53NonSecure_ParserTask",
            (SALTaskFunc) &IPC_MainParserTask,
            (void * const)&(IPC_PARSER_TASK_STK[IPC_SVC_CH_CA53_NONSECURE][0]),
            128,
            SAL_PRIO_IPC_PARSER, NULL_PTR
        );

        (void) SAL_TaskCreate
        (
            &(IPC_PARSER_TASK_ID[IPC_SVC_CH_CA72_NONSECURE]),
            (const uint8 *) "IPC_A72NonSecure_ParserTask",
            (SALTaskFunc) &IPC_SubParserTask,
            (void * const)&(IPC_PARSER_TASK_STK[IPC_SVC_CH_CA72_NONSECURE][0]),
            128,
            SAL_PRIO_IPC_PARSER, NULL_PTR
        );
    }
}

