/*
***************************************************************************************************
*
*   FileName : ipc.h
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

#ifndef IPC_API_H
#define IPC_API_H

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
//IPC block or non block option
#define IPC_O_NONBLOCK                  (0x00000000U)
#define IPC_O_BLOCK                     (0x00000001U)

//IPC ack or non ack option
#define IPC_O_NONACK                    (0x00000000U)
#define IPC_O_ACK                       (0x00000001U)
#define IPC_O_ACK_BIT                   (0x00000100U)

//IPC reserved0 option
#define IPC_O_RESERVED0                 (0x00000000U)

//IPC reserved1 option
#define IPC_O_RESERVED1                 (0x00000000U)

#define IPC_MODE_0                      (IPC_O_RESERVED1 | IPC_O_RESERVED0 | IPC_O_ACK_BIT | IPC_O_NONBLOCK)
#define IPC_MODE_1                      (IPC_O_RESERVED1 | IPC_O_RESERVED0 | IPC_O_ACK_BIT | IPC_O_BLOCK)

#define IPC_O_BLOCK_MASK                (0x000000FFU)
#define IPC_O_ACK_MASK                  (0x0000FF00U)

#define IPC_DEVICE_ST_NO_OPEN           (0)
#define IPC_DEVICE_ST_OPENED            (1)
#define IPC_DEVICE_ST_ENABLED           (2)
#define IPC_DEVICE_ST_DISABLED          (3)

/***********************************************************************************
*   IPC Protocol
************************************************************************************/
#define IPC_SYNC                        (0xFF)
#define IPC_START1                      (0x55)
#define IPC_START2                      (0xAA)
#define IPC_RESERVED1                   (0x00)
#define IPC_RESERVED2                   (0x00)
#define IPC_RESERVED3                   (0x00)
//
//  SYNC: 1byte ,  START1: 1byte ,  START2: 1byte ,  CMD1: 2byte ,  CMD2: 2byte ,  LENGTH: 2byte ,  DATA: variable ,  CRC: 2byte
//
#define IPC_MAX_PACKET_SIZE             (0x400)   // 1024
#define IPC_PACKET_PREPARE_SIZE         (0x09)
#define IPC_PACKET_PREFIX_SIZE          (0x0B)    // 11
#define IPC_PROCESS_NUM                 (0x0A)    // 10

enum
{
    IPC_APP_ID_UBOOT                    = 0,
    IPC_APP_ID_IPC                      = 1,
    IPC_APP_ID_ARMTF                    = 2,
    IPC_APP_ID_A7SR                     = 3,
    IPC_APP_ID_FWUG                     = 4,
    IPC_APP_ID_A53MR                    = 5,
    IPC_APP_ID_MAX                      = 6              //MAX
};

typedef enum IPCPingError
{
    IPC_PING_SUCCESS                    = 0,             /* Ping success */
    IPC_PING_ERR_INIT,                                   /* My ipc initialize failed */
    IPC_PING_ERR_NOT_READY,                              /* Other IPC not open */
    IPC_PING_ERR_SENDER_MBOX,                            /* My(sender) mbox is not set or error */
    IPC_PING_ERR_RECEIVER_MBOX,                          /* Receiver mbox is not set or error*/
    IPC_PING_ERR_SEND,                                   /* Can not send data. Maybe receiver mbox interrupt is busy*/
    IPC_PING_ERR_RESPOND,                                /* Receiver does not send respond data. */
    MAX_IPC_PING_ERR,
} IPCPingError_t;

typedef struct IPCOption
{
    uint32                              isBlock;
    uint32                              isAck;
} IPCOption_t;

typedef struct IPCCtlParam
{
    uint32                              vMin;            /* Timeout in deciseconds  when blocking mode*/
    uint32                              vTime;           /* Minimum number of characters when blocking mode */
} __attribute__((packed)) IPCCtlParam_t;

typedef struct IPCPingInfo
{
    IPCPingError_t                      piPingResult;
    uint32                              piSendByte;
    uint32                              piResponseTime;
    uint32                              piCompareResult; /*Data compare results. */
} __attribute__((packed)) IPCPingInfo_t;

typedef struct IPCChStatus
{
    uint32                              gIPCChStatus;
    uint32                              gIPCChIsOpen;
    uint32                              gIPCChCallCount;
} IPCChStatus_t;

typedef struct IPCCbFunc
{
    IPCCallback                         ipcCbFunc;
    void*                               ipcArg1;
    void*                               ipcArg2;
} IPCCbFunc_t;

extern IPCCbFunc_t  cbFunc[IPC_SVC_CH_MAX][IPC_PROCESS_NUM];

extern void IPC_CreateIPCParserTask
(
    void
);


int32 IPC_Open
(
    uint32                              siCh,
    uint32                              uiOptions
);

int32 IPC_Close
(
    uint32                             siCh
);

uint16 IPC_CalcCrc16
(
    const uint8*                        pucBuffer,
    uint32                              uiLength,
    uint16                              uiInit
);

int32 IPC_Ioctl
(
    uint32                              siCh,
    uint32                              uiOpt,
    void*                               pPar1,
    void*                               pPar2,
    void*                               pPar3,
    void*                               pPar4
); //QAC

int32 IPC_SendPacketWithIPCHeader
(
    IPCSvcCh_t                          siCh,
    uint16                              uiCmd1,
    uint16                              uiCmd2,
    const uint8*                        pucData,
    uint16                              uiLength
);
#endif /* _IPC_H_ */

