/*
***************************************************************************************************
*
*   FileName : ipc.c
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

#include "sal_com.h"
#include "sal_internal.h"
#include "mbox.h"
#include "ipc.h"
#include "ipc_api.h"
#include "ipc_typedef.h"
#include "ipc_buffer.h"
#include "ipc_cmd.h"
#include "ipc_ctl.h"
#include "ipc_parser.h"
#include "ipc_os.h"

static IPCChStatus_t    icpChStatus[IPC_SVC_CH_MAX] = {0, };
IPCCbFunc_t             cbFunc[IPC_SVC_CH_MAX][IPC_PROCESS_NUM];

int32 IPC_Open
(
    uint32                              siCh,
    uint32                              uiOptions
)
{
    int32 lRet;

    (void) SAL_CoreCriticalEnter();
    gIPCHandler[siCh].ihIpcOption.isBlock = (uint32)(uiOptions & IPC_O_BLOCK_MASK);
    gIPCHandler[siCh].ihIpcOption.isAck   = (uint32)((uiOptions & IPC_O_ACK_MASK) >> 8U);

    if(icpChStatus[siCh].gIPCChIsOpen == 0U)
    {
        lRet = IPC_SUCCESS;
    }
    else
    {
        lRet = IPC_ERR_BUSY;
    }
    (void) SAL_CoreCriticalExit();

    if(lRet == IPC_SUCCESS)
    {
        IPC_StructInit((IPCSvcCh_t) siCh, &gIPCHandler[siCh]);

        // open driver
        lRet = IPC_Initialize((IPCSvcCh_t) siCh);

        if(lRet == IPC_SUCCESS)
        {
            icpChStatus[siCh].gIPCChStatus = IPC_DEVICE_ST_OPENED;
            (void) SAL_CoreCriticalEnter();
            icpChStatus[siCh].gIPCChCallCount   = 0;
            icpChStatus[siCh].gIPCChIsOpen      = 1;
            (void) SAL_CoreCriticalExit();
        }
        else
        {
            lRet = IPC_ERR_NOTREADY;
        }
    }

    return lRet;
}

int32 IPC_Close
(
    uint32                              siCh
)
{
    int32  lRet;
    uint32 isOpen;
    uint32 callCount;

    lRet = IPC_SUCCESS;

    if ((int32) icpChStatus[siCh].gIPCChStatus != IPC_DEVICE_ST_OPENED)
    {
        lRet = IPC_ERR_NOTREADY;
    }
    else
    {
        icpChStatus[siCh].gIPCChStatus = IPC_DEVICE_ST_NO_OPEN;

        //close driver
        (void) SAL_CoreCriticalEnter();
        isOpen      = icpChStatus[siCh].gIPCChIsOpen;
        callCount   = icpChStatus[siCh].gIPCChCallCount;
        (void) SAL_CoreCriticalExit();

        if((isOpen == 1U ) && (callCount == 0U))
        {
            IPC_Release((IPCSvcCh_t) siCh);
            (void) SAL_CoreCriticalEnter();
            icpChStatus[siCh].gIPCChIsOpen = 0U;
            (void) SAL_CoreCriticalExit();
        }
        else if (callCount != 0U)
        {
            lRet = IPC_ERR_BUSY;
        }
        else
        {
            lRet = IPC_ERR_COMMON;
        }
    }

    return lRet;
}

uint16 IPC_CalcCrc16
(
    const uint8 *                       pucBuffer,
    uint32                              uiLength,
    uint16                              uiInit
)
{
    uint32 i;
    uint32 temp;
    uint16 crcCode;
    static const uint16 crc16Table[256] =
    {
        0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50a5U, 0x60c6U, 0x70e7U,
        0x8108U, 0x9129U, 0xa14aU, 0xb16bU, 0xc18cU, 0xd1adU, 0xe1ceU, 0xf1efU,
        0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52b5U, 0x4294U, 0x72f7U, 0x62d6U,
        0x9339U, 0x8318U, 0xb37bU, 0xa35aU, 0xd3bdU, 0xc39cU, 0xf3ffU, 0xe3deU,
        0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64e6U, 0x74c7U, 0x44a4U, 0x5485U,
        0xa56aU, 0xb54bU, 0x8528U, 0x9509U, 0xe5eeU, 0xf5cfU, 0xc5acU, 0xd58dU,
        0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76d7U, 0x66f6U, 0x5695U, 0x46b4U,
        0xb75bU, 0xa77aU, 0x9719U, 0x8738U, 0xf7dfU, 0xe7feU, 0xd79dU, 0xc7bcU,
        0x48c4U, 0x58e5U, 0x6886U, 0x78a7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
        0xc9ccU, 0xd9edU, 0xe98eU, 0xf9afU, 0x8948U, 0x9969U, 0xa90aU, 0xb92bU,
        0x5af5U, 0x4ad4U, 0x7ab7U, 0x6a96U, 0x1a71U, 0x0a50U, 0x3a33U, 0x2a12U,
        0xdbfdU, 0xcbdcU, 0xfbbfU, 0xeb9eU, 0x9b79U, 0x8b58U, 0xbb3bU, 0xab1aU,
        0x6ca6U, 0x7c87U, 0x4ce4U, 0x5cc5U, 0x2c22U, 0x3c03U, 0x0c60U, 0x1c41U,
        0xedaeU, 0xfd8fU, 0xcdecU, 0xddcdU, 0xad2aU, 0xbd0bU, 0x8d68U, 0x9d49U,
        0x7e97U, 0x6eb6U, 0x5ed5U, 0x4ef4U, 0x3e13U, 0x2e32U, 0x1e51U, 0x0e70U,
        0xff9fU, 0xefbeU, 0xdfddU, 0xcffcU, 0xbf1bU, 0xaf3aU, 0x9f59U, 0x8f78U,
        0x9188U, 0x81a9U, 0xb1caU, 0xa1ebU, 0xd10cU, 0xc12dU, 0xf14eU, 0xe16fU,
        0x1080U, 0x00a1U, 0x30c2U, 0x20e3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
        0x83b9U, 0x9398U, 0xa3fbU, 0xb3daU, 0xc33dU, 0xd31cU, 0xe37fU, 0xf35eU,
        0x02b1U, 0x1290U, 0x22f3U, 0x32d2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
        0xb5eaU, 0xa5cbU, 0x95a8U, 0x8589U, 0xf56eU, 0xe54fU, 0xd52cU, 0xc50dU,
        0x34e2U, 0x24c3U, 0x14a0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
        0xa7dbU, 0xb7faU, 0x8799U, 0x97b8U, 0xe75fU, 0xf77eU, 0xc71dU, 0xd73cU,
        0x26d3U, 0x36f2U, 0x0691U, 0x16b0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
        0xd94cU, 0xc96dU, 0xf90eU, 0xe92fU, 0x99c8U, 0x89e9U, 0xb98aU, 0xa9abU,
        0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18c0U, 0x08e1U, 0x3882U, 0x28a3U,
        0xcb7dU, 0xdb5cU, 0xeb3fU, 0xfb1eU, 0x8bf9U, 0x9bd8U, 0xabbbU, 0xbb9aU,
        0x4a75U, 0x5a54U, 0x6a37U, 0x7a16U, 0x0af1U, 0x1ad0U, 0x2ab3U, 0x3a92U,
        0xfd2eU, 0xed0fU, 0xdd6cU, 0xcd4dU, 0xbdaaU, 0xad8bU, 0x9de8U, 0x8dc9U,
        0x7c26U, 0x6c07U, 0x5c64U, 0x4c45U, 0x3ca2U, 0x2c83U, 0x1ce0U, 0x0cc1U,
        0xef1fU, 0xff3eU, 0xcf5dU, 0xdf7cU, 0xaf9bU, 0xbfbaU, 0x8fd9U, 0x9ff8U,
        0x6e17U, 0x7e36U, 0x4e55U, 0x5e74U, 0x2e93U, 0x3eb2U, 0x0ed1U, 0x1ef0U
    };

    crcCode = uiInit;

    if (pucBuffer != NULL_PTR)
    {
        for (i = 0; i < uiLength; i++)
        {
            temp    = ((((uint32) crcCode) >> 8U) ^ pucBuffer[i]) & ((uint32) 0xff);
            crcCode = crc16Table[temp] ^ ((uint16) (((uint32) crcCode) << 8U));
        }
    }

    return crcCode;
}

int32 IPC_Ioctl
(
    uint32                              siCh,
    uint32                              uiOpt,
    void *                              pPar1,
    void *                              pPar2,
    void *                              pPar3,
    void *                              pPar4
) //QAC
{
    int32           lRet;
    uint8 *         buff;
    uint32          isOpen;
    uint32          size;
    uint32          vTime;
    uint32          vMin;
    uint32 *        cmd;
    uint32 *        isReady;
    IPCPingInfo_t   pingInfo;

    (void) pPar4;
    lRet = IPC_SUCCESS;

    if ((int32) icpChStatus[siCh].gIPCChStatus != IPC_DEVICE_ST_OPENED)
    {
        lRet = IPC_ERR_NOTREADY;
    }
    else
    {
        (void) SAL_CoreCriticalEnter();
        isOpen = icpChStatus[siCh].gIPCChIsOpen;
        icpChStatus[siCh].gIPCChCallCount++;
        (void) SAL_CoreCriticalExit();

        if(isOpen == 1U)
        {
            switch ((IPCIoCmd_t) uiOpt)
            {
                case IOCTL_IPC_WRITE:
                {
                    if((pPar1 != NULL_PTR) && (pPar2 != NULL_PTR))
                    {
                        cmd  = (uint32 *) pPar1;
                        buff = (uint8 *) pPar2;
                        size = (pPar3 != NULL_PTR) ? *((const uint32 *)pPar3) : 0U;
                        lRet = IPC_Write((IPCSvcCh_t) siCh, cmd, buff, size);

                        if(lRet < IPC_SUCCESS)
                        {
                            lRet = IPC_ERR_COMMON;
                        }
                        else
                        {
                            lRet = IPC_SUCCESS;
                        }
                    }
                    else
                    {
                        lRet = IPC_ERR_ARGUMENT;
                    }

                    break;
                }

                case IOCTL_IPC_READ:
                {
                    if ((pPar1 != NULL_PTR) && (pPar2 != NULL_PTR))
                    {
                        buff = (uint8 *) pPar1;
                        size = *((uint32 *) pPar2);
                        (void) SAL_CoreCriticalEnter();
                        isOpen = icpChStatus[siCh].gIPCChIsOpen;
                        icpChStatus[siCh].gIPCChCallCount++;
                        (void) SAL_CoreCriticalExit();

                        if(isOpen == 1U)
                        {
                            lRet = IPC_Read((IPCSvcCh_t) siCh, buff, size, gIPCHandler[siCh].ihIpcOption.isBlock);
                        }
                        else
                        {
                            lRet = IPC_ERR_COMMON;
                        }

                        (void) SAL_CoreCriticalEnter();
                        icpChStatus[siCh].gIPCChCallCount--;
                        (void) SAL_CoreCriticalExit();
                    }
                    else
                    {
                        lRet = IPC_ERR_ARGUMENT;
                    }

                    break;
                }

                case IOCTL_IPC_SET_PARAM:
                {
                    if((pPar1 != NULL_PTR) && (pPar2 != NULL_PTR))
                    {
                        vTime = *((uint32 *) pPar1);
                        vMin  = *((uint32 *) pPar2);
                        IPC_CommonLock((IPCSvcCh_t) siCh);
                        gIPCHandler[siCh].ihSetParam.vTime = vTime;
                        gIPCHandler[siCh].ihSetParam.vMin  = vMin;
                        IPC_CommonUnlock((IPCSvcCh_t) siCh);
                    }
                    else
                    {
                        lRet = IPC_ERR_ARGUMENT;
                    }

                    break;
                }

                case IOCTL_IPC_GET_PARAM:
                {
                    if((pPar1 != NULL_PTR) && (pPar2 != NULL_PTR))
                    {
                        vTime = *((uint32 *) pPar1);
                        vMin  = *((uint32 *) pPar2);

                        IPC_CommonLock((IPCSvcCh_t) siCh);
                        vTime = gIPCHandler[siCh].ihSetParam.vTime;
                        vMin  = gIPCHandler[siCh].ihSetParam.vMin;
                        IPC_CommonUnlock((IPCSvcCh_t) siCh);
                    }
                    else
                    {
                        lRet = IPC_ERR_ARGUMENT;
                    }

                    break;
                }

                case IOCTL_IPC_PING_TEST:
                {
                    pingInfo.piPingResult    = IPC_PING_ERR_INIT;
                    pingInfo.piSendByte      = 0;
                    pingInfo.piResponseTime  = 0;
                    pingInfo.piCompareResult = 0;

                    (void) IPC_PingTest((IPCSvcCh_t) siCh, &pingInfo);

                    if(pingInfo.piPingResult == IPC_PING_SUCCESS)
                    {
                        IPC_DBG("CA53 result value : %d\n", pingInfo.piPingResult);
                        IPC_DBG("CA53 response time : %d\n", pingInfo.piResponseTime);
                    }
                    else
                    {
                        lRet = IPC_ERR_COMMON;
                    }

                    break;
                }

                case IOCTL_IPC_ISREADY:
                {
                    if(pPar1 != NULL_PTR)
                    {
                        isReady = (uint32 *) pPar1;

                        if(gIPCHandler[siCh].ihIpcStatus < IPC_READY)
                        {
                            *isReady = 0;
                        }
                        else
                        {
                            *isReady = 1;
                        }
                    }
                    else
                    {
                        lRet = IPC_ERR_ARGUMENT;
                    }

                    break;
                }

                default:
                {
                    lRet = IPC_ERR_ARGUMENT;
                    break;
                }
            }
        }
        else
        {
            lRet = IPC_ERR_COMMON;
        }

        (void) SAL_CoreCriticalEnter();
        icpChStatus[siCh].gIPCChCallCount--;
        (void) SAL_CoreCriticalExit();
    }

    return lRet;
}

int32 IPC_SendPacketWithIPCHeader
(
    IPCSvcCh_t                          siCh,
    uint16                              uiCmd1,
    uint16                              uiCmd2,
    const uint8 *                       pucData,
    uint16                              uiLength
)
{
    int32  ret;
    uint16 cnt;
    uint16 crc;
    uint32 packet_size;
    uint32 cmd[8];
    static uint8 packet_send[IPC_MAX_PACKET_SIZE];

    ret         = 0;
    cnt         = 0;
    crc         = 0;
    packet_size = 0;

    // packaging the ipc packet
    IPC_WriteLock(siCh);
    packet_send[0] = IPC_SYNC;
    packet_send[1] = IPC_START1;
    packet_send[2] = IPC_START2;
    packet_send[3] = (uint8) (((uint16) uiCmd1)   >> ((uint16) 8)) & ((uint8) 0xFF);
    packet_send[4] = ((uint8) ((uint16) uiCmd1)   &  ((uint8) 0xFF));
    packet_send[5] = (uint8) (((uint16) uiCmd2)   >> ((uint16) 8)) & ((uint8) 0xFF);
    packet_send[6] = ((uint8) ((uint16) uiCmd2)   &  ((uint8) 0xFF));
    packet_send[7] = (uint8) (((uint16) uiLength) >> ((uint16) 8)) & ((uint8) 0xFF);
    packet_send[8] = ((uint8) ((uint16) uiLength) &  ((uint8) 0xFF));

    if ((uiLength == ((uint16) 0)) || (pucData == NULL_PTR))
    {
        packet_send[IPC_PACKET_PREPARE_SIZE] = 0; //dummy data
        cnt = 1; //CS:Useless Assignment - Remove initialized value //dummy data length
        packet_size = IPC_PACKET_PREFIX_SIZE + 1;
    }
    else
    {
        for (cnt = 0; cnt < uiLength; cnt++)
        {
            packet_send[((uint16) IPC_PACKET_PREPARE_SIZE) + cnt] = pucData[cnt];
        }

        packet_size = (((uint32) IPC_PACKET_PREFIX_SIZE) + uiLength);
    }

    crc = IPC_CalcCrc16(&packet_send[0], (packet_size - 2U), 0);
    packet_send[((uint16) IPC_PACKET_PREPARE_SIZE) + cnt]               = (uint8) ((crc >> ((uint16) 8)) & ((uint16) 0xFF));
    packet_send[((uint16) IPC_PACKET_PREPARE_SIZE) + cnt + ((uint8) 1)] = (uint8) (crc & ((uint16) 0xFF));

    cmd[0] = 0; /* sequence ID */
    cmd[1] = (((uint32) WRITE_CMD) << 16U) | ((uint32) IPC_WRITE);
    cmd[2] = packet_size;
    cmd[3] = 0;
    cmd[4] = 0;
    cmd[5] = 0;

    // send the ipc packet
    ret = IPC_Ioctl((uint32) siCh, (uint32) IOCTL_IPC_WRITE, (void *)&cmd[0], (void *)&packet_send[0], (void *)&packet_size, NULL_PTR);
    IPC_WriteUnlock(siCh);

    return ret;
}

